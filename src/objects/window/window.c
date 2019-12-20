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
 #include <pw3270.h>
 #include <pw3270/toolbar.h>
 #include <pw3270/application.h>
 #include <pw3270/actions.h>

 static void get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
 static void set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);

 G_DEFINE_TYPE(pw3270ApplicationWindow, pw3270ApplicationWindow, GTK_TYPE_APPLICATION_WINDOW);

 enum {
	PROP_NONE,
	PROP_ACTION_NAMES,
 };

 static void destroy(GtkWidget *widget) {

	size_t ix;
	pw3270ApplicationWindow * window = PW3270_APPLICATION_WINDOW(widget);

	debug("%s(%p)",__FUNCTION__,widget);

	// Update actions
	gchar ** actions = g_action_group_list_actions(G_ACTION_GROUP(widget));

	for(ix = 0; actions[ix]; ix++) {
		GAction * action = g_action_map_lookup_action(G_ACTION_MAP(widget), actions[ix]);

		if(action && PW3270_IS_ACTION(action)) {
			pw3270_action_set_terminal_widget(action,NULL);
		}

	}

	g_strfreev(actions);

	// Destroy popups
	for(ix = 0; ix < G_N_ELEMENTS(window->popups); ix++) {
		if(window->popups[ix]) {
			gtk_widget_destroy(window->popups[ix]);
			window->popups[ix] = NULL;
		}
	}

	GTK_WIDGET_CLASS(pw3270ApplicationWindow_parent_class)->destroy(widget);

 }

 static void pw3270ApplicationWindow_class_init(pw3270ApplicationWindowClass *klass) {

	GTK_WIDGET_CLASS(klass)->destroy = destroy;

 	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->set_property	= set_property;
	object_class->get_property	= get_property;

	g_object_class_install_property(
		object_class,
		PROP_ACTION_NAMES,
		g_param_spec_string ("action-names",
			N_("Action Names"),
			N_("The name of the actions in the header bar"),
			NULL,
			G_PARAM_WRITABLE|G_PARAM_READABLE)
	);


 }

 void set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec G_GNUC_UNUSED(*pspec)) {

	if(prop_id == PROP_ACTION_NAMES) {
		pw3270_window_set_header_action_names(GTK_WIDGET(object), g_value_get_string(value));
	}

 }

 void get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) {

 	if(prop_id == PROP_ACTION_NAMES) {
    	g_value_take_string(value,pw3270_window_get_action_names(GTK_WIDGET(object)));
	}

 }

 void on_page_changed(GtkNotebook *notebook, GtkWidget G_GNUC_UNUSED(*child), guint G_GNUC_UNUSED(page_num), gpointer G_GNUC_UNUSED(user_data)) {
 	gtk_notebook_set_show_tabs(notebook,gtk_notebook_get_n_pages(notebook) > 1);
 }

 static void pw3270ApplicationWindow_init(pw3270ApplicationWindow *widget) {

	GtkBox * vBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL,0));

	widget->notebook = GTK_NOTEBOOK(gtk_notebook_new());
	gtk_notebook_set_scrollable(widget->notebook,TRUE);
	gtk_notebook_set_show_tabs(widget->notebook,FALSE);
	gtk_notebook_set_show_border(widget->notebook, FALSE);
	gtk_notebook_set_group_name(widget->notebook,PACKAGE_NAME ":Terminals");
	g_signal_connect(G_OBJECT(widget->notebook), "page-added", G_CALLBACK(on_page_changed), widget);
	g_signal_connect(G_OBJECT(widget->notebook), "page-removed", G_CALLBACK(on_page_changed), widget);

	widget->toolbar  = GTK_TOOLBAR(pw3270_toolbar_new());
	gtk_box_pack_start(vBox,GTK_WIDGET(widget->toolbar),FALSE,TRUE,0);
	gtk_box_pack_start(vBox,GTK_WIDGET(widget->notebook),TRUE,TRUE,0);

	gtk_widget_show_all(GTK_WIDGET(vBox));
	gtk_container_add(GTK_CONTAINER(widget),GTK_WIDGET(vBox));

	//
	// Setup tn3270 actions.
	//
	pw3270_window_add_actions(GTK_WIDGET(widget));

	// Map special actions
	{
		size_t ix;

		GAction * actions[] = {
			pw3270_action_host_properties_new(),
			pw3270_set_color_action_new(),
			pw3270_action_session_properties_new(),
			pw3270_file_transfer_action_new()
		};

		for(ix = 0; ix < G_N_ELEMENTS(actions); ix++) {
			debug("Inserting %s",g_action_get_name(actions[ix]));
			g_action_map_add_action(G_ACTION_MAP(widget),actions[ix]);
		}

	}

	//
	// Setup Window actions.
	//
	static GActionEntry actions[] = {

		{
			.name = "open",
			.activate = pw3270_window_open_activated,
		},

		{
			.name = "close",
			.activate = pw3270_window_close_activated,
		},

	};

	g_action_map_add_action_entries(
		G_ACTION_MAP(widget),
		actions,
		G_N_ELEMENTS(actions),
		widget
	);

	//
	// Setup toolbar
	//

	{

		/*
		pw3270_toolbar_set_actions(
			GTK_WIDGET(widget->toolbar),
			"win.copy,win.paste,win.select-all,separator,win.connect,win.disconnect,separator,win.session.properties,win.file.transfer,win.print,app.quit"
		);
		*/

		g_action_map_add_action(
			G_ACTION_MAP(widget),
			G_ACTION(g_property_action_new("toolbar", widget->toolbar, "visible"))
		);

		g_action_map_add_action(
			G_ACTION_MAP(widget),
			G_ACTION(g_property_action_new("menubar", widget, "show-menubar"))
		);

		//g_autofree gchar * action_names = pw3270_toolbar_get_actions(GTK_WIDGET(widget->toolbar));
		//debug("[%s]",action_names);

	}

 }

 GtkWidget * pw3270_application_window_new(GtkApplication * application) {

	gchar *title = _( "IBM 3270 Terminal emulator" );

	g_return_val_if_fail(PW3270_IS_APPLICATION(application),NULL);

	size_t ix;

	g_autoptr(GSettings) settings = pw3270_application_get_settings(G_APPLICATION(application));

	g_return_val_if_fail(GTK_IS_APPLICATION(application), NULL);
	pw3270ApplicationWindow * window =
		g_object_new(
			PW3270_TYPE_APPLICATION_WINDOW,
			"application", application,
			NULL);

	//
	// Get builder
	//
	g_autoptr(GtkBuilder) builder = pw3270_application_get_builder("window.xml");

	// Load popup menus.
	const gchar * popup_menus[G_N_ELEMENTS(window->popups)] = {
		"popup-over-selected-area",
		"popup-over-unselected-area",
		"popup-when-offline"
	};

	for(ix = 0; ix < G_N_ELEMENTS(popup_menus); ix++) {

		GObject * model = gtk_builder_get_object(builder, popup_menus[ix]);
		if(model) {
			window->popups[ix] = gtk_menu_new_from_model(G_MENU_MODEL(model));
			gtk_menu_attach_to_widget(GTK_MENU(window->popups[ix]),GTK_WIDGET(window),NULL);
		}

	}

	if(pw3270_application_get_ui_style(G_APPLICATION(application)) == PW3270_UI_STYLE_GNOME) {

		// Create header bar

		GtkHeaderBar * header = GTK_HEADER_BAR(gtk_header_bar_new());
		gtk_window_set_titlebar(GTK_WINDOW(window), GTK_WIDGET(header));
		gtk_header_bar_set_show_close_button(header,TRUE);

		gtk_header_bar_set_title(header,title);
		g_settings_bind(
			settings,
			"has-subtitle",
			header,
			"has-subtitle",
			G_SETTINGS_BIND_DEFAULT
		);

		// Show the new header
		gtk_widget_show_all(GTK_WIDGET(header));

		// g_autofree gchar * header_actions = g_settings_get_string(settings, "header-action-names");
		// pw3270_window_set_header_action_names(GTK_WIDGET(window), header_actions);

		g_settings_bind(
			settings,
			"header-action-names",
			window,
			"action-names",
			G_SETTINGS_BIND_DEFAULT
		);

	} else {

		gtk_window_set_title(GTK_WINDOW(window), title);

	}

	// Setup and show main window
	g_settings_bind(
		settings,
		"menubar-visible",
		window,
		"show-menubar",
		G_SETTINGS_BIND_DEFAULT
	);

	g_settings_bind(
		settings,
		"toolbar-visible",
		window->toolbar,
		"visible",
		G_SETTINGS_BIND_DEFAULT
	);

	g_settings_bind(
		settings,
		"toolbar-action-names",
		window->toolbar,
		"action-names",
		G_SETTINGS_BIND_DEFAULT
	);

	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_window_set_default_size (GTK_WINDOW (window), 800, 500);

	// gtk_window_set_interactive_debugging(TRUE);

	return GTK_WIDGET(window);

 }

 void pw3270_window_set_current_page(GtkWidget *window, gint page_num) {

	g_return_if_fail(PW3270_IS_APPLICATION_WINDOW(window));

	GtkNotebook * notebook = PW3270_APPLICATION_WINDOW(window)->notebook;

	debug("Selecting tab %d", page_num);

	gtk_notebook_set_current_page(notebook, page_num);
	gtk_widget_grab_focus(gtk_notebook_get_nth_page(notebook, page_num));

 }

 void pw3270_window_set_subtitle(GtkWidget *window, const gchar *subtitle) {

	g_return_if_fail(PW3270_IS_APPLICATION_WINDOW(window));

 	GtkWidget * title_bar = gtk_window_get_titlebar(GTK_WINDOW(window));

 	if(title_bar && GTK_IS_HEADER_BAR(title_bar) && gtk_header_bar_get_has_subtitle(GTK_HEADER_BAR(title_bar))) {
		gtk_header_bar_set_subtitle(GTK_HEADER_BAR(title_bar), subtitle);
	}

 }

 void pw3270_application_generic_activated(GSimpleAction * action, GVariant *parameter, gpointer application) {

	debug("%s",__FUNCTION__);

 }

