/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes paul.mattes@case.edu), de emulação de terminal 3270 para acesso a
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
 * Este programa está nomeado como - e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

/**
 * @brief Declares the pw3270 application.
 *
 */

#ifndef PW3270_APPLICATION_H_INCLUDED

#define PW3270_APPLICATION_H_INCLUDED

#include <gtk/gtk.h>
#include <lib3270.h>

G_BEGIN_DECLS

#define PW3270_TYPE_APPLICATION				(pw3270Application_get_type ())
#define PW3270_APPLICATION(inst)			(G_TYPE_CHECK_INSTANCE_CAST ((inst), \
													PW3270_TYPE_APPLICATION, pw3270Application))
#define PW3270_APPLICATION_CLASS(class)		(G_TYPE_CHECK_CLASS_CAST ((class),   \
													PW3270_TYPE_APPLICATION, pw3270ApplicationClass))
#define PW3270_IS_APPLICATION(inst)			(G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
													PW3270_TYPE_APPLICATION))
#define PW3270_IS_APPLICATION_CLASS(class)	(G_TYPE_CHECK_CLASS_TYPE ((class),   \
													PW3270_TYPE_APPLICATION))
#define PW3270_APPLICATION_GET_CLASS(inst)	(G_TYPE_INSTANCE_GET_CLASS ((inst),  \
												GTK_TYPE_APPLICATION, pw3270ApplicationClass))

typedef enum _pw3270_ui_style {
	PW3270_UI_STYLE_CLASSICAL,		///< @brief Interface "classica", com menu e toolbar.
	PW3270_UI_STYLE_GNOME,			///< @brief Interface padrão gnome.
	PW3270_UI_STYLE_AUTOMATIC,		///< @brief Auto defined UI-Style
} PW3270_UI_STYLE;


typedef struct _pw3270ApplicationClass   pw3270ApplicationClass;
typedef struct _pw3270Application        pw3270Application;

GType			  pw3270Application_get_type();
GtkApplication	* pw3270_application_new(const gchar *application_id, GApplicationFlags flags);
void			  pw3270_application_open_file(GtkApplication *application, GtkWindow **window, GFile *file);

/// @brief Get application settings.
/// @param app	The pw3270 application object.
/// @return The internal settings object (Do not unref it).
GSettings		* pw3270_application_settings_new();
GSettings		* pw3270_application_get_settings(GApplication *app);

/// @brief Get boolean from gsettings.
gboolean		  pw3270_application_get_boolean(GApplication *app, const gchar *option_name, gboolean def);

GList			* pw3270_application_get_keypad_models(GApplication *app);

void			  pw3270_application_set_ui_style(GApplication *app, PW3270_UI_STYLE type);
PW3270_UI_STYLE	  pw3270_application_get_ui_style(GApplication *app);

void			  pw3270_application_set_log_filename(GApplication *app, const gchar *filename);
const gchar		* pw3270_application_get_log_filename(GApplication *app);


// Plugins
void			  pw3270_application_plugin_foreach(GApplication *app, GFunc func, gpointer user_data);

/// @brief Call plugin method.
void			  pw3270_application_plugin_call(GApplication *app, const gchar *method, gpointer user_data);

GSList			* pw3270_application_get_plugins(GApplication *app);

// Tools
GtkBuilder		* pw3270_application_builder_new(GApplication *application);

void			  gtk_container_remove_all(GtkContainer *container);

// Actions
void pw3270_application_print_copy_activated(GAction *action, GVariant *parameter, GtkWidget *terminal);
void pw3270_application_save_copy_activated(GAction *action, GVariant *parameter, GtkWidget *terminal);

// Settings
GtkWidget * pw3270_header_settings_new();

// Tools
H3270			* pw3270_get_active_session();
GtkWidget		* pw3270_get_active_terminal();

G_END_DECLS


#endif // PW3270_WINDOW_H_INCLUDED
