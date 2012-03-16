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
 * Este programa está nomeado como private.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

#include <gtk/gtk.h>

#define ENABLE_NLS
#define GETTEXT_PACKAGE PACKAGE_NAME

#include <libintl.h>
#include <glib/gi18n.h>

#include <lib3270.h>

G_BEGIN_DECLS


 struct _v3270Class
 {
	GtkWidgetClass parent_class;

	/* Signals */
	void 		(*activate)(GtkWidget *widget);
	void 		(*toggle_changed)(v3270 *widget,LIB3270_TOGGLE toggle_id,gboolean toggle_state,const gchar *toggle_name);
	void 		(*message_changed)(v3270 *widget, LIB3270_MESSAGE id);
	void 		(*luname_changed)(GtkWidget *widget,const gchar *luname);
	gboolean	(*keypress)(GtkWidget *widget,guint keyval,GdkModifierType state);

 };

/*--[ Defines]---------------------------------------------------------------------------------------*/

 #define OIA_TOP_MARGIN 2

 #define KEY_FLAG_SHIFT	0x0001
 #define KEY_FLAG_ALT	0x0002

 enum
 {
 	SIGNAL_TOGGLE_CHANGED,
 	SIGNAL_MESSAGE_CHANGED,
 	SIGNAL_LUNAME_CHANGED,
 	SIGNAL_KEYPRESS,
 	SIGNAL_CONNECTED,
 	SIGNAL_DISCONNECTED,
 	SIGNAL_UPDATE_CONFIG,
 	SIGNAL_MODEL_CHANGED,
 	SIGNAL_SELECTING,
 	SIGNAL_POPUP,
 	SIGNAL_PASTENEXT,

 	LAST_SIGNAL
 };

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 G_GNUC_INTERNAL guint v3270_widget_signal[LAST_SIGNAL];

/*--[ Prototipes ]-----------------------------------------------------------------------------------*/

const GtkWidgetClass	* v3270_get_parent_class(void);

gboolean	  v3270_draw(GtkWidget * widget, cairo_t * cr);
void 		  v3270_draw_oia(cairo_t *cr, H3270 *host, int row, int cols, struct v3270_metrics *metrics, GdkColor *color, GdkRectangle *rect);

#if ! GTK_CHECK_VERSION(3,0,0)
 gboolean	  v3270_expose(GtkWidget * widget, GdkEventExpose *event);
#endif // GTK 3

void		  v3270_draw_shift_status(v3270 *terminal);
void		  v3270_draw_alt_status(v3270 *terminal);

void		  v3270_update_cursor_surface(v3270 *widget,unsigned char chr,unsigned short attr);

void		  v3270_register_io_handlers(v3270Class *cls);

void		  v3270_draw_element(cairo_t *cr, unsigned char chr, unsigned short attr, const struct v3270_metrics *metrics, GdkRectangle *rect, GdkColor *color);
void		  v3270_draw_char(cairo_t *cr, unsigned char chr, unsigned short attr, const struct v3270_metrics *metrics, GdkRectangle *rect, GdkColor *fg, GdkColor *bg);

void		  v3270_start_timer(GtkWidget *terminal);
void		  v3270_stop_timer(GtkWidget *terminal);

void		  v3270_draw_connection(cairo_t *cr, H3270 *host, struct v3270_metrics *metrics, GdkColor *color, GdkRectangle *rect);
void		  v3270_draw_ssl_status(cairo_t *cr, H3270 *host, struct v3270_metrics *metrics, GdkColor *color, GdkRectangle *rect);

void		  v3270_reload(GtkWidget * widget);
void		  v3270_update_char(H3270 *session, int addr, unsigned char chr, unsigned short attr, unsigned char cursor);

void		  v3270_update_font_metrics(v3270 *terminal, cairo_t *cr, int width, int height);

void		  v3270_update_cursor_rect(v3270 *widget, GdkRectangle *rect, unsigned char chr, unsigned short attr);

void 		  v3270_update_luname(GtkWidget *widget,const gchar *name);
void		  v3270_update_message(v3270 *widget, LIB3270_MESSAGE id);
void		  v3270_update_cursor(H3270 *session, unsigned short row, unsigned short col, unsigned char c, unsigned short attr);
void		  v3270_update_oia(H3270 *session, LIB3270_FLAG id, unsigned char on);

// Keyboard & Mouse
gboolean	  v3270_key_press_event(GtkWidget *widget, GdkEventKey *event);
gboolean	  v3270_key_release_event(GtkWidget *widget, GdkEventKey *event);
void 	 	  v3270_key_commit(GtkIMContext *imcontext, gchar *str, v3270 *widget);
gboolean	  v3270_button_press_event(GtkWidget *widget, GdkEventButton *event);
gboolean	  v3270_button_release_event(GtkWidget *widget, GdkEventButton*event);
gboolean	  v3270_motion_notify_event(GtkWidget *widget, GdkEventMotion *event);
void		  v3270_emit_popup(v3270 *widget, int baddr, GdkEventButton *event);

G_END_DECLS
