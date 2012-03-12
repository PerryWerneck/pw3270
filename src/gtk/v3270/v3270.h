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

 #include <lib3270.h>

 #define V3270_H_INCLUDED 1

 G_BEGIN_DECLS

 #define GTK_TYPE_V3270				(v3270_get_type ())
 #define GTK_V3270(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_V3270, v3270))
 #define GTK_V3270_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_V3270, v3270Class))
 #define GTK_IS_V3270(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_V3270))
 #define GTK_IS_V3270_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_V3270))
 #define GTK_V3270_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_V3270, v3270Class))


 typedef struct _v3270			v3270;
 typedef struct _v3270Class		v3270Class;

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
	V3270_COLOR_SELECTED_BORDER,

	V3270_COLOR_CURSOR_BACKGROUND,
	V3270_COLOR_CROSS_HAIR,

	// Oia Colors (Must be the last block)
	V3270_COLOR_OIA_BACKGROUND,
	V3270_COLOR_OIA_FOREGROUND,
	V3270_COLOR_OIA_SEPARATOR,
	V3270_COLOR_OIA_STATUS_OK,
	V3270_COLOR_OIA_STATUS_INVALID,

	V3270_COLOR_COUNT
 };

 #define V3270_COLOR_OIA_STATUS_WARNING V3270_COLOR_OIA_STATUS_OK

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
	V3270_OIA_CAPS,				/**< Caps indication ("A" or blank) */
	V3270_OIA_SHIFT,			/**< Shift Status */
	V3270_OIA_TYPEAHEAD,		/**< Typeahead indication ("T" or blank) */
	V3270_OIA_INSERT,			/**< Insert mode indication (Special symbol/"I" or blank) */
	V3270_OIA_SCRIPT,			/**< Script indication  ("S" or blank) */
	V3270_OIA_PRINTER,			/**< Printer indication ("P" or blank) */
	V3270_OIA_LUNAME,			/**< LU Name */
	V3270_OIA_SPINNER,			/**< command timing spinner */
	V3270_OIA_TIMER,			/**< command timing (mmm:ss, or blank) */
	V3270_OIA_CURSOR_POSITION,	/**< cursor position (rrr/ccc or blank) */

	V3270_OIA_FIELD_COUNT

 } V3270_OIA_FIELD;


 #define V3270_COLOR_OIA_SPINNER 	V3270_COLOR_OIA_FOREGROUND
 #define V3270_COLOR_OIA_LUNAME		V3270_COLOR_OIA_FOREGROUND
 #define V3270_COLOR_OIA_INSERT		V3270_COLOR_OIA_FOREGROUND

 #ifndef v3270char
    #define v3270char void
 #endif // v3270_char

 struct v3270_metrics
 {
	guint width;
	guint height;
	guint ascent;
	guint descent;

	guint spacing;

	guint left;
	guint top;
 };

 struct _v3270
 {
	GtkWidget parent;

	/* private */
	int selecting : 1;
#if GTK_CHECK_VERSION(3,0,0)

#else
    gint width;
    gint height;
#endif // GTK_CHECK_VERSION(3,0,0)

	GSource					* timer;
	GtkIMContext			* input_method;
	unsigned short			  keyflags;

	/* Font info */
	gchar * font_family;
	cairo_font_weight_t		  font_weight;
	cairo_scaled_font_t		* font_scaled;
	cairo_surface_t			* surface;

	struct v3270_metrics	  metrics;

	/* Colors */
	GdkColor				  color[V3270_COLOR_COUNT+1];	/**< Terminal widget colors */

	/* Regions */
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

	/* lib3270 stuff */
	H3270   				* host;							/**< Related 3270 session */

 };

 GtkWidget		* v3270_new(void);
 GType 			  v3270_get_type(void);

 void			  v3270_set_colors(GtkWidget *widget, const gchar *);
 void			  v3270_set_color(GtkWidget *widget, enum V3270_COLOR id, const gchar *name);

 void			  v3270_set_font_family(GtkWidget *widget, const gchar *name);
 const gchar	* v3270_get_font_family(GtkWidget *widget);

 H3270			* v3270_get_session(GtkWidget *widget);

 int			  v3270_connect(GtkWidget *widget, const gchar *host);
 void			  v3270_disconnect(GtkWidget *widget);

 // Clipboard
 gboolean		  v3270_copy(v3270 *widget);

G_END_DECLS

#endif // V3270_H_INCLUDED
