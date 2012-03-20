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
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
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
#include "globals.h"
#include "appres.h"

#include "ansic.h"
#include "actionsc.h"
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
};

LIB3270_EXPORT unsigned char lib3270_get_toggle(H3270 *session, LIB3270_TOGGLE ix)
{
	CHECK_SESSION_HANDLE(session);

	if(ix < 0 || ix >= N_TOGGLES)
		return 0;
	return (unsigned char) session->toggle[ix].value != 0;
}

/*
 * Call the internal update routine
 */
static void toggle_notify(H3270 *session, struct lib3270_toggle *t, LIB3270_TOGGLE ix)
{
	Trace("%s: ix=%d upcall=%p",__FUNCTION__,ix,t->upcall);
	t->upcall(session, t, TT_INTERACTIVE);

	if(session->update_toggle)
		session->update_toggle(session,ix,t->value,TT_INTERACTIVE,toggle_names[ix]);

}

LIB3270_EXPORT void lib3270_set_toggle(H3270 *session, LIB3270_TOGGLE ix, int value)
{
	char v = value ? True : False;
	struct lib3270_toggle * t;

	CHECK_SESSION_HANDLE(session);

	if(ix < 0 || ix >= LIB3270_TOGGLE_COUNT)
		return;

	t = &session->toggle[ix];

	if(v == t->value)
		return;

	t->value = v;

	toggle_notify(session,t,ix);
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

static void toggle_monocase(H3270 *session, struct lib3270_toggle *t, LIB3270_TOGGLE_TYPE tt)
{
	screen_disp(session);
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
	session->toggle[LIB3270_TOGGLE_MONOCASE].upcall 		= toggle_monocase;

#if defined(X3270_TRACE)
	session->toggle[LIB3270_TOGGLE_DS_TRACE].upcall			= toggle_dsTrace;
	session->toggle[LIB3270_TOGGLE_SCREEN_TRACE].upcall		= toggle_screenTrace;
	session->toggle[LIB3270_TOGGLE_EVENT_TRACE].upcall		= toggle_eventTrace;
#endif

#if defined(X3270_ANSI)
	session->toggle[LIB3270_TOGGLE_LINE_WRAP].upcall			= toggle_lineWrap;
#endif

	static const LIB3270_TOGGLE active_by_default[] =
	{
		LIB3270_TOGGLE_CURSOR_BLINK,
		LIB3270_TOGGLE_CURSOR_POS,
		LIB3270_TOGGLE_BEEP,
	};

	for(f=0;f< (sizeof(active_by_default)/sizeof(active_by_default[0])); f++)
	{
		session->toggle[active_by_default[f]].value = True;
	}


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
	{
		LIB3270_TOGGLE	  ix	= disable_on_shutdown[f];
		struct toggle	* t		= &session->toggle[ix];

		if(t->value)
		{
			t->value = False;
			t->upcall(session,&toggle[f],TT_FINAL);

			if(session->update_toggle)
				session->update_toggle(session,ix,t->value,TT_FINAL,toggle_names[ix]);
		}
	}
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

