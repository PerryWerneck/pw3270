/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob o nome G3270.
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
 * Este programa está nomeado como host.c e possui 1078 linhas de código.
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


/*
 *	host.c
 *		This module handles the ibm_hosts file, connecting to and
 *		disconnecting from hosts, and state changes on the host
 *		connection.
 */

#include <malloc.h>
#include "globals.h"
// #include "appres.h"
#include "resources.h"

//#include "actionsc.h"
#include "hostc.h"
#include "statusc.h"
#include "popupsc.h"
#include "telnetc.h"
#include "trace_dsc.h"
#include "utilc.h"
#include "xioc.h"

#include <errno.h>
#include <lib3270/internals.h>

#define RECONNECT_MS		2000	/* 2 sec before reconnecting to host */
#define RECONNECT_ERR_MS	5000	/* 5 sec before reconnecting to host */

static void try_reconnect(H3270 *session);

/*
 * Strip qualifiers from a hostname.
 * Returns the hostname part in a newly-malloc'd string.
 * 'needed' is returned True if anything was actually stripped.
 * Returns NULL if there is a syntax error.
 */ /*
static char *
split_host(H3270 *hSession, char *s, char *ansi, char *std_ds, char *passthru,
	char *non_e, char *secure, char *no_login, char *xluname,
	char **port, char *needed)
{
	char *lbracket = CN;
	char *at = CN;
	char *r = NULL;
	char colon = False;

	*ansi = False;
	*std_ds = False;
	*passthru = False;
	*non_e = False;
	*secure = False;
	*xluname = '\0';
	*port = CN;

	*needed = False;

	//
	// Hostname syntax is:
	// Zero or more optional prefixes (A:, S:, P:, N:, L:, C:).
	// An optional LU name separated by '@'.
	// A hostname optionally in square brackets (which quote any colons
	// in the name).
	// An optional port name or number separated from the hostname by a
	// space or colon.
	// No additional white space or colons are allowed.
	//

	// Strip leading whitespace.
	while (*s && isspace(*s))
		s++;

	if (!*s)
	{
		popup_an_error(hSession,_( "Empty hostname" ));
		goto split_fail;
	}

	// Strip trailing whitespace.
	while (isspace(*(s + strlen(s) - 1)))
		*(s + strlen(s) - 1) = '\0';

	// Start with the prefixes.
	while (*s && *(s + 1) && isalpha(*s) && *(s + 1) == ':') {
		switch (*s) {
		case 'a':
		case 'A':
			*ansi = True;
			break;
		case 's':
		case 'S':
			*std_ds = True;
			break;
		case 'p':
		case 'P':
			*passthru = True;
			break;
		case 'n':
		case 'N':
			*non_e = True;
			break;
#if defined(HAVE_LIBSSL)
		case 'l':
		case 'L':
			*secure = True;
			break;
#else
		case 'l':
		case 'L':
			popup_system_error(hSession,	_( "SSL error" ),
											_( "Unable to connect to secure hosts" ),
											_( "This version of %s was built without support for secure sockets layer (SSL)." ),
											PACKAGE_NAME
											);

			goto split_fail;
#endif // HAVE_LIBSSL
		case 'c':
		case 'C':
			*no_login = True;
			break;
		default:
			popup_system_error(hSession,	_( "Hostname syntax error" ),
											_( "Can't connect to host" ),
											_( "Option '%c:' is not supported" ),
											*s );

			goto split_fail;
		}
		*needed = True;
		s += 2;

		// Allow whitespace around the prefixes.
		while (*s && isspace(*s))
			s++;
	}

	// Process the LU name.
	lbracket = strchr(s, '[');
	at = strchr(s, '@');
	if (at != CN && lbracket != CN && at > lbracket)
		at = CN;
	if (at != CN) {
		char *t;
		char *lu_last = at - 1;

		if (at == s) {
			popup_system_error(hSession,NULL,_("Hostname syntax error"),"%s",_("Empty LU name"));
			goto split_fail;
		}
		while (lu_last < s && isspace(*lu_last))
			lu_last--;
		for (t = s; t <= lu_last; t++) {
			if (isspace(*t)) {
				char *u = t + 1;

				while (isspace(*u))
					u++;
				if (*u != '@') {
					popup_system_error(hSession,NULL,_("Hostname syntax error"),"%s",_("Space in LU name"));
					goto split_fail;
				}
				break;
			}
			if (t - s < LIB3270_LUNAME_LENGTH) {
				xluname[t - s] = *t;
			}
		}
		xluname[t - s] = '\0';
		s = at + 1;
		while (*s && isspace(*s))
			s++;
		*needed = True;
	}

	//
	// Isolate the hostname.
	// At this point, we've found its start, so we can malloc the buffer
	// that will hold the copy.
	///
	if (lbracket != CN) {
		char *rbracket;

		// Check for junk before the '['.
		if (lbracket != s) {
			popup_system_error(hSession,NULL,_("Hostname syntax error"),"%s",_("Text before '['"));
			goto split_fail;
		}

		s = r = NewString(lbracket + 1);

		//
		// Take whatever is inside square brackets, including
		// whitespace, unmodified -- except for empty strings.
		//
		rbracket = strchr(s, ']');
		if (rbracket == CN) {
			popup_system_error(hSession,NULL,_("Hostname syntax error"),"%s",_("Missing ']'"));
			goto split_fail;
		}
		if (rbracket == s) {
			popup_system_error(hSession,NULL,_("Hostname syntax error"),"%s",_("Empty hostname"));
			goto split_fail;
		}
		*rbracket = '\0';

		// Skip over any whitespace after the bracketed name.
		s = rbracket + 1;
		while (*s && isspace(*s))
			s++;
		if (!*s)
			goto split_success;
		colon = (*s == ':');
	} else {
		char *name_end;

		// Check for an empty string.
		if (!*s || *s == ':') {
			popup_an_error(hSession,"Empty hostname");
			goto split_fail;
		}

		s = r = NewString(s);

		// Find the end of the hostname.
		while (*s && !isspace(*s) && *s != ':')
			s++;
		name_end = s;

		// If the terminator is whitespace, skip the rest of it.
		while (*s && isspace(*s))
			s++;

		//
		// If there's nothing but whitespace (or nothing) after the
		// name, we're done.
		//
		if (*s == '\0') {
			*name_end = '\0';
			goto split_success;
		}
		colon = (*s == ':');
		*name_end = '\0';
	}

	//
	// If 'colon' is set, 's' points at it (or where it was).  Skip
	// it and any whitespace that follows.
	//
	if (colon) {
		s++;
		while (*s && isspace(*s))
			s++;
		if (!*s) {
			popup_system_error(hSession,NULL,_("Hostname syntax error"),"%s",_("Empty port name"));
			goto split_fail;
		}
	}

	//
	// Set the portname and find its end.
	// Note that trailing spaces were already stripped, so the end of the
	// portname must be a NULL.
	//
	*port = s;
	*needed = True;
	while (*s && !isspace(*s) && *s != ':')
		s++;
	if (*s != '\0') {
		popup_system_error(hSession,NULL,_("Hostname syntax error"),"%s",_("Multiple port names"));
		goto split_fail;
	}
	goto split_success;

split_fail:
	lib3270_free(r);
	r = CN;

split_success:
	return r;
}
*/

/*
int lib3270_connect(H3270 *hSession, int wait)
{
	int		  rc;

	CHECK_SESSION_HANDLE(hSession);

	lib3270_main_iterate(hSession,0);

	if (CONNECTED || HALF_CONNECTED)
		return EBUSY;

	if(!hSession->host.full)
		return EINVAL;

	if (hSession->auto_reconnect_inprogress)
		return EBUSY;

	if(PCONNECTED)
		return EBUSY;

	rc = do_connect(hSession);
	if(rc)
		return rc;

	if(wait)
	{
		while(!IN_ANSI && !IN_3270)
		{
			lib3270_main_iterate(hSession,1);

			if(!PCONNECTED)
			{
				return ENOTCONN;
			}
		}
	}

	return rc;
}
*/

/*
 * Called from timer to attempt an automatic reconnection.
 */
static void try_reconnect(H3270 *session)
{
	lib3270_write_log(session,"3270","Starting auto-reconnect (Host: %s)",session->host.full ? session->host.full : "-");
	session->auto_reconnect_inprogress = 0;
	lib3270_connect(session,0);
}

LIB3270_EXPORT int lib3270_disconnect(H3270 *h)
{
	host_disconnect(h,0);
	return 0;
}

void host_disconnect(H3270 *hSession, int failed)
{
    CHECK_SESSION_HANDLE(hSession);

	if (CONNECTED || HALF_CONNECTED)
	{
		// Disconecting, disable input
		remove_input_calls(hSession);
		net_disconnect(hSession);

		trace("Disconnected (Failed: %d Reconnect: %d in_progress: %d)",failed,lib3270_get_toggle(hSession,LIB3270_TOGGLE_RECONNECT),hSession->auto_reconnect_inprogress);
		if (lib3270_get_toggle(hSession,LIB3270_TOGGLE_RECONNECT) && !hSession->auto_reconnect_inprogress)
		{
			/* Schedule an automatic reconnection. */
			hSession->auto_reconnect_inprogress = 1;
			(void) AddTimeOut(failed ? RECONNECT_ERR_MS: RECONNECT_MS, hSession, try_reconnect);
		}

		/*
		 * Remember a disconnect from ANSI mode, to keep screen tracing
		 * in sync.
		 */
#if defined(X3270_TRACE) /*[*/
		if (IN_ANSI && lib3270_get_toggle(hSession,LIB3270_TOGGLE_SCREEN_TRACE))
			trace_ansi_disc(hSession);
#endif /*]*/

		lib3270_set_disconnected(hSession);
	}
}

/* The host has entered 3270 or ANSI mode, or switched between them. */
void host_in3270(H3270 *hSession, LIB3270_CSTATE new_cstate)
{
	Boolean now3270 = (new_cstate == CONNECTED_3270 ||
			   new_cstate == CONNECTED_SSCP ||
			   new_cstate == CONNECTED_TN3270E);

	hSession->cstate = new_cstate;
	hSession->ever_3270 = now3270;
	lib3270_st_changed(hSession, LIB3270_STATE_3270_MODE, now3270);
}

void lib3270_set_connected(H3270 *hSession)
{
	hSession->cstate	= CONNECTED_INITIAL;
	hSession->starting	= 1;	// Enable autostart

	lib3270_st_changed(hSession, LIB3270_STATE_CONNECT, True);
	if(hSession->update_connect)
		hSession->update_connect(hSession,1);
}

void lib3270_set_disconnected(H3270 *hSession)
{
	CHECK_SESSION_HANDLE(hSession);

	hSession->cstate	= LIB3270_NOT_CONNECTED;
	hSession->starting	= 0;

	set_status(hSession,OIA_FLAG_UNDERA,False);
	lib3270_st_changed(hSession,LIB3270_STATE_CONNECT, False);
	status_changed(hSession,LIB3270_MESSAGE_DISCONNECTED);
	if(hSession->update_connect)
		hSession->update_connect(hSession,0);
}

/* Register a function interested in a state change. */
LIB3270_EXPORT void lib3270_register_schange(H3270 *h, LIB3270_STATE_CHANGE tx, void (*func)(H3270 *, int, void *),void *data)
{
	struct lib3270_state_callback *st;

    CHECK_SESSION_HANDLE(h);

	st 			= (struct lib3270_state_callback *) lib3270_malloc(sizeof(struct lib3270_state_callback));
	st->func	= func;

	if (h->st_last[tx])
		h->st_last[tx]->next = st;
	else
		h->st_callbacks[tx] = st;

	h->st_last[tx] = st;

}

/* Signal a state change. */
void lib3270_st_changed(H3270 *h, LIB3270_STATE tx, int mode)
{
#if defined(DEBUG)

	static const char * state_name[LIB3270_STATE_USER] =
	{
		"LIB3270_STATE_RESOLVING",
		"LIB3270_STATE_HALF_CONNECT",
		"LIB3270_STATE_CONNECT",
		"LIB3270_STATE_3270_MODE",
		"LIB3270_STATE_LINE_MODE",
		"LIB3270_STATE_REMODEL",
		"LIB3270_STATE_PRINTER",
		"LIB3270_STATE_EXITING",
		"LIB3270_STATE_CHARSET",

	};

#endif // DEBUG

	struct lib3270_state_callback *st;

    CHECK_SESSION_HANDLE(h);

	trace("%s is %d on session %p",state_name[tx],mode,h);

	for (st = h->st_callbacks[tx];st;st = st->next)
	{
//		trace("st=%p func=%p",st,st->func);
		st->func(h,mode,st->data);
	}

	trace("%s ends",__FUNCTION__);
}

static void update_host(H3270 *h)
{
	Replace(h->host.full,
			lib3270_strdup_printf(
				"%s%s:%s",
					h->options&LIB3270_OPTION_SSL ? "tn3270s://" : "tn3270://",
					h->host.current,
					h->host.srvc
		));

	trace("hosturl=[%s]",h->host.full);

}

LIB3270_EXPORT const char * lib3270_set_url(H3270 *h, const char *n)
{
    CHECK_SESSION_HANDLE(h);

	if(n && n != h->host.full)
	{
		static const struct _sch
		{
			LIB3270_OPTION	  opt;
			const char		* text;
			const char		* srvc;
		} sch[] =
		{
			{ LIB3270_OPTION_DEFAULTS,  "tn3270://",	"telnet"	},
			{ LIB3270_OPTION_SSL,		"tn3270s://",	"telnets"	},
			{ LIB3270_OPTION_DEFAULTS,  "telnet://",	"telnet"	},
			{ LIB3270_OPTION_DEFAULTS,  "telnets://",	"telnets"	},
			{ LIB3270_OPTION_SSL,		"L://",			"telnets"	},

			{ LIB3270_OPTION_SSL,		"L:",			"telnets"	}	// The compatibility should be the last option
		};

		char					* str 		= strdup(n);
		char					* hostname 	= str;
		const char 				* srvc		= "telnet";
		char					* ptr;
		char					* query		= "";
		int						  f;

		trace("%s(%s)",__FUNCTION__,str);
		h->options = LIB3270_OPTION_DEFAULTS;

		for(f=0;f < sizeof(sch)/sizeof(sch[0]);f++)
		{
			size_t sz = strlen(sch[f].text);
			if(!strncasecmp(hostname,sch[f].text,sz))
			{
				h->options	 = sch[f].opt;
				srvc		 = sch[f].srvc;
				hostname	+= sz;
				break;
			}
		}

		if(!*hostname)
			return h->host.current;

		ptr = strchr(hostname,':');
		if(ptr)
		{
			*(ptr++) = 0;
			srvc  = ptr;
			query = strchr(ptr,'?');

			trace("QUERY=[%s]",query);

			if(query)
				*(query++) = 0;
			else
				query = "";
		}

		trace("SRVC=[%s]",srvc);

		Replace(h->host.current,strdup(hostname));
		Replace(h->host.srvc,strdup(srvc));

		update_host(h);

		free(str);
	}

	return h->host.current;
}

LIB3270_EXPORT const char * lib3270_get_hostname(H3270 *h)
{
    CHECK_SESSION_HANDLE(h);

	if(h->host.current)
		return h->host.current;

	return "";
}

LIB3270_EXPORT void lib3270_set_hostname(H3270 *h, const char *hostname)
{
    CHECK_SESSION_HANDLE(h);
	Replace(h->host.current,strdup(hostname));
	update_host(h);
}

LIB3270_EXPORT const char * lib3270_get_srvcname(H3270 *h)
{
    CHECK_SESSION_HANDLE(h);
    if(h->host.srvc)
		return h->host.srvc;
	return "telnet";
}

LIB3270_EXPORT void lib3270_set_srvcname(H3270 *h, const char *srvc)
{
    CHECK_SESSION_HANDLE(h);
	Replace(h->host.srvc,strdup(srvc));
	update_host(h);
}

LIB3270_EXPORT const char * lib3270_get_host(H3270 *h)
{
    CHECK_SESSION_HANDLE(h);
	return h->host.full;
}

/*
LIB3270_EXPORT int lib3270_reconnect(H3270 *hSession,int wait)
{
	int rc;

    CHECK_SESSION_HANDLE(hSession);

	if (CONNECTED || HALF_CONNECTED)
		return EBUSY;

	if (!hSession->host.full)
		return EINVAL;

	if (hSession->auto_reconnect_inprogress)
		return EBUSY;

	rc = lib3270_connect(hSession,wait);

	if(rc)
	{
		hSession->auto_reconnect_inprogress = 0;
		return rc;
	}

	return 0;
}
*/

LIB3270_EXPORT const char * lib3270_get_luname(H3270 *h)
{
    CHECK_SESSION_HANDLE(h);
	return h->connected_lu;
}

