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
 #include <lib3270/trace.h>


/*--[ Globals ]--------------------------------------------------------------------------------------*/

 static GtkAction *action_scroll[] = { NULL, NULL, NULL, NULL };

/*--[ Implement ]------------------------------------------------------------------------------------*/

gint v3270_get_offset_at_point(v3270 *widget, gint x, gint y)
{
	GdkPoint point;
	int r,c;

	g_return_val_if_fail(widget->font.width > 0,-1);

	if(x > 0 && y > 0)
	{
		point.x = ((x-widget->font.left)/widget->font.width);
		point.y = ((y-widget->font.top)/widget->font.spacing);

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
		v3270_disable_updates(GTK_WIDGET(widget));
		lib3270_set_cursor_address(widget->host,baddr);
		lib3270_unselect(widget->host);
		widget->selecting = 1;
		v3270_enable_updates(GTK_WIDGET(widget));
		break;


	default:
		// Move selected area
		trace("%s: default action",__FUNCTION__);
		widget->selection.baddr = baddr;
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
		if(lib3270_select_word_at(GTK_V3270(widget)->host,baddr))
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

static V3270_OIA_FIELD get_field_from_event(v3270 *widget, GdkEventButton *event)
{
	if(event->y >= widget->oia_rect->y)
	{
		V3270_OIA_FIELD f;

		for(f=0;f<V3270_OIA_FIELD_COUNT;f++)
		{
			if(event->x >= widget->oia_rect[f].x && event->x <= (widget->oia_rect[f].x+widget->oia_rect[f].width))
				return f;
		}
	}

	return V3270_OIA_FIELD_INVALID;
}

gboolean v3270_button_press_event(GtkWidget *widget, GdkEventButton *event)
{
	int baddr = v3270_get_offset_at_point(GTK_V3270(widget),event->x,event->y);

	if(baddr >= 0)
	{
		GTK_V3270(widget)->selected_field = V3270_OIA_FIELD_INVALID;

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
	}
	else if(event->button == 1 && event->type == GDK_BUTTON_PRESS)
	{
		GTK_V3270(widget)->selected_field = get_field_from_event(GTK_V3270(widget),event);
		trace("%s field=%d",__FUNCTION__,GTK_V3270(widget)->selected_field);
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

		if(GTK_V3270(widget)->selected_field != V3270_OIA_FIELD_INVALID && GTK_V3270(widget)->selected_field == get_field_from_event(GTK_V3270(widget),event))
		{
			gboolean handled = FALSE;

			trace("%s field %d was clicked event=%p",__FUNCTION__,GTK_V3270(widget)->selected_field,event);

			g_signal_emit(widget,	v3270_widget_signal[SIGNAL_FIELD], 0,
									lib3270_connected(GTK_V3270(widget)->host) ? TRUE : FALSE,
									GTK_V3270(widget)->selected_field,
									event,
									&handled);

			trace("Handled: %s",handled ? "Yes": "No");

			if(!handled)
				gdk_beep();

		}

		GTK_V3270(widget)->selected_field = V3270_OIA_FIELD_INVALID;

		break;

	default:
		trace("%s button=%d type=%d",__FUNCTION__,event->button,event->type);
	}

	return FALSE;
}

static void update_mouse_pointer(GtkWidget *widget, int baddr)
{
	v3270	* terminal	= GTK_V3270(widget);
//	int		  id		= 0;

	if(baddr >= 0 && terminal->pointer_id == LIB3270_POINTER_UNLOCKED)
	{
		gdk_window_set_cursor(gtk_widget_get_window(widget),v3270_cursor[lib3270_get_pointer(terminal->host,baddr)]);
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
	v3270		* terminal	= GTK_V3270(widget);
	int			  baddr;

	if(!lib3270_connected(terminal->host))
	{
		gdk_window_set_cursor(gtk_widget_get_window(widget),v3270_cursor[LIB3270_POINTER_LOCKED]);
		return FALSE;
	}

	baddr = v3270_get_offset_at_point(terminal,event->x,event->y);

	if(baddr >= 0)
	{

		if(terminal->selecting)		// Select region
		{
			lib3270_select_to(terminal->host,baddr);
		}
		if(terminal->moving) 	// Move selected area
		{
			terminal->selection.baddr = lib3270_drag_selection(terminal->host,terminal->pointer,terminal->selection.baddr,baddr);
		}
		else
		{
			terminal->pointer = lib3270_get_selection_flags(terminal->host,baddr);
			update_mouse_pointer(widget,baddr);
		}
	}
	else if(event->y >= terminal->oia_rect->y)
	{
		int id = LIB3270_POINTER_PROTECTED;

		if(event->x >= terminal->oia_rect[V3270_OIA_SSL].x && event->x <= (terminal->oia_rect[V3270_OIA_SSL].x + terminal->oia_rect[V3270_OIA_SSL].width))
		{
			switch(lib3270_get_secure(terminal->host))
			{
			case LIB3270_SSL_UNSECURE:	/**< No secure connection */
				id = LIB3270_POINTER_QUESTION;
				break;

			case LIB3270_SSL_NEGOTIATING:	/**< Negotiating SSL */
				id = LIB3270_POINTER_WAITING;
				break;

			case LIB3270_SSL_NEGOTIATED:	/**< Connection secure, no CA or self-signed */
				id = LIB3270_POINTER_QUESTION;
				break;

			case LIB3270_SSL_SECURE:	/**< Connection secure with CA check */
				id = LIB3270_POINTER_QUESTION;
				break;

			default:
				id = LIB3270_POINTER_LOCKED;
			}
		}

		gdk_window_set_cursor(gtk_widget_get_window(widget),v3270_cursor[id]);
	}

	return FALSE;
}

void v3270_set_scroll_action(GtkWidget *widget, GdkScrollDirection direction, GtkAction *action)
{
 	g_return_if_fail(GTK_IS_V3270(widget));
	action_scroll[((int) direction) & 0x03] = action;
}

gboolean v3270_scroll_event(GtkWidget *widget, GdkEventScroll *event)
{
	H3270	* hSession	= v3270_get_session(widget);

	lib3270_trace_event(hSession,"scroll event direction=%d",(int) event->direction);

	if(lib3270_get_program_message(hSession) != LIB3270_MESSAGE_NONE || event->direction < 0 || event->direction > G_N_ELEMENTS(action_scroll))
	{
		lib3270_trace_event(hSession,"  dropped (not available)\n");
		return FALSE;
	}

	lib3270_trace_event(hSession,"\n");

	trace("Scroll: %d Action: %p",event->direction,action_scroll[event->direction]);

	if(action_scroll[event->direction])
		gtk_action_activate(action_scroll[event->direction]);

	return TRUE;
 }
