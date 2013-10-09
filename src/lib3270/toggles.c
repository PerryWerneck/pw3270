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
 * Este programa está nomeado como toggles.c e possui 253 linhas de código.
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
 *	toggles.c
 *		This module handles toggles.
 */

#include <errno.h>
#include <lib3270/config.h>
#include "toggle.h"
#include "globals.h"
//#include "appres.h"

#include "ansic.h"
//#include "actionsc.h"
#include "ctlrc.h"
#include "popupsc.h"
#include "screenc.h"
#include "trace_dsc.h"
#include "togglesc.h"
#include "api.h"

static const char *toggle_names[LIB3270_TOGGLE_COUNT] =
{
		"monocase",
		"cursorblink",
		"showtiming",
		"cursorpos",
		"dstrace",
		"linewrap",
		"blankfill",
		"screentrace",
		"eventtrace",
		"marginedpaste",
		"rectselect",
		"crosshair",
		"fullscreen",
		"reconnect",
		"insert",
		"smartpaste",
		"bold",
		"keepselected",
		"underline",
		"autoconnect",
		"kpalternative",			/**< Keypad +/- move to next/previous field */
		"beep",						/**< Beep on errors */
		"fieldattr",				/**< View Field attribute */
		"altscreen",				/**< auto resize on altscreen */
		"keepalive",				/**< Enable network keep-alive with SO_KEEPALIVE */


};

LIB3270_EXPORT unsigned char lib3270_get_toggle(H3270 *session, LIB3270_TOGGLE ix)
{
	CHECK_SESSION_HANDLE(session);

	if(ix < 0 || ix >= N_TOGGLES)
		return 0;

	return session->toggle[ix].value != 0;
}

/*
 * Call the internal update routine
 */
static void toggle_notify(H3270 *session, struct lib3270_toggle *t, LIB3270_TOGGLE ix)
{
	trace("%s: ix=%d upcall=%p",__FUNCTION__,ix,t->upcall);
	t->upcall(session, t, TT_INTERACTIVE);

	if(session->update_toggle)
		session->update_toggle(session,ix,t->value,TT_INTERACTIVE,toggle_names[ix]);

}

LIB3270_EXPORT int lib3270_set_toggle(H3270 *session, LIB3270_TOGGLE ix, int value)
{
	char v = value ? True : False;
	struct lib3270_toggle * t;

	CHECK_SESSION_HANDLE(session);

	if(ix < 0 || ix >= LIB3270_TOGGLE_COUNT)
		return -EINVAL;

	t = &session->toggle[ix];

	if(v == t->value)
		return 0;

	t->value = v;

	toggle_notify(session,t,ix);
	return 1;
}

LIB3270_EXPORT int lib3270_toggle(H3270 *session, LIB3270_TOGGLE ix)
{
	struct lib3270_toggle	*t;

	CHECK_SESSION_HANDLE(session);

	if(ix < 0 || ix >= LIB3270_TOGGLE_COUNT)
		return 0;

	t = &session->toggle[ix];

	t->value = t->value ? False : True;
	toggle_notify(session,t,ix);

	return (int) t->value;
}

static void toggle_altscreen(H3270 *session, struct lib3270_toggle *t, LIB3270_TOGGLE_TYPE tt)
{
	if(!session->screen_alt)
		set_viewsize(session,t->value ? 24 : session->maxROWS,80);
}

static void toggle_redraw(H3270 *session, struct lib3270_toggle *t, LIB3270_TOGGLE_TYPE tt)
{
	session->display(session);
}

/*
 * No-op toggle.
 */
static void toggle_nop(H3270 *session, struct lib3270_toggle *t, LIB3270_TOGGLE_TYPE tt unused)
{
}

static void toggle_keepalive(H3270 *session, struct lib3270_toggle *t unused, LIB3270_TOGGLE_TYPE tt unused)
{
	if(session->sock > 0)
	{
		// Update keep-alive option
		int optval = t->value ? 1 : 0;

		if (setsockopt(session->sock, SOL_SOCKET, SO_KEEPALIVE, (char *)&optval, sizeof(optval)) < 0)
		{
			popup_a_sockerr(session, N_( "Can´t %s network keep-alive" ), optval ? _( "enable" ) : _( "disable" ));
		}
		else
		{
			trace_dsn(session,"Network keep-alive is %s\n",optval ? "enabled" : "disabled" );
		}

	}
}

/*
 * Called from system initialization code to handle initial toggle settings.
 */
void initialize_toggles(H3270 *session)
{
	int f;

	for(f=0;f<LIB3270_TOGGLE_COUNT;f++)
		session->toggle[f].upcall	= toggle_nop;

	session->toggle[LIB3270_TOGGLE_RECTANGLE_SELECT].upcall	= toggle_rectselect;
	session->toggle[LIB3270_TOGGLE_MONOCASE].upcall 		= toggle_redraw;
	session->toggle[LIB3270_TOGGLE_UNDERLINE].upcall 		= toggle_redraw;
	session->toggle[LIB3270_TOGGLE_ALTSCREEN].upcall 		= toggle_altscreen;
	session->toggle[LIB3270_TOGGLE_ALTSCREEN].upcall 		= toggle_altscreen;
	session->toggle[LIB3270_TOGGLE_KEEP_ALIVE].upcall		= toggle_keepalive;

	static const LIB3270_TOGGLE active_by_default[] =
	{
		LIB3270_TOGGLE_CURSOR_BLINK,
		LIB3270_TOGGLE_CURSOR_POS,
		LIB3270_TOGGLE_BEEP,
		LIB3270_TOGGLE_ALTSCREEN,
		LIB3270_TOGGLE_KEEP_ALIVE
	};

	for(f=0;f< (sizeof(active_by_default)/sizeof(active_by_default[0])); f++)
		session->toggle[active_by_default[f]].value = True;

	for(f=0;f<LIB3270_TOGGLE_COUNT;f++)
	{
		if(session->toggle[f].value)
			session->toggle[f].upcall(session,&session->toggle[f],TT_INITIAL);
	}

}

/*
 * Called from system exit code to handle toggles.
 */
void shutdown_toggles(H3270 *session)
{
#if defined(X3270_TRACE)
	static const LIB3270_TOGGLE disable_on_shutdown[] = {DS_TRACE, EVENT_TRACE, SCREEN_TRACE};

	int f;

	for(f=0;f< (sizeof(disable_on_shutdown)/sizeof(disable_on_shutdown[0])); f++)
		lib3270_set_toggle(session,disable_on_shutdown[f],0);

#endif
}

LIB3270_EXPORT const char * lib3270_get_toggle_name(LIB3270_TOGGLE_ID ix)
{
	if(ix < N_TOGGLES)
		return toggle_names[ix];
	return "";
}

LIB3270_EXPORT LIB3270_TOGGLE lib3270_get_toggle_id(const char *name)
{
	if(name)
	{
		int f;
		for(f=0;f<N_TOGGLES;f++)
		{
			if(!strcasecmp(name,toggle_names[f]))
				return f;
		}
	}
	return -1;
}

