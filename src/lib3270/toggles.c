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
// #include "menubarc.h"
#include "popupsc.h"
#include "screenc.h"
#include "trace_dsc.h"
#include "togglesc.h"
#include "api.h"



static void no_callback(H3270 *h, int value, LIB3270_TOGGLE_TYPE reason)
{
}

LIB3270_EXPORT int lib3270_register_tchange(H3270 *h, LIB3270_TOGGLE_ID ix, void (*func)(H3270 *, int, LIB3270_TOGGLE_TYPE))
{
	struct toggle *t;

	CHECK_SESSION_HANDLE(h);

	if(ix < 0 || ix >= LIB3270_TOGGLE_COUNT)
		return EINVAL;

	t = &appres.toggle[ix];

	if(func)
	{
		t->callback = func;
		t->callback(h, t->value, (int) TT_INITIAL);
	}
	else
	{
		t->callback = no_callback;
	}

	return 0;
}

LIB3270_EXPORT unsigned char lib3270_get_toggle(H3270 *session, LIB3270_TOGGLE ix)
{
	CHECK_SESSION_HANDLE(session);

	if(ix < 0 || ix >= N_TOGGLES)
		return 0;
	return (unsigned char) appres.toggle[ix].value != 0;
}

/*
 * Generic toggle stuff
 */
static void do_toggle_reason(H3270 *session, LIB3270_TOGGLE ix, LIB3270_TOGGLE_TYPE reason)
{
	struct toggle *t = &appres.toggle[ix];

	/*
	 * Change the value, call the internal update routine, and reset the
	 * menu label(s).
	 */
	toggle_toggle(t);
	t->upcall(session, t, reason);
	t->callback(session,t->value, (int) reason);

	if(session->update_toggle)
		session->update_toggle(session,ix,t->value,reason,toggle_names[ix]);

//	notify_toggle_changed(session, ix, t->value, reason);

}

LIB3270_EXPORT int lib3270_set_toggle(H3270 *session, LIB3270_TOGGLE ix, int value)
{
	Boolean v = ((Boolean) (value != 0)); // Convert int in Boolean

	struct toggle	*t;

	CHECK_SESSION_HANDLE(session);

	if(ix < 0 || ix >= LIB3270_TOGGLE_COUNT)
		return 0;

	t = &appres.toggle[ix];

	if(t->value == v)
		return 0;

	do_toggle_reason(session, ix, TT_INTERACTIVE);

	return -1;
}

LIB3270_EXPORT int lib3270_toggle(H3270 *session, LIB3270_TOGGLE ix)
{
	CHECK_SESSION_HANDLE(session);

	if(ix < 0 || ix >= LIB3270_TOGGLE_COUNT)
		return EINVAL;

	do_toggle_reason(session, ix, TT_INTERACTIVE);

	return 0;
}

/*
 * Called from system initialization code to handle initial toggle settings.
 */
void initialize_toggles(H3270 *session, struct toggle *toggle)
{
	int f;

	for(f=0;f<N_TOGGLES;f++)
	{
		toggle[f].callback	= no_callback;
		toggle[f].upcall	= toggle_nop;
	}

#if defined(X3270_TRACE)
	toggle[DS_TRACE].upcall			= toggle_dsTrace;
	toggle[SCREEN_TRACE].upcall		= toggle_screenTrace;
	toggle[EVENT_TRACE].upcall		= toggle_eventTrace;
#endif

#if defined(X3270_ANSI)
	toggle[LINE_WRAP].upcall		= toggle_lineWrap;
#endif

#if defined(X3270_TRACE)
	if(toggle[DS_TRACE].value)
		toggle[DS_TRACE].upcall(session, &toggle[DS_TRACE],TT_INITIAL);

	if(toggle[EVENT_TRACE].value)
		toggle[EVENT_TRACE].upcall(session, &toggle[EVENT_TRACE],TT_INITIAL);

	if(toggle[SCREEN_TRACE].value)
		toggle[SCREEN_TRACE].upcall(session, &toggle[SCREEN_TRACE],TT_INITIAL);
#endif

#if defined(DEFAULT_TOGGLE_CURSOR_POS)
	toggle[CURSOR_POS].value = True;
#endif /*]*/

#if defined(DEFAULT_TOGGLE_RECTANGLE_SELECT)
	toggle[RECTANGLE_SELECT].value = True;
#endif

}

/*
 * Called from system exit code to handle toggles.
 */
void shutdown_toggles(H3270 *session, struct toggle *toggle)
{
#if defined(X3270_TRACE)
	// Clean up the data stream trace monitor window.
	if(toggle[DS_TRACE].value)
	{
		toggle[DS_TRACE].value = False;
		toggle_dsTrace(session, &toggle[DS_TRACE], TT_FINAL);
	}

	if(toggle[EVENT_TRACE].value)
	{
		toggle[EVENT_TRACE].value = False;
		toggle_dsTrace(session, &toggle[EVENT_TRACE], TT_FINAL);
	}

	// Clean up the screen trace file.
	if (toggle[SCREEN_TRACE].value)
	{
		toggle[SCREEN_TRACE].value = False;
		toggle_screenTrace(session, &toggle[SCREEN_TRACE], TT_FINAL);
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
	int f;

	for(f=0;f<N_TOGGLES;f++)
	{
		if(!strcasecmp(name,toggle_names[f]))
			return f;
	}

	return -1;
}

