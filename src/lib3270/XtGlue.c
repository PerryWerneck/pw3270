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
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como XtGlue.c e possui 896 linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */

/* glue for missing Xt code */

#include "globals.h"
#include "api.h"
#if defined(_WIN32) /*[*/
#include "appres.h"
#include "trace_dsc.h"
#include "xioc.h"
#endif /*]*/
#include "utilc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "X11keysym.h"

#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>

#include <lib3270.h>

#if defined(_WIN32) /*[*/

	#include <windows.h>
	#include <winsock2.h>
	#include <ws2tcpip.h>

#else /*][*/

	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>

	#if defined(SEPARATE_SELECT_H) /*[*/
		#include <sys/select.h>
	#endif /*]*/
#endif /*]*/

#include "resolverc.h"

#define InputReadMask	0x1
#define InputExceptMask	0x2
#define InputWriteMask	0x4

#define MILLION		1000000L

/*---[ Callbacks ]------------------------------------------------------------------------------------------*/

static void DefaultRemoveTimeOut(unsigned long timer);
static unsigned long DefaultAddTimeOut(unsigned long interval_ms, H3270 *session, void (*proc)(H3270 *session));

static unsigned long DefaultAddInput(int source, H3270 *session, void (*fn)(H3270 *session));
static unsigned long DefaultAddExcept(int source, H3270 *session, void (*fn)(H3270 *session));

#if !defined(_WIN32) /*[*/
static unsigned long DefaultAddOutput(int source, H3270 *session, void (*fn)(H3270 *session));
#endif

static void DefaultRemoveInput(unsigned long id);

static int DefaultProcessEvents(int block);

static void	dunno(H3270 *session)
{

}

static const struct lib3270_callbacks default_callbacks =
{
	sizeof(struct lib3270_callbacks),

	DefaultAddTimeOut, 		// unsigned long (*AddTimeOut)(unsigned long interval_ms, void (*proc)(void));
	DefaultRemoveTimeOut,	// void (*RemoveTimeOut)(unsigned long timer);

	DefaultAddInput, 		// unsigned long (*AddInput)(int source, void (*fn)(void));
	DefaultRemoveInput,		// void	(*RemoveInput)(unsigned long id);

	DefaultAddExcept, 		// unsigned long (*AddExcept)(int source, void (*fn)(void));

	#if !defined(_WIN32) /*[*/
	DefaultAddOutput, 		// 	unsigned long (*AddOutput)(int source, void (*fn)(void));
	#endif /*]*/

	NULL, 		// int (*CallAndWait)(int(*callback)(void *), void *parm);

	NULL, 		// int (*Wait)(int seconds);
	DefaultProcessEvents,	// int (*RunPendingEvents)(int wait);
	dunno


};

static const struct lib3270_callbacks *callbacks = &default_callbacks;

/*---[ Implement default calls ]----------------------------------------------------------------------------*/

/* Timeouts. */

#if defined(_WIN32) /*[*/
static void ms_ts(unsigned long long *u)
{
	FILETIME t;

	/* Get the system time, in 100ns units. */
	GetSystemTimeAsFileTime(&t);
	memcpy(u, &t, sizeof(unsigned long long));

	/* Divide by 10,000 to get ms. */
	*u /= 10000ULL;
}
#endif /*]*/

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
static timeout_t *timeouts = TN;

static unsigned long DefaultAddTimeOut(unsigned long interval_ms, H3270 *session, void (*proc)(H3270 *session))
{
	timeout_t *t_new;
	timeout_t *t;
	timeout_t *prev = TN;

	Trace("%s session=%p proc=%p",__FUNCTION__,session,proc);

	t_new = (timeout_t *)Malloc(sizeof(timeout_t));
	memset(t_new,0,sizeof(timeout_t));

	t_new->proc = proc;
	t_new->session = session;
	t_new->in_play = False;
#if defined(_WIN32) /*[*/
	ms_ts(&t_new->ts);
	t_new->ts += interval_ms;
#else /*][*/
	(void) gettimeofday(&t_new->tv, NULL);
	t_new->tv.tv_sec += interval_ms / 1000L;
	t_new->tv.tv_usec += (interval_ms % 1000L) * 1000L;
	if (t_new->tv.tv_usec > MILLION) {
		t_new->tv.tv_sec += t_new->tv.tv_usec / MILLION;
		t_new->tv.tv_usec %= MILLION;
	}
#endif /*]*/

	/* Find where to insert this item. */
	for (t = timeouts; t != TN; t = t->next) {
#if defined(_WIN32) /*[*/
		if (t->ts > t_new->ts)
#else /*][*/
		if (t->tv.tv_sec > t_new->tv.tv_sec ||
		    (t->tv.tv_sec == t_new->tv.tv_sec &&
		     t->tv.tv_usec > t_new->tv.tv_usec))
#endif /*]*/
			break;
		prev = t;
	}

	/* Insert it. */
	if (prev == TN) {	/* Front. */
		t_new->next = timeouts;
		timeouts = t_new;
	} else if (t == TN) {	/* Rear. */
		t_new->next = TN;
		prev->next = t_new;
	} else {				/* Middle. */
		t_new->next = t;
		prev->next = t_new;
	}

	Trace("Timeout added: %p",t_new);

	return (unsigned long)t_new;
}

static void DefaultRemoveTimeOut(unsigned long timer)
{
	timeout_t *st = (timeout_t *)timer;
	timeout_t *t;
	timeout_t *prev = TN;

	Trace("Removing timeout: %p",st);

	if (st->in_play)
		return;
	for (t = timeouts; t != TN; t = t->next) {
		if (t == st) {
			if (prev != TN)
				prev->next = t->next;
			else
				timeouts = t->next;
			Free(t);
			return;
		}
		prev = t;
	}
}

/* Input events. */
typedef struct input {
        struct input *next;
        int source;
        int condition;
        void (*proc)(H3270 *session);
        H3270 *session;
} input_t;
static input_t *inputs = (input_t *)NULL;
static Boolean inputs_changed = False;

static unsigned long DefaultAddInput(int source, H3270 *session, void (*fn)(H3270 *session))
{
	input_t *ip;

	Trace("%s session=%p proc=%p",__FUNCTION__,session,fn);

	ip = (input_t *) Malloc(sizeof(input_t));
	memset(ip,0,sizeof(input_t));

	ip->source = source;
	ip->condition = InputReadMask;
	ip->proc = fn;
	ip->session = session;
	ip->next = inputs;
	inputs = ip;
	inputs_changed = True;

	Trace("%s: fd=%d callback=%p handle=%p",__FUNCTION__,source,fn,ip);

	return (unsigned long) ip;
}

static unsigned long DefaultAddExcept(int source, H3270 *session, void (*fn)(H3270 *session))
{
#if defined(_WIN32) /*[*/
	return 0;
#else /*][*/
	input_t *ip;

	Trace("%s session=%p proc=%p",__FUNCTION__,session,fn);

	ip = (input_t *)Malloc(sizeof(input_t));
	memset(ip,0,sizeof(input_t));

	ip->source = source;
	ip->condition = InputExceptMask;
	ip->proc = fn;
	ip->session = session;
	ip->next = inputs;
	inputs = ip;
	inputs_changed = True;

	Trace("%s: fd=%d callback=%p handle=%p",__FUNCTION__,source,fn,ip);

	return (unsigned long)ip;
#endif /*]*/
}

#if !defined(_WIN32) /*[*/
static unsigned long DefaultAddOutput(int source, H3270 *session, void (*fn)(H3270 *session))
{
	input_t *ip;

	Trace("%s session=%p proc=%p",__FUNCTION__,session,fn);

	ip = (input_t *)Malloc(sizeof(input_t));
	memset(ip,0,sizeof(input_t));

	ip->source = source;
	ip->condition = InputWriteMask;
	ip->proc = fn;
	ip->session = session;
	ip->next = inputs;
	inputs = ip;
	inputs_changed = True;

	Trace("%s: fd=%d callback=%p handle=%p",__FUNCTION__,source,fn,ip);

	return (unsigned long)ip;
}
#endif /*]*/

static void DefaultRemoveInput(unsigned long id)
{
	input_t *ip;
	input_t *prev = (input_t *)NULL;

	Trace("%s: fhandle=%p",__FUNCTION__,(input_t *) id);

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

	Free(ip);
	inputs_changed = True;
}

#if defined(_WIN32) /*[*/
#define MAX_HA	256
#endif /*]*/

/* Event dispatcher. */
static int DefaultProcessEvents(int block)
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
			ha[nha++] = (HANDLE)ip->source;
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
			Trace("%s",__FUNCTION__);
			(*ip->proc)(ip->session);
			Trace("%s",__FUNCTION__);
			processed_any = True;
			if (inputs_changed)
				goto retry;
		}

#if !defined(_WIN32)
		if (((unsigned long)ip->condition & InputWriteMask) && FD_ISSET(ip->source, &wfds))
		{
			Trace("%s",__FUNCTION__);
			(*ip->proc)(ip->session);
			Trace("%s",__FUNCTION__);
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
		while ((t = timeouts) != TN) {
#if defined(_WIN32)
			if (t->ts <= now) {
#else
			if (t->tv.tv_sec < now.tv_sec ||
			    (t->tv.tv_sec == now.tv_sec &&
			     t->tv.tv_usec < now.tv_usec)) {
#endif
				timeouts = t->next;
				t->in_play = True;
				Trace("%s",__FUNCTION__);
				(*t->proc)(t->session);
				Trace("%s",__FUNCTION__);
				processed_any = True;
				Free(t);
			} else
				break;
		}
	}

	if (inputs_changed)
		goto retry;

	return processed_any;

}

/*---[ Implement external calls ]---------------------------------------------------------------------------*/

void * Malloc(size_t len)
{
	char *r;

	r = malloc(len);
	if (r == (char *)NULL)
		Error(NULL,"Out of memory");
	return r;
}

void * Calloc(size_t nelem, size_t elsize)
{
	int		  sz = nelem * elsize;
	char	* r = malloc(sz);

	if(!r)
		Error(NULL,"Out of memory");

	memset(r, 0, sz);
	return r;
}

void * Realloc(void *p, size_t len)
{
	p = realloc(p, len);
	if (p == NULL)
		Error(NULL,"Out of memory");
	return p;
}

void Free(void *p)
{
	if(p)
		free(p);
}

void * lib3270_calloc(size_t elsize, size_t nelem, void *ptr)
{
	size_t sz = nelem * elsize;

	if(ptr)
		ptr = realloc(ptr,sz);
	else
		ptr = malloc(sz);

	if(ptr)
		memset(ptr,0,sz);
	else
		Error(NULL,"Out of memory");

	return ptr;
}


static struct {
	const char *name;
	KeySym keysym;
} latin1[] = {
	{ "space", XK_space },
	{ "exclam", XK_exclam },
	{ "quotedbl", XK_quotedbl },
	{ "numbersign", XK_numbersign },
	{ "dollar", XK_dollar },
	{ "percent", XK_percent },
	{ "ampersand", XK_ampersand },
	{ "apostrophe", XK_apostrophe },
	{ "quoteright", XK_quoteright },
	{ "parenleft", XK_parenleft },
	{ "parenright", XK_parenright },
	{ "asterisk", XK_asterisk },
	{ "plus", XK_plus },
	{ "comma", XK_comma },
	{ "minus", XK_minus },
	{ "period", XK_period },
	{ "slash", XK_slash },
	{ "0", XK_0 },
	{ "1", XK_1 },
	{ "2", XK_2 },
	{ "3", XK_3 },
	{ "4", XK_4 },
	{ "5", XK_5 },
	{ "6", XK_6 },
	{ "7", XK_7 },
	{ "8", XK_8 },
	{ "9", XK_9 },
	{ "colon", XK_colon },
	{ "semicolon", XK_semicolon },
	{ "less", XK_less },
	{ "equal", XK_equal },
	{ "greater", XK_greater },
	{ "question", XK_question },
	{ "at", XK_at },
	{ "A", XK_A },
	{ "B", XK_B },
	{ "C", XK_C },
	{ "D", XK_D },
	{ "E", XK_E },
	{ "F", XK_F },
	{ "G", XK_G },
	{ "H", XK_H },
	{ "I", XK_I },
	{ "J", XK_J },
	{ "K", XK_K },
	{ "L", XK_L },
	{ "M", XK_M },
	{ "N", XK_N },
	{ "O", XK_O },
	{ "P", XK_P },
	{ "Q", XK_Q },
	{ "R", XK_R },
	{ "S", XK_S },
	{ "T", XK_T },
	{ "U", XK_U },
	{ "V", XK_V },
	{ "W", XK_W },
	{ "X", XK_X },
	{ "Y", XK_Y },
	{ "Z", XK_Z },
	{ "bracketleft", XK_bracketleft },
	{ "backslash", XK_backslash },
	{ "bracketright", XK_bracketright },
	{ "asciicircum", XK_asciicircum },
	{ "underscore", XK_underscore },
	{ "grave", XK_grave },
	{ "quoteleft", XK_quoteleft },
	{ "a", XK_a },
	{ "b", XK_b },
	{ "c", XK_c },
	{ "d", XK_d },
	{ "e", XK_e },
	{ "f", XK_f },
	{ "g", XK_g },
	{ "h", XK_h },
	{ "i", XK_i },
	{ "j", XK_j },
	{ "k", XK_k },
	{ "l", XK_l },
	{ "m", XK_m },
	{ "n", XK_n },
	{ "o", XK_o },
	{ "p", XK_p },
	{ "q", XK_q },
	{ "r", XK_r },
	{ "s", XK_s },
	{ "t", XK_t },
	{ "u", XK_u },
	{ "v", XK_v },
	{ "w", XK_w },
	{ "x", XK_x },
	{ "y", XK_y },
	{ "z", XK_z },
	{ "braceleft", XK_braceleft },
	{ "bar", XK_bar },
	{ "braceright", XK_braceright },
	{ "asciitilde", XK_asciitilde },
	{ "nobreakspace", XK_nobreakspace },
	{ "exclamdown", XK_exclamdown },
	{ "cent", XK_cent },
	{ "sterling", XK_sterling },
	{ "currency", XK_currency },
	{ "yen", XK_yen },
	{ "brokenbar", XK_brokenbar },
	{ "section", XK_section },
	{ "diaeresis", XK_diaeresis },
	{ "copyright", XK_copyright },
	{ "ordfeminine", XK_ordfeminine },
	{ "guillemotleft", XK_guillemotleft },
	{ "notsign", XK_notsign },
	{ "hyphen", XK_hyphen },
	{ "registered", XK_registered },
	{ "macron", XK_macron },
	{ "degree", XK_degree },
	{ "plusminus", XK_plusminus },
	{ "twosuperior", XK_twosuperior },
	{ "threesuperior", XK_threesuperior },
	{ "acute", XK_acute },
	{ "mu", XK_mu },
	{ "paragraph", XK_paragraph },
	{ "periodcentered", XK_periodcentered },
	{ "cedilla", XK_cedilla },
	{ "onesuperior", XK_onesuperior },
	{ "masculine", XK_masculine },
	{ "guillemotright", XK_guillemotright },
	{ "onequarter", XK_onequarter },
	{ "onehalf", XK_onehalf },
	{ "threequarters", XK_threequarters },
	{ "questiondown", XK_questiondown },
	{ "Agrave", XK_Agrave },
	{ "Aacute", XK_Aacute },
	{ "Acircumflex", XK_Acircumflex },
	{ "Atilde", XK_Atilde },
	{ "Adiaeresis", XK_Adiaeresis },
	{ "Aring", XK_Aring },
	{ "AE", XK_AE },
	{ "Ccedilla", XK_Ccedilla },
	{ "Egrave", XK_Egrave },
	{ "Eacute", XK_Eacute },
	{ "Ecircumflex", XK_Ecircumflex },
	{ "Ediaeresis", XK_Ediaeresis },
	{ "Igrave", XK_Igrave },
	{ "Iacute", XK_Iacute },
	{ "Icircumflex", XK_Icircumflex },
	{ "Idiaeresis", XK_Idiaeresis },
	{ "ETH", XK_ETH },
	{ "Eth", XK_Eth },
	{ "Ntilde", XK_Ntilde },
	{ "Ograve", XK_Ograve },
	{ "Oacute", XK_Oacute },
	{ "Ocircumflex", XK_Ocircumflex },
	{ "Otilde", XK_Otilde },
	{ "Odiaeresis", XK_Odiaeresis },
	{ "multiply", XK_multiply },
	{ "Ooblique", XK_Ooblique },
	{ "Ugrave", XK_Ugrave },
	{ "Uacute", XK_Uacute },
	{ "Ucircumflex", XK_Ucircumflex },
	{ "Udiaeresis", XK_Udiaeresis },
	{ "Yacute", XK_Yacute },
	{ "THORN", XK_THORN },
	{ "Thorn", XK_Thorn },
	{ "ssharp", XK_ssharp },
	{ "agrave", XK_agrave },
	{ "aacute", XK_aacute },
	{ "acircumflex", XK_acircumflex },
	{ "atilde", XK_atilde },
	{ "adiaeresis", XK_adiaeresis },
	{ "aring", XK_aring },
	{ "ae", XK_ae },
	{ "ccedilla", XK_ccedilla },
	{ "egrave", XK_egrave },
	{ "eacute", XK_eacute },
	{ "ecircumflex", XK_ecircumflex },
	{ "ediaeresis", XK_ediaeresis },
	{ "igrave", XK_igrave },
	{ "iacute", XK_iacute },
	{ "icircumflex", XK_icircumflex },
	{ "idiaeresis", XK_idiaeresis },
	{ "eth", XK_eth },
	{ "ntilde", XK_ntilde },
	{ "ograve", XK_ograve },
	{ "oacute", XK_oacute },
	{ "ocircumflex", XK_ocircumflex },
	{ "otilde", XK_otilde },
	{ "odiaeresis", XK_odiaeresis },
	{ "division", XK_division },
	{ "oslash", XK_oslash },
	{ "ugrave", XK_ugrave },
	{ "uacute", XK_uacute },
	{ "ucircumflex", XK_ucircumflex },
	{ "udiaeresis", XK_udiaeresis },
	{ "yacute", XK_yacute },
	{ "thorn", XK_thorn },
	{ "ydiaeresis", XK_ydiaeresis },

	// The following are, umm, hacks to allow symbolic names for
	// control codes.
#if !defined(_WIN32)
	{ "BackSpace", 0x08 },
	{ "Tab", 0x09 },
	{ "Linefeed", 0x0a },
	{ "Return", 0x0d },
	{ "Escape", 0x1b },
	{ "Delete", 0x7f },
#endif

	{ (char *)NULL, NoSymbol }
};

KeySym StringToKeysym(char *s)
{
	int i;

	if (strlen(s) == 1 && (*(unsigned char *)s & 0x7f) > ' ')
		return (KeySym)*(unsigned char *)s;
	for (i = 0; latin1[i].name != (char *)NULL; i++) {
		if (!strcmp(s, latin1[i].name))
			return latin1[i].keysym;
	}
	return NoSymbol;
}

const char * KeysymToString(KeySym k)
{
	int i;

	for (i = 0; latin1[i].name != (char *)NULL; i++) {
		if (latin1[i].keysym == k)
			return latin1[i].name;
	}
	return (char *)NULL;
}


/* Timeouts. */

unsigned long AddTimeOut(unsigned long interval_ms, H3270 *session, void (*proc)(H3270 *session))
{
	if(callbacks->AddTimeOut)
		return callbacks->AddTimeOut(interval_ms,session,proc);
	return 0;
}

void RemoveTimeOut(unsigned long timer)
{
	if(callbacks->RemoveTimeOut)
		return callbacks->RemoveTimeOut(timer);
}

unsigned long AddInput(int source, H3270 *session, void (*fn)(H3270 *session))
{
	if(callbacks->AddInput)
		return callbacks->AddInput(source,session,fn);
	return 0;
}

unsigned long AddExcept(int source, H3270 *session, void (*fn)(H3270 *session))
{
	if(callbacks->AddExcept)
		return callbacks->AddExcept(source,session,fn);
	return 0;
}

#if !defined(_WIN32) /*[*/
unsigned long AddOutput(int source, H3270 *session, void (*fn)(H3270 *session))
{
	if(callbacks->AddOutput)
		return callbacks->AddOutput(source,session,fn);
	return 0;
}
#endif /*]*/

void RemoveInput(unsigned long id)
{
	if(callbacks->RemoveInput)
		callbacks->RemoveInput(id);
}

int LIB3270_EXPORT lib3270_register_handlers(const struct lib3270_callbacks *cbk)
{
	if(!cbk)
		return EINVAL;

	if(cbk->sz != sizeof(struct lib3270_callbacks))
		return EINVAL;

	callbacks = cbk;
	return 0;

}

LIB3270_EXPORT LIB3270_CSTATE lib3270_get_connection_state(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return h->cstate;
}

LIB3270_EXPORT int lib3270_pconnected(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return (((int) h->cstate) >= (int)RESOLVING);
}

LIB3270_EXPORT int lib3270_half_connected(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return (h->cstate == RESOLVING || h->cstate == PENDING);
}

LIB3270_EXPORT int lib3270_connected(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return ((int) h->cstate >= (int)CONNECTED_INITIAL);
}

LIB3270_EXPORT int lib3270_in_neither(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return (h->cstate == CONNECTED_INITIAL);
}

LIB3270_EXPORT int lib3270_in_ansi(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return (h->cstate == CONNECTED_ANSI || h->cstate == CONNECTED_NVT);
}

LIB3270_EXPORT int lib3270_in_3270(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return (h->cstate == CONNECTED_3270 || h->cstate == CONNECTED_TN3270E || h->cstate == CONNECTED_SSCP);
}

LIB3270_EXPORT int lib3270_in_sscp(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return (h->cstate == CONNECTED_SSCP);
}

LIB3270_EXPORT int lib3270_in_tn3270e(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return (h->cstate == CONNECTED_TN3270E);
}

LIB3270_EXPORT int lib3270_in_e(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return (h->cstate >= CONNECTED_INITIAL_E);
}

LIB3270_EXPORT void * lib3270_get_widget(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return h->widget;
}

LIB3270_EXPORT int lib3270_call_thread(int(*callback)(H3270 *h, void *), H3270 *h, void *parm)
{
	int rc;
	CHECK_SESSION_HANDLE(h);

	if(h->set_timer)
		h->set_timer(h,1);

	lib3270_main_iterate(0);
	if(callbacks->callthread)
		rc = callbacks->callthread(callback,h,parm);
	else
		rc = callback(h,parm);
	lib3270_main_iterate(0);

	if(h->set_timer)
		h->set_timer(h,0);

	return rc;
}

LIB3270_EXPORT void lib3270_main_iterate(int wait)
{
	if(callbacks->RunPendingEvents)
		callbacks->RunPendingEvents(wait);
}

LIB3270_EXPORT int lib3270_wait(seconds)
{
	time_t end;

	if(callbacks->Wait)
		return callbacks->Wait(seconds);

	// Alternative wait call
	end = time(0) + seconds;

	while(time(0) < end)
	{
		lib3270_main_iterate(1);
	}

	return 0;
}

LIB3270_EXPORT void lib3270_ring_bell(H3270 *session)
{
	CHECK_SESSION_HANDLE(session);

	if(lib3270_get_toggle(session,LIB3270_TOGGLE_BEEP))
		callbacks->ring_bell(session);
}


