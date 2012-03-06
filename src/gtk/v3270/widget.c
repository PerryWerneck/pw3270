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
 #include "v3270.h"
 #include "private.h"
 #include "marshal.h"
 #include "../common/common.h"

 #define WIDTH_IN_PIXELS(terminal,x) (x * cols)
 #define HEIGHT_IN_PIXELS(terminal,x) (x * (rows+1))

 #define CONTENTS_WIDTH(terminal) (cols * terminal->metrics.width)
 #define CONTENTS_HEIGHT(terminal) (((rows+1) * terminal->metrics.spacing)+OIA_TOP_MARGIN+2)


/*
 * http://gnomejournal.org/article/34/writing-a-widget-using-cairo-and-gtk28
 * http://developer.gnome.org/gtk3/3.3/ch25s02.html
 * http://zetcode.com/tutorials/cairographicstutorial/cairotext
 */

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 G_DEFINE_TYPE(v3270, v3270, GTK_TYPE_WIDGET);

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 guint				  v3270_widget_signal[LAST_SIGNAL]	= { 0 };
 static GdkCursor	* v3270_cursor[LIB3270_CURSOR_USER]	= { 0 };

/*--[ Prototipes ]-----------------------------------------------------------------------------------*/

 // http://git.gnome.org/browse/gtk+/tree/gtk/gtkdrawingarea.c?h=gtk-3-0

static void v3270_realize			(GtkWidget		* widget);
static void v3270_size_allocate		(GtkWidget		* widget,
									 GtkAllocation	* allocation);
static void v3270_send_configure	(v3270			* terminal);

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

	default:
		return;

	}

}

static void v3270_class_init(v3270Class *klass)
{
	GObjectClass	* gobject_class	= G_OBJECT_CLASS(klass);
	GtkWidgetClass	* widget_class	= GTK_WIDGET_CLASS(klass);

	widget_class->realize 				= v3270_realize;
	widget_class->size_allocate			= v3270_size_allocate;
	widget_class->key_press_event		= v3270_key_press_event;
	widget_class->key_release_event		= v3270_key_release_event;
	widget_class->focus_in_event		= v3270_focus_in_event;
	widget_class->focus_out_event		= v3270_focus_out_event;
	widget_class->button_press_event	= v3270_button_press_event;
	widget_class->button_release_event	= v3270_button_release_event;

	klass->activate						= v3270_activate;
	klass->toggle_changed 				= v3270_toggle_changed;
	klass->message_changed 				= v3270_update_message;
	klass->luname_changed				= v3270_update_luname;

#if GTK_CHECK_VERSION(3,0,0)

	widget_class->destroy 				= v3270_destroy;
	widget_class->draw 					= v3270_draw;

#else

	{
		GtkObjectClass *object_class = (GtkObjectClass*) klass;

		object_class->destroy = v3270_destroy;
	}

	widget_class->expose_event = v3270_expose;


#endif // GTK3

	v3270_register_io_handlers(klass);


	// Cursors
	v3270_cursor[LIB3270_CURSOR_NORMAL] 	= gdk_cursor_new(GDK_XTERM);
	v3270_cursor[LIB3270_CURSOR_WAITING]	= gdk_cursor_new(GDK_WATCH);
	v3270_cursor[LIB3270_CURSOR_LOCKED]		= gdk_cursor_new(GDK_X_CURSOR);

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
						pw3270_VOID__VOID,
						G_TYPE_NONE, 0);

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

}

void v3270_update_font_metrics(v3270 *terminal, cairo_t *cr, int width, int height)
{
	// update font metrics
 	static const int font_size[] = { 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 18, 20, 22, 24, 26, 28, 32, 36, 40, 48, 56, 64, 72, 0 };
	int f, rows, cols, hFont;
	int size = font_size[0];

	cairo_font_extents_t extents;

	lib3270_get_screen_size(terminal->host,&rows,&cols);

	cairo_select_font_face(cr, terminal->font_family, CAIRO_FONT_SLANT_NORMAL,terminal->font_weight);

 	for(f=0;font_size[f];f++)
 	{
        cairo_set_font_size(cr,font_size[f]);
        cairo_font_extents(cr,&extents);

		if( HEIGHT_IN_PIXELS(terminal,(extents.height+extents.descent)) < height && WIDTH_IN_PIXELS(terminal,extents.max_x_advance) < width )
			size = font_size[f];
 	}

	cairo_set_font_size(cr,size);


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

static void changed(H3270 *session, int bstart, int bend)
{
//	gtk_widget_queue_draw(GTK_WIDGET(session->widget));
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
		gdk_window_set_cursor(gtk_widget_get_window(widget),v3270_cursor[id]);
	}
}

static void update_connect(H3270 *session, unsigned char connected)
{
	v3270 *widget = GTK_V3270(session->widget);

	if(connected)
	{
		widget->cursor.show |= 2;
		g_signal_emit(GTK_WIDGET(widget), v3270_widget_signal[SIGNAL_CONNECTED], 0);
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

static void v3270_init(v3270 *widget)
{
	trace("%s",__FUNCTION__);
	widget->host = lib3270_session_new("");
	widget->host->widget = widget;

	widget->host->update			= v3270_update_char;
	widget->host->changed			= changed;
	widget->host->set_timer 		= set_timer;

	widget->host->update_luname		= update_luname;
	widget->host->configure			= update_screen_size;
	widget->host->update_status 	= update_message;
	widget->host->update_cursor 	= v3270_update_cursor;
	widget->host->update_toggle 	= update_toggle;
	widget->host->update_oia		= v3270_update_oia;
	widget->host->cursor			= select_cursor;
	widget->host->update_connect	= update_connect;
	widget->host->update_model		= update_model;

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
 	gchar	**clr;
 	guint	  cnt;
 	int		  f;

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
					"#FFFF00,"			// V3270_COLOR_SELECTED_BORDER

					"#00FF00," 			// V3270_COLOR_CURSOR
					"#00FF00," 			// V3270_COLOR_CROSS_HAIR

					"#000000,"	 		// V3270_COLOR_OIA_BACKGROUND
					"#00FF00,"			// V3270_COLOR_OIA
					"#7890F0,"			// V3270_COLOR_OIA_SEPARATOR
					"#FFFFFF,"			// V3270_COLOR_OIA_STATUS_OK
					"#FF0000";			// V3270_COLOR_OIA_STATUS_INVALID

	}

 	clr = g_strsplit(colors,",",V3270_COLOR_COUNT+1);
 	cnt = g_strv_length(clr);
 	switch(cnt)
 	{
	case 0:
	case 1:
		break;

	case 29:
		for(f=0;f < V3270_COLOR_SELECTED_BORDER;f++)
			v3270_set_color(widget,f,clr[f]);

		v3270_set_color(widget,V3270_COLOR_SELECTED_BORDER,clr[V3270_COLOR_SELECTED_BG]);

		for(f=V3270_COLOR_SELECTED_BORDER+1;f < V3270_COLOR_COUNT;f++)
			v3270_set_color(widget,f,clr[f-1]);

		break;

	case V3270_COLOR_COUNT:	// Complete string
		for(f=0;f < V3270_COLOR_COUNT;f++)
			v3270_set_color(widget,f,clr[f]);
		break;

	default:
		for(f=0;f < cnt;f++)
			v3270_set_color(widget,f,clr[f]);
		for(f=cnt; f < V3270_COLOR_COUNT;f++)
			v3270_set_color(widget,f,clr[cnt-1]);

		v3270_set_color(widget,V3270_COLOR_OIA_BACKGROUND,clr[0]);
		v3270_set_color(widget,V3270_COLOR_SELECTED_BG,clr[0]);

 	}

	g_strfreev(clr);

	g_signal_emit(widget,v3270_widget_signal[SIGNAL_UPDATE_CONFIG], 0, "colors", colors);

	v3270_reload(widget);
}

void v3270_set_color(GtkWidget *widget, enum V3270_COLOR id, const gchar *name)
{
	v3270 * terminal = GTK_V3270(widget);

	if(id >= V3270_COLOR_COUNT)
		return;

	gdk_color_parse(name,terminal->color+id);

#if(GTK_CHECK_VERSION(3,0,0))

#else
	gdk_colormap_alloc_color(gtk_widget_get_default_colormap(),terminal->color+id,TRUE,TRUE);
#endif

}

void v3270_set_font_family(GtkWidget *widget, const gchar *name)
{
	v3270 * terminal;

	g_return_if_fail(GTK_IS_V3270(widget));

	terminal = GTK_V3270(widget);

	if(terminal->font_family)
		g_free(terminal->font_family);

	if(!name)
	{
		// TODO (perry#3#): Get default font family from currrent style
		name = "courier new";
	}

	terminal->font_family = g_strdup(name);
	terminal->font_weight = CAIRO_FONT_WEIGHT_NORMAL;

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

	g_return_if_fail(GTK_IS_V3270(widget));

	terminal = GTK_V3270(widget);

	if(host)
	{
		set_string_to_config("host","uri","%s",host);
		rc = lib3270_connect(terminal->host,host,0);
	}
	else
	{
		gchar *hs = get_string_from_config("host","uri","");

		trace("[%s]",hs);

		if(*hs)
			rc = lib3270_connect(terminal->host,hs,0);

		g_free(hs);
	}

	trace("%s exits with rc=%d (%s)",__FUNCTION__,rc,strerror(rc));

	return rc;
}

gboolean v3270_focus_in_event(GtkWidget *widget, GdkEventFocus *event)
{
	v3270 * terminal = GTK_V3270(widget);

	gtk_im_context_focus_in(terminal->input_method);

	return 0;
}

gboolean v3270_focus_out_event(GtkWidget *widget, GdkEventFocus *event)
{
	v3270 * terminal = GTK_V3270(widget);

	gtk_im_context_focus_out(terminal->input_method);

	return 0;
}

static void v3270_activate(GtkWidget *widget)
{
	v3270 * terminal = GTK_V3270(widget);

	trace("%s: %p",__FUNCTION__,terminal);

	if(lib3270_connected(terminal->host))
		lib3270_enter(terminal->host);
	else
		v3270_connect(widget,NULL);
}
