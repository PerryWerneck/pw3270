/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270.
 *
 * Copyright (C) <2008> <Banco do Brasil S.A.>
 *
 * Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
 * os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
 * Free Software Foundation.
 *
 * Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
 * GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
 * A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
 * obter mais detalhes.
 *
 * Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como iocalls.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

#include "globals.h"
#include <sys/time.h>
#include <sys/types.h>
#include "xioc.h"
#include "telnetc.h"
#include "utilc.h"

#define MILLION			1000000L
#define InputReadMask	0x1
#define InputExceptMask	0x2
#define InputWriteMask	0x4

#if defined(_WIN32)
	#define MAX_HA	256
#endif

/*---[ Standard calls ]-------------------------------------------------------------------------------------*/

static void   internal_remove_timeout(void *timer);
static void	  internal_remove_source(void *id);
static void * internal_add_timeout(unsigned long interval_ms, H3270 *session, void (*proc)(H3270 *session));

#ifdef WIN32
	static void * internal_add_input(HANDLE source, H3270 *session, void (*fn)(H3270 *session));
	static void * internal_add_output(HANDLE source, H3270 *session, void (*fn)(H3270 *session));
	static void * internal_add_except(HANDLE source, H3270 *session, void (*fn)(H3270 *session));
#else
	static void * internal_add_input(int source, H3270 *session, void (*fn)(H3270 *session));
	static void * internal_add_output(int source, H3270 *session, void (*fn)(H3270 *session));
	static void * internal_add_except(int source, H3270 *session, void (*fn)(H3270 *session));
#endif // WIN32

static int 	  internal_callthread(int(*callback)(H3270 *, void *), H3270 *session, void *parm);
static int	  internal_wait(H3270 *hSession, int seconds);

static int	  internal_event_dispatcher(H3270 *hSession, int block);
static void	  internal_ring_bell(H3270 *);

/*---[ Active callbacks ]-----------------------------------------------------------------------------------*/

 static void	* (*add_timeout)(unsigned long interval_ms, H3270 *session, void (*proc)(H3270 *session))
					= internal_add_timeout;

 static void	  (*remove_timeout)(void *timer)
					= internal_remove_timeout;

 static void	  (*remove_source)(void *id)
					= internal_remove_source;

#ifdef WIN32
 static void	* (*add_input)(HANDLE source, H3270 *session, void (*fn)(H3270 *session))
					= internal_add_input;

 static void	* (*add_output)(HANDLE source, H3270 *session, void (*fn)(H3270 *session))
					= internal_add_output;

 static void 	* (*add_except)(HANDLE source, H3270 *session, void (*fn)(H3270 *session))
					= internal_add_except;
#else
 static void	* (*add_input)(int source, H3270 *session, void (*fn)(H3270 *session))
					= internal_add_input;

 static void	* (*add_output)(int source, H3270 *session, void (*fn)(H3270 *session))
					= internal_add_output;

 static void 	* (*add_except)(int source, H3270 *session, void (*fn)(H3270 *session))
					= internal_add_except;
#endif // WIN32

 static int 	  (*callthread)(int(*callback)(H3270 *, void *), H3270 *session, void *parm)
					= internal_callthread;

 static int		  (*wait)(H3270 *hSession, int seconds)
					= internal_wait;

 static int 	  (*event_dispatcher)(H3270 *hSession,int wait)
					= internal_event_dispatcher;

 static void	  (*ring_bell)(H3270 *)
					= internal_ring_bell;

/*---[ Typedefs ]-------------------------------------------------------------------------------------------*/

 typedef struct timeout
 {
	struct timeout *next;
#if defined(_WIN32) /*[*/
	unsigned long long ts;
#else /*][*/
	struct timeval tv;
#endif /*]*/
	void (*proc)(H3270 *session);
	H3270 *session;
	Boolean in_play;
 } timeout_t;

 #define TN	(timeout_t *)NULL

 /* Input events. */
typedef struct input
{
        struct input *next;
#if defined(_WIN32)
        HANDLE source;
#else
        int source;
#endif // WIN32
        int condition;
        void (*proc)(H3270 *session);
        H3270 *session;
} input_t;



/*---[ Statics ]--------------------------------------------------------------------------------------------*/

 static timeout_t	* timeouts			= NULL;
 static input_t 	* inputs 			= NULL;
 static Boolean		  inputs_changed	= False;

/*---[ Implement ]------------------------------------------------------------------------------------------*/


/* Timeouts */

#if defined(_WIN32)
static void ms_ts(unsigned long long *u)
{
	FILETIME t;

	/* Get the system time, in 100ns units. */
	GetSystemTimeAsFileTime(&t);
	memcpy(u, &t, sizeof(unsigned long long));

	/* Divide by 10,000 to get ms. */
	*u /= 10000ULL;
}
#endif

static void * internal_add_timeout(unsigned long interval_ms, H3270 *session, void (*proc)(H3270 *session))
{
	timeout_t *t_new;
	timeout_t *t;
	timeout_t *prev = TN;

	trace("%s session=%p proc=%p interval=%ld",__FUNCTION__,session,proc,interval_ms);

	t_new = (timeout_t *) lib3270_malloc(sizeof(timeout_t));

	t_new->proc = proc;
	t_new->session = session;
	t_new->in_play = False;

#if defined(_WIN32)
	ms_ts(&t_new->ts);
	t_new->ts += interval_ms;
#else

	gettimeofday(&t_new->tv, NULL);
	t_new->tv.tv_sec += interval_ms / 1000L;
	t_new->tv.tv_usec += (interval_ms % 1000L) * 1000L;

	if (t_new->tv.tv_usec > MILLION)
	{
		t_new->tv.tv_sec += t_new->tv.tv_usec / MILLION;
		t_new->tv.tv_usec %= MILLION;
	}
#endif /*]*/

	/* Find where to insert this item. */
	for (t = timeouts; t != TN; t = t->next)
	{
#if defined(_WIN32)
		if (t->ts > t_new->ts)
#else
		if (t->tv.tv_sec > t_new->tv.tv_sec || (t->tv.tv_sec == t_new->tv.tv_sec && t->tv.tv_usec > t_new->tv.tv_usec))
#endif
		break;

		prev = t;
	}

	// Insert it.
	if (prev == TN)
	{	// Front.
		t_new->next = timeouts;
		timeouts = t_new;
	}
	else if (t == TN)
	{	// Rear.
		t_new->next = TN;
		prev->next = t_new;
	}
	else
	{	// Middle.
		t_new->next = t;
		prev->next = t_new;
	}

	trace("Timeout %p added with value %ld",t_new,interval_ms);

	return t_new;
}

static void internal_remove_timeout(void * timer)
{
	timeout_t *st = (timeout_t *)timer;
	timeout_t *t;
	timeout_t *prev = TN;

	trace("Removing timeout: %p",st);

	if (st->in_play)
		return;

	for (t = timeouts; t != TN; t = t->next)
	{
		if (t == st)
		{
			if (prev != TN)
				prev->next = t->next;
			else
				timeouts = t->next;
			lib3270_free(t);
			return;
		}
		prev = t;
	}
}

/* Input events. */

#ifdef WIN32
static void * internal_add_input(HANDLE source, H3270 *session, void (*fn)(H3270 *session))
#else
static void * internal_add_input(int source, H3270 *session, void (*fn)(H3270 *session))
#endif // WIN32
{
	input_t *ip = (input_t *) lib3270_malloc(sizeof(input_t));

	ip->source 		= source;
	ip->condition	= InputReadMask;
	ip->proc		= fn;
	ip->session 	= session;
	ip->next 		= inputs;
	inputs 			= ip;
	inputs_changed 	= True;

	return ip;
}

#ifdef WIN32
static void * internal_add_output(HANDLE source, H3270 *session, void (*fn)(H3270 *session))
#else
static void * internal_add_output(int source, H3270 *session, void (*fn)(H3270 *session))
#endif // WIN32
{
	input_t *ip = (input_t *) lib3270_malloc(sizeof(input_t));

	ip->source 		= source;
	ip->condition	= InputWriteMask;
	ip->proc		= fn;
	ip->session 	= session;
	ip->next 		= inputs;
	inputs 			= ip;
	inputs_changed 	= True;

	return ip;
}


#if defined(_WIN32)
static void * internal_add_except(HANDLE source, H3270 *session, void (*fn)(H3270 *session))
{
	return 0;
}
#else
static void * internal_add_except(int source, H3270 *session, void (*fn)(H3270 *session))
{
	input_t *ip = (input_t *) lib3270_malloc(sizeof(input_t));

	ip->source 		= source;
	ip->condition	= InputExceptMask;
	ip->proc		= fn;
	ip->session		= session;
	ip->next 		= inputs;
	inputs 			= ip;
	inputs_changed	= True;

	trace("%s session=%p proc=%p handle=%p",__FUNCTION__,ip->session,ip->proc,ip);

	return ip;
}
#endif // WIN32

static void internal_remove_source(void *id)
{
	input_t *ip;
	input_t *prev = (input_t *)NULL;

	for (ip = inputs; ip != (input_t *)NULL; ip = ip->next)
	{
		if (ip == (input_t *)id)
			break;

		prev = ip;
	}

	if (ip == (input_t *)NULL)
	{
		lib3270_write_log(NULL,"lib3270","Invalid call to (%s): Input %p wasnt found in the list",__FUNCTION__,id);
		return;
	}

	if (prev != (input_t *)NULL)
		prev->next = ip->next;
	else
		inputs = ip->next;

	lib3270_free(ip);
	inputs_changed = True;
}

/* Event dispatcher. */
static int internal_event_dispatcher(H3270 *hSession, int block)
{
#if defined(_WIN32)
	HANDLE ha[MAX_HA];
	DWORD events;
	DWORD tmo;
	unsigned long long now;
	int i;
#else
	fd_set rfds, wfds, xfds;
	int ns;
	struct timeval now, twait, *tp;
	int events;
#endif
	input_t *ip;
	struct timeout *t;
	int processed_any = 0;

retry:

	inputs_changed = 0;

	// If we've processed any input, then don't block again.
	if(processed_any)
		block = 0;

	events = 0;

#if defined(_WIN32)

	for (ip = inputs; ip != (input_t *)NULL; ip = ip->next)
	{
		if ((unsigned long)ip->condition & InputReadMask)
			ha[events++] = ip->source;
		if ((unsigned long)ip->condition & InputWriteMask)
			ha[events++] = ip->source;
	}

	if (block)
	{
		if (timeouts != TN)
		{
			ms_ts(&now);
			if (now > timeouts->ts)
				tmo = 0;
			else
				tmo = timeouts->ts - now;
		}
		else
		{
			// Block for 1 second (at maximal)
			tmo = 1;
		}
	}
	else if (!events)
	{
		// No block & no events, return
		return processed_any;
	}
	else
	{
		tmo = 1;
	}

	if(events)
	{
		DWORD ret = WaitForMultipleObjects(events, ha, FALSE, tmo);

		if (ret == WAIT_FAILED)
		{
			lib3270_popup_dialog(	hSession,
									LIB3270_NOTIFY_ERROR,
									_( "Network error" ),
									_( "WaitForMultipleObjects() failed when processing for events." ),
									"%s",
									lib3270_win32_strerror(GetLastError()));
			lib3270_disconnect(hSession);
		}
		else
		{
			inputs_changed = False;

			for (i = 0, ip = inputs; ip != (input_t *)NULL; ip = ip->next, i++)
			{
				if(ret == WAIT_OBJECT_0 + i)
				{
					(*ip->proc)(ip->session);
					processed_any = True;
					if (inputs_changed)
						goto retry;
				}
			}
		}
	}
	else if(block)
	{
		Sleep(100);
	}

#else

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&xfds);

	for (ip = inputs; ip != (input_t *)NULL; ip = ip->next)
	{
		if ((unsigned long)ip->condition & InputReadMask)
		{
			FD_SET(ip->source, &rfds);
			events++;
		}

		if ((unsigned long)ip->condition & InputWriteMask)
		{
			FD_SET(ip->source, &wfds);
			events++;
		}

		if ((unsigned long)ip->condition & InputExceptMask)
		{
			FD_SET(ip->source, &xfds);
			events++;
		}
	}

	if (block)
	{
		if (timeouts != TN)
		{
			(void) gettimeofday(&now, (void *)NULL);
			twait.tv_sec = timeouts->tv.tv_sec - now.tv_sec;
			twait.tv_usec = timeouts->tv.tv_usec - now.tv_usec;
			if (twait.tv_usec < 0L) {
				twait.tv_sec--;
				twait.tv_usec += MILLION;
			}
			if (twait.tv_sec < 0L)
				twait.tv_sec = twait.tv_usec = 0L;
			tp = &twait;
		}
		else
		{
			twait.tv_sec = 1;
			twait.tv_usec = 0L;
			tp = &twait;
		}
	}
	else
	{
		twait.tv_sec  = 1;
		twait.tv_usec = 0L;
		tp = &twait;

		if(!events)
			return processed_any;
	}

	ns = select(FD_SETSIZE, &rfds, &wfds, &xfds, tp);

	if (ns < 0 && errno != EINTR)
	{
		lib3270_popup_dialog(	hSession,
								LIB3270_NOTIFY_ERROR,
								_( "Network error" ),
								_( "Select() failed when processing for events." ),
								"%s",
								strerror(errno));
	}
	else
	{
		for (ip = inputs; ip != (input_t *) NULL; ip = ip->next)
		{
			if (((unsigned long)ip->condition & InputReadMask) && FD_ISSET(ip->source, &rfds))
			{
				(*ip->proc)(ip->session);
				processed_any = True;
				if (inputs_changed)
					goto retry;
			}

			if (((unsigned long)ip->condition & InputWriteMask) && FD_ISSET(ip->source, &wfds))
			{
				(*ip->proc)(ip->session);
				processed_any = True;
				if (inputs_changed)
					goto retry;
			}

			if (((unsigned long)ip->condition & InputExceptMask) && FD_ISSET(ip->source, &xfds))
			{
				(*ip->proc)(ip->session);
				processed_any = True;
				if (inputs_changed)
					goto retry;
			}
		}
	}

#endif

	// See what's expired.
	if (timeouts != TN)
	{
#if defined(_WIN32)
		ms_ts(&now);
#else
		(void) gettimeofday(&now, (void *)NULL);
#endif

		while ((t = timeouts) != TN)
		{
#if defined(_WIN32)
			if (t->ts <= now)
#else
			if (t->tv.tv_sec < now.tv_sec ||(t->tv.tv_sec == now.tv_sec && t->tv.tv_usec < now.tv_usec))
#endif
			{
				timeouts = t->next;
				t->in_play = True;
				(*t->proc)(t->session);
				processed_any = True;
				lib3270_free(t);
			} else
				break;
		}
	}

	if (inputs_changed)
		goto retry;

	return processed_any;

}

static int internal_callthread(int(*callback)(H3270 *, void *), H3270 *session, void *parm)
{
	return callback(session,parm);
}

static int internal_wait(H3270 *hSession, int seconds)
{
	time_t end;

	// Alternative wait call
	end = time(0) + seconds;

	while(time(0) < end)
	{
		lib3270_main_iterate(hSession,1);
	}

	return 0;
}

static void internal_ring_bell(H3270 *session)
{
	return;
}

/* External entry points */

void * AddTimeOut(unsigned long interval_ms, H3270 *session, void (*proc)(H3270 *session))
{
	void *timer;
	CHECK_SESSION_HANDLE(session);
	timer = add_timeout(interval_ms,session,proc);
	trace("Timeout %p created with %ld ms",timer,interval_ms);
	return timer;
}

void RemoveTimeOut(void * timer)
{
	trace("Removing timeout %p",timer);
	return remove_timeout(timer);
}

#ifdef WIN32
void * AddInput(HANDLE source, H3270 *session, void (*fn)(H3270 *session))
#else
void * AddInput(int source, H3270 *session, void (*fn)(H3270 *session))
#endif // WIN32
{
	CHECK_SESSION_HANDLE(session);
	return add_input(source,session,fn);
}

#ifdef WIN32
void * AddOutput(HANDLE source, H3270 *session, void (*fn)(H3270 *session))
#else
void * AddOutput(int source, H3270 *session, void (*fn)(H3270 *session))
#endif // WIN32
{
	CHECK_SESSION_HANDLE(session);
	return add_output(source,session,fn);
}

#ifdef WIN32
void * AddExcept(HANDLE source, H3270 *session, void (*fn)(H3270 *session))
#else
void * AddExcept(int source, H3270 *session, void (*fn)(H3270 *session))
#endif // WIN32
{
	CHECK_SESSION_HANDLE(session);
	return add_except(source,session,fn);
}

void RemoveSource(void * id)
{
	remove_source(id);
}

void x_except_on(H3270 *h)
{
	if(h->excepting)
		return;

	if(h->reading)
		RemoveSource(h->ns_read_id);

#ifdef WIN32
	h->ns_exception_id = AddExcept(h->sockEvent, h, net_exception);
	h->excepting = 1;

	if(h->reading)
		h->ns_read_id = AddInput(h->sockEvent, h, net_input);
#else
	h->ns_exception_id = AddExcept(h->sock, h, net_exception);
	h->excepting = 1;

	if(h->reading)
		h->ns_read_id = AddInput(h->sock, h, net_input);
#endif // WIN32
}

void remove_input_calls(H3270 *session)
{
	if(session->ns_read_id)
	{
		RemoveSource(session->ns_read_id);
		session->ns_read_id	= NULL;
		session->reading = 0;
	}
	if(session->ns_exception_id)
	{
		RemoveSource(session->ns_exception_id);
		session->ns_exception_id = NULL;
		session->excepting = 0;
	}
	if(session->ns_write_id)
	{
		RemoveSource(session->ns_write_id);
		session->ns_write_id = NULL;
		session->writing = 0;
	}
}

LIB3270_EXPORT void lib3270_register_time_handlers(void * (*add)(unsigned long interval_ms, H3270 *session, void (*proc)(H3270 *session)), void (*rm)(void *timer))
{
	if(add)
		add_timeout = add;

	if(rm)
		remove_timeout = rm;

}

LIB3270_EXPORT int lib3270_register_handlers(const struct lib3270_callbacks *cbk)
{
	if(!cbk)
		return EINVAL;

	if(cbk->sz != sizeof(struct lib3270_callbacks))
		return EINVAL;

	lib3270_register_time_handlers(cbk->AddTimeOut,cbk->RemoveTimeOut);

	if(cbk->AddInput)
		add_input = cbk->AddInput;

	if(cbk->AddOutput)
		add_output = cbk->AddOutput;

	if(cbk->RemoveSource)
		remove_source = cbk->RemoveSource;

	if(cbk->AddExcept)
		add_except = cbk->AddExcept;

	if(cbk->callthread)
		callthread = cbk->callthread;

	if(cbk->Wait)
		wait = cbk->Wait;

	if(cbk->event_dispatcher)
		event_dispatcher = cbk->event_dispatcher;

	if(cbk->ring_bell)
		ring_bell = cbk->ring_bell;

	return 0;

}

LIB3270_EXPORT int lib3270_call_thread(int(*callback)(H3270 *h, void *), H3270 *h, void *parm)
{
	int rc;
	CHECK_SESSION_HANDLE(h);

	h->set_timer(h,1);

	lib3270_main_iterate(h,0);
	rc = callthread(callback,h,parm);
	lib3270_main_iterate(h,0);

	h->set_timer(h,0);

	return rc;
}

LIB3270_EXPORT void lib3270_main_iterate(H3270 *hSession, int block)
{
	CHECK_SESSION_HANDLE(hSession);
	event_dispatcher(hSession,block);
}

LIB3270_EXPORT int lib3270_wait(H3270 *hSession, int seconds)
{
	wait(hSession,seconds);
	return 0;
}

LIB3270_EXPORT int lib3270_wait_for_ready(H3270 *hSession, int seconds)
{
	time_t	end = time(0)+seconds;

	if(!lib3270_lock_status(hSession))
		return 0;

	while(time(0) < end)
	{
		event_dispatcher(hSession,1);

		if(!lib3270_lock_status(hSession))
			return 0;

		if(!lib3270_connected(hSession))
			return ENOTCONN;

	}

	return ETIMEDOUT;
}


LIB3270_EXPORT void lib3270_ring_bell(H3270 *session)
{
	CHECK_SESSION_HANDLE(session);
	if(lib3270_get_toggle(session,LIB3270_TOGGLE_BEEP))
		ring_bell(session);
}



