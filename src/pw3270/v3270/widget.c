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
 * Este programa está nomeado como widget.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <gtk/gtk.h>
 #include <pw3270.h>
 #include <lib3270.h>
 #include <lib3270/session.h>
 #include <lib3270/actions.h>
 #include <lib3270/log.h>
 #include <malloc.h>
 #include "v3270.h"
 #include "private.h"
 #include "accessible.h"
 #include "marshal.h"

 #define WIDTH_IN_PIXELS(terminal,x) (x * cols)
 #define HEIGHT_IN_PIXELS(terminal,x) (x * (rows+1))

 #define CONTENTS_WIDTH(terminal) (cols * terminal->metrics.width)
 #define CONTENTS_HEIGHT(terminal) (((rows+1) * terminal->metrics.spacing)+OIA_TOP_MARGIN+2)

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 G_DEFINE_TYPE(v3270, v3270, GTK_TYPE_WIDGET);

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 guint		  		  v3270_widget_signal[LAST_SIGNAL]	= { 0 };
 GdkCursor			* v3270_cursor[V3270_CURSOR_COUNT]	= { 0 };

/*--[ Prototipes ]-----------------------------------------------------------------------------------*/

 // http://git.gnome.org/browse/gtk+/tree/gtk/gtkdrawingarea.c?h=gtk-3-0

static void			  v3270_realize				(	GtkWidget		* widget) ;
static void			  v3270_size_allocate		(	GtkWidget		* widget,
													GtkAllocation	* allocation );
static void			  v3270_send_configure		(	v3270			* terminal );
static AtkObject	* v3270_get_accessible		(	GtkWidget 		* widget );

// Signals
static void v3270_activate			(GtkWidget *widget);

gboolean v3270_focus_in_event(GtkWidget *widget, GdkEventFocus *event);
gboolean v3270_focus_out_event(GtkWidget *widget, GdkEventFocus *event);

#if GTK_CHECK_VERSION(3,0,0)

static void 	v3270_destroy		(GtkWidget		* object);

#else

static void 	v3270_destroy		(GtkObject		* object);

#endif // gtk3

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void v3270_cursor_draw(v3270 *widget)
{
	int 			pos = lib3270_get_cursor_address(widget->host);
	unsigned char	c;
	unsigned short	attr;

	lib3270_get_contents(widget->host,pos,pos,&c,&attr);
	v3270_update_cursor_surface(widget,c,attr);
	gtk_widget_queue_draw_area(	GTK_WIDGET(widget),
								widget->cursor.rect.x,widget->cursor.rect.y,
								widget->cursor.rect.width,widget->cursor.rect.height);

}

static void v3270_toggle_changed(v3270 *widget,LIB3270_TOGGLE toggle_id, gboolean toggle_state,const gchar *toggle_name)
{
	trace("%s: toggle %d (%s)=%s",__FUNCTION__,toggle_id,toggle_name,toggle_state ? "Yes" : "No");

	switch(toggle_id)
	{
	case LIB3270_TOGGLE_CURSOR_POS:
	case LIB3270_TOGGLE_CROSSHAIR:
		v3270_reload(GTK_WIDGET(widget));
		gtk_widget_queue_draw(GTK_WIDGET(widget));
		break;

	case LIB3270_TOGGLE_CURSOR_BLINK:
		widget->cursor.show |= 1;
		break;

	case LIB3270_TOGGLE_INSERT:
		v3270_draw_ins_status(widget);
		v3270_cursor_draw(widget);
		break;

	case LIB3270_TOGGLE_BOLD:
		v3270_reload(GTK_WIDGET(widget));
		gtk_widget_queue_draw(GTK_WIDGET(widget));
		break;

	default:
		return;

	}

}

static void loghandler(H3270 *session, const char *module, int rc, const char *fmt, va_list args)
{
	g_logv(module,rc ? G_LOG_LEVEL_WARNING : G_LOG_LEVEL_MESSAGE, fmt, args);
}

static gboolean v3270_popup_menu(GtkWidget * widget)
{
	GdkEventButton event;

	memset(&event,0,sizeof(event));

	event.time	 = gtk_get_current_event_time();
	event.button = 3;
	event.type 	 = GDK_BUTTON_PRESS;

	v3270_emit_popup(	GTK_V3270(widget),
						lib3270_get_cursor_address(GTK_V3270(widget)->host),
						&event );

	return TRUE;
}

#if GTK_CHECK_VERSION(3,0,0)

static GtkSizeRequestMode get_request_mode(GtkWidget *widget)
{
	int rows, cols;

	trace("%s",__FUNCTION__);

	lib3270_get_screen_size(GTK_V3270(widget)->host,&rows,&cols);

	return rows > cols ? GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT : GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;

}

void get_preferred_height(GtkWidget *widget, gint *minimum_height, gint *natural_height)
{
	int height = GTK_V3270(widget)->minimum_height;

	if(minimum_height)
		*minimum_height = height ? height : 10;

	if(natural_height)
		*natural_height = 400;

}

void get_preferred_width(GtkWidget *widget, gint *minimum_width, gint *natural_width)
{
	int width = GTK_V3270(widget)->minimum_width;

	if(minimum_width)
		*minimum_width = width ? width : 10;

	if(natural_width)
		*natural_width = 600;
}

/*
void get_preferred_height_for_width(GtkWidget *widget, gint width, gint *minimum_height, gint *natural_height)
{
	trace("%s",__FUNCTION__);

	if(minimum_height)
		*minimum_height = 10;

	if(natural_height)
		*natural_height = 10;
}

static void get_preferred_width_for_height(GtkWidget *widget,gint height, gint *minimum_width, gint *natural_width)
{
	trace("%s",__FUNCTION__);

	if(minimum_width)
		*minimum_width = 10;

	if(natural_width)
		*natural_width = 10;

}
*/

#endif // GTK(3,0,0)

void v3270_popup_message(GtkWidget *widget, LIB3270_NOTIFY type , const gchar *title, const gchar *message, const gchar *text)
{
	GtkWidget		* dialog;
	GtkWidget		* toplevel	= NULL;
	GtkMessageType	  msgtype	= GTK_MESSAGE_WARNING;
	GtkButtonsType	  buttons	= GTK_BUTTONS_OK;

	if(widget && GTK_IS_WIDGET(widget))
		toplevel = gtk_widget_get_toplevel(GTK_WIDGET(widget));

	if(type == LIB3270_NOTIFY_CRITICAL)
	{
		msgtype	= GTK_MESSAGE_ERROR;
		buttons = GTK_BUTTONS_CLOSE;
	}

	if(!title)
		title = _( "Error" );

	if(message)
	{
		dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(toplevel),GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,msgtype,buttons,"%s",message);
		if(text)
			gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog),"%s",text);
	}
	else if(text)
	{
		dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(toplevel),GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,msgtype,buttons,"%s",text);
	}
	else
	{
		dialog = gtk_message_dialog_new_with_markup(GTK_WINDOW(toplevel),GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,msgtype,buttons,"%s",title);
	}

	gtk_window_set_title(GTK_WINDOW(dialog),title);
	gtk_widget_show_all(dialog);
	gtk_dialog_run(GTK_DIALOG (dialog));
	gtk_widget_destroy(dialog);
}

static void v3270_class_init(v3270Class *klass)
{
	GObjectClass	* gobject_class	= G_OBJECT_CLASS(klass);
	GtkWidgetClass	* widget_class	= GTK_WIDGET_CLASS(klass);

	lib3270_set_log_handler(loghandler);

	widget_class->realize 							= v3270_realize;
	widget_class->size_allocate						= v3270_size_allocate;
	widget_class->key_press_event					= v3270_key_press_event;
	widget_class->key_release_event					= v3270_key_release_event;
	widget_class->focus_in_event					= v3270_focus_in_event;
	widget_class->focus_out_event					= v3270_focus_out_event;
	widget_class->button_press_event				= v3270_button_press_event;
	widget_class->button_release_event				= v3270_button_release_event;
	widget_class->motion_notify_event				= v3270_motion_notify_event;
	widget_class->popup_menu						= v3270_popup_menu;

	/* Accessibility support */
	widget_class->get_accessible 					= v3270_get_accessible;

	klass->activate									= v3270_activate;
	klass->toggle_changed 							= v3270_toggle_changed;
	klass->message_changed 							= v3270_update_message;
	klass->luname_changed							= v3270_update_luname;
	klass->popup_message							= v3270_popup_message;

#if GTK_CHECK_VERSION(3,0,0)

	widget_class->get_preferred_height				= get_preferred_height;
	widget_class->get_preferred_width				= get_preferred_width;
//	widget_class->get_preferred_width_for_height	= get_preferred_width_for_height;
//	widget_class->get_preferred_height_for_width	= get_preferred_height_for_width;

	widget_class->destroy 							= v3270_destroy;
	widget_class->draw 								= v3270_draw;

#else

	{
		GtkObjectClass *object_class = (GtkObjectClass*) klass;

		object_class->destroy = v3270_destroy;
	}

	widget_class->expose_event = v3270_expose;


#endif // GTK3

	v3270_register_io_handlers(klass);

	// Cursors
	{
#ifdef WIN32
		static const gchar	* cr[V3270_CURSOR_COUNT] =
		{
			"ibeam",	//	V3270_CURSOR_UNPROTECTED
			"wait",		//	V3270_CURSOR_WAITING
			"arrow",	//	V3270_CURSOR_LOCKED
			"arrow",	//	V3270_CURSOR_PROTECTED
			"hand",		//	V3270_CURSOR_MOVE_SELECTION
			"sizenwse",	//	V3270_CURSOR_SELECTION_TOP_LEFT
			"sizenesw",	//	V3270_CURSOR_SELECTION_TOP_RIGHT
			"sizens",	//	V3270_CURSOR_SELECTION_TOP
			"sizenesw",	//	V3270_CURSOR_SELECTION_BOTTOM_LEFT
			"sizenwse",	//	V3270_CURSOR_SELECTION_BOTTOM_RIGHT
			"sizens",	//	V3270_CURSOR_SELECTION_BOTTOM
			"sizewe",	//	V3270_CURSOR_SELECTION_LEFT
			"sizewe",	//	V3270_CURSOR_SELECTION_RIGHT
		};
#else
		static const int	  cr[V3270_CURSOR_COUNT] =
		{
			GDK_XTERM,					// V3270_CURSOR_UNPROTECTED
			GDK_WATCH,					// V3270_CURSOR_WAITING
			GDK_X_CURSOR,				// V3270_CURSOR_LOCKED
			GDK_ARROW,					// V3270_CURSOR_PROTECTED
			GDK_HAND1,					// V3270_CURSOR_MOVE_SELECTION
			GDK_TOP_LEFT_CORNER, 		// V3270_CURSOR_SELECTION_TOP_LEFT
			GDK_TOP_RIGHT_CORNER,		// V3270_CURSOR_SELECTION_TOP_RIGHT
			GDK_TOP_SIDE,				// V3270_CURSOR_SELECTION_TOP
			GDK_BOTTOM_LEFT_CORNER,		// V3270_CURSOR_SELECTION_BOTTOM_LEFT
			GDK_BOTTOM_RIGHT_CORNER,	// V3270_CURSOR_SELECTION_BOTTOM_RIGHT
			GDK_BOTTOM_SIDE,			// V3270_CURSOR_SELECTION_BOTTOM
			GDK_LEFT_SIDE,				// V3270_CURSOR_SELECTION_LEFT
			GDK_RIGHT_SIDE,				// V3270_CURSOR_SELECTION_RIGHT
		};
#endif // WIN32

		int f;

		for(f=0;f<V3270_CURSOR_COUNT;f++)
		{
	#ifdef WIN32
			v3270_cursor[f] = gdk_cursor_new_from_name(gdk_display_get_default(),cr[f]);
	#else
			v3270_cursor[f] = gdk_cursor_new(cr[f]);
	#endif
		}
	}

	// Signals
	widget_class->activate_signal =
		g_signal_new(	"activate",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
						G_STRUCT_OFFSET (v3270Class, activate),
						NULL, NULL,
						pw3270_VOID__VOID,
						G_TYPE_NONE, 0);

	v3270_widget_signal[SIGNAL_TOGGLE_CHANGED] =
		g_signal_new(	"toggle_changed",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (v3270Class, toggle_changed),
						NULL, NULL,
						pw3270_VOID__VOID_ENUM_BOOL_POINTER,
						G_TYPE_NONE, 3, G_TYPE_UINT, G_TYPE_BOOLEAN, G_TYPE_STRING);

	v3270_widget_signal[SIGNAL_MESSAGE_CHANGED] =
		g_signal_new(	"message_changed",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (v3270Class, message_changed),
						NULL, NULL,
						pw3270_VOID__VOID_ENUM,
						G_TYPE_NONE, 1, G_TYPE_UINT);

	v3270_widget_signal[SIGNAL_LUNAME_CHANGED] =
		g_signal_new(	"luname_changed",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (v3270Class, luname_changed),
						NULL, NULL,
						pw3270_VOID__VOID_POINTER,
						G_TYPE_NONE, 1, G_TYPE_STRING);

	v3270_widget_signal[SIGNAL_KEYPRESS] =
		g_signal_new(	"keypress",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_LAST,
						G_STRUCT_OFFSET (v3270Class, keypress),
						NULL, NULL,
						pw3270_BOOL__VOID_UINT_ENUM,
						G_TYPE_BOOLEAN, 2, G_TYPE_UINT, G_TYPE_UINT);

	v3270_widget_signal[SIGNAL_CONNECTED] =
		g_signal_new(	"connected",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						pw3270_VOID__POINTER,
						G_TYPE_NONE, 1, G_TYPE_STRING);

	v3270_widget_signal[SIGNAL_DISCONNECTED] =
		g_signal_new(	"disconnected",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						pw3270_VOID__VOID,
						G_TYPE_NONE, 0);

	v3270_widget_signal[SIGNAL_UPDATE_CONFIG] =
		g_signal_new(	"update_config",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						pw3270_VOID__VOID_POINTER_POINTER,
						G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);

	v3270_widget_signal[SIGNAL_MODEL_CHANGED] =
		g_signal_new(	"model_changed",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						pw3270_VOID__VOID_UINT_POINTER,
						G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_STRING);

	v3270_widget_signal[SIGNAL_SELECTING] =
		g_signal_new(	"selecting",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						pw3270_VOID__VOID_BOOL,
						G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

	v3270_widget_signal[SIGNAL_POPUP] =
		g_signal_new(	"popup",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_LAST,
						0,
						NULL, NULL,
						pw3270_BOOL__VOID_BOOL_BOOL_POINTER,
						G_TYPE_BOOLEAN, 3, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN, G_TYPE_POINTER);

	v3270_widget_signal[SIGNAL_PASTENEXT] =
		g_signal_new(	"pastenext",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						pw3270_VOID__VOID_BOOL,
						G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

	v3270_widget_signal[SIGNAL_CLIPBOARD] =
		g_signal_new(	"has_text",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						pw3270_VOID__VOID_BOOL,
						G_TYPE_NONE, 1, G_TYPE_BOOLEAN);

	v3270_widget_signal[SIGNAL_CHANGED] =
		g_signal_new(	"changed",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						0,
						NULL, NULL,
						pw3270_VOID__VOID_UINT_UINT,
						G_TYPE_NONE, 2, G_TYPE_UINT, G_TYPE_UINT);

	v3270_widget_signal[SIGNAL_MESSAGE] =
		g_signal_new(	"popup_message",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST,
						G_STRUCT_OFFSET (v3270Class, popup_message),
						NULL, NULL,
						pw3270_VOID__VOID_UINT_POINTER_POINTER_POINTER,
						G_TYPE_NONE, 4, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

}

void v3270_update_font_metrics(v3270 *terminal, cairo_t *cr, int width, int height)
{
	// update font metrics
 	static const int font_size[] = { 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 22, 24, 26, 28, 32, 36, 40, 48, 56, 64, 72, 0 };
	int f, rows, cols, hFont;
	int size = font_size[0];

	cairo_font_extents_t extents;

	lib3270_get_screen_size(terminal->host,&rows,&cols);

	terminal->font_weight = lib3270_get_toggle(terminal->host,LIB3270_TOGGLE_BOLD) ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL;

	cairo_select_font_face(cr, terminal->font_family, CAIRO_FONT_SLANT_NORMAL,terminal->font_weight);

 	for(f=0;font_size[f];f++)
 	{
        cairo_set_font_size(cr,font_size[f]);
        cairo_font_extents(cr,&extents);

		if(f == 0)
		{
			terminal->minimum_width  = (cols * extents.max_x_advance);
			terminal->minimum_height = ((rows+1) * (extents.height + extents.descent)) + (OIA_TOP_MARGIN+2);
		}

		if( HEIGHT_IN_PIXELS(terminal,(extents.height+extents.descent)) < height && WIDTH_IN_PIXELS(terminal,extents.max_x_advance) < width )
			size = font_size[f];
 	}

	cairo_set_font_size(cr,size);

#if !GTK_CHECK_VERSION(3,0,0)
	gtk_widget_set_size_request(GTK_WIDGET(terminal),terminal->minimum_width,terminal->minimum_height);
#endif // !GTK(3,0,0)

/*
	double sx, sy;
	cairo_matrix_t font_matrix;

	cairo_set_font_size(cr,10);
	cairo_font_extents(cr,&extents);

	trace("font - extents.height=%f  extents.width=%f",extents.height,extents.max_x_advance);

	sx = ((double) width) / (((double) terminal->cols) * extents.max_x_advance);
	sy = ((double) height) / (((double) terminal->rows) * extents.height);

	trace("sy=%f sx=%f ",sy,sx);

	cairo_get_font_matrix(cr,&font_matrix);
	cairo_matrix_scale(&font_matrix, sx, sy);
	cairo_set_font_matrix(cr,&font_matrix);
*/

	/* Save scaled font for use on next drawings */
	if(terminal->font_scaled)
		cairo_scaled_font_destroy(terminal->font_scaled);

	terminal->font_scaled = cairo_get_scaled_font(cr);
	cairo_scaled_font_reference(terminal->font_scaled);

	cairo_scaled_font_extents(terminal->font_scaled,&extents);

	terminal->metrics.width    = (int) extents.max_x_advance;
	terminal->metrics.height   = (int) extents.height;
	terminal->metrics.ascent   = (int) extents.ascent;
	terminal->metrics.descent  = (int) extents.descent;

	hFont = terminal->metrics.height + terminal->metrics.descent;

	// Create new cursor surface
	if(terminal->cursor.surface)
		cairo_surface_destroy(terminal->cursor.surface);

	terminal->cursor.surface = gdk_window_create_similar_surface(gtk_widget_get_window(GTK_WIDGET(terminal)),CAIRO_CONTENT_COLOR,terminal->metrics.width,hFont);

	// Center image
	size = CONTENTS_WIDTH(terminal);
	terminal->metrics.left = (width >> 1) - ((size) >> 1);

	terminal->metrics.spacing = height / (rows+2);
	if(terminal->metrics.spacing < hFont)
		terminal->metrics.spacing = hFont;

	size = CONTENTS_HEIGHT(terminal);

	terminal->metrics.top = (height >> 1) - (size >> 1);

}

static void set_timer(H3270 *session, unsigned char on)
{
	GtkWidget *widget = GTK_WIDGET(session->widget);

	if(on)
		v3270_start_timer(widget);
	else
		v3270_stop_timer(widget);

}

static void update_toggle(H3270 *session, LIB3270_TOGGLE ix, unsigned char value, LIB3270_TOGGLE_TYPE reason, const char *name)
{
	g_signal_emit(GTK_WIDGET(session->widget), v3270_widget_signal[SIGNAL_TOGGLE_CHANGED], 0, (guint) ix, (gboolean) (value != 0), (gchar *) name);
}

static void update_message(H3270 *session, LIB3270_MESSAGE id)
{
	g_signal_emit(GTK_WIDGET(session->widget), v3270_widget_signal[SIGNAL_MESSAGE_CHANGED], 0, (gint) id);
}

static void update_luname(H3270 *session, const char *name)
{
	g_signal_emit(GTK_WIDGET(session->widget), v3270_widget_signal[SIGNAL_LUNAME_CHANGED], 0, (gchar *) name);
}

static void select_cursor(H3270 *session, LIB3270_CURSOR id)
{
	GtkWidget *widget = GTK_WIDGET(session->widget);

	if(gtk_widget_get_realized(widget) && gtk_widget_get_has_window(widget))
	{
		GTK_V3270(widget)->pointer_id = id;
		v3270_update_mouse_pointer(widget);
	}
}

static void ctlr_done(H3270 *session)
{
	GtkWidget *widget = GTK_WIDGET(session->widget);

	if(gtk_widget_get_realized(widget) && gtk_widget_get_has_window(widget))
	{
		v3270_update_mouse_pointer(widget);
	}

}

static void update_connect(H3270 *session, unsigned char connected)
{
	v3270 *widget = GTK_V3270(session->widget);

	if(connected)
	{
		widget->cursor.show |= 2;
		g_signal_emit(GTK_WIDGET(widget), v3270_widget_signal[SIGNAL_CONNECTED], 0, session->full_current_host);
	}
	else
	{
		widget->cursor.show &= ~2;
		g_signal_emit(GTK_WIDGET(widget), v3270_widget_signal[SIGNAL_DISCONNECTED], 0);
	}

	gtk_widget_queue_draw(GTK_WIDGET(widget));
}

static void update_screen_size(H3270 *session,unsigned short rows, unsigned short cols)
{
//	trace("Widget %p changes to %dx%d",session->widget,cols,rows);
	v3270_reload(GTK_WIDGET(session->widget));
	gtk_widget_queue_draw(GTK_WIDGET(session->widget));
}

static void update_model(H3270 *session, const char *name, int model, int rows, int cols)
{
	g_signal_emit(GTK_WIDGET(session->widget),v3270_widget_signal[SIGNAL_MODEL_CHANGED], 0, (guint) model, name);
}

static void changed(H3270 *session, int offset, int len)
{
	GtkWidget 		* widget	= session->widget;
	GtkAccessible	* obj		= GTK_V3270(widget)->accessible;

//	trace("%s: offset=%d len=%d",__FUNCTION__,offset,len)

	if(obj)
	{
		// Get new text, notify atk
		gsize	  bytes_written	= 0;
		char	* text 			= lib3270_get_text(session,offset,len);

		if(text)
		{
			GError	* error		= NULL;
			gchar	* utfchar	= g_convert_with_fallback(	text,
																-1,
																"UTF-8",
																lib3270_get_charset(session),
																" ",
																NULL,
																&bytes_written,
																&error );

			free(text);

			if(error)
			{
				g_warning("%s failed: %s",__FUNCTION__,error->message);
				g_error_free(error);
			}

			if(utfchar)
			{
				g_signal_emit_by_name(obj, "text-insert", offset, bytes_written, utfchar);
				g_free(utfchar);
			}

		}
	}

	g_signal_emit(GTK_WIDGET(widget),v3270_widget_signal[SIGNAL_CHANGED], 0, (guint) offset, (guint) len);

}

static void set_selection(H3270 *session, unsigned char status)
{
	GtkWidget	* widget	= GTK_WIDGET(session->widget);
	g_signal_emit(widget,v3270_widget_signal[SIGNAL_SELECTING], 0, status ? TRUE : FALSE);
}

static void update_selection(H3270 *session, int start, int end)
{
	// Selected region changed
	GtkWidget		* widget	= GTK_WIDGET(session->widget);
	GtkAccessible	* atk_obj	= GTK_V3270(widget)->accessible;

	if(atk_obj)
		g_signal_emit_by_name(atk_obj,"text-selection-changed");

}

static void message(H3270 *session, LIB3270_NOTIFY id , const char *title, const char *message, const char *text)
{
	g_signal_emit(	GTK_WIDGET(session->widget), v3270_widget_signal[SIGNAL_MESSAGE], 0,
							(int) id,
							(gchar *) title,
							(gchar *) message,
							(gchar *) text );

}

static void v3270_init(v3270 *widget)
{
	trace("%s",__FUNCTION__);
	widget->host = lib3270_session_new("");

	if(widget->host->sz != sizeof(H3270))
	{
		g_error( _( "Unexpected signature in H3270 object, possible version mismatch in lib3270") );
		return;
	}

	widget->host->widget			= widget;

	widget->host->update			= v3270_update_char;
	widget->host->changed			= changed;
	widget->host->set_timer 		= set_timer;

	widget->host->set_selection		= set_selection;
	widget->host->update_selection	= update_selection;

	widget->host->update_luname		= update_luname;
	widget->host->configure			= update_screen_size;
	widget->host->update_status 	= update_message;
	widget->host->update_cursor 	= v3270_update_cursor;
	widget->host->update_toggle 	= update_toggle;
	widget->host->update_oia		= v3270_update_oia;
	widget->host->cursor			= select_cursor;
	widget->host->update_connect	= update_connect;
	widget->host->update_model		= update_model;
	widget->host->changed			= changed;
	widget->host->ctlr_done			= ctlr_done;
	widget->host->message			= message;

	// Setup input method
	widget->input_method 			= gtk_im_multicontext_new();
    g_signal_connect(G_OBJECT(widget->input_method),"commit",G_CALLBACK(v3270_key_commit),widget);

	gtk_widget_set_can_default(GTK_WIDGET(widget),TRUE);
	gtk_widget_set_can_focus(GTK_WIDGET(widget),TRUE);

	// Setup events
    gtk_widget_add_events(GTK_WIDGET(widget),GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK|GDK_BUTTON_PRESS_MASK|GDK_BUTTON_MOTION_MASK|GDK_BUTTON_RELEASE_MASK|GDK_POINTER_MOTION_MASK|GDK_ENTER_NOTIFY_MASK);

}

GtkWidget * v3270_new(void)
{
	return g_object_new(GTK_TYPE_V3270, NULL);
}

#if GTK_CHECK_VERSION(3,0,0)
static void v3270_destroy(GtkWidget *widget)
#else
static void v3270_destroy(GtkObject *widget)
#endif
{
	v3270 * terminal = GTK_V3270(widget);

	trace("%s %p",__FUNCTION__,widget);

	if(terminal->accessible)
	{
		gtk_accessible_set_widget(terminal->accessible, NULL);
		g_object_unref(terminal->accessible);
		terminal->accessible = NULL;
	}

	if(terminal->host)
	{
		lib3270_session_free(terminal->host);
		terminal->host = NULL;
	}

	if(terminal->font_family)
	{
		g_free(terminal->font_family);
		terminal->font_family = 0;
	}

	if(terminal->font_scaled)
	{
		cairo_scaled_font_destroy(terminal->font_scaled);
		terminal->font_scaled = NULL;
	}

	if(terminal->surface)
	{
		cairo_surface_destroy(terminal->surface);
		terminal->surface = NULL;
	}

	if(terminal->cursor.surface)
	{
		cairo_surface_destroy(terminal->cursor.surface);
		terminal->cursor.surface = NULL;
	}

	if(terminal->timer)
	{
		g_source_destroy(terminal->timer);
		while(terminal->timer)
			g_source_unref(terminal->timer);
	}

	if(terminal->cursor.timer)
	{
		g_source_destroy(terminal->cursor.timer);
		while(terminal->cursor.timer)
			g_source_unref(terminal->cursor.timer);
	}

	if(terminal->input_method)
	{
		g_object_unref(terminal->input_method);
		terminal->input_method = NULL;
	}

	if(terminal->clipboard)
	{
		g_free(terminal->clipboard);
		terminal->clipboard = NULL;
	}

#if GTK_CHECK_VERSION(3,0,0)
	GTK_WIDGET_CLASS(v3270_parent_class)->destroy(widget);
#else
	GTK_OBJECT_CLASS(v3270_parent_class)->destroy(widget);
#endif // GTK3

}

static gboolean timer_tick(v3270 *widget)
{
	if(lib3270_get_toggle(widget->host,LIB3270_TOGGLE_CURSOR_BLINK))
	{
		widget->cursor.show ^= 1;
		gtk_widget_queue_draw_area(GTK_WIDGET(widget),	widget->cursor.rect.x,
														widget->cursor.rect.y,
														widget->cursor.rect.width,
														widget->cursor.rect.height );
	}

	return TRUE;
}

static void release_timer(v3270 *widget)
{
	widget->cursor.timer = NULL;
}

static void v3270_realize(GtkWidget	* widget)
{
	if(!gtk_widget_get_has_window(widget))
	{
		GTK_WIDGET_CLASS(v3270_parent_class)->realize(widget);
	}
	else
	{
		GtkAllocation allocation;
		GdkWindow *window;
		GdkWindowAttr attributes;
		gint attributes_mask;


		gtk_widget_set_realized (widget, TRUE);

		gtk_widget_get_allocation (widget, &allocation);

		attributes.window_type = GDK_WINDOW_CHILD;
		attributes.x = allocation.x;
		attributes.y = allocation.y;
		attributes.width = allocation.width;
		attributes.height = allocation.height;
		attributes.wclass = GDK_INPUT_OUTPUT;
		attributes.visual = gtk_widget_get_visual (widget);
		attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;

		attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;

		window = gdk_window_new(gtk_widget_get_parent_window (widget),&attributes, attributes_mask);
		gdk_window_set_user_data (window, widget);
		gtk_widget_set_window(widget, window);

		gtk_im_context_set_client_window(GTK_V3270(widget)->input_method,window);

	}

#if !GTK_CHECK_VERSION(3,0,0)
	widget->style = gtk_style_attach (widget->style, widget->window);
	gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);
#endif // !GTK3

	v3270_reload(widget);

	v3270_send_configure(GTK_V3270(widget));

	if(!GTK_V3270(widget)->cursor.timer)
	{
		// Setup cursor blink timer
		v3270 *terminal = GTK_V3270(widget);

		terminal->cursor.timer = g_timeout_source_new(500);
		g_source_set_callback(terminal->cursor.timer,(GSourceFunc) timer_tick, widget, (GDestroyNotify) release_timer);

		g_source_attach(terminal->cursor.timer, NULL);
		g_source_unref(terminal->cursor.timer);
	}

}

static void v3270_size_allocate(GtkWidget * widget, GtkAllocation * allocation)
{
	g_return_if_fail(GTK_IS_V3270(widget));
	g_return_if_fail(allocation != NULL);

//	trace("Widget size changes to %dx%d",allocation->width,allocation->height);

	gtk_widget_set_allocation(widget, allocation);

#if !GTK_CHECK_VERSION(3,0,0)
	{
		v3270 *terminal = GTK_V3270(widget);

		terminal->width  = allocation->width;
		terminal->height = allocation->height;
	}
#endif

	if(gtk_widget_get_realized(widget))
	{
		if(gtk_widget_get_has_window(widget))
			gdk_window_move_resize(gtk_widget_get_window (widget),allocation->x, allocation->y,allocation->width, allocation->height);

		v3270_reload(widget);
		v3270_send_configure(GTK_V3270(widget));
	}
}

static void v3270_send_configure(v3270 * terminal)
{
	GtkAllocation allocation;
	GtkWidget *widget;
	GdkEvent *event = gdk_event_new(GDK_CONFIGURE);

	widget = GTK_WIDGET(terminal);

	gtk_widget_get_allocation(widget, &allocation);

	event->configure.window = g_object_ref(gtk_widget_get_window(widget));
	event->configure.send_event = TRUE;
	event->configure.x = allocation.x;
	event->configure.y = allocation.y;
	event->configure.width = allocation.width;
	event->configure.height = allocation.height;

#if( !GTK_CHECK_VERSION(3,0,0))
	terminal->width  = allocation.width;
	terminal->height = allocation.height;
#endif

	gtk_widget_event(widget, event);
	gdk_event_free(event);
}

void v3270_set_colors(GtkWidget *widget, const gchar *colors)
{
	g_return_if_fail(GTK_IS_V3270(widget));

	if(!colors)
	{
		colors =	"#000000,"			// V3270_COLOR_BACKGROUND
					"#7890F0,"			// V3270_COLOR_BLUE
					"#FF0000,"			// V3270_COLOR_RED
					"#FF00FF,"			// V3270_COLOR_PINK
					"#00FF00,"			// V3270_COLOR_GREEN
					"#00FFFF,"			// V3270_COLOR_TURQUOISE
					"#FFFF00,"			// V3270_COLOR_YELLOW
					"#FFFFFF,"			// V3270_COLOR_WHITE
					"#000000,"			// V3270_COLOR_BLACK
					"#000080,"			// V3270_COLOR_DARK_BLUE
					"#FFA200,"			// V3270_COLOR_ORANGE
					"#800080,"			// V3270_COLOR_PURPLE
					"#008000,"			// V3270_COLOR_DARK_GREEN
					"#008080,"			// V3270_COLOR_DARK_TURQUOISE
					"#A0A000,"			// V3270_COLOR_MUSTARD
					"#C0C0C0,"			// V3270_COLOR_GRAY

					"#00FF00,"			// V3270_COLOR_FIELD_DEFAULT
					"#FF0000,"			// V3270_COLOR_FIELD_INTENSIFIED
					"#00FFFF,"			// V3270_COLOR_FIELD_PROTECTED
					"#FFFFFF,"			// V3270_COLOR_FIELD_PROTECTED_INTENSIFIED

					"#404040,"			// V3270_COLOR_SELECTED_BG
					"#FFFFFF,"			// V3270_COLOR_SELECTED_FG,

					"#00FF00," 			// V3270_COLOR_CROSS_HAIR

					"#000000,"	 		// V3270_COLOR_OIA_BACKGROUND
					"#00FF00,"			// V3270_COLOR_OIA
					"#7890F0,"			// V3270_COLOR_OIA_SEPARATOR
					"#FFFFFF,"			// V3270_COLOR_OIA_STATUS_OK
					"#FF0000";			// V3270_COLOR_OIA_STATUS_INVALID

	}

	v3270_set_color_table(GTK_V3270(widget)->color,colors);
	g_signal_emit(widget,v3270_widget_signal[SIGNAL_UPDATE_CONFIG], 0, "colors", colors);
	v3270_reload(widget);

}

void v3270_set_color(GtkWidget *widget, enum V3270_COLOR id, GdkColor *color)
{
	g_return_if_fail(GTK_IS_V3270(widget));

	GTK_V3270(widget)->color[id] = *color;

#if !GTK_CHECK_VERSION(3,0,0)
	gdk_colormap_alloc_color(gtk_widget_get_default_colormap(),color,TRUE,TRUE);
#endif // !GTK(3,0,0)

}
GdkColor * v3270_get_color(GtkWidget *widget, enum V3270_COLOR id)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);
 	return GTK_V3270(widget)->color+id;
}

const GdkColor * v3270_get_color_table(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);
 	return GTK_V3270(widget)->color;
}

void v3270_set_mono_color_table(GdkColor *clr, const gchar *fg, const gchar *bg)
{
	int f;

	gdk_color_parse(bg,clr);
	gdk_color_parse(fg,clr+1);

	for(f=2;f<V3270_COLOR_COUNT;f++)
		clr[f] = clr[1];

	clr[V3270_COLOR_BLACK]			= *clr;
	clr[V3270_COLOR_OIA_BACKGROUND]	= *clr;
	clr[V3270_COLOR_SELECTED_BG]	= clr[V3270_COLOR_WHITE];
	clr[V3270_COLOR_SELECTED_FG]	= clr[V3270_COLOR_BLACK];


}

void v3270_set_color_table(GdkColor *table, const gchar *colors)
{
 	gchar	**clr;
 	guint	  cnt;
 	int		  f;

 	clr = g_strsplit(colors,",",V3270_COLOR_COUNT+1);
 	cnt = g_strv_length(clr);
 	switch(cnt)
 	{
	case V3270_COLOR_COUNT:	// Complete string
		for(f=0;f < V3270_COLOR_COUNT;f++)
			gdk_color_parse(clr[f],table+f);
		break;

	default:

		g_warning("Color table has %d elements; should be %d.",cnt,V3270_COLOR_COUNT);

		for(f=0;f < cnt;f++)
			gdk_color_parse(clr[f],table+f);

		for(f=cnt; f < V3270_COLOR_COUNT;f++)
			gdk_color_parse(clr[cnt-1],table+f);

		clr[V3270_COLOR_OIA_BACKGROUND] = clr[0];
		clr[V3270_COLOR_SELECTED_BG] 	= clr[0];

 	}

	g_strfreev(clr);

}

void v3270_set_font_family(GtkWidget *widget, const gchar *name)
{
	v3270 * terminal;

	g_return_if_fail(GTK_IS_V3270(widget));

	terminal = GTK_V3270(widget);

	if(terminal->font_family)
	{
		if(!g_strcasecmp(terminal->font_family,name))
			return;
		g_free(terminal->font_family);
		terminal->font_family = NULL;
	}

	if(!name)
	{
		// TODO (perry#3#): Get default font family from currrent style
		name = "courier new";
	}

	terminal->font_family = g_strdup(name);
	terminal->font_weight = lib3270_get_toggle(terminal->host,LIB3270_TOGGLE_BOLD) ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL;

	trace("%s: %s (%p)",__FUNCTION__,terminal->font_family,terminal->font_family);

	g_signal_emit(widget,v3270_widget_signal[SIGNAL_UPDATE_CONFIG], 0, "font-family", name);

	v3270_reload(widget);
	gtk_widget_queue_draw(widget);


}

const gchar	* v3270_get_font_family(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);
	return GTK_V3270(widget)->font_family;
}

void v3270_disconnect(GtkWidget *widget)
{
	g_return_if_fail(GTK_IS_V3270(widget));
	lib3270_disconnect(GTK_V3270(widget)->host);
}

H3270 * v3270_get_session(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

	return GTK_V3270(widget)->host;
}

int v3270_connect(GtkWidget *widget, const gchar *host)
{
	v3270 * terminal;
	int		rc = -1;

	trace("%s widget=%p host=%p",__FUNCTION__,widget,host);

	g_return_val_if_fail(GTK_IS_V3270(widget),EINVAL);

	terminal = GTK_V3270(widget);

	rc = lib3270_connect(terminal->host,host,0);

	trace("%s exits with rc=%d (%s)",__FUNCTION__,rc,strerror(rc));

	return rc;
}

static gboolean notify_focus(GtkWidget *widget, GdkEventFocus *event)
{
	GtkAccessible *obj = GTK_V3270(widget)->accessible;

	if(obj)
		g_signal_emit_by_name (obj, "focus-event", event->in);

	return FALSE;
}
gboolean v3270_focus_in_event(GtkWidget *widget, GdkEventFocus *event)
{
	v3270 * terminal = GTK_V3270(widget);

	gtk_im_context_focus_in(terminal->input_method);

	return notify_focus(widget,event);
}

gboolean v3270_focus_out_event(GtkWidget *widget, GdkEventFocus *event)
{
	v3270 * terminal = GTK_V3270(widget);

	gtk_im_context_focus_out(terminal->input_method);

	return notify_focus(widget,event);
}

static void v3270_activate(GtkWidget *widget)
{
	v3270 * terminal = GTK_V3270(widget);

	trace("%s: %p",__FUNCTION__,terminal);

	if(lib3270_connected(terminal->host))
		lib3270_enter(terminal->host);
	else if(lib3270_get_host(terminal->host))
		v3270_connect(widget,NULL);
	else
		g_warning("Terminal widget %p activated without connection or valid hostname",terminal);
}

const GtkWidgetClass * v3270_get_parent_class(void)
{
	return GTK_WIDGET_CLASS(v3270_parent_class);
}

static AtkObject * v3270_get_accessible(GtkWidget * widget)
{
	v3270 * terminal = GTK_V3270(widget);

//	trace("%s acc=%p",__FUNCTION__,terminal->accessible);

	if(!terminal->accessible)
	{
		terminal->accessible = g_object_new(GTK_TYPE_V3270_ACCESSIBLE,NULL);
		atk_object_initialize(ATK_OBJECT(terminal->accessible), widget);
		gtk_accessible_set_widget(GTK_ACCESSIBLE(terminal->accessible),widget);
		g_object_ref(terminal->accessible);
	}

	return ATK_OBJECT(terminal->accessible);
}

GtkIMContext * v3270_get_im_context(GtkWidget *widget)
{
	return GTK_V3270(widget)->input_method;
}

gboolean v3270_get_toggle(GtkWidget *widget, LIB3270_TOGGLE ix)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),FALSE);

	if(ix < LIB3270_TOGGLE_COUNT)
		return lib3270_get_toggle(GTK_V3270(widget)->host,ix) ? TRUE : FALSE;

	return FALSE;
}

void v3270_set_host(GtkWidget *widget, const gchar *uri)
{
	g_return_if_fail(GTK_IS_V3270(widget));
	g_return_if_fail(uri != NULL);
	lib3270_set_host(GTK_V3270(widget)->host,uri);
}
