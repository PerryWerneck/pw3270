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

 GtkWidget * pw3270_terminal_new(GtkWidget *widget) {

	g_return_val_if_fail(PW3270_IS_APPLICATION_WINDOW(widget),NULL);

	pw3270ApplicationWindow * window = PW3270_APPLICATION_WINDOW(widget);
 	GtkWidget * terminal = v3270_new();

 	append_terminal_page(window,terminal);

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


