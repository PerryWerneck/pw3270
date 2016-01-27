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
 * Este programa está nomeado como private.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#include <gtk/gtk.h>

#define ENABLE_NLS
#define GETTEXT_PACKAGE PACKAGE_NAME

#include <libintl.h>
#include <glib/gi18n.h>

#ifndef V3270_H_INCLUDED
	#include <pw3270/v3270.h>
#endif

G_BEGIN_DECLS

 struct _v3270Class
 {
	GtkWidgetClass parent_class;

	/* Signals */
	void 		(*activate)(GtkWidget *widget);
	void 		(*toggle_changed)(v3270 *widget,LIB3270_TOGGLE toggle_id,gboolean toggle_state,const gchar *toggle_name);
	void 		(*message_changed)(v3270 *widget, LIB3270_MESSAGE id);
	void 		(*popup_message)(GtkWidget *widget, LIB3270_NOTIFY id , const gchar *title, const gchar *message, const gchar *text);
	gboolean	(*keypress)(GtkWidget *widget,guint keyval,GdkModifierType state);

 };

/*--[ Defines]---------------------------------------------------------------------------------------*/

 #define OIA_TOP_MARGIN 2

 #define KEY_FLAG_SHIFT	0x0001

 #ifndef WIN32
  #define KEY_FLAG_ALT	0x0002
 #endif // !WIN32

 enum
 {
 	SIGNAL_TOGGLE_CHANGED,
 	SIGNAL_MESSAGE_CHANGED,
 	SIGNAL_KEYPRESS,
 	SIGNAL_CONNECTED,
 	SIGNAL_DISCONNECTED,
 	SIGNAL_UPDATE_CONFIG,
 	SIGNAL_MODEL_CHANGED,
 	SIGNAL_SELECTING,
 	SIGNAL_POPUP,
 	SIGNAL_PASTENEXT,
 	SIGNAL_CLIPBOARD,
 	SIGNAL_CHANGED,
 	SIGNAL_MESSAGE,
 	SIGNAL_FIELD,
 	SIGNAL_PRINT,

 	LAST_SIGNAL
 };


/*--[ Globals ]--------------------------------------------------------------------------------------*/

 #define V3270_CURSOR_UNPROTECTED				LIB3270_CURSOR_EDITABLE
 #define V3270_CURSOR_WAITING					LIB3270_CURSOR_WAITING
 #define V3270_CURSOR_LOCKED					LIB3270_CURSOR_LOCKED

 #define V3270_CURSOR_PROTECTED					LIB3270_CURSOR_USER
 #define V3270_CURSOR_MOVE_SELECTION			LIB3270_CURSOR_USER+1
 #define V3270_CURSOR_SELECTION_TOP_LEFT		LIB3270_CURSOR_USER+2
 #define V3270_CURSOR_SELECTION_TOP_RIGHT		LIB3270_CURSOR_USER+3
 #define V3270_CURSOR_SELECTION_TOP				LIB3270_CURSOR_USER+4
 #define V3270_CURSOR_SELECTION_BOTTOM_LEFT		LIB3270_CURSOR_USER+5
 #define V3270_CURSOR_SELECTION_BOTTOM_RIGHT	LIB3270_CURSOR_USER+6
 #define V3270_CURSOR_SELECTION_BOTTOM			LIB3270_CURSOR_USER+7
 #define V3270_CURSOR_SELECTION_LEFT			LIB3270_CURSOR_USER+8
 #define V3270_CURSOR_SELECTION_RIGHT			LIB3270_CURSOR_USER+9
 #define V3270_CURSOR_QUESTION					LIB3270_CURSOR_USER+10

 #define V3270_CURSOR_COUNT						LIB3270_CURSOR_USER+11


 struct v3270_ssl_status_msg
 {
	long			  id;
	const gchar		* icon;
	const gchar		* text;
	const gchar		* message;
 };

/*--[ Widget data ]----------------------------------------------------------------------------------*/

 struct _v3270
 {
	GtkWidget parent;

	// flags
	int selecting		: 1;	/**< Selecting region */
	int moving			: 1;	/**< Moving selected region */
	int resizing		: 1;	/**< Resizing selected region */
	int table			: 1;	/**< Copy mode is table */
	int scaled_fonts	: 1;	/**< Use scaled fonts */
	int drawing			: 1;	/**< Draw widget? */

#if GTK_CHECK_VERSION(3,0,0)

#else
    gint width;
    gint height;
#endif // GTK_CHECK_VERSION(3,0,0)

	GSource					* timer;
	GtkIMContext			* input_method;
	unsigned short			  keyflags;

	struct
	{
	    char                * text;                 /**< Clipboard contents (lib3270 charset) */
        int                   baddr;	        	/**< Selection addr */
	} selection;

	LIB3270_CURSOR 			  pointer_id;
	unsigned char			  pointer;				/**< Mouse pointer ID */

	V3270_OIA_FIELD			  selected_field;		/**< Clicked OIA field */

	// Font info
	cairo_surface_t			* surface;
	v3270FontInfo			  font;

	gint     				  minimum_width;
	gint					  minimum_height;

	// Colors
	GdkRGBA					  color[V3270_COLOR_COUNT];	/**< Terminal widget colors */

	// Regions
	GdkRectangle			  oia_rect[V3270_OIA_FIELD_COUNT];

	struct
	{
		unsigned char 		  show;							/**< Cursor flag */
		unsigned char 		  chr;							/**< Char at cursor position */
		unsigned short 		  attr;							/**< Attribute at cursor position */
		GdkRectangle		  rect;							/**< Cursor rectangle */
		GSource				* timer;						/**< Cursor blinking timer */
		cairo_surface_t		* surface;						/**< Cursor image */
	} cursor;

	// Acessibility
	GtkAccessible			* accessible;

	// Session
	H3270   				* host;							/**< Related 3270 session */
	gchar					* session_name;					/**< Session name (for window title) */

	// Auto disconnect
	struct
	{
		time_t					  timestamp;				/**< Last action in this widget */
		guint					  disconnect;				/**< Time (in minutes) for auto disconnect */
		GSource					* timer;					/**< Auto disconnect timer */
	} activity;

	// Scripting
	struct
	{
		int					  blink : 1;
		gchar				  id;						/**< Script indicator */
		GSource				* timer;
	} script;

 };

/*--[ Properties ]-----------------------------------------------------------------------------------*/

 enum
 {
	PROP_0,

	/* Construct */
	PROP_TYPE,


	/* Widget properties */
	PROP_ONLINE,
	PROP_SELECTION,
	PROP_MODEL,
	PROP_LUNAME,
	PROP_AUTO_DISCONNECT,

	/* Toggles - always the last one, the real values are PROP_TOGGLE+LIB3270_TOGGLE */
	PROP_TOGGLE
 };

 #define PROP_LAST (PROP_TOGGLE+LIB3270_TOGGLE_COUNT)


/*--[ Globals ]--------------------------------------------------------------------------------------*/

 G_GNUC_INTERNAL guint		  v3270_widget_signal[LAST_SIGNAL];
 G_GNUC_INTERNAL GdkCursor	* v3270_cursor[V3270_CURSOR_COUNT];
 G_GNUC_INTERNAL GParamSpec	* v3270_properties[PROP_LAST];

/*--[ Prototipes ]-----------------------------------------------------------------------------------*/

const GtkWidgetClass	* v3270_get_parent_class(void);

gboolean	  v3270_draw(GtkWidget * widget, cairo_t * cr);
void 		  v3270_draw_oia(cairo_t *cr, H3270 *host, int row, int cols, v3270FontInfo *metrics, GdkRGBA *color, GdkRectangle *rect);
void		  v3270_update_mouse_pointer(GtkWidget *widget);

#if ! GTK_CHECK_VERSION(2,18,0)
	G_GNUC_INTERNAL void gtk_widget_get_allocation(GtkWidget *widget,GtkAllocation *allocation);
#endif // !GTK(2,18)

#if ! GTK_CHECK_VERSION(2,20,0)
	#define gtk_widget_get_realized(w)		GTK_WIDGET_REALIZED(w)
	#define gtk_widget_set_realized(w,r)	if(r) { GTK_WIDGET_SET_FLAGS(w,GTK_REALIZED); } else { GTK_WIDGET_UNSET_FLAGS(w,GTK_REALIZED); }
#endif // !GTK(2,20)

#if ! GTK_CHECK_VERSION(2,22,0)
	#define gtk_accessible_set_widget(a,w)	g_object_set_data(G_OBJECT(a),"widget",w)
	#define gtk_accessible_get_widget(a)	GTK_WIDGET(g_object_get_data(G_OBJECT(a),"widget"))

	G_GNUC_INTERNAL cairo_surface_t * gdk_window_create_similar_surface(GdkWindow *window, cairo_content_t content, int width, int height);

#endif // !GTK(2,22)


#if ! GTK_CHECK_VERSION(3,0,0)
gboolean	  v3270_expose(GtkWidget * widget, GdkEventExpose *event);
#endif // GTK 3

void		  v3270_draw_shift_status(v3270 *terminal);
void		  v3270_draw_alt_status(v3270 *terminal);
void		  v3270_draw_ins_status(v3270 *terminal);

void		  v3270_clear_clipboard(v3270 *terminal);

void		  v3270_update_cursor_surface(v3270 *widget,unsigned char chr,unsigned short attr);

void		  v3270_register_io_handlers(v3270Class *cls);

void 		  v3270_draw_char(cairo_t *cr, unsigned char chr, unsigned short attr, H3270 *session, v3270FontInfo *font, GdkRectangle *rect, GdkRGBA *fg, GdkRGBA *bg);
void		  v3270_draw_text(cairo_t *cr, const GdkRectangle *rect, v3270FontInfo *font, const char *str);

void		  v3270_start_timer(GtkWidget *terminal);
void		  v3270_stop_timer(GtkWidget *terminal);

void		  v3270_draw_connection(cairo_t *cr, H3270 *host, v3270FontInfo *metrics, GdkRGBA *color, const GdkRectangle *rect);
void		  v3270_draw_ssl_status(cairo_t *cr, H3270 *host, v3270FontInfo *metrics, GdkRGBA *color, GdkRectangle *rect);

void		  v3270_update_char(H3270 *session, int addr, unsigned char chr, unsigned short attr, unsigned char cursor);

void		  v3270_update_font_metrics(v3270 *terminal, cairo_t *cr, int width, int height);

void		  v3270_update_cursor_rect(v3270 *widget, GdkRectangle *rect, unsigned char chr, unsigned short attr);

void		  v3270_update_message(v3270 *widget, LIB3270_MESSAGE id);
void		  v3270_update_cursor(H3270 *session, unsigned short row, unsigned short col, unsigned char c, unsigned short attr);
void		  v3270_update_oia(H3270 *session, LIB3270_FLAG id, unsigned char on);
void		  v3270_update_ssl(H3270 *session, LIB3270_SSL_STATE state);

G_GNUC_INTERNAL void v3270_update_luname(GtkWidget *widget,const gchar *name);
G_GNUC_INTERNAL void v3270_init_properties(GObjectClass * gobject_class);
G_GNUC_INTERNAL	void v3270_queue_draw_area(GtkWidget *widget, gint x, gint y, gint width, gint height);

G_GNUC_INTERNAL void v3270_disable_updates(GtkWidget *widget);
G_GNUC_INTERNAL void v3270_enable_updates(GtkWidget *widget);

// Keyboard & Mouse
gboolean	  v3270_key_press_event(GtkWidget *widget, GdkEventKey *event);
gboolean	  v3270_key_release_event(GtkWidget *widget, GdkEventKey *event);
void 	 	  v3270_key_commit(GtkIMContext *imcontext, gchar *str, v3270 *widget);
gboolean	  v3270_button_press_event(GtkWidget *widget, GdkEventButton *event);
gboolean	  v3270_button_release_event(GtkWidget *widget, GdkEventButton*event);
gboolean	  v3270_motion_notify_event(GtkWidget *widget, GdkEventMotion *event);
void		  v3270_emit_popup(v3270 *widget, int baddr, GdkEventButton *event);
gint 		  v3270_get_offset_at_point(v3270 *widget, gint x, gint y);
gboolean	  v3270_scroll_event(GtkWidget *widget, GdkEventScroll *event);

G_GNUC_INTERNAL const struct v3270_ssl_status_msg * v3270_get_ssl_status_msg(GtkWidget *widget);


G_END_DECLS
