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

#include "globals.h"
// #include "appres.h"
#include "resources.h"

#include "actionsc.h"
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
static char * stoken(char **s)
{
	char *r;
	char *ss = *s;

	if (!*ss)
		return NULL;
	r = ss;
	while (*ss && *ss != ' ' && *ss != '\t')
		ss++;
	if (*ss) {
		*ss++ = '\0';
		while (*ss == ' ' || *ss == '\t')
			ss++;
	}
	*s = ss;
	return r;
}
*/

/*
 * Read the host file
 */ /*
void
hostfile_init(void)
{
	FILE *hf;
	char buf[1024];
	static Boolean hostfile_initted = False;
	struct host *h;
	char *hostfile_name;

	if (hostfile_initted)
		return;

	hostfile_initted = True;
	hostfile_name = appres.hostsfile;
	if (hostfile_name == CN)
		hostfile_name = xs_buffer("%s/ibm_hosts", appres.conf_dir);
	else
		hostfile_name = do_subst(appres.hostsfile, True, True);
	hf = fopen(hostfile_name, "r");
	if (hf != (FILE *)NULL) {
		while (fgets(buf, sizeof(buf), hf)) {
			char *s = buf;
			char *name, *entry_type, *hostname;
			char *slash;

			if (strlen(buf) > (unsigned)1 &&
			    buf[strlen(buf) - 1] == '\n') {
				buf[strlen(buf) - 1] = '\0';
			}
			while (isspace(*s))
				s++;
			if (!*s || *s == '#')
				continue;
			name = stoken(&s);
			entry_type = stoken(&s);
			hostname = stoken(&s);
			if (!name || !entry_type || !hostname) {
				popup_an_error("Bad %s syntax, entry skipped",
				    ResHostsFile);
				continue;
			}
			h = (struct host *)lib3270_malloc(sizeof(*h));
			if (!split_hier(NewString(name), &h->name,
						&h->parents)) {
				lib3270_free(h);
				continue;
			}
			h->hostname = NewString(hostname);

			//
			// Quick syntax extension to allow the hosts file to
			// specify a port as host/port.
			//
			if ((slash = strchr(h->hostname, '/')))
				*slash = ':';

			if (!strcmp(entry_type, "primary"))
				h->entry_type = PRIMARY;
			else
				h->entry_type = ALIAS;
			if (*s)
				h->loginstring = NewString(s);
			else
				h->loginstring = CN;
			h->prev = last_host;
			h->next = (struct host *)NULL;
			if (last_host)
				last_host->next = h;
			else
				hosts = h;
			last_host = h;
		}
		(void) fclose(hf);
	} else if (appres.hostsfile != CN) {
		popup_an_errno(errno, "Cannot open " ResHostsFile " '%s'",
				appres.hostsfile);
	}
	lib3270_free(hostfile_name);

// #if defined(X3270_DISPLAY)
// 	save_recent(CN);
// #endif
}
*/

/*
 * Look up a host in the list.  Turns aliases into real hostnames, and
 * finds loginstrings.
 */ /*
static int
hostfile_lookup(const char *name, char **hostname, char **loginstring)
{
	struct host *h;

	hostfile_init();
	for (h = hosts; h != (struct host *)NULL; h = h->next) {
		if (h->entry_type == RECENT)
			continue;
		if (!strcmp(name, h->name)) {
			*hostname = h->hostname;
			if (h->loginstring != CN) {
				*loginstring = h->loginstring;
			} else {
				*loginstring = appres.login_macro;
			}
			return 1;
		}
	}
	return 0;
}
*/

/*
#if defined(LOCAL_PROCESS)
// Recognize and translate "-e" options.
static const char *
parse_localprocess(const char *s)
{
	int sl = strlen(OptLocalProcess);

	if (!strncmp(s, OptLocalProcess, sl)) {
		if (s[sl] == ' ')
			return(s + sl + 1);
		else if (s[sl] == '\0') {
			char *r;

			r = getenv("SHELL");
			if (r != CN)
				return r;
			else
				return "/bin/sh";
		}
	}
	return CN;
}
#endif
*/

/*
 * Strip qualifiers from a hostname.
 * Returns the hostname part in a newly-malloc'd string.
 * 'needed' is returned True if anything was actually stripped.
 * Returns NULL if there is a syntax error.
 */
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

	/*
	 * Hostname syntax is:
	 *  Zero or more optional prefixes (A:, S:, P:, N:, L:, C:).
	 *  An optional LU name separated by '@'.
	 *  A hostname optionally in square brackets (which quote any colons
	 *   in the name).
	 *  An optional port name or number separated from the hostname by a
	 *  space or colon.
	 * No additional white space or colons are allowed.
	 */

	/* Strip leading whitespace. */
	while (*s && isspace(*s))
		s++;

	if (!*s)
	{
		popup_an_error(hSession,_( "Empty hostname" ));
		goto split_fail;
	}

	/* Strip trailing whitespace. */
	while (isspace(*(s + strlen(s) - 1)))
		*(s + strlen(s) - 1) = '\0';

	/* Start with the prefixes. */
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

		/* Allow whitespace around the prefixes. */
		while (*s && isspace(*s))
			s++;
	}

	/* Process the LU name. */
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

	/*
	 * Isolate the hostname.
	 * At this point, we've found its start, so we can malloc the buffer
	 * that will hold the copy.
	 */
	if (lbracket != CN) {
		char *rbracket;

		/* Check for junk before the '['. */
		if (lbracket != s) {
			popup_system_error(hSession,NULL,_("Hostname syntax error"),"%s",_("Text before '['"));
			goto split_fail;
		}

		s = r = NewString(lbracket + 1);

		/*
		 * Take whatever is inside square brackets, including
		 * whitespace, unmodified -- except for empty strings.
		 */
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

		/* Skip over any whitespace after the bracketed name. */
		s = rbracket + 1;
		while (*s && isspace(*s))
			s++;
		if (!*s)
			goto split_success;
		colon = (*s == ':');
	} else {
		char *name_end;

		/* Check for an empty string. */
		if (!*s || *s == ':') {
			popup_an_error(hSession,"Empty hostname");
			goto split_fail;
		}

		s = r = NewString(s);

		/* Find the end of the hostname. */
		while (*s && !isspace(*s) && *s != ':')
			s++;
		name_end = s;

		/* If the terminator is whitespace, skip the rest of it. */
		while (*s && isspace(*s))
			s++;

		/*
		 * If there's nothing but whitespace (or nothing) after the
		 * name, we're done.
		 */
		if (*s == '\0') {
			*name_end = '\0';
			goto split_success;
		}
		colon = (*s == ':');
		*name_end = '\0';
	}

	/*
	 * If 'colon' is set, 's' points at it (or where it was).  Skip
	 * it and any whitespace that follows.
	 */
	if (colon) {
		s++;
		while (*s && isspace(*s))
			s++;
		if (!*s) {
			popup_system_error(hSession,NULL,_("Hostname syntax error"),"%s",_("Empty port name"));
			goto split_fail;
		}
	}

	/*
	 * Set the portname and find its end.
	 * Note that trailing spaces were already stripped, so the end of the
	 * portname must be a NULL.
	 */
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

static int do_connect(H3270 *hSession, const char *n)
{
	char nb[2048];				/* name buffer */
	char *s;					/* temporary */
	char *chost = NULL;			/* to whom we will connect */
//	char *ps = CN;
	char *port = CN;
	Boolean resolving;
	Boolean pending;
	static Boolean ansi_host;
	Boolean has_colons = False;

	if (lib3270_connected(hSession) || hSession->auto_reconnect_inprogress)
		return EBUSY;

	/* Skip leading blanks. */
	while (*n == ' ')
		n++;

	if (!*n)
	{
		popup_an_error(hSession,_( "Invalid (empty) hostname" ));
		return -1;
	}

	/* Save in a modifiable buffer. */
	(void) strncpy(nb, n, 2047);

	/* Strip trailing blanks. */
	s = nb + strlen(nb) - 1;
	while (*s == ' ')
		*s-- = '\0';

	/* Remember this hostname, as the last hostname we connected to. */
	lib3270_set_host(hSession,nb);

	{
		Boolean needed;

		/* Strip off and remember leading qualifiers. */
		if ((s = split_host(hSession, nb, &ansi_host, &hSession->std_ds_host,
		    &hSession->passthru_host, &hSession->non_tn3270e_host, &hSession->ssl_host,
		    &hSession->no_login_host, hSession->luname, &port,
		    &needed)) == CN)
			return EINVAL;

		chost = s;

		/* Default the port. */
		if (port == CN)
			port = "telnet";
	}

	/*
	 * Store the original name in globals, even if we fail the connect
	 * later:
	 *  current_host is the hostname part, stripped of qualifiers, luname
	 *   and port number
	 *  full_current_host is the entire string, for use in reconnecting
	 */
	Replace(hSession->current_host, CN);

	has_colons = (strchr(chost, ':') != NULL);

	Replace(hSession->qualified_host,
	    xs_buffer("%s%s%s%s:%s",
		    hSession->ssl_host? "L:": "",
		    has_colons? "[": "",
		    chost,
		    has_colons? "]": "",
		    port));


	/* Attempt contact. */
	hSession->ever_3270 = False;

	if(net_connect(hSession, chost, port, 0, &resolving,&pending) != 0 && !resolving)
	{
		/* Redundantly signal a disconnect. */
		lib3270_set_disconnected(hSession);
		return -1;
	}

	chost = lib3270_free(chost);

	/* Still thinking about it? */
	if (resolving)
	{
		hSession->cstate = RESOLVING;
		lib3270_st_changed(hSession, LIB3270_STATE_RESOLVING, True);
		return 0;
	}

	/* Success. */

	/* Setup socket I/O. */
//	add_input_calls(hSession,net_input,net_exception);
#ifdef _WIN32
	hSession->ns_exception_id	= AddExcept((int) hSession->sockEvent, hSession, net_exception);
	hSession->ns_read_id		= AddInput((int) hSession->sockEvent, hSession, net_input);
#else
	hSession->ns_exception_id	= AddExcept(hSession->sock, hSession, net_exception);
	hSession->ns_read_id		= AddInput(hSession->sock, hSession, net_input);
#endif // WIN32

	hSession->excepting	= 1;
	hSession->reading 	= 1;


	/* Set state and tell the world. */
	if (pending)
	{
		hSession->cstate = PENDING;
		lib3270_st_changed(hSession, LIB3270_STATE_HALF_CONNECT, True);
	}
	else
	{
		lib3270_set_connected(hSession);
	}

	return 0;
}

/**
 * Connect to selected host.
 *
 * @param h		Session handle.
 * @param n		Hostname (null to reconnect to the last one;
 * @param wait	Wait for connection ok before return.
 *
 * @return 0 if the connection was ok, non zero on error.
 *
 */
int lib3270_connect(H3270 *h, const char *n, int wait)
{
	int rc;

	CHECK_SESSION_HANDLE(h);

	lib3270_main_iterate(h,0);

	if(h->auto_reconnect_inprogress)
		return EAGAIN;

	if(PCONNECTED)
		return EBUSY;

	if(!n)
	{
		n = h->full_current_host;
		if(!n)
			return EINVAL;
	}

	rc = do_connect(h,n);
	if(rc)
		return rc;

	if(wait)
	{
		while(!IN_ANSI && !IN_3270)
		{
			lib3270_main_iterate(h,1);

			if(!PCONNECTED)
			{
				return ENOTCONN;
			}
		}
	}

	return rc;
}

/*
 * Called from timer to attempt an automatic reconnection.
 */
static void try_reconnect(H3270 *session)
{
	lib3270_write_log(session,"3270","Starting auto-reconnect (Host: %s)",session->full_current_host ? session->full_current_host : "-");
	session->auto_reconnect_inprogress = 0;
	lib3270_reconnect(session,0);
}

LIB3270_EXPORT void lib3270_disconnect(H3270 *h)
{
	host_disconnect(h,0);
}

void host_disconnect(H3270 *h, int failed)
{
    CHECK_SESSION_HANDLE(h);

	if (CONNECTED || HALF_CONNECTED)
	{
		// Disconecting, disable input
		remove_input_calls(h);
		net_disconnect(h);

		trace("Disconnected (Failed: %d Reconnect: %d in_progress: %d)",failed,lib3270_get_toggle(h,LIB3270_TOGGLE_RECONNECT),h->auto_reconnect_inprogress);
		if (lib3270_get_toggle(h,LIB3270_TOGGLE_RECONNECT) && !h->auto_reconnect_inprogress)
		{
			/* Schedule an automatic reconnection. */
			h->auto_reconnect_inprogress = 1;
			(void) AddTimeOut(failed ? RECONNECT_ERR_MS: RECONNECT_MS, h, try_reconnect);
		}

		/*
		 * Remember a disconnect from ANSI mode, to keep screen tracing
		 * in sync.
		 */
#if defined(X3270_TRACE) /*[*/
		if (IN_ANSI && lib3270_get_toggle(h,LIB3270_TOGGLE_SCREEN_TRACE))
			trace_ansi_disc();
#endif /*]*/

		lib3270_set_disconnected(h);
	}
}

/* The host has entered 3270 or ANSI mode, or switched between them. */
void host_in3270(H3270 *session, LIB3270_CSTATE new_cstate)
{
	Boolean now3270 = (new_cstate == CONNECTED_3270 ||
			   new_cstate == CONNECTED_SSCP ||
			   new_cstate == CONNECTED_TN3270E);

	session->cstate = new_cstate;
	session->ever_3270 = now3270;
	lib3270_st_changed(session, LIB3270_STATE_3270_MODE, now3270);
}

void lib3270_set_connected(H3270 *hSession)
{
	hSession->cstate = CONNECTED_INITIAL;
	lib3270_st_changed(hSession, LIB3270_STATE_CONNECT, True);
	if(hSession->update_connect)
		hSession->update_connect(hSession,1);
}

void lib3270_set_disconnected(H3270 *hSession)
{
	CHECK_SESSION_HANDLE(hSession);

	hSession->cstate = NOT_CONNECTED;
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
#if defined(DEBUG) || defined(ANDROID)

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

LIB3270_EXPORT const char * lib3270_set_host(H3270 *h, const char *n)
{
    CHECK_SESSION_HANDLE(h);

	if(n && n != h->full_current_host)
	{
		char *new_hostname = strdup(n);

		trace("new hostname is \"%s\"",new_hostname);

		if(h->full_current_host)
			lib3270_free(h->full_current_host);

		h->full_current_host = new_hostname;

	}

	return h->full_current_host;
}

LIB3270_EXPORT const char * lib3270_get_host(H3270 *h)
{
    CHECK_SESSION_HANDLE(h);
	return h->full_current_host;
}

LIB3270_EXPORT int lib3270_reconnect(H3270 *h,int wait)
{
	int rc;

    CHECK_SESSION_HANDLE(h);

	if (CONNECTED || HALF_CONNECTED)
		return EBUSY;

	if (h->full_current_host == CN)
		return EINVAL;

	if (h->auto_reconnect_inprogress)
		return EBUSY;

	rc = lib3270_connect(h,h->full_current_host,wait);

	if(rc)
	{
		h->auto_reconnect_inprogress = 0;
		return rc;
	}

	return 0;
}

LIB3270_EXPORT const char * lib3270_get_luname(H3270 *h)
{
    CHECK_SESSION_HANDLE(h);
	return h->connected_lu;
}

