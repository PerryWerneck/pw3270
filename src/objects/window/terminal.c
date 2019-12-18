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

 static void session_changed(GtkWidget *terminal, GtkWidget *label) {

	gtk_label_set_text(GTK_LABEL(label),v3270_get_session_name(terminal));

	// Do I have to change the window title?
	GtkWidget * toplevel = gtk_widget_get_toplevel(terminal);
	if(PW3270_IS_APPLICATION_WINDOW(toplevel)) {

		pw3270ApplicationWindow * window = PW3270_APPLICATION_WINDOW(toplevel);

		if(gtk_widget_has_default(terminal)) {
			g_autofree gchar * title = v3270_get_session_title(terminal);
			gtk_window_set_title(GTK_WINDOW(window), title);
		}

	}

 }

 static gboolean on_terminal_focus(GtkWidget *terminal, GdkEvent G_GNUC_UNUSED(*event), GtkWindow * window) {

	if(gtk_window_get_default_widget(window) == terminal) {
		return FALSE;
	}

 	// Store the active terminal widget.
	gtk_widget_grab_default(terminal);
	debug("Terminal %p is now default",terminal);

	// Change window title
	g_autofree gchar * title = v3270_get_session_title(terminal);
	gtk_window_set_title(window, title);

	pw3270_window_set_subtitle(GTK_WIDGET(window), v3270_is_connected(terminal) ? _("Connected to host") : _("Disconnected from host"));

	// Update actions
	size_t ix;
	gchar ** actions = g_action_group_list_actions(G_ACTION_GROUP(window));

	for(ix = 0; actions[ix]; ix++) {

//		debug("%s",actions[ix]);

		GAction * action = g_action_map_lookup_action(G_ACTION_MAP(window), actions[ix]);

		if(action && PW3270_IS_ACTION(action)) {
			pw3270_action_set_terminal_widget(action,terminal);
		}

	}

	g_strfreev(actions);

 	return FALSE;
 }

 static void check_for_session_changed(GtkWidget *terminal) {

	struct SessionDescriptor * session = (struct SessionDescriptor *) g_object_get_data(G_OBJECT(terminal),"session-descriptor");

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


 static void on_terminal_destroy(GtkWidget *terminal, GtkWindow * window) {

	check_for_session_changed(terminal);

	if(gtk_window_get_default_widget(window) != terminal) {
		return;
	}

	gtk_window_set_default(window,NULL);
	pw3270_window_set_subtitle(GTK_WIDGET(window), _("Disconnected from host"));

	// Update actions
	size_t ix;
	gchar ** actions = g_action_group_list_actions(G_ACTION_GROUP(window));

	for(ix = 0; actions[ix]; ix++) {

		GAction * action = g_action_map_lookup_action(G_ACTION_MAP(window), actions[ix]);

		if(action && PW3270_IS_ACTION(action)) {
			pw3270_action_set_terminal_widget(action,NULL);
		}

	}

	g_strfreev(actions);

 }

 static void disconnected(GtkWidget *terminal, GtkWindow * window) {

 	debug("%s",__FUNCTION__);

 	if(terminal != gtk_window_get_default_widget(window))
		return;

	pw3270_window_set_subtitle(GTK_WIDGET(window), _("Disconnected from host"));

 }

 static void connected(GtkWidget *terminal, const gchar *host, GtkWindow * window) {

 	debug("%s(%s)",__FUNCTION__,host);

 	if(terminal != gtk_window_get_default_widget(window))
		return;

	pw3270_window_set_subtitle(GTK_WIDGET(window), _("Connected to host"));

 }

 static void on_close_tab(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal) {

	GtkNotebook * notebook = GTK_NOTEBOOK(gtk_widget_get_parent(terminal));
	gtk_notebook_remove_page(notebook,gtk_notebook_page_num(notebook, terminal));

 }

 static gboolean on_popup_menu(GtkWidget *widget, gboolean selected, gboolean online, GdkEvent *event, pw3270ApplicationWindow * window) {

	GtkWidget * popup = window->popups[PW3270_APP_WINDOW_POPUP_OVER_UNSELECTED_AREA];

	if(!online && window->popups[PW3270_APP_WINDOW_POPUP_WHEN_OFFLINE])
		popup = window->popups[PW3270_APP_WINDOW_POPUP_WHEN_OFFLINE];
	else if(selected && window->popups[PW3270_APP_WINDOW_POPUP_OVER_SELECTED_AREA])
		popup = window->popups[PW3270_APP_WINDOW_POPUP_OVER_SELECTED_AREA];
	else
		popup = window->popups[PW3270_APP_WINDOW_POPUP_DEFAULT];

	if(!popup)
		return FALSE;

	gtk_widget_show_all(popup);
	gtk_menu_set_screen(GTK_MENU(popup), gtk_widget_get_screen(widget));
	gtk_menu_popup_at_pointer(GTK_MENU(popup), event);

	return TRUE;

 }

 static gint append_terminal_page(pw3270ApplicationWindow * window, GtkWidget * terminal) {

 	GtkWidget * label	= gtk_label_new(v3270_get_session_name(terminal));
 	GtkWidget * tab		= gtk_box_new(GTK_ORIENTATION_HORIZONTAL,2);
 	GtkWidget * button	= gtk_button_new_from_icon_name("window-close-symbolic",GTK_ICON_SIZE_MENU);

 	gtk_button_set_relief(GTK_BUTTON(button),GTK_RELIEF_NONE);

 	debug("notebook: %p", window->notebook);

	g_signal_connect(G_OBJECT(terminal), "focus-in-event", G_CALLBACK(on_terminal_focus), window);
	g_signal_connect(G_OBJECT(terminal), "session_changed", G_CALLBACK(session_changed),label);
	g_signal_connect(G_OBJECT(terminal), "disconnected", G_CALLBACK(disconnected),window);
	g_signal_connect(G_OBJECT(terminal), "connected", G_CALLBACK(connected),window);
	g_signal_connect(G_OBJECT(terminal), "destroy", G_CALLBACK(on_terminal_destroy),window);
	g_signal_connect(G_OBJECT(terminal), "popup", G_CALLBACK(on_popup_menu), window);

	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(on_close_tab), terminal);

 	gtk_box_pack_start(GTK_BOX(tab),label,FALSE,FALSE,0);
 	gtk_box_pack_end(GTK_BOX(tab),button,FALSE,FALSE,0);

	gtk_widget_show_all(terminal);
	gtk_widget_show_all(tab);

	gint page = gtk_notebook_append_page(window->notebook,terminal,tab);

	gtk_notebook_set_tab_detachable(window->notebook,terminal,TRUE);
	gtk_notebook_set_tab_reorderable(window->notebook,terminal,TRUE);

	// Setup session.

//	H3270 * hSession = v3270_get_session(terminal);

	return page;

 }

 static void save_settings(GtkWidget *terminal, struct SessionDescriptor * session) {

	session->changed = FALSE;

	v3270_to_key_file(terminal,session->key_file,"terminal");
	v3270_accelerator_map_to_key_file(terminal, session->key_file, "accelerators");

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

 GtkWidget * pw3270_terminal_new(GtkWidget *widget, const gchar *session_file) {

	struct SessionDescriptor * descriptor;

	g_return_val_if_fail(PW3270_IS_APPLICATION_WINDOW(widget),NULL);

	pw3270ApplicationWindow * window = PW3270_APPLICATION_WINDOW(widget);
 	GtkWidget * terminal = v3270_new();

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
			g_warning("Can't find default settings file \"%s\"",default_settings);
		}

	}

	if(error) {

		g_warning(error->message);
		g_error_free(error);
		error = NULL;

	} else {

		v3270_load_key_file(terminal,descriptor->key_file,NULL);
		v3270_accelerator_map_load_key_file(terminal,descriptor->key_file,NULL);

	}

 	append_terminal_page(window,terminal);

 	// Setup signals.
 	g_signal_connect(G_OBJECT(terminal),"save-settings",G_CALLBACK(save_settings),descriptor);
 	g_signal_connect(G_OBJECT(terminal),"toggle_changed",G_CALLBACK(toggle_changed),descriptor);
 	g_signal_connect(G_OBJECT(terminal),"print-done",G_CALLBACK(print_done),descriptor);
 	g_signal_connect(G_OBJECT(terminal),"print-setup",G_CALLBACK(print_setup),descriptor);

	return terminal;

 }

 gint pw3270_window_append_page(GtkWidget *widget, GFile * file) {

	g_return_val_if_fail(PW3270_IS_APPLICATION_WINDOW(widget),-1);

	g_autofree gchar *path = g_file_get_path(file);
	debug("Path: \"%s\"",path);

	if(path) {

		// It's a session file
		pw3270_terminal_new(widget, path);
		return 0;

	}

	g_autofree gchar * scheme = g_file_get_uri_scheme(file);

	if(!(g_ascii_strcasecmp(scheme,"tn3270") && g_ascii_strcasecmp(scheme,"tn3270s"))) {

		// It's a TN3270 URL.

		GtkWidget * terminal = v3270_new();

		g_autofree gchar * uri = g_file_get_uri(file);
		size_t sz = strlen(uri);

		if(sz > 0 && uri[sz-1] == '/')
			uri[sz-1] = 0;

		v3270_set_url(terminal,uri);
		append_terminal_page(PW3270_APPLICATION_WINDOW(widget), terminal);
		return 0;

	}

	// Create a default window.
	{
		GtkWidget * terminal = v3270_new();
		g_warning("Unexpected session URL, creating a default window");
		append_terminal_page(PW3270_APPLICATION_WINDOW(widget), terminal);
	}

	return -1;

 }


