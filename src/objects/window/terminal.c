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
 * Este programa está nomeado como - e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "private.h"

 #include <glib.h>
 #include <glib/gstdio.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <stdlib.h>

 #include <pw3270/actions.h>
 #include <lib3270/toggle.h>
 #include <v3270/settings.h>
 #include <v3270/actions.h>
 #include <v3270/print.h>

 struct SessionDescriptor
 {
 	gboolean	  changed;		///< @brief Save file?
	GKeyFile	* key_file;
	gchar		  filename[1];
 };

 static void destroy(GtkWidget G_GNUC_UNUSED(*terminal), struct SessionDescriptor * session) {

	if(session->changed) {

		session->changed = FALSE;

        GError * error = NULL;
        g_key_file_save_to_file(session->key_file,session->filename,&error);

        if(error) {

			g_warning("Can't save \"%s\": %s",session->filename,error->message);
			g_error_free(error);

        } else {

			g_message("Session was saved to %s",session->filename);

        }

	}

 }

 static void save_settings(GtkWidget *terminal, struct SessionDescriptor * session) {

	session->changed = FALSE;

	debug("%s(%p,%p)",__FUNCTION__,terminal,session);

	v3270_to_key_file(terminal,session->key_file,"terminal");
	v3270_accelerator_map_to_key_file(terminal, session->key_file, "accelerators");

	/*
	GtkWidget * window = gtk_widget_get_toplevel(terminal);

	if(PW3270_IS_APPLICATION_WINDOW(window) && pw3270_application_window_get_active_terminal(window) == terminal) {

		debug("%s on active terminal, saving window settings",__FUNCTION__);
		GList * keypad = pw3270_application_window_get_keypads(window);

		while(keypad) {

			g_key_file_set_boolean(
				session->key_file,
				"keypads",
				gtk_widget_get_name(GTK_WIDGET(keypad->data)),
				gtk_widget_get_visible(GTK_WIDGET(keypad->data))
			);
			keypad = g_list_next(keypad);

		}

	}
	*/

	g_key_file_save_to_file(session->key_file,session->filename,NULL);

 }

 static void toggle_changed(G_GNUC_UNUSED GtkWidget *widget, G_GNUC_UNUSED LIB3270_TOGGLE_ID toggle_id, gboolean toggle_state, const gchar *toggle_name, struct SessionDescriptor * session) {
	debug("%s(%s)=%s",__FUNCTION__,toggle_name,toggle_state ? "ON" : "OFF");
	g_key_file_set_boolean(session->key_file,"terminal",toggle_name,toggle_state);
	session->changed = TRUE;
 }

 static void print_done(G_GNUC_UNUSED GtkWidget *widget, GtkPrintOperation *operation, GtkPrintOperationResult result, struct SessionDescriptor * session) {
 	debug("%s(%u)",__FUNCTION__,(unsigned int) result);

 	if(result != GTK_PRINT_OPERATION_RESULT_APPLY)
		return;

	debug("%s: Saving print settings",__FUNCTION__);

	v3270_print_operation_to_key_file(operation,session->key_file);

	g_key_file_save_to_file(session->key_file,session->filename,NULL);
	session->changed = FALSE;
 }

 static void print_setup(G_GNUC_UNUSED GtkWidget *widget, GtkPrintOperation *operation, struct SessionDescriptor * session) {

 	debug("%s(%p)",__FUNCTION__,operation);
	v3270_print_operation_load_key_file(operation,session->key_file);

 }

 static void close_settings(struct SessionDescriptor * session) {

 	if(session->key_file) {

		if(session->changed) {
			g_message("Saving file %s",session->filename);
			g_key_file_save_to_file(session->key_file,session->filename,NULL);
			session->changed = FALSE;
		} else {
			g_message("Closing file %s",session->filename);
		}

		g_key_file_free(session->key_file);
		session->key_file = NULL;
 	}

 	g_free(session);
 }

 const gchar * v3270_get_session_filename(GtkWidget *widget) {

 	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

	const struct SessionDescriptor * descriptor = (const struct SessionDescriptor *) g_object_get_data(G_OBJECT(widget),"session-descriptor");

	if(descriptor)
		return descriptor->filename;

	return NULL;
 }

 void v3270_set_session_filename(GtkWidget *terminal, const gchar *filename) {

 	struct SessionDescriptor * old_session = (struct SessionDescriptor *) g_object_get_data(G_OBJECT(terminal),"session-descriptor");
	struct SessionDescriptor * new_session = (struct SessionDescriptor *) g_malloc0(sizeof(struct SessionDescriptor) + strlen(filename));

	if(old_session) {
		memcpy(new_session,old_session,sizeof(struct SessionDescriptor));
	}

	strcpy(new_session->filename,filename);
	new_session->key_file = g_key_file_new();

	v3270_to_key_file(terminal,new_session->key_file,NULL);
	v3270_accelerator_map_to_key_file(terminal,new_session->key_file,NULL);

	GError *error = NULL;
	g_key_file_save_to_file(new_session->key_file,new_session->filename,&error);

	if(error) {

		g_message("Can't save file \"%s\": %s",new_session->filename,error->message);

		GtkWidget * dialog = gtk_message_dialog_new_with_markup(
								GTK_WINDOW(gtk_widget_get_toplevel(terminal)),
								GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
								GTK_MESSAGE_ERROR,
								GTK_BUTTONS_CANCEL,
								_("Can't save file \"%s\""),new_session->filename
							);

		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s",error->message);

		gtk_window_set_title(GTK_WINDOW(dialog),_("Can't save session file"));

		gtk_widget_show_all(dialog);

		g_signal_connect(dialog,"close",G_CALLBACK(gtk_widget_destroy),NULL);
		g_signal_connect(dialog,"response",G_CALLBACK(gtk_widget_destroy),NULL);

		g_error_free(error);
		g_key_file_free(new_session->key_file);
		g_free(new_session);

	} else {

		new_session->changed = FALSE;
		g_object_set_data_full(G_OBJECT(terminal),"session-descriptor",new_session,(GDestroyNotify) close_settings);

	}


 }

 GKeyFile * v3270_get_session_keyfile(GtkWidget *widget) {

 	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

	const struct SessionDescriptor * descriptor = (const struct SessionDescriptor *) g_object_get_data(G_OBJECT(widget),"session-descriptor");

	if(descriptor)
		return descriptor->key_file;

	return NULL;
 }

 GtkWidget * pw3270_terminal_new(const gchar *session_file) {

 	GtkWidget * terminal = v3270_new();

 	gtk_widget_show_all(terminal);

 	struct SessionDescriptor * descriptor = NULL;

 	if(session_file) {

		// Use the supplied session file
		descriptor = g_malloc0(sizeof(struct SessionDescriptor) + strlen(session_file));
		strcpy(descriptor->filename,session_file);

 	} else {

		// No session file, use the default one.
		g_autofree gchar * filename = g_build_filename(g_get_user_config_dir(),G_STRINGIFY(PRODUCT_NAME) ".conf",NULL);

		descriptor = g_malloc0(sizeof(struct SessionDescriptor) + strlen(filename));
		strcpy(descriptor->filename,filename);

 	}

 	// Setup session file;
 	GError *error = NULL;
	g_object_set_data_full(G_OBJECT(terminal),"session-descriptor",descriptor,(GDestroyNotify) close_settings);

	descriptor->key_file = g_key_file_new();

	if(g_file_test(descriptor->filename,G_FILE_TEST_IS_REGULAR)) {

		// Found session file, open it.
        if(!g_key_file_load_from_file(descriptor->key_file,descriptor->filename,G_KEY_FILE_NONE,&error)) {
			g_warning("Can't load \"%s\"",descriptor->filename);
        } else {
			g_message("Loading session properties from %s",descriptor->filename);
        }

	} else {

		// No session file, load the defaults (if available).
		lib3270_autoptr(char) default_settings = lib3270_build_data_filename("defaults.conf",NULL);
		if(g_file_test(default_settings,G_FILE_TEST_IS_REGULAR)) {
			if(!g_key_file_load_from_file(descriptor->key_file,default_settings,G_KEY_FILE_NONE,&error)) {
				g_warning("Can't load \"%s\"",default_settings);
			} else {
				g_message("Loading session properties from %s",default_settings);
			}
		} else {
#ifdef DEBUG
			g_message("Can't find default settings file \"%s\"",default_settings);
#else
			g_warning("Can't find default settings file \"%s\"",default_settings);
#endif // DEBUG
		}

	}

	if(error) {

		g_warning(error->message);
		g_error_free(error);
		error = NULL;

	} else {

		// Got key file, load it.
		v3270_load_key_file(terminal,descriptor->key_file,NULL);
		v3270_accelerator_map_load_key_file(terminal,descriptor->key_file,NULL);

		if(g_key_file_has_group(descriptor->key_file,"environment")) {

			// Has environment group, set values.
			gchar **keys = g_key_file_get_keys(descriptor->key_file,"environment",NULL,NULL);

			if(keys) {
				size_t ix;
				for(ix=0;keys[ix];ix++) {
					g_autofree gchar * value = g_key_file_get_string(descriptor->key_file,"environment",keys[ix],NULL);
					if(value) {
#ifdef _WIN32
						g_autofree gchar * env = g_strconcat(keys[ix],"=",value,NULL);
						putenv(env);
#else
						if(setenv(keys[ix],value,1)) {
							g_warning("Can't set \"%s\" to \"%s\"",keys[ix],value);
						}
#endif // _WIN32
					}
				}

				g_strfreev(keys);
			}
		}
	}

 	// Setup signals.
 	g_signal_connect(G_OBJECT(terminal),"save-settings",G_CALLBACK(save_settings),descriptor);
 	g_signal_connect(G_OBJECT(terminal),"toggle_changed",G_CALLBACK(toggle_changed),descriptor);
 	g_signal_connect(G_OBJECT(terminal),"print-done",G_CALLBACK(print_done),descriptor);
 	g_signal_connect(G_OBJECT(terminal),"print-setup",G_CALLBACK(print_setup),descriptor);
	g_signal_connect(G_OBJECT(terminal),"destroy", G_CALLBACK(destroy),descriptor);

	return terminal;
 }

 GtkWidget * pw3270_application_window_new_tab(GtkWidget *widget, const gchar *session_file) {

	g_return_val_if_fail(PW3270_IS_APPLICATION_WINDOW(widget),NULL);

	GtkWidget * terminal = pw3270_terminal_new(session_file);

	pw3270_window_set_current_page(widget,pw3270_application_window_append_page(widget,terminal));

	return terminal;

 }

 gboolean v3270_allow_custom_settings(GtkWidget *widget) {

#if defined(DEBUG)

	return TRUE;

#else

	const struct SessionDescriptor * descriptor = (const struct SessionDescriptor *) g_object_get_data(G_OBJECT(widget),"session-descriptor");

	if(!(descriptor && *descriptor->filename))
		return FALSE;

	if(g_access(descriptor->filename,W_OK))
		return FALSE;

#ifdef _WIN32
	return TRUE;
#else
	return !g_str_has_prefix(descriptor->filename,g_get_user_config_dir());
#endif // _WIN32

#endif // DEBUG

 }


