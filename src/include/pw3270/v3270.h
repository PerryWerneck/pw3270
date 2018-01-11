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
 * Este programa está nomeado como v3270.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

#ifndef V3270_H_INCLUDED

 #include <gtk/gtk.h>
 #include <lib3270.h>
 #include <lib3270/popup.h>
 #include <lib3270/filetransfer.h>

 #define V3270_H_INCLUDED 1

 G_BEGIN_DECLS

 #define GTK_TYPE_V3270				(v3270_get_type ())
 #define GTK_V3270(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_V3270, v3270))
 #define GTK_V3270_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_V3270, v3270Class))
 #define GTK_IS_V3270(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_V3270))
 #define GTK_IS_V3270_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_V3270))
 #define GTK_V3270_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_V3270, v3270Class))

#if ! GTK_CHECK_VERSION(3,0,0)
    #define GdkRGBA                                     GdkColor
    #define gdk_cairo_set_source_rgba(cr,cl)            gdk_cairo_set_source_color(cr,cl)
    #define gdk_rgba_parse(a,b)                         gdk_color_parse(b,a)
    #define gdk_rgba_to_string(c)                       gdk_color_to_string(c)
    #define gdk_rgba_equal(a,b)                         gdk_color_equal(a,b)
    #define gdk_rgba_to_string(c)                       gdk_color_to_string(c)
    #define gtk_color_selection_set_current_rgba(w,c)   gtk_color_selection_set_current_color(w,c)
    #define gtk_color_selection_get_current_rgba(w,c)   gtk_color_selection_get_current_color(w,c)
	#define gtk_color_selection_set_previous_rgba(w,c)  gtk_color_selection_set_previous_color(w,c)
	#define gtk_color_selection_set_current_rgba(w,c)   gtk_color_selection_set_current_color(w,c)
#endif // !GTK(3,0,0)


 typedef struct _v3270			v3270;
 typedef struct _v3270Class		v3270Class;

 /**
  * @brief Informações para desenho de fontes com o cairo.
  *
  */
 typedef struct _v3270FontInfo {

	guint					  width;
	guint					  height;
	guint					  ascent;
	guint					  descent;

	guint					  spacing;

	guint					  left;
	guint					  top;

	gchar 					* family;
	cairo_font_weight_t		  weight;
	cairo_scaled_font_t		* scaled;

 } v3270FontInfo;

 enum V3270_COLOR
 {
	V3270_COLOR_BACKGROUND,
	V3270_COLOR_BLUE,
	V3270_COLOR_RED,
	V3270_COLOR_PINK,
	V3270_COLOR_GREEN,
	V3270_COLOR_TURQUOISE,
	V3270_COLOR_YELLOW,
	V3270_COLOR_WHITE,
	V3270_COLOR_BLACK,
	V3270_COLOR_DARK_BLUE,
	V3270_COLOR_ORANGE,
	V3270_COLOR_PURPLE,
	V3270_COLOR_DARK_GREEN,
	V3270_COLOR_DARK_TURQUOISE,
	V3270_COLOR_MUSTARD,
	V3270_COLOR_GRAY,

	V3270_COLOR_FIELD,
	V3270_COLOR_FIELD_INTENSIFIED,
	V3270_COLOR_FIELD_PROTECTED,
	V3270_COLOR_FIELD_PROTECTED_INTENSIFIED,

	V3270_COLOR_SELECTED_BG,
	V3270_COLOR_SELECTED_FG,

	V3270_COLOR_CROSS_HAIR,

	// Oia Colors (Must be the last block)
	V3270_COLOR_OIA_BACKGROUND,
	V3270_COLOR_OIA_FOREGROUND,
	V3270_COLOR_OIA_SEPARATOR,
	V3270_COLOR_OIA_STATUS_OK,
	V3270_COLOR_OIA_STATUS_WARNING,
	V3270_COLOR_OIA_STATUS_INVALID,

	V3270_COLOR_COUNT
 };

// #define V3270_COLOR_OIA_STATUS_WARNING V3270_COLOR_OIA_STATUS_OK

 typedef enum _v3270_oia_field
 {
	V3270_OIA_UNDERA,			/**< "A" underlined */
	V3270_OIA_CONNECTION,		/**< solid box if connected, "?" in a box if not */
	V3270_OIA_MESSAGE,			/**< message area */
	V3270_OIA_SSL,				/**< SSL Status */
								/**< Meta indication ("M" or blank) */
	V3270_OIA_ALT,				/**< Alt indication ("A" or blank) */
								/**< Compose indication ("C" or blank) */
								/**< Compose first character */
	V3270_OIA_SHIFT,			/**< Shift Status */
	V3270_OIA_TYPEAHEAD,		/**< Typeahead indication ("T" or blank) */
	V3270_OIA_INSERT,			/**< Insert mode indication (Special symbol/"I" or blank) */
	V3270_OIA_SCRIPT,			/**< Script indication  ("S" or blank) */
	V3270_OIA_LUNAME,			/**< LU Name */
	V3270_OIA_SPINNER,			/**< command timing spinner */
	V3270_OIA_TIMER,			/**< command timing (mmm:ss, or blank) */
	V3270_OIA_CURSOR_POSITION,	/**< cursor position (rrr/ccc or blank) */

//	V3270_OIA_CAPS,				/**< Caps indication ("A" or blank) */

#ifdef HAVE_PRINTER
	V3270_OIA_PRINTER,			/**< Printer indication ("P" or blank) */
#endif // HAVE_PRINTER

	V3270_OIA_FIELD_COUNT

 } V3270_OIA_FIELD;

 #define V3270_OIA_FIELD_INVALID	((V3270_OIA_FIELD) -1)


 #define V3270_COLOR_OIA_SPINNER 	V3270_COLOR_OIA_FOREGROUND
 #define V3270_COLOR_OIA_LUNAME		V3270_COLOR_OIA_FOREGROUND
 #define V3270_COLOR_OIA_INSERT		V3270_COLOR_OIA_FOREGROUND

 #ifndef v3270char
    #define v3270char void
 #endif // v3270_char

 LIB3270_EXPORT	GtkWidget		* v3270_new(void);
 LIB3270_EXPORT	GType 			  v3270_get_type(void);

 LIB3270_EXPORT	void		  	  v3270_reload(GtkWidget * widget);

 LIB3270_EXPORT	void			  v3270_set_font_family(GtkWidget *widget, const gchar *name);
 LIB3270_EXPORT	const gchar		* v3270_get_font_family(GtkWidget *widget);

 LIB3270_EXPORT	H3270			* v3270_get_session(GtkWidget *widget);
 LIB3270_EXPORT gboolean		  v3270_is_connected(GtkWidget *widget);

 LIB3270_EXPORT	int				  v3270_connect(GtkWidget *widget);
 LIB3270_EXPORT	void			  v3270_disconnect(GtkWidget *widget);

 LIB3270_EXPORT	int				  v3270_set_host_charset(GtkWidget *widget, const gchar *name);

 LIB3270_EXPORT	void			  v3270_set_auto_disconnect(GtkWidget *widget, guint minutes);
 LIB3270_EXPORT guint			  v3270_get_auto_disconnect(GtkWidget *widget);


 // Clipboard
 typedef enum _v3270_select_format
 {
	V3270_SELECT_TEXT,
	V3270_SELECT_TABLE,

	V3270_SELECT_MAX
 } V3270_SELECT_FORMAT;

 LIB3270_EXPORT	void              v3270_copy(GtkWidget *widget, V3270_SELECT_FORMAT mode, gboolean cut);
 LIB3270_EXPORT	void              v3270_copy_append(GtkWidget *widget);
 LIB3270_EXPORT	gchar   		* v3270_get_selected(GtkWidget *widget, gboolean cut);
 LIB3270_EXPORT	gchar		    * v3270_get_copy(GtkWidget *widget);
 LIB3270_EXPORT	void              v3270_set_copy(GtkWidget *widget, const gchar *text);

 LIB3270_EXPORT int				  v3270_run_script(GtkWidget *widget, const gchar *script);

 LIB3270_EXPORT	gchar			* v3270_get_text(GtkWidget *widget,int offset, int len);
 LIB3270_EXPORT	gchar			* v3270_get_region(GtkWidget *widget, gint start_pos, gint end_pos, gboolean all);

 LIB3270_EXPORT	void			  v3270_set_string(GtkWidget *widget, const gchar *str);
 LIB3270_EXPORT	void			  v3270_tab(GtkWidget *widget);
 LIB3270_EXPORT	void			  v3270_backtab(GtkWidget *widget);

 // Cut & Paste
 LIB3270_EXPORT	gboolean  		  v3270_get_selection_bounds(GtkWidget *widget, gint *start, gint *end);
 LIB3270_EXPORT	void 			  v3270_unselect(GtkWidget *widget);
 LIB3270_EXPORT void 			  v3270_select_all(GtkWidget *widget);
 LIB3270_EXPORT	void 			  v3270_paste(GtkWidget *widget);
 LIB3270_EXPORT	void 			  v3270_paste_string(GtkWidget *widget, const gchar *text, const gchar *encoding);
 LIB3270_EXPORT	void	  		  v3270_select_region(GtkWidget *widget, gint start, gint end);

 // Colors
 LIB3270_EXPORT	void			  v3270_set_colors(GtkWidget *widget, const gchar *);
 LIB3270_EXPORT	void 			  v3270_set_color_table(GdkRGBA *table, const gchar *colors);
 LIB3270_EXPORT	const GdkRGBA	* v3270_get_color_table(GtkWidget *widget);
 LIB3270_EXPORT	void 			  v3270_set_mono_color_table(GdkRGBA *table, const gchar *fg, const gchar *bg);
 LIB3270_EXPORT	void		  	  v3270_draw_element(cairo_t *cr, unsigned char chr, unsigned short attr, H3270 *session, v3270FontInfo *font, GdkRectangle *rect, GdkRGBA *color);
 LIB3270_EXPORT	void			  v3270_set_color(GtkWidget *widget, enum V3270_COLOR id, GdkRGBA *color);
 LIB3270_EXPORT	GdkRGBA 		* v3270_get_color(GtkWidget *widget, enum V3270_COLOR id);

 // Misc
 LIB3270_EXPORT	GtkIMContext	* v3270_get_im_context(GtkWidget *widget);
 LIB3270_EXPORT	gboolean		  v3270_get_toggle(GtkWidget *widget, LIB3270_TOGGLE ix);
 LIB3270_EXPORT	void			  v3270_popup_message(GtkWidget *widget, LIB3270_NOTIFY type, const gchar *title, const gchar *message, const gchar *text);
 LIB3270_EXPORT	const gchar		* v3270_get_session_name(GtkWidget *widget);
 LIB3270_EXPORT	void			  v3270_set_session_name(GtkWidget *widget, const gchar *name);
 LIB3270_EXPORT int				  v3270_set_script(GtkWidget *widget, const gchar id, gboolean on);
 LIB3270_EXPORT void			  v3270_set_scaled_fonts(GtkWidget *widget, gboolean on);
 LIB3270_EXPORT void			  v3270_set_session_options(GtkWidget *widget, LIB3270_OPTION options);
 LIB3270_EXPORT int				  v3270_set_session_color_type(GtkWidget *widget, unsigned short colortype);
 LIB3270_EXPORT int				  v3270_set_host_type(GtkWidget *widget, const char *name);
 LIB3270_EXPORT	const gchar		* v3270_set_url(GtkWidget *widget, const gchar *uri);
 LIB3270_EXPORT	const gchar		* v3270_get_hostname(GtkWidget *widget);
 LIB3270_EXPORT const char		* v3270_get_luname(GtkWidget *widget);
 LIB3270_EXPORT GtkWidget		* v3270_get_default_widget(void);

 LIB3270_EXPORT	void			  v3270_remap_from_xml(GtkWidget *widget, const gchar *path);

 // Keyboard & Mouse special actions
 LIB3270_EXPORT gboolean		  v3270_set_keyboard_action(GtkWidget *widget, const gchar *key_name, GtkAction *action);
 LIB3270_EXPORT void			  v3270_set_scroll_action(GtkWidget *widget, GdkScrollDirection direction, GtkAction *action);

 // SSL & Security
 LIB3270_EXPORT const gchar		* v3270_get_ssl_status_icon(GtkWidget *widget);
 LIB3270_EXPORT const gchar		* v3270_get_ssl_status_text(GtkWidget *widget);
 LIB3270_EXPORT const gchar		* v3270_get_ssl_status_message(GtkWidget *widget);
 LIB3270_EXPORT	void			  v3270_popup_security_dialog(GtkWidget *widget);


 // File transfer
 LIB3270_EXPORT gint			  v3270_transfer_file(GtkWidget *widget, LIB3270_FT_OPTION options, const gchar *local, const gchar *remote, int lrecl, int blksize, int primspace, int secspace, int dft);

 // Auxiliary widgets
 LIB3270_EXPORT GtkWidget		* v3270_host_select_new(GtkWidget *widget);
 LIB3270_EXPORT void			  v3270_select_host(GtkWidget *widget);


 G_END_DECLS

#endif // V3270_H_INCLUDED
