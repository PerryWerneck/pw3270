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
 * Este programa está nomeado como pw3270.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

#ifndef PW3270_H_INCLUDED

 #include <gtk/gtk.h>
 #include <lib3270/config.h>
 #include <lib3270.h>

 #define PW3270_H_INCLUDED 1

 // pw3270 window
 G_BEGIN_DECLS

 #define GTK_TYPE_PW3270				(pw3270_get_type ())
 #define GTK_PW3270(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_PW3270, pw3270))
 #define GTK_PW3270_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_PW3270, pw3270Class))
 #define GTK_IS_PW3270(obj)				(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_PW3270))
 #define GTK_IS_PW3270_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_PW3270))
 #define GTK_PW3270_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_PW3270, pw3270Class))

 typedef struct _pw3270				pw3270;
 typedef struct _pw3270Class		pw3270Class;


 LIB3270_EXPORT GtkWidget	* pw3270_new(const gchar *host, const gchar *systype, unsigned short colors);
 LIB3270_EXPORT const gchar * pw3270_set_host(GtkWidget *widget, const gchar *uri);
 LIB3270_EXPORT const gchar	* pw3270_get_hostname(GtkWidget *widget);
 LIB3270_EXPORT void		  pw3270_connect_host(GtkWidget *widget, const gchar *uri);

 LIB3270_EXPORT gboolean 	  pw3270_get_toggle(GtkWidget *widget, LIB3270_TOGGLE ix);
 LIB3270_EXPORT gboolean	  pw3270_set_toggle_by_name(GtkWidget *widget, const gchar *name, gboolean flag);
 LIB3270_EXPORT	H3270		* pw3270_get_session(GtkWidget *widget);
 LIB3270_EXPORT	GtkWidget	* pw3270_get_terminal_widget(GtkWidget *widget);

 LIB3270_EXPORT GtkWidget	* pw3270_get_toplevel(void);

 LIB3270_EXPORT gchar		* pw3270_build_filename(GtkWidget *widget, const gchar *first_element, ...);
 LIB3270_EXPORT void		  pw3270_save_window_size(GtkWidget *widget, const gchar *name);
 LIB3270_EXPORT void		  pw3270_restore_window(GtkWidget *widget, const gchar *name);

 LIB3270_EXPORT const gchar	* pw3270_get_session_name(GtkWidget *widget);
 LIB3270_EXPORT void		  pw3270_set_session_name(GtkWidget *widget, const gchar *name);
 LIB3270_EXPORT void		  pw3270_set_session_options(GtkWidget *widget, LIB3270_OPTION options);
 LIB3270_EXPORT int			  pw3270_set_session_color_type(GtkWidget *widget, unsigned short color_type);

 LIB3270_EXPORT gchar		* pw3270_get_filename(GtkWidget *widget, const gchar *group, const gchar *key, GtkFileFilter **filter, const gchar *title);

 LIB3270_EXPORT gchar 		* pw3270_get_string(GtkWidget *widget, const gchar *group, const gchar *key, const gchar *def);
 LIB3270_EXPORT void 		  pw3270_set_string(GtkWidget *widget, const gchar *group, const gchar *key, const gchar *val);

 LIB3270_EXPORT gint		  pw3270_get_integer(GtkWidget *widget, const gchar *group, const gchar *key, gint def);
 LIB3270_EXPORT void		  pw3270_set_integer(GtkWidget *widget, const gchar *group, const gchar *key, gint val);

 LIB3270_EXPORT gboolean	  pw3270_get_boolean(GtkWidget *widget, const gchar *group, const gchar *key, gboolean def);
 LIB3270_EXPORT void 		  pw3270_set_boolean(GtkWidget *widget, const gchar *group, const gchar *key, gint val);

 LIB3270_EXPORT gchar       * pw3270_get_datadir(const gchar *first_element, ...);

 LIB3270_EXPORT gchar       * pw3270_file_chooser(GtkFileChooserAction action, const gchar *name,  const gchar *title, const gchar *file, const gchar *ext);

 LIB3270_EXPORT	void		  pw3270_set_host_charset(GtkWidget *widget, const gchar *name);

 typedef enum pw3270_src
 {
 	PW3270_SRC_ALL,			/**< Screen contents */
 	PW3270_SRC_SELECTED,	/**< Selected region */
	PW3270_SRC_COPY,		/**< Copy buffer */

 	PW3270_SRC_USER
 } PW3270_SRC;

 LIB3270_EXPORT int 		  pw3270_print(GtkWidget *widget, GObject *action, GtkPrintOperationAction oper, PW3270_SRC src);

#ifdef HAVE_GTKMAC
	#include <gtk-mac-bundle.h>
	LIB3270_EXPORT GtkMacBundle	* pw3270_get_bundle(void);
#endif

 G_END_DECLS

#endif // PW3270_H_INCLUDED
