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
 #include "v3270.h"
 #include "private.h"
 #include <lib3270/selection.h>
 #include <lib3270/actions.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

static int decode_position(v3270 *widget, gdouble x, gdouble y)
{
	GdkPoint point;
	int r,c;

	point.x = ((x-widget->metrics.left)/widget->metrics.width);
	point.y = ((y-widget->metrics.top)/widget->metrics.spacing);

	lib3270_get_screen_size(widget->host,&r,&c);

	if(point.x >= 0 && point.y >= 0 && point.x < c && point.y < r)
		return (point.y * c) + point.x;

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
		lib3270_select_word(GTK_V3270(widget)->host,baddr);
		break;

	case GDK_3BUTTON_PRESS:		// Triple clock - Select field
		lib3270_select_field_at(GTK_V3270(widget)->host,baddr);
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
	int baddr = decode_position(GTK_V3270(widget),event->x,event->y);

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


gboolean v3270_motion_notify_event(GtkWidget *widget, GdkEventMotion *event)
{
	v3270	* terminal = GTK_V3270(widget);
	int		  baddr = decode_position(terminal,event->x,event->y);

	if(baddr < 0)
		return FALSE;

	if(terminal->selecting)
	{
		// Select area
		lib3270_select_to(terminal->host,baddr);
	}
	else if(terminal->moving)
	{
		// Move selected area
		terminal->selection_addr = lib3270_drag_selection(terminal->host,terminal->pointer,terminal->selection_addr,baddr);
	}
	else if(terminal->pointer_id == LIB3270_CURSOR_NORMAL)
	{
		unsigned char new_pointer = lib3270_get_selection_flags(terminal->host,baddr);
		if(new_pointer != terminal->pointer)
		{
			GdkWindow *window = gtk_widget_get_window(widget);
			trace("Pointer changes to %04x",new_pointer);

			switch(new_pointer & 0x1F)
			{
			case 0x10:
				gdk_window_set_cursor(window,v3270_cursor[V3270_CURSOR_MOVE_SELECTION]);
				break;

			case 0x12:
				gdk_window_set_cursor(window,v3270_cursor[V3270_CURSOR_SELECTION_TOP]);
				break;

			case 0x16:
				gdk_window_set_cursor(window,v3270_cursor[V3270_CURSOR_SELECTION_TOP_RIGHT]);
				break;

			case 0x14:
				gdk_window_set_cursor(window,v3270_cursor[V3270_CURSOR_SELECTION_RIGHT]);
				break;

			case 0x11:
				gdk_window_set_cursor(window,v3270_cursor[V3270_CURSOR_SELECTION_LEFT]);
				break;

			case 0x19:
				gdk_window_set_cursor(window,v3270_cursor[V3270_CURSOR_SELECTION_BOTTOM_LEFT]);
				break;

			case 0x18:
				gdk_window_set_cursor(window,v3270_cursor[V3270_CURSOR_SELECTION_BOTTOM]);
				break;

			case 0x1c:
				gdk_window_set_cursor(window,v3270_cursor[V3270_CURSOR_SELECTION_BOTTOM_RIGHT]);
				break;

			case 0x13:
				gdk_window_set_cursor(window,v3270_cursor[V3270_CURSOR_SELECTION_TOP_LEFT]);
				break;

			default:
				gdk_window_set_cursor(window,v3270_cursor[V3270_CURSOR_NORMAL]);

			}

			terminal->pointer = new_pointer;
		}
	}

	return FALSE;
}
