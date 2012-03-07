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
 * Este programa está nomeado como actions.c e possui 877 linhas de código.
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
 *	actions.c
 *		The X actions table and action debugging code.
 */

#include "globals.h"
#include "appres.h"

#include "actionsc.h"
#include "hostc.h"
#include "kybdc.h"
#include "popupsc.h"
#include "printc.h"
#include "resources.h"
#include "togglesc.h"
#include "trace_dsc.h"
#include "utilc.h"
#include "xioc.h"

#if defined(X3270_FT) /*[*/
#include "ftc.h"
#endif /*]*/
#if defined(X3270_DISPLAY) /*[*/
#include "keypadc.h"
#endif /*]*/
#if defined(X3270_DISPLAY) || defined(C3270) || defined(WC3270) /*[*/
#include "screenc.h"
#endif /*]*/

/*

#if defined(X3270_DISPLAY)
#include <X11/keysym.h>

#define MODMAP_SIZE	8
#define MAP_SIZE	13
#define MAX_MODS_PER	4
static struct {
        const char *name[MAX_MODS_PER];
        unsigned int mask;
	Boolean is_meta;
} skeymask[MAP_SIZE] = {
	{ { "Shift" }, ShiftMask, False },
	{ { (char *)NULL } //, LockMask, False },
	{ { "Ctrl" }, ControlMask, False },
	{ { CN }, Mod1Mask, False },
	{ { CN }, Mod2Mask, False },
	{ { CN }, Mod3Mask, False },
	{ { CN }, Mod4Mask, False },
	{ { CN }, Mod5Mask, False },
	{ { "Button1" }, Button1Mask, False },
	{ { "Button2" }, Button2Mask, False },
	{ { "Button3" }, Button3Mask, False },
	{ { "Button4" }, Button4Mask, False },
	{ { "Button5" }, Button5Mask, False }
};
static Boolean know_mods = False;
#endif */

/* Actions that are aliases for other actions. */
/*
static char *aliased_actions[] = {
	"Close", "HardPrint", "Open", NULL
};
*/
enum iaction ia_cause;
const char *ia_name[] = {
	"String", "Paste", "Screen redraw", "Keypad", "Default", "Key",
	"Macro", "Script", "Peek", "Typeahead", "File transfer", "Command",
	"Keymap", "Idle"
};


/*
 * Return a name for an action.
 */ /*
const char * action_name(XtActionProc action)
{
	// TODO (perry#1#): Remove all calls to action_name; move all action processing to main program.
	return "Action";
}
*/
/*
 * Check the number of argument to an action, and possibly pop up a usage
 * message.
 *
 * Returns 0 if the argument count is correct, -1 otherwise.
 */ /*
int
check_usage(XtActionProc action, Cardinal nargs, Cardinal nargs_min,
    Cardinal nargs_max)
{
	if (nargs >= nargs_min && nargs <= nargs_max)
		return 0;
	if (nargs_min == nargs_max)
		popup_an_error("Action requires %d argument%s",action, nargs_min, nargs_min == 1 ? "" : "s");
	else
		popup_an_error("Action requires %d or %d arguments",nargs_min, nargs_max);
//	cancel_if_idle_command();
	return -1;
} */

/*
 * Wrapper for calling an action internally.
 */
void
action_internal(XtActionProc action, enum iaction cause, const char *parm1,
    const char *parm2)
{
	Cardinal count = 0;
	String parms[2];

	/* Duplicate the parms, because XtActionProc doesn't grok 'const'. */
	if (parm1 != CN) {
		parms[0] = NewString(parm1);
		count++;
		if (parm2 != CN) {
			parms[1] = NewString(parm2);
			count++;
		}
	}

	ia_cause = cause;
	(*action)((Widget) NULL, (XEvent *) NULL,
	    count ? parms : (String *) NULL,
	    &count);

	/* Free the parm copies. */
	switch (count) {
	    case 2:
		Free(parms[1]);
		/* fall through... */
	    case 1:
		Free(parms[0]);
		break;
	    default:
		break;
	}
}


