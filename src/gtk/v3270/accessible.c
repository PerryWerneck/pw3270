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
 * Este programa está nomeado como accessible.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <lib3270/config.h>

 #define ENABLE_NLS
 #define GETTEXT_PACKAGE PACKAGE_NAME

 #include <gtk/gtk.h>
 #include <libintl.h>
 #include <glib/gi18n.h>

 #include <pw3270.h>
 #include <malloc.h>
 #include "v3270.h"
 #include "private.h"
 #include "accessible.h"

// References:
//
//	http://git.gnome.org/browse/gtk+/tree/gtk/a11y/gtkwidgetaccessible.c
//	http://git.gnome.org/browse/gtk+/tree/gtk/a11y/gtkentryaccessible.c
//


/*--[ Prototipes ]-----------------------------------------------------------------------------------*/

static void atk_component_interface_init 		(AtkComponentIface		*iface);
static void atk_action_interface_init        	(AtkActionIface       *iface);
static void v3270_accessible_class_init			(v3270AccessibleClass	*klass);
static void v3270_accessible_init				(v3270Accessible		*widget);

static void atk_text_interface_init				(AtkTextIface			*iface);

/*--[ Widget definition ]----------------------------------------------------------------------------*/

G_DEFINE_TYPE_WITH_CODE (v3270Accessible, v3270_accessible, GTK_TYPE_ACCESSIBLE,
							G_IMPLEMENT_INTERFACE (ATK_TYPE_COMPONENT, atk_component_interface_init)
							G_IMPLEMENT_INTERFACE (ATK_TYPE_ACTION, atk_action_interface_init)
							G_IMPLEMENT_INTERFACE (ATK_TYPE_TEXT, atk_text_interface_init)
						)

//							G_IMPLEMENT_INTERFACE (ATK_TYPE_EDITABLE_TEXT, atk_editable_text_interface_init) )
//                         G_IMPLEMENT_INTERFACE (ATK_TYPE_ACTION, atk_action_interface_init)
//                         G_IMPLEMENT_INTERFACE (ATK_TYPE_EDITABLE_TEXT, atk_editable_text_interface_init)
//                         G_IMPLEMENT_INTERFACE (ATK_TYPE_TEXT, atk_text_interface_init)

/*--[ Implement ]------------------------------------------------------------------------------------*/

static const gchar * v3270_accessible_get_description (AtkObject *accessible)
{
	GtkWidget *widget = gtk_accessible_get_widget(GTK_ACCESSIBLE (accessible));

	if (widget == NULL)
		return NULL;

	return _( "3270 screen" );
}

static void v3270_accessible_class_init(v3270AccessibleClass *klass)
{
	AtkObjectClass *class = ATK_OBJECT_CLASS (klass);

	class->get_description = v3270_accessible_get_description;

/*
	class->focus_event = gtk_widget_accessible_focus_event;

	klass->notify_gtk = gtk_widget_accessible_notify_gtk;

	class->get_parent = gtk_widget_accessible_get_parent;
	class->ref_relation_set = gtk_widget_accessible_ref_relation_set;
	class->ref_state_set = gtk_widget_accessible_ref_state_set;
	class->get_index_in_parent = gtk_widget_accessible_get_index_in_parent;
	class->initialize = gtk_widget_accessible_initialize;
	class->get_attributes = gtk_widget_accessible_get_attributes;
	class->focus_event = gtk_widget_accessible_focus_event;
*/
}

static gint v3270_accessible_get_n_actions(AtkAction *action)
{
  return 1;
}

static const gchar* v3270_accessible_action_get_name (AtkAction *action, gint i)
{
  if (i != 0)
    return NULL;

  return "activate";
}

static gboolean v3270_accessible_do_action(AtkAction *action, gint i)
{
	GtkWidget *widget = gtk_accessible_get_widget (GTK_ACCESSIBLE (action));

	if(widget == NULL)
		return FALSE;

	if(!gtk_widget_get_sensitive (widget) || !gtk_widget_get_visible (widget))
		return FALSE;

	if (i != 0)
		return FALSE;

	gtk_widget_activate(widget);

	return TRUE;
}

static void atk_action_interface_init(AtkActionIface *iface)
{
	iface->get_name			= v3270_accessible_action_get_name;
	iface->get_n_actions	= v3270_accessible_get_n_actions;
	iface->do_action		= v3270_accessible_do_action;

/*
	iface->get_keybinding = gtk_entry_accessible_get_keybinding;
*/
}

static gunichar v3270_accessible_get_character_at_offset(AtkText *atk_text, gint offset)
{
	GtkWidget * widget = gtk_accessible_get_widget(GTK_ACCESSIBLE (atk_text));

	if(widget == NULL)
	{
		H3270	* host = v3270_get_session(widget);
		gchar	* text = lib3270_get_text(host,offset,1);

		if(text)
		{
			gunichar	  unichar;
			gsize		  bytes_written;
			GError		* error		= NULL;
			gchar		* utfstring	= g_convert_with_fallback(	text,
																-1,
																"UTF-8",
																lib3270_get_charset(host),
																" ",
																NULL,
																&bytes_written,
																&error );

			if(error)
			{
				g_warning("%s failed: %s",__FUNCTION__,error->message);
				g_error_free(error);
			}
			unichar = *utfstring;

			g_free(utfstring);

			return unichar;
		}

	}

	return '\0';
}

static gint v3270_accessible_get_caret_offset(AtkText *text)
{
	GtkWidget *widget = gtk_accessible_get_widget(GTK_ACCESSIBLE(text));

	if (widget == NULL)
		return 0;

	return lib3270_get_cursor_address(GTK_V3270(widget)->host);
}

static gint v3270_accessible_get_character_count(AtkText *text)
{
	int rows,cols;
	GtkWidget *widget = gtk_accessible_get_widget(GTK_ACCESSIBLE(text));

	if(!widget)
		return 0;

	return lib3270_get_length(GTK_V3270(widget)->host);
}

static gint v3270_accessible_get_offset_at_point(AtkText *atk_text, gint x, gint y, AtkCoordType  coords)
{
	gint 		x_window,
				y_window,
				x_widget,
				y_widget;
	GdkWindow * window;
	GtkWidget * widget = gtk_accessible_get_widget(GTK_ACCESSIBLE(atk_text));

	if(!widget)
		return -1;

	window = gtk_widget_get_window(widget);
	gdk_window_get_origin(window, &x_widget, &y_widget);

	switch(coords)
	{
	case ATK_XY_SCREEN:
		x -= x_widget;
		y -= y_widget;
		break;

	case ATK_XY_WINDOW:
		window = gdk_window_get_toplevel(window);
		gdk_window_get_origin (window, &x_window, &y_window);
		x = x - x_widget + x_window;
		y = y - y_widget + y_window;
		break;

	default:
		return -1;

	}

	return v3270_get_offset_at_point(GTK_V3270(widget),x,y);
}

static void v3270_accessible_get_character_extents(	AtkText      *text,
													gint          offset,
													gint         *x,
													gint         *y,
													gint         *width,
													gint         *height,
													AtkCoordType  coords )
{
	v3270		* widget = (v3270 *) gtk_accessible_get_widget(GTK_ACCESSIBLE (text));
	int 		  rows, cols;
	GdkWindow	* window;
	gint 		  x_window, y_window;

	if (widget == NULL)
		return;

	lib3270_get_screen_size(widget->host,&rows,&cols);

	// Get screen position
	window = gtk_widget_get_window(GTK_WIDGET(widget));
	gdk_window_get_origin(window, &x_window, &y_window);

	// Get screen position
	*x          = x_window + widget->metrics.left + ((offset/cols) * widget->metrics.width);
	*y          = y_window + widget->metrics.top  + ((offset%cols) * widget->metrics.spacing);
	*width      = widget->metrics.width;
	*height     = widget->metrics.height+widget->metrics.descent;

	if(coords == ATK_XY_WINDOW)
	{
		// Correct position based on toplevel
		window = gdk_window_get_toplevel(window);
		gdk_window_get_origin(window, &x_window, &y_window);
		*x -= x_window;
		*y -= y_window;
	}

	trace("%s: offset=%d x=%d y=%d",__FUNCTION__,offset,x,y);

}

static gchar * v3270_accessible_get_text_at_offset(AtkText *atk_text, gint offset, AtkTextBoundary boundary_type, gint *start_offset, gint *end_offset)
{
	GtkWidget	* widget = gtk_accessible_get_widget(GTK_ACCESSIBLE (atk_text));
	H3270		* host;
	char		* text;
	int			  rows,cols,pos;

	if(!widget)
		return NULL;

	host = GTK_V3270(widget)->host;
	lib3270_get_screen_size(host,&rows,&cols);

	switch(boundary_type)
	{
		case ATK_TEXT_BOUNDARY_CHAR:			// Boundary is the boundary between characters
												// (including non-printing characters)

			text = lib3270_get_text(host,offset,1);
			break;

		case ATK_TEXT_BOUNDARY_WORD_START:		// Boundary is the start (i.e. first character) of a word.
			return g_strdup("ATK_TEXT_BOUNDARY_WORD_START");
			break;

		case ATK_TEXT_BOUNDARY_WORD_END:		// Boundary is the end (i.e. last character) of a word.
			return g_strdup("ATK_TEXT_BOUNDARY_WORD_END");
			break;

		case ATK_TEXT_BOUNDARY_SENTENCE_START:	// Boundary is the first character in a sentence.
			return g_strdup("ATK_TEXT_BOUNDARY_SENTENCE_START");
			break;

		case ATK_TEXT_BOUNDARY_SENTENCE_END:	// Boundary is the last (terminal) character in
												// a sentence; in languages which use "sentence stop" punctuation such as English, the boundary is thus the '.', '?', or
												// similar terminal punctuation character.
			return g_strdup("ATK_TEXT_BOUNDARY_SENTENCE_END");
			break;


		case ATK_TEXT_BOUNDARY_LINE_START:		// Boundary is the initial character of the content or a character immediately following a newline,
												// linefeed, or return character.
			pos = (offset/cols)*cols;
			if(pos == offset)
				offset++;
			text = lib3270_get_text(host,pos,(offset-pos));
			break;


		case ATK_TEXT_BOUNDARY_LINE_END:		// Boundary is the linefeed, or return character.
			return g_strdup("ATK_TEXT_BOUNDARY_LINE_END");
			break;

	}

	if(text)
	{
		gsize	  bytes_written;
		GError	* error		= NULL;
		gchar	* utfchar	= g_convert_with_fallback(	text,
															-1,
															"UTF-8",
															lib3270_get_charset(host),
															" ",
															NULL,
															&bytes_written,
															&error );

		if(error)
		{
			g_warning("%s failed: %s",__FUNCTION__,error->message);
			g_error_free(error);
		}

		free(text);
		return utfchar;
	}

	return NULL;
}

static gchar * v3270_accessible_get_text(AtkText *atk_text, gint start_pos, gint end_pos)
{
	GtkWidget	* widget	= gtk_accessible_get_widget(GTK_ACCESSIBLE (atk_text));
	char		* text;
	H3270		* host;
	gchar		* utftext	= NULL;

	if(widget == NULL)
		return NULL;

	host = v3270_get_session(widget);
	if(!host)
		return NULL;

	if(!lib3270_connected(host))
		return g_strdup( "" );

	text = lib3270_get_text(host,start_pos,end_pos < start_pos ? -1 : (end_pos - start_pos));

	if(text)
	{
		gsize	  bytes_written;
		GError	* error		= NULL;

		utftext =  g_convert_with_fallback(text,-1,"UTF-8",lib3270_get_charset(host)," ",NULL,&bytes_written, &error);

		if(error)
		{
			g_warning("%s failed: %s",__FUNCTION__,error->message);
			g_error_free(error);
		}

		free(text);

		trace("%s:\n%s\n",__FUNCTION__,utftext);

	}

	return utftext;
}

static gboolean v3270_set_caret_offset(AtkText *text, gint offset)
{
	GtkWidget *widget = gtk_accessible_get_widget (GTK_ACCESSIBLE (text));
	if (widget == NULL)
	return FALSE;

	trace("%s - offset=%d",__FUNCTION__,offset);

	lib3270_set_cursor_address(GTK_V3270(widget)->host,offset);

	return TRUE;
}

static void atk_text_interface_init(AtkTextIface *iface)
{
	iface->get_text 				= v3270_accessible_get_text;
	iface->get_character_at_offset	= v3270_accessible_get_character_at_offset;

	iface->get_text_at_offset		= v3270_accessible_get_text_at_offset;

	iface->get_character_count 		= v3270_accessible_get_character_count;
	iface->get_caret_offset 		= v3270_accessible_get_caret_offset;
	iface->set_caret_offset 		= v3270_set_caret_offset;

	iface->get_character_extents 	= v3270_accessible_get_character_extents;
	iface->get_offset_at_point 		= v3270_accessible_get_offset_at_point;

/*
  iface->get_text_before_offset = gtk_label_accessible_get_text_before_offset;

  iface->get_text_after_offset = gtk_label_accessible_get_text_after_offset;

  iface->get_n_selections = gtk_label_accessible_get_n_selections;
  iface->get_selection = gtk_label_accessible_get_selection;
  iface->add_selection = gtk_label_accessible_add_selection;
  iface->remove_selection = gtk_label_accessible_remove_selection;
  iface->set_selection = gtk_label_accessible_set_selection;

  iface->get_run_attributes = gtk_label_accessible_get_run_attributes;
  iface->get_default_attributes = gtk_label_accessible_get_default_attributes;
*/
}


static void v3270_accessible_init(v3270Accessible *widget)
{
	AtkObject *obj = ATK_OBJECT(widget);
	obj->role	= ATK_ROLE_TEXT;
}


void v3270_accessible_get_extents(AtkComponent *component, gint *x, gint *y,gint *width,gint *height, AtkCoordType coord_type)
{
	GtkWidget *widget = gtk_accessible_get_widget(GTK_ACCESSIBLE (component));
	GdkWindow *window;
	gint x_window, y_window;
	GtkAllocation allocation;

	if (widget == NULL)
		return;

	gtk_widget_get_allocation (widget, &allocation);
	*width	= allocation.width;
	*height	= allocation.height;

	if(gtk_widget_get_parent(widget))
	{
		*x = allocation.x;
		*y = allocation.y;
		window = gtk_widget_get_parent_window (widget);
	}
	else
	{
		*x = 0;
		*y = 0;
		window = gtk_widget_get_window (widget);
	}

	gdk_window_get_origin(window, &x_window, &y_window);
	*x += x_window;
	*y += y_window;

	if (coord_type == ATK_XY_WINDOW)
	{
		gint x_toplevel, y_toplevel;

		window = gdk_window_get_toplevel (gtk_widget_get_window (widget));
		gdk_window_get_origin (window, &x_toplevel, &y_toplevel);

		*x -= x_toplevel;
		*y -= y_toplevel;
	}
}

static void v3270_accessible_get_size(AtkComponent *component,gint *width, gint *height)
{
  GtkWidget *widget = gtk_accessible_get_widget (GTK_ACCESSIBLE (component));

  if (widget == NULL)
    return;

  *width = gtk_widget_get_allocated_width (widget);
  *height = gtk_widget_get_allocated_height (widget);
}

static gboolean v3270_accessible_grab_focus(AtkComponent *component)
{
	GtkWidget *widget = gtk_accessible_get_widget (GTK_ACCESSIBLE (component));
	GtkWidget *toplevel;

	if (!widget)
		return FALSE;

	gtk_widget_grab_focus (widget);

	toplevel = gtk_widget_get_toplevel (widget);
	if (gtk_widget_is_toplevel (toplevel))
	{
		#ifdef GDK_WINDOWING_X11
			gtk_window_present_with_time (GTK_WINDOW (toplevel),gdk_x11_get_server_time(gtk_widget_get_window(widget)));
		#else
			gtk_window_present (GTK_WINDOW (toplevel));
		#endif
	}

	return TRUE;
}


static void atk_component_interface_init(AtkComponentIface *iface)
{
  iface->get_extents	= v3270_accessible_get_extents;
  iface->get_size		= v3270_accessible_get_size;
  iface->grab_focus		= v3270_accessible_grab_focus;

/*
  iface->get_layer = gtk_widget_accessible_get_layer;
  iface->set_extents = gtk_widget_accessible_set_extents;
  iface->set_position = gtk_widget_accessible_set_position;
  iface->set_size = gtk_widget_accessible_set_size;
*/
}

void v3270_acessible_set_state(GtkAccessible *obj, LIB3270_MESSAGE id)
{
	#ifdef DEBUG
		#define STATE_MESSAGE(x,c) { #x, x, c }
	#else
		#define STATE_MESSAGE(x,c) { x, c }
	#endif

	static const struct _state
	{
	#ifdef DEBUG
		const gchar		* dbg;
	#endif
		AtkStateType	  atkstate;
		V3270_STATE		  flag;
	} table[] =
	{
		STATE_MESSAGE(ATK_STATE_BUSY,			V3270_STATE_BUSY			),
		STATE_MESSAGE(ATK_STATE_EDITABLE,		V3270_STATE_EDITABLE		),
		STATE_MESSAGE(ATK_STATE_ENABLED,		V3270_STATE_ENABLED			),
		STATE_MESSAGE(ATK_STATE_INVALID_ENTRY,	V3270_STATE_INVALID_ENTRY	),
	};

	V3270_STATE state = GTK_V3270_ACCESSIBLE(obj)->state;
	V3270_STATE bits;
	int			f;

	switch(id)
	{
	case LIB3270_MESSAGE_NONE:
		state = V3270_STATE_EDITABLE|V3270_STATE_ENABLED;
		break;

	case LIB3270_MESSAGE_SYSWAIT:
	case LIB3270_MESSAGE_TWAIT:
	case LIB3270_MESSAGE_RESOLVING:
	case LIB3270_MESSAGE_CONNECTING:
		state = V3270_STATE_BUSY;
		break;

	case LIB3270_MESSAGE_CONNECTED:
	case LIB3270_MESSAGE_AWAITING_FIRST:
		state = V3270_STATE_ENABLED;
		break;

	case LIB3270_MESSAGE_DISCONNECTED:
		state = 0;
		break;

	case LIB3270_MESSAGE_MINUS:
	case LIB3270_MESSAGE_INHIBIT:
	case LIB3270_MESSAGE_X:
		break;

	case LIB3270_MESSAGE_PROTECTED:
	case LIB3270_MESSAGE_NUMERIC:
	case LIB3270_MESSAGE_OVERFLOW:
	case LIB3270_MESSAGE_KYBDLOCK:
		state = V3270_STATE_INVALID_ENTRY|V3270_STATE_EDITABLE|V3270_STATE_ENABLED;
		break;

	}

	if(state == GTK_V3270_ACCESSIBLE(obj)->state)
		return;

	bits = GTK_V3270_ACCESSIBLE(obj)->state ^ state;

	trace("State change from %04x to %04x (bits=%04x)",
				GTK_V3270_ACCESSIBLE(obj)->state,
				state, bits );

	for(f=0;f<G_N_ELEMENTS(table);f++)
	{
		if(bits & table[f].flag)
		{
			trace("State %s is %s",table[f].dbg,(state & table[f].flag) ? "Yes" : "No");
			atk_object_notify_state_change(ATK_OBJECT(obj),table[f].atkstate,(state & table[f].flag) ? TRUE : FALSE);
		}
	}

	GTK_V3270_ACCESSIBLE(obj)->state = state;
}

