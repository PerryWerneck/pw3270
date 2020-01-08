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

//---[ Gtk Label with customized popup-menu ]---------------------------------------------------------------------------------------

 typedef struct _pw3270TabLabel {

 	GtkLabel parent;

 } pw3270TabLabel;

 typedef struct _pw3270TabLabelClass {

 	GtkLabelClass parent_class;

 } pw3270TabLabelClass;

 G_DEFINE_TYPE(pw3270TabLabel, pw3270TabLabel, GTK_TYPE_LABEL);


 static void popup_menu_detach(GtkWidget G_GNUC_UNUSED(*label), GtkMenu *menu) {

 	debug("%s",__FUNCTION__)
 	gtk_widget_destroy(GTK_WIDGET(menu));

 }

 static gboolean tab_label_button_press(GtkWidget *label, GdkEventButton *event) {

	if (event->button == 3 && event->type == GDK_BUTTON_PRESS) {

		GtkWidget * menu = gtk_menu_new();

		debug("menu=%p",menu);

		gtk_menu_attach_to_widget(GTK_MENU(menu), GTK_WIDGET(label), popup_menu_detach);
		g_signal_emit_by_name(GTK_LABEL(label),"populate-popup",menu);
		gtk_menu_popup_at_widget(GTK_MENU(menu),label,GDK_GRAVITY_SOUTH_WEST,GDK_GRAVITY_NORTH_WEST,(GdkEvent *) event);

		return TRUE;
	}

	return FALSE;
 }

 static void pw3270TabLabel_class_init(pw3270TabLabelClass *klass) {

	GTK_WIDGET_CLASS(klass)->button_press_event = tab_label_button_press;

 }

 static void pw3270TabLabel_init(pw3270TabLabel G_GNUC_UNUSED(*widget)) {

 }

//----------------------------------------------------------------------------------------------------------------------------------

 static gboolean on_terminal_focus(GtkWidget *terminal, GdkEvent G_GNUC_UNUSED(*event), GtkWindow * window);
 static void session_changed(GtkWidget *terminal, GtkWidget *label);
 static void disconnected(GtkWidget *terminal, GtkWindow * window);
 static void connected(GtkWidget *terminal, const gchar *host, GtkWindow * window);
 static void destroy(GtkWidget *terminal, GtkWindow * window);
 static void close_page(GtkButton *button, GtkWidget *terminal);
 static gboolean on_popup_menu(GtkWidget *widget, gboolean selected, gboolean online, GdkEvent *event, pw3270ApplicationWindow * window);
 static void label_populate_popup(GtkLabel *label, GtkMenu *menu, GtkWidget *terminal);

 gint pw3270_application_window_append_page(pw3270ApplicationWindow * window, GtkWidget * terminal) {

 	GtkWidget * label	=
		GTK_WIDGET(
			g_object_new(
			pw3270TabLabel_get_type(),
			"label", v3270_get_session_name(terminal),
			"selectable", TRUE,
			NULL)
		);

 	// gtk_label_new(v3270_get_session_name(terminal));

 	GtkWidget * tab		= gtk_box_new(GTK_ORIENTATION_HORIZONTAL,2);
 	GtkWidget * button	= gtk_button_new_from_icon_name("window-close-symbolic",GTK_ICON_SIZE_MENU);

 	gtk_button_set_relief(GTK_BUTTON(button),GTK_RELIEF_NONE);

 	debug("notebook: %p", window->notebook);

	g_signal_connect(G_OBJECT(label), "populate-popup", G_CALLBACK(label_populate_popup), terminal);

	g_signal_connect(G_OBJECT(terminal), "focus-in-event", G_CALLBACK(on_terminal_focus), window);
	g_signal_connect(G_OBJECT(terminal), "session_changed", G_CALLBACK(session_changed),label);
	g_signal_connect(G_OBJECT(terminal), "disconnected", G_CALLBACK(disconnected),window);
	g_signal_connect(G_OBJECT(terminal), "connected", G_CALLBACK(connected),window);
	g_signal_connect(G_OBJECT(terminal), "popup", G_CALLBACK(on_popup_menu), window);
	g_signal_connect(G_OBJECT(terminal), "destroy", G_CALLBACK(destroy),window);

	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(close_page), terminal);

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

		if(action) {

			if(V3270_IS_ACTION(action)) {
				v3270_action_set_terminal_widget(action,terminal);
			} else if(PW3270_IS_ACTION(action)) {
				pw3270_action_set_terminal_widget(action,terminal);
			}

		}

	}

	g_strfreev(actions);

 	return FALSE;
 }

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

 static void destroy(GtkWidget *terminal, GtkWindow * window) {

	if(gtk_window_get_default_widget(window) != terminal) {
		debug("Terminal %p was destroyed (Default one is %p)",__FUNCTION__,gtk_window_get_default_widget(window));
		return;
	}

	debug("Default terminal %p was destroyed",__FUNCTION__);

	gtk_window_set_default(window,NULL);
	pw3270_window_set_subtitle(GTK_WIDGET(window), _("Disconnected from host"));

	// Update actions
	size_t ix;
	gchar ** actions = g_action_group_list_actions(G_ACTION_GROUP(window));

	for(ix = 0; actions[ix]; ix++) {

		GAction * action = g_action_map_lookup_action(G_ACTION_MAP(window), actions[ix]);

		if(action) {

			if(PW3270_IS_ACTION(action)) {
				pw3270_action_set_terminal_widget(action,NULL);
			} else if(V3270_IS_ACTION(action)) {
				v3270_action_set_terminal_widget(action,NULL);
			}

		}

	}

	g_strfreev(actions);

 }

  static void close_page(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal) {

	g_object_ref(terminal);

	GtkNotebook * notebook = GTK_NOTEBOOK(gtk_widget_get_parent(terminal));
	gtk_notebook_remove_page(notebook,gtk_notebook_page_num(notebook, terminal));

	g_object_unref(terminal);

 }

 static void rename_session(GtkWidget G_GNUC_UNUSED(*widget), GtkWidget *terminal) {
 	debug("%s",__FUNCTION__);
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

 static void label_populate_popup(GtkLabel *label, GtkMenu *menu, GtkWidget *terminal) {

	static const struct Item {
		const gchar * label;
		GCallback	  callback;
	} items[] = {

		{
			.label = N_("_Rename session"),
			.callback = G_CALLBACK(rename_session)
		},

		{
			.label = N_("_Close session"),
			.callback = G_CALLBACK(close_page)
		}

	};

	size_t ix;

	debug("%s",__FUNCTION__);

	for(ix = 0; ix < G_N_ELEMENTS(items); ix++) {
		GtkWidget *item = gtk_menu_item_new_with_mnemonic(gettext(items[ix].label));
		g_signal_connect(G_OBJECT(item),"activate",items[ix].callback,terminal);
		gtk_widget_show_all(item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
	}

 }
