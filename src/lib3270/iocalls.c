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
static void * internal_add_timeout(unsigned long interval_ms, H3270 *session, void (*proc)(H3270 *session));

static void * internal_add_input(int source, H3270 *session, void (*fn)(H3270 *session));
static void * internal_add_except(int source, H3270 *session, void (*fn)(H3270 *session));

static void   internal_remove_input(void *id);

// static int	  internal_process_events(int block);

static int 	  internal_callthread(int(*callback)(H3270 *, void *), H3270 *session, void *parm);
static int	  internal_wait(int seconds);

static int	  internal_event_dispatcher(int block);
static void	  internal_ring_bell(H3270 *);

/*---[ Active callbacks ]-----------------------------------------------------------------------------------*/

 static void	* (*add_timeout)(unsigned long interval_ms, H3270 *session, void (*proc)(H3270 *session))
					= internal_add_timeout;

 static void	  (*remove_timeout)(void *timer)
					= internal_remove_timeout;

 static void	* (*add_input)(int source, H3270 *session, void (*fn)(H3270 *session))
					= internal_add_input;

 static void	  (*remove_input)(void *id)
					= internal_remove_input;

 static void 	* (*add_except)(int source, H3270 *session, void (*fn)(H3270 *session))
					= internal_add_except;

 static int 	  (*callthread)(int(*callback)(H3270 *, void *), H3270 *session, void *parm)
					= internal_callthread;

 static int		  (*wait)(int seconds)
					= internal_wait;

 static int 	  (*event_dispatcher)(int wait)
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
        int source;
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

	trace("%s session=%p proc=%p",__FUNCTION__,session,proc);

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

static void * internal_add_input(int source, H3270 *session, void (*fn)(H3270 *session))
{
	input_t *ip = (input_t *) lib3270_malloc(sizeof(input_t));

	trace("%s session=%p proc=%p",__FUNCTION__,session,fn);

	ip->source = source;
	ip->condition = InputReadMask;
	ip->proc = fn;
	ip->session = session;
	ip->next = inputs;
	inputs = ip;
	inputs_changed = True;

	trace("%s: fd=%d callback=%p handle=%p",__FUNCTION__,source,fn,ip);

	return ip;
}

static void * internal_add_except(int source, H3270 *session, void (*fn)(H3270 *session))
{
#if defined(_WIN32)
	return 0;
#else
	input_t *ip = (input_t *) lib3270_malloc(sizeof(input_t));

	trace("%s session=%p proc=%p",__FUNCTION__,session,fn);

	ip->source 		= source;
	ip->condition	= InputExceptMask;
	ip->proc		= fn;
	ip->session		= session;
	ip->next 		= inputs;
	inputs 			= ip;
	inputs_changed	= True;

	trace("%s: fd=%d callback=%p handle=%p",__FUNCTION__,source,fn,ip);

	return ip;
#endif
}

static void internal_remove_input(void *id)
{
	input_t *ip;
	input_t *prev = (input_t *)NULL;

	trace("%s: fhandle=%p",__FUNCTION__,(input_t *) id);

	for (ip = inputs; ip != (input_t *)NULL; ip = ip->next)
	{
		if (ip == (input_t *)id)
			break;

		prev = ip;
	}

	if (ip == (input_t *)NULL)
		return;

	if (prev != (input_t *)NULL)
		prev->next = ip->next;
	else
		inputs = ip->next;

	lib3270_free(ip);
	inputs_changed = True;
}

/* Event dispatcher. */
static int internal_event_dispatcher(int block)
{
#if defined(_WIN32)
	HANDLE ha[MAX_HA];
	DWORD nha;
	DWORD tmo;
	DWORD ret;
	unsigned long long now;
	int i;
#else
	fd_set rfds, wfds, xfds;
	int ns;
	struct timeval now, twait, *tp;
#endif
	input_t *ip, *ip_next;
	struct timeout *t;
	Boolean any_events;
	int processed_any = 0;

    retry:

	// If we've processed any input, then don't block again.

	if(processed_any)
		block = 0;
	any_events = False;
#if defined(_WIN32)
	nha = 0;
#else
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&xfds);
#endif

	for (ip = inputs; ip != (input_t *)NULL; ip = ip->next)
	{
		if ((unsigned long)ip->condition & InputReadMask)
		{
#if defined(_WIN32)
			ha[nha++] = (HANDLE) ip->source;
#else
			FD_SET(ip->source, &rfds);
#endif
			any_events = True;
		}
#if !defined(_WIN32)
		if ((unsigned long)ip->condition & InputWriteMask)
		{
			FD_SET(ip->source, &wfds);
			any_events = True;
		}
		if ((unsigned long)ip->condition & InputExceptMask)
		{
			FD_SET(ip->source, &xfds);
			any_events = True;
		}
#endif
	}

	if (block)
	{
		if (timeouts != TN) {
#if defined(_WIN32)
			ms_ts(&now);
			if (now > timeouts->ts)
				tmo = 0;
			else
				tmo = timeouts->ts - now;
#else
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
#endif
			any_events = True;
		} else {
			// Block for 1 second (at maximal)
#if defined(_WIN32)
			tmo = 1;
#else
			twait.tv_sec = 1;
			twait.tv_usec = 0L;
			tp = &twait;
#endif
		}
	}
	else
	{
#if defined(_WIN32)
		tmo = 1;
#else
		twait.tv_sec = twait.tv_usec = 0L;
		tp = &twait;
#endif
	}

	if (!any_events)
		return processed_any;

#if defined(_WIN32)
	ret = WaitForMultipleObjects(nha, ha, FALSE, tmo);
	if (ret == WAIT_FAILED)
	{
#else
	ns = select(FD_SETSIZE, &rfds, &wfds, &xfds, tp);
	if (ns < 0)
	{
		if (errno != EINTR)
			Warning(NULL, "process_events: select() failed" );
#endif
		return processed_any;
	}

	inputs_changed = False;

#if defined(_WIN32)
	for (i = 0, ip = inputs; ip != (input_t *)NULL; ip = ip_next, i++)
	{
#else
	for (ip = inputs; ip != (input_t *) NULL; ip = ip_next)
	{
#endif
		ip_next = ip->next;
		if (((unsigned long)ip->condition & InputReadMask) &&
#if defined(_WIN32)
		    ret == WAIT_OBJECT_0 + i)
		{
#else
		    FD_ISSET(ip->source, &rfds))
		{
#endif
			(*ip->proc)(ip->session);
			processed_any = True;
			if (inputs_changed)
				goto retry;
		}

#if !defined(_WIN32)
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
#endif
	}

	// See what's expired.
	if (timeouts != TN) {
#if defined(_WIN32)
		ms_ts(&now);
#else
		(void) gettimeofday(&now, (void *)NULL);
#endif

		while ((t = timeouts) != TN)
		{
#if defined(_WIN32)
			if (t->ts <= now) {
#else
			if (t->tv.tv_sec < now.tv_sec ||(t->tv.tv_sec == now.tv_sec && t->tv.tv_usec < now.tv_usec))
			{
#endif
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
	callback(session,parm);
	return 0;
}

static int internal_wait(int seconds)
{
	time_t end;

	// Alternative wait call
	end = time(0) + seconds;

	while(time(0) < end)
	{
		lib3270_main_iterate(&h3270,1);
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
	CHECK_SESSION_HANDLE(session);
	return add_timeout(interval_ms,session,proc);
}

void RemoveTimeOut(void * timer)
{
	return remove_timeout(timer);
}

void * AddInput(int source, H3270 *session, void (*fn)(H3270 *session))
{
	CHECK_SESSION_HANDLE(session);
	return add_input(source,session,fn);
}

void * AddExcept(int source, H3270 *session, void (*fn)(H3270 *session))
{
	CHECK_SESSION_HANDLE(session);
	return add_except(source,session,fn);
}

void RemoveInput(void * id)
{
	remove_input(id);
}

void x_except_on(H3270 *h)
{
	if(h->excepting)
		return;

	if(h->reading)
		RemoveInput(h->ns_read_id);

#ifdef WIN32
	h->ns_exception_id = AddExcept((int) h->sockEvent, h, net_exception);
	h->excepting = 1;

	if(h->reading)
		h->ns_read_id = AddInput( (int) h->sockEvent, h, net_input);
#else
	h->ns_exception_id = AddExcept(h->sock, h, net_exception);
	h->excepting = 1;

	if(h->reading)
		h->ns_read_id = AddInput(h->sock, h, net_input);
#endif // WIN32
}

/*
void add_input_calls(H3270 *session, void (*in)(H3270 *session), void (*exc)(H3270 *session))
{
#ifdef _WIN32
	session->ns_exception_id	= AddExcept((int) session->sockEvent, session, exc);
	session->ns_read_id			= AddInput((int) session->sockEvent, session, in);
#else
	session->ns_exception_id	= AddExcept(session->sock, session, exc);
	session->ns_read_id			= AddInput(session->sock, session, in);
#endif // WIN32

	session->excepting	= 1;
	session->reading 	= 1;
}
*/

void remove_input_calls(H3270 *session)
{
	if(session->ns_read_id)
	{
		RemoveInput(session->ns_read_id);
		session->ns_read_id	= NULL;
		session->reading = 0;
	}
	if(session->ns_exception_id)
	{
		RemoveInput(session->ns_exception_id);
		session->ns_exception_id = NULL;
		session->excepting = 0;
	}
}

LIB3270_EXPORT int lib3270_register_handlers(const struct lib3270_callbacks *cbk)
{
	if(!cbk)
		return EINVAL;

	if(cbk->sz != sizeof(struct lib3270_callbacks))
		return EINVAL;

	if(cbk->AddTimeOut)
		add_timeout = cbk->AddTimeOut;

	if(cbk->RemoveTimeOut)
		remove_timeout = cbk->RemoveTimeOut;

	if(cbk->AddInput)
		add_input = cbk->AddInput;

	if(cbk->RemoveInput)
		remove_input = cbk->RemoveInput;

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
//	int rc;
	CHECK_SESSION_HANDLE(h);

	if(h->set_timer)
		h->set_timer(h,1);

	lib3270_main_iterate(h,0);
	callthread(callback,h,parm);
	lib3270_main_iterate(h,0);

	if(h->set_timer)
		h->set_timer(h,0);

	return 0;
}

LIB3270_EXPORT void lib3270_main_iterate(H3270 *session, int block)
{
	CHECK_SESSION_HANDLE(session);
	event_dispatcher(block);
}

LIB3270_EXPORT int lib3270_wait(seconds)
{
	wait(seconds);
	return 0;
}

LIB3270_EXPORT void lib3270_ring_bell(H3270 *session)
{
	CHECK_SESSION_HANDLE(session);
	if(lib3270_get_toggle(session,LIB3270_TOGGLE_BEEP))
		ring_bell(session);
}



