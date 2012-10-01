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
 * Este programa está nomeado como keyboard.c e possui - linhas de código.
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

 #include <pw3270.h>
 #include <lib3270.h>
 #include <lib3270/actions.h>
 #include <lib3270/log.h>
 #include <gtk/gtk.h>
 #include <string.h>
 #include <gdk/gdk.h>

 #include <pw3270/v3270.h>
 #include "private.h"

#if GTK_CHECK_VERSION(3,0,0)
	#include <gdk/gdkkeysyms-compat.h>
#else
	#include <gdk/gdkkeysyms.h>
#endif

#ifndef GDK_ALT_MASK
	#define GDK_ALT_MASK GDK_MOD1_MASK
#endif

#ifndef GDK_NUMLOCK_MASK
	#define GDK_NUMLOCK_MASK GDK_MOD2_MASK
#endif

/*--[ Globals ]--------------------------------------------------------------------------------------*/

	 static struct _keycode
	 {
		guint			  keyval;
		GdkModifierType	  state;
		int				  (*exec)(H3270 *session);
		GtkAction		* action;
	 } keycode[] =
	 {
		{ GDK_Left,				0,					lib3270_cursor_left,	NULL	},
		{ GDK_Up,				0,					lib3270_cursor_up,		NULL	},
		{ GDK_Right,			0,					lib3270_cursor_right,	NULL	},
		{ GDK_Down,				0,					lib3270_cursor_down,	NULL	},
		{ GDK_Tab,				0,					lib3270_nextfield,		NULL	},
		{ GDK_ISO_Left_Tab,		GDK_SHIFT_MASK,		lib3270_previousfield,	NULL	},
		{ GDK_KP_Left,			0,					lib3270_cursor_left,	NULL	},
		{ GDK_KP_Up,			0,					lib3270_cursor_up,		NULL	},
		{ GDK_KP_Right,			0,					lib3270_cursor_right,	NULL	},
		{ GDK_KP_Down,			0,					lib3270_cursor_down,	NULL	},

		{ GDK_KP_Add,			GDK_NUMLOCK_MASK,	NULL,					NULL	},
		{ GDK_KP_Subtract,		GDK_NUMLOCK_MASK,	NULL,					NULL	},

		{ GDK_3270_PrintScreen,	0,					NULL,					NULL	},
		{ GDK_Sys_Req,			0,					lib3270_sysreq,			NULL	},

		{ GDK_Print,			GDK_CONTROL_MASK,	NULL,					NULL	},
		{ GDK_Print,			GDK_SHIFT_MASK,		lib3270_sysreq,			NULL	},
		{ GDK_Control_R,		0,					NULL,					NULL	},
		{ GDK_Control_L,		0,					NULL,					NULL	},

#ifdef WIN32
		{ GDK_Pause,			0,					NULL,					NULL	},
#endif
	};

/*--[ Implement ]------------------------------------------------------------------------------------*/

 #define keyval_is_alt() (event->keyval == GDK_Alt_L || event->keyval == GDK_Meta_L || event->keyval == GDK_ISO_Level3_Shift)

 static void update_keyboard_state(v3270 *terminal, GdkEventKey *event, gboolean status)
 {
	if(event->keyval == GDK_Shift_R || event->keyval == GDK_Shift_L)
	{
		if(status)
			terminal->keyflags |= KEY_FLAG_SHIFT;
		else
			terminal->keyflags &= ~KEY_FLAG_SHIFT;
		v3270_draw_shift_status(terminal);
	}

	if(keyval_is_alt())
	{
		if(status)
			terminal->keyflags |= KEY_FLAG_ALT;
		else
			terminal->keyflags &= ~KEY_FLAG_ALT;
		v3270_draw_alt_status(terminal);
	}

 }

 static gboolean check_keypress(v3270 *widget, GdkEventKey *event)
 {
	int			f;
	int			state	= event->state & (GDK_SHIFT_MASK|GDK_CONTROL_MASK|GDK_ALT_MASK);
	gboolean	handled = FALSE;

#ifdef WIN32
	// FIXME (perry#1#): Find a better way!
	if( event->keyval == 0xffffff && event->hardware_keycode == 0x0013)
		event->keyval = GDK_Pause;

	// Windows sets <ctrl> in left/right control
	else if(state & GDK_CONTROL_MASK && (event->keyval == GDK_Control_R || event->keyval == GDK_Control_L))
		state &= ~GDK_CONTROL_MASK;
#endif

	g_signal_emit(GTK_WIDGET(widget), v3270_widget_signal[SIGNAL_KEYPRESS], 0, event->keyval, state, &handled);
	if(handled)
		return TRUE;

	if(event->keyval >= GDK_F1 && event->keyval <= GDK_F12 && !(state & (GDK_CONTROL_MASK|GDK_ALT_MASK)))
	{
		int pfcode = (event->keyval - GDK_F1) + ((state & GDK_SHIFT_MASK) ? 13 : 1);

		trace("PF%02d",pfcode);

		if(pfcode > 0 && pfcode < 25)
		{
			lib3270_pfkey(widget->host,pfcode);
			return TRUE;
		}
	}

	for(f=0; f < G_N_ELEMENTS(keycode);f++)
	{
		if(keycode[f].keyval == event->keyval && state == keycode[f].state)
		{
			trace("%s: id=%d action=%p",__FUNCTION__,f,keycode[f].action);
			if(keycode[f].action)
				gtk_action_activate(keycode[f].action);
			else if(keycode[f].exec)
				keycode[f].exec(widget->host);
			else
				return FALSE;
			return TRUE;

		}
	}

 	return FALSE;
 }

 gboolean v3270_set_keyboard_action(GtkWidget *widget, const gchar *key_name, GtkAction *action)
 {
	guint			keyval;
	GdkModifierType	state;
	int				f;

	g_return_val_if_fail(GTK_IS_V3270(widget),FALSE);

	gtk_accelerator_parse(key_name,&keyval,&state);

	for(f=0; f < G_N_ELEMENTS(keycode);f++)
	{
		if(keycode[f].keyval == keyval && keycode[f].state == state)
		{
			keycode[f].action = action;
			return TRUE;
		}
	}

	return FALSE;
 }

 gboolean v3270_key_press_event(GtkWidget *widget, GdkEventKey *event)
 {
	v3270 * terminal = GTK_V3270(widget);

 	update_keyboard_state(terminal,event,TRUE);

	if(gtk_im_context_filter_keypress(terminal->input_method,event))
		return TRUE;

	if(check_keypress(terminal,event))
	{
		gtk_im_context_reset(terminal->input_method);
		return TRUE;
	}

	return FALSE;

 }

 gboolean v3270_key_release_event(GtkWidget *widget, GdkEventKey *event)
 {
	v3270 * terminal = GTK_V3270(widget);

 	update_keyboard_state(terminal,event,FALSE);

	if(gtk_im_context_filter_keypress(terminal->input_method,event))
		return TRUE;

	return FALSE;

 }

 void v3270_tab(GtkWidget *widget)
 {
	g_return_if_fail(GTK_IS_V3270(widget));
	lib3270_nextfield(GTK_V3270(widget)->host);
 }

 void v3270_backtab(GtkWidget *widget)
 {
	g_return_if_fail(GTK_IS_V3270(widget));
	lib3270_previousfield(GTK_V3270(widget)->host);
 }

 void v3270_set_string(GtkWidget *widget, const gchar *str)
 {
 	H3270 *host;
	gchar *utf;

	g_return_if_fail(GTK_IS_V3270(widget));

	host = GTK_V3270(widget)->host;

	utf = g_convert((char *) str, -1, lib3270_get_charset(host), "UTF-8", NULL, NULL, NULL);

	if(utf)
	{
		lib3270_set_string(host, (const unsigned char *) utf);
		g_free(utf);
	}

 }

 void v3270_key_commit(GtkIMContext *imcontext, gchar *str, v3270 *widget)
 {
	gchar *utf = g_convert((char *) str, -1, lib3270_get_charset(widget->host), "UTF-8", NULL, NULL, NULL);

	if(utf)
	{
		lib3270_input_string(GTK_V3270(widget)->host, (const unsigned char *) utf);
//		lib3270_set_string(GTK_V3270(widget)->host, (const unsigned char *) utf);
		g_free(utf);
	}
	else
	{
		lib3270_ring_bell(widget->host);
	}
 }


