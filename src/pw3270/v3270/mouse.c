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
 * Este programa está nomeado como mouse.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <gtk/gtk.h>
 #include <gdk/gdk.h>
 #include <pw3270.h>
 #include <v3270.h>
 #include "private.h"
 #include <lib3270/selection.h>
 #include <lib3270/actions.h>
 #include <lib3270/log.h>

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 static GtkAction *action_scroll[] = { NULL, NULL, NULL, NULL };

/*--[ Implement ]------------------------------------------------------------------------------------*/

gint v3270_get_offset_at_point(v3270 *widget, gint x, gint y)
{
	GdkPoint point;
	int r,c;

	if(x > 0 && y > 0)
	{
		point.x = ((x-widget->metrics.left)/widget->metrics.width);
		point.y = ((y-widget->metrics.top)/widget->metrics.spacing);

		lib3270_get_screen_size(widget->host,&r,&c);

		if(point.x >= 0 && point.y >= 0 && point.x < c && point.y < r)
			return (point.y * c) + point.x;
	}

	return -1;
}

static void single_click(v3270 *widget, int baddr)
{
	switch(lib3270_get_selection_flags(widget->host,baddr))
	{
	case 0x00:
		// Unselected area, move cursor and remove selection
		lib3270_set_cursor_address(widget->host,baddr);
		lib3270_unselect(widget->host);
		widget->selecting = 1;
		break;


	default:
		// Move selected area
		trace("%s: default action",__FUNCTION__);
		widget->selection_addr = baddr;
		widget->moving = 1;
	}


}

static void button_1_press(GtkWidget *widget, GdkEventType type, int baddr)
{
	switch(type)
	{
	case GDK_BUTTON_PRESS: 		// Single click - set mode
		single_click(GTK_V3270(widget),baddr);
		break;

	case GDK_2BUTTON_PRESS:		// Double click - Select word
		if(lib3270_select_word_at(GTK_V3270(widget)->host,baddr));
			lib3270_ring_bell(GTK_V3270(widget)->host);
		break;

	case GDK_3BUTTON_PRESS:		// Triple clock - Select field
		if(lib3270_select_field_at(GTK_V3270(widget)->host,baddr))
			lib3270_ring_bell(GTK_V3270(widget)->host);
		break;

#ifdef DEBUG
	default:
		trace("Unexpected button 1 type %d",type);
#endif
	}
}

void v3270_emit_popup(v3270 *widget, int baddr, GdkEventButton *event)
{
	unsigned char	  chr = 0;
	unsigned short	  attr;
	gboolean		  handled = FALSE;

	lib3270_get_contents(widget->host,baddr,baddr,&chr,&attr);

	g_signal_emit(GTK_WIDGET(widget), v3270_widget_signal[SIGNAL_POPUP], 0,
									(attr & LIB3270_ATTR_SELECTED) ? TRUE : FALSE,
									lib3270_connected(widget->host) ? TRUE : FALSE,
									event,
									&handled);

	if(handled)
		return;

	gdk_beep();
}

gboolean v3270_button_press_event(GtkWidget *widget, GdkEventButton *event)
{
	int baddr = v3270_get_offset_at_point(GTK_V3270(widget),event->x,event->y);

	if(baddr < 0)
		return FALSE;

//	trace("%s button=%d type=%d",__FUNCTION__,event->button,event->type);

	switch(event->button)
	{
	case 1:		// Left button
		button_1_press(widget,event->type,baddr);
		break;

	case 3:		// Right button
		if(event->type == GDK_BUTTON_PRESS)
			v3270_emit_popup(GTK_V3270(widget),baddr,event);
		break;

	default:
		trace("%s button=%d type=%d",__FUNCTION__,event->button,event->type);
	}

	return FALSE;
}

gboolean v3270_button_release_event(GtkWidget *widget, GdkEventButton*event)
{
	switch(event->button)
	{
	case 1:
		GTK_V3270(widget)->selecting = 0;
		GTK_V3270(widget)->moving	 = 0;
		GTK_V3270(widget)->resizing	 = 0;
		break;

	default:
		trace("%s button=%d type=%d",__FUNCTION__,event->button,event->type);
	}


	return FALSE;
}


static void update_mouse_pointer(GtkWidget *widget, int baddr)
{
	v3270 *terminal = GTK_V3270(widget);

	if(baddr >= 0 && terminal->pointer_id == LIB3270_CURSOR_EDITABLE)
	{
		int id = terminal->pointer;

		switch(lib3270_get_selection_flags(terminal->host,baddr) & 0x8f)
		{
		case 0x80:
			id = V3270_CURSOR_MOVE_SELECTION;
			break;

		case 0x82:
			id = V3270_CURSOR_SELECTION_TOP;
			break;

		case 0x86:
			id = V3270_CURSOR_SELECTION_TOP_RIGHT;
			break;

		case 0x84:
			id = V3270_CURSOR_SELECTION_RIGHT;
			break;

		case 0x81:
			id = V3270_CURSOR_SELECTION_LEFT;
			break;

		case 0x89:
			id = V3270_CURSOR_SELECTION_BOTTOM_LEFT;
			break;

		case 0x88:
			id = V3270_CURSOR_SELECTION_BOTTOM;
			break;

		case 0x8c:
			id = V3270_CURSOR_SELECTION_BOTTOM_RIGHT;
			break;

		case 0x83:
			id = V3270_CURSOR_SELECTION_TOP_LEFT;
			break;

		default:
			id = lib3270_is_protected(terminal->host,baddr) ? V3270_CURSOR_PROTECTED : V3270_CURSOR_UNPROTECTED;

		}

		gdk_window_set_cursor(gtk_widget_get_window(widget),v3270_cursor[id]);
	}

}

void v3270_update_mouse_pointer(GtkWidget *widget)
{
	gint	  x, y;

#if GTK_CHECK_VERSION(3,4,0)
	#warning Implement gdk_window_get_device_position
#endif // GTK(3,4,0)

	gtk_widget_get_pointer(widget,&x,&y);
	update_mouse_pointer(widget,v3270_get_offset_at_point(GTK_V3270(widget),x,y));
}

gboolean v3270_motion_notify_event(GtkWidget *widget, GdkEventMotion *event)
{
	v3270	* terminal	= GTK_V3270(widget);
	int		  baddr		= v3270_get_offset_at_point(terminal,event->x,event->y);

	if(baddr >= 0)
	{
		if(terminal->selecting)		// Select region
		{
			lib3270_select_to(terminal->host,baddr);
		}
		if(terminal->moving) 	// Move selected area
		{
			terminal->selection_addr = lib3270_drag_selection(terminal->host,terminal->pointer,terminal->selection_addr,baddr);
		}
		else
		{
			terminal->pointer = lib3270_get_selection_flags(terminal->host,baddr);
			update_mouse_pointer(widget,baddr);
		}
	}

	return FALSE;
}

void v3270_set_scroll_action(GtkWidget *widget, GdkScrollDirection direction, GtkAction *action)
{
 	g_return_if_fail(GTK_IS_V3270(widget));
	action_scroll[((int) direction) & 0x03] = action;
}

gboolean	  v3270_scroll_event(GtkWidget *widget, GdkEventScroll *event, gpointer user_data)
{
	v3270	* terminal	= GTK_V3270(widget);

	if(lib3270_get_program_message(terminal->host) != LIB3270_MESSAGE_NONE || event->direction < 0 || event->direction > G_N_ELEMENTS(action_scroll))
		return FALSE;

	trace("Scroll: %d Action: %p",event->direction,action_scroll[event->direction]);

	if(action_scroll[event->direction])
		gtk_action_activate(action_scroll[event->direction]);

	return TRUE;
 }
