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

 static void on_terminal_destroy(GtkWidget *terminal, GtkWindow * window) {

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


 static gboolean bg_auto_connect(GtkWidget *terminal) {
 	v3270_reconnect(terminal);
	return FALSE;
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

	H3270 * hSession = v3270_get_session(terminal);

	if(lib3270_get_toggle(hSession,LIB3270_TOGGLE_CONNECT_ON_STARTUP))
		g_idle_add((GSourceFunc) bg_auto_connect, terminal);

	return page;

 }

 static void save_settings(GtkWidget *terminal, const gchar *filename) {

        GError *error = NULL;
		GKeyFile * key_file = g_key_file_new();

		g_key_file_load_from_file(key_file,filename,G_KEY_FILE_NONE,&error);

		if(error) {

			g_warning("Can't load \"%s\": %s",filename,error->message);
			g_error_free(error);
			return;

		}

        v3270_to_key_file(terminal,key_file,"terminal");
        v3270_accelerator_map_to_key_file(terminal, key_file, "accelerators");

        g_key_file_save_to_file(key_file,filename,&error);

        if(error) {

			g_warning("Can't save \"%s\": %s",filename,error->message);
			g_error_free(error);

        } else {

			g_message("Session properties save to %s",filename);
        }

        g_key_file_free(key_file);


 }


 GtkWidget * pw3270_terminal_new(GtkWidget *widget, const gchar *session_file) {

	g_return_val_if_fail(PW3270_IS_APPLICATION_WINDOW(widget),NULL);

	pw3270ApplicationWindow * window = PW3270_APPLICATION_WINDOW(widget);
 	GtkWidget * terminal = v3270_new();

 	gchar * filename;

 	if(session_file) {

		// Use the supplied session file
		filename = g_strdup(session_file);

 	} else {

		// No session file, use the default one.
		filename = g_build_filename(g_get_user_config_dir(),G_STRINGIFY(PRODUCT_NAME) ".conf",NULL);

 	}

 	// Setup session file;
 	GError *error = NULL;
	g_object_set_data_full(G_OBJECT(terminal),"session-file-name",filename,g_free);

	GKeyFile * key_file = g_key_file_new();

	if(g_file_test(filename,G_FILE_TEST_IS_REGULAR)) {

		// Found session file, open it.
        if(!g_key_file_load_from_file(key_file,filename,G_KEY_FILE_NONE,&error)) {
			g_warning("Can't load \"%s\"",filename);
        } else {
			g_message("Loading session properties from %s",filename);
        }

	} else {

		// No session file, load the defaults (if available).
		lib3270_autoptr(char) default_settings = lib3270_build_data_filename("defaults.conf",NULL);
		if(g_file_test(default_settings,G_FILE_TEST_IS_REGULAR)) {
			if(!g_key_file_load_from_file(key_file,default_settings,G_KEY_FILE_NONE,&error)) {
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

		v3270_load_key_file(terminal,key_file,NULL);
		v3270_accelerator_map_load_key_file(terminal,key_file,NULL);

	}

	g_key_file_free(key_file);

 	append_terminal_page(window,terminal);

 	// Setup signals.
 	g_signal_connect(G_OBJECT(terminal),"save-settings",G_CALLBACK(save_settings),filename);

	return terminal;

 }

 gint pw3270_window_append_page(GtkWidget *widget, GFile * file) {

	g_return_val_if_fail(PW3270_IS_APPLICATION_WINDOW(widget),-1);

	pw3270ApplicationWindow * window = PW3270_APPLICATION_WINDOW(widget);
 	GtkWidget * terminal = v3270_new();

	// Identify argument.
	g_autofree gchar * scheme = g_file_get_uri_scheme(file);

	if(!(g_ascii_strcasecmp(scheme,"tn3270") && g_ascii_strcasecmp(scheme,"tn3270s"))) {

		// Is a 3270 URL.
		g_autofree gchar * uri = g_file_get_uri(file);
		size_t sz = strlen(uri);

		if(sz > 0 && uri[sz-1] == '/')
			uri[sz-1] = 0;

		v3270_set_url(terminal,uri);

	} else {

		g_message("Unexpected URI scheme: \"%s\"",scheme);

	}

 	return append_terminal_page(window, terminal);

 }


