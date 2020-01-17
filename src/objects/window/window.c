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

	pw3270_application_window_set_active_terminal(widget,NULL);

	// Destroy popups
	for(ix = 0; ix < G_N_ELEMENTS(window->popups); ix++) {
		if(window->popups[ix]) {
			gtk_widget_destroy(window->popups[ix]);
			window->popups[ix] = NULL;
		}
	}

	{
		g_autoptr(GSettings) settings = pw3270_application_window_settings_new();

		g_settings_set_int(settings, "width", window->state.width);
		g_settings_set_int(settings, "height", window->state.height);
		g_settings_set_boolean(settings, "is-maximized", window->state.is_maximized);
		g_settings_set_boolean(settings, "is-fullscreen", window->state.is_fullscreen);
	}

	GTK_WIDGET_CLASS(pw3270ApplicationWindow_parent_class)->destroy(widget);

 }

 static void size_allocate(GtkWidget *widget, GtkAllocation *allocation) {

 	// https://developer.gnome.org/SaveWindowState/
	GTK_WIDGET_CLASS(pw3270ApplicationWindow_parent_class)->size_allocate(widget, allocation);

	pw3270ApplicationWindow * window = PW3270_APPLICATION_WINDOW(widget);

	if(!(window->state.is_maximized || window->state.is_fullscreen)) {
		gtk_window_get_size(GTK_WINDOW (widget), &window->state.width, &window->state.height);
	}

 }

 static gboolean window_state_event(GtkWidget *widget, GdkEventWindowState *event) {

  	// https://developer.gnome.org/SaveWindowState/
  	gboolean res = GDK_EVENT_PROPAGATE;

  	if(GTK_WIDGET_CLASS(pw3270ApplicationWindow_parent_class)->window_state_event != NULL) {
		res = GTK_WIDGET_CLASS(pw3270ApplicationWindow_parent_class)->window_state_event(widget, event);
  	}

  	{
  		pw3270ApplicationWindow * window = PW3270_APPLICATION_WINDOW(widget);

		window->state.is_maximized = (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) == 0 ? 0 : 1;
		window->state.is_fullscreen = (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN) == 0 ? 0 : 1;
  	}

	return res;
 }

 static void constructed(GObject *object) {

	// https://developer.gnome.org/SaveWindowState/

 	pw3270ApplicationWindow * window = PW3270_APPLICATION_WINDOW(object);

	g_autoptr(GSettings) settings = pw3270_application_window_settings_new();
	if(settings) {

		// https://developer.gnome.org/SaveWindowState/
		window->state.width = g_settings_get_int (settings, "width");
		window->state.height = g_settings_get_int (settings, "height");
		window->state.is_maximized = g_settings_get_boolean (settings, "is-maximized") ? 1 : 0;
		window->state.is_fullscreen = g_settings_get_boolean (settings, "is-fullscreen") ? 1 : 0;

	}

	gtk_window_set_default_size(GTK_WINDOW (object), window->state.width, window->state.height);

	if(window->state.is_maximized)
		gtk_window_maximize(GTK_WINDOW(object));

	if(window->state.is_fullscreen)
		gtk_window_fullscreen(GTK_WINDOW (object));

	G_OBJECT_CLASS (pw3270ApplicationWindow_parent_class)->constructed (object);
 }

 static void pw3270ApplicationWindow_class_init(pw3270ApplicationWindowClass *klass) {

	{
		GtkWidgetClass *widget = GTK_WIDGET_CLASS(klass);
		widget->destroy = destroy;
		widget->window_state_event = window_state_event;
		widget->size_allocate = size_allocate;
	}

	{
		GObjectClass *object_class = G_OBJECT_CLASS(klass);

		object_class->set_property	= set_property;
		object_class->get_property	= get_property;
		object_class->constructed = constructed;

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

 }

 void set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec G_GNUC_UNUSED(*pspec)) {

	if(prop_id == PROP_ACTION_NAMES) {
		pw3270_window_set_header_action_names(GTK_WIDGET(object), g_value_get_string(value));
	}

 }

 void get_property(GObject *object, guint prop_id, GValue *value, GParamSpec G_GNUC_UNUSED(*pspec)) {

 	if(prop_id == PROP_ACTION_NAMES) {
    	g_value_take_string(value,pw3270_window_get_action_names(GTK_WIDGET(object)));
	}

 }

 static void pw3270ApplicationWindow_init(pw3270ApplicationWindow *widget) {

	// Setup defaults
	widget->state.width = 800;
	widget->state.height = 500;
	widget->state.is_maximized = 0;
	widget->state.is_fullscreen = 0;

	// Create contents
	GtkBox * vBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL,0));

	widget->notebook = GTK_NOTEBOOK(gtk_notebook_new());
	gtk_notebook_set_scrollable(widget->notebook,TRUE);
	gtk_notebook_set_show_tabs(widget->notebook,FALSE);
	gtk_notebook_set_show_border(widget->notebook, FALSE);
	gtk_notebook_set_group_name(widget->notebook,PACKAGE_NAME ":Terminals");

	widget->toolbar  = GTK_TOOLBAR(pw3270_toolbar_new());
	gtk_box_pack_start(vBox,GTK_WIDGET(widget->toolbar),FALSE,TRUE,0);
	gtk_box_pack_start(vBox,GTK_WIDGET(widget->notebook),TRUE,TRUE,0);

	gtk_widget_show_all(GTK_WIDGET(vBox));
	gtk_container_add(GTK_CONTAINER(widget),GTK_WIDGET(vBox));

	//
	// Setup tn3270 actions.
	//
	g_action_map_add_v3270_actions(G_ACTION_MAP(widget));
	g_action_map_add_lib3270_actions(G_ACTION_MAP(widget));
	g_action_map_add_lib3270_toggles(G_ACTION_MAP(widget));

	// Map special actions
	{
		size_t ix;

		GAction * actions[] = {
			pw3270_action_host_properties_new(),
			pw3270_action_session_properties_new(),
			pw3270_set_color_action_new(),

			pw3270_action_save_session_as_new(),

			pw3270_file_transfer_action_new(),

			pw3270_action_window_close_new(),

			pw3270_action_connect_new(),

			v3270_pfkey_action_new(),
			v3270_pakey_action_new(),

		};

		for(ix = 0; ix < G_N_ELEMENTS(actions); ix++) {

			if(!g_action_get_name(actions[ix])) {
				g_warning("Window special action %u is unnamed",(unsigned int) ix);
			} else {
				g_action_map_add_action(G_ACTION_MAP(widget),actions[ix]);
			}

		}

	}

	//
	// Setup toolbar
	//

	{

		g_action_map_add_action(
			G_ACTION_MAP(widget),
			G_ACTION(g_property_action_new("toolbar", widget->toolbar, "visible"))
		);

		g_action_map_add_action(
			G_ACTION_MAP(widget),
			G_ACTION(g_property_action_new("menubar", widget, "show-menubar"))
		);

	}

 }

 static void page_added(GtkNotebook *notebook, GtkWidget *child, guint G_GNUC_UNUSED(page_num), GtkApplication * application) {

	debug("%s(%p)",__FUNCTION__,child);

 	gtk_notebook_set_show_tabs(notebook,gtk_notebook_get_n_pages(notebook) > 1);

 	// Call plugins
  	int (*call)(GtkWidget *);

  	GSList * item;
  	for(item = pw3270_application_get_plugins(G_APPLICATION(application)); item; item = g_slist_next(item)) {
		if(g_module_symbol((GModule *) item->data, "pw3270_plugin_page_added", (gpointer *) &call)) {
			call(child);
       }
  	}

 }

 static void page_removed(GtkNotebook *notebook, GtkWidget *child, guint G_GNUC_UNUSED(page_num), GtkApplication * application) {

	debug("%s(%p)",__FUNCTION__,child);

 	gtk_notebook_set_show_tabs(notebook,gtk_notebook_get_n_pages(notebook) > 1);

 	// Call plugins
  	int (*call)(GtkWidget *);

  	GSList * item;
  	for(item = pw3270_application_get_plugins(G_APPLICATION(application)); item; item = g_slist_next(item)) {
		if(g_module_symbol((GModule *) item->data, "pw3270_plugin_page_removed", (gpointer *) &call)) {
			call(child);
       }
  	}

 }

 GtkWidget * pw3270_application_window_new(GtkApplication * application, const gchar *session_file) {

	gchar *title = _( "IBM 3270 Terminal emulator" );

	g_return_val_if_fail(PW3270_IS_APPLICATION(application),NULL);

	size_t ix;

	g_return_val_if_fail(GTK_IS_APPLICATION(application), NULL);
	pw3270ApplicationWindow * window =
		g_object_new(
			PW3270_TYPE_APPLICATION_WINDOW,
			"application", application,
			NULL);

	g_signal_connect(G_OBJECT(window->notebook), "page-added", G_CALLBACK(page_added), application);
	g_signal_connect(G_OBJECT(window->notebook), "page-removed", G_CALLBACK(page_removed), application);

	//
	// Get builder
	//
	g_autoptr(GtkBuilder) builder = pw3270_application_get_builder("window.xml");

	// Load popup menus.
	const gchar * popup_menus[PW3270_APP_WINDOW_POPUP_COUNT] = {
		"popup-over-selected-area",		// PW3270_APP_WINDOW_POPUP_OVER_SELECTED_AREA
		"popup-over-unselected-area",	// PW3270_APP_WINDOW_POPUP_OVER_UNSELECTED_AREA
		"popup-over-oia",				// PW3270_APP_WINDOW_POPUP_OVER_OIA
		"popup-when-offline"			// PW3270_APP_WINDOW_POPUP_WHEN_OFFLINE
	};

	for(ix = 0; ix < G_N_ELEMENTS(popup_menus); ix++) {

		GObject * model = gtk_builder_get_object(builder, popup_menus[ix]);
		if(model) {
			window->popups[ix] = gtk_menu_new_from_model(G_MENU_MODEL(model));
			gtk_menu_attach_to_widget(GTK_MENU(window->popups[ix]),GTK_WIDGET(window),NULL);
		}

	}

	// Setup and show main window
	{
		g_autoptr(GSettings) settings = pw3270_application_window_settings_new();

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

		g_settings_bind(
			settings,
			"toolbar-style",
			window->toolbar,
			"style",
			G_SETTINGS_BIND_DEFAULT
		);

		g_settings_bind(
			settings,
			"toolbar-icon-size",
			window->toolbar,
			"icon-size",
			G_SETTINGS_BIND_DEFAULT
		);

	}

	// Setup default position and size
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);

	// Create terminal widget
	GtkWidget * terminal = pw3270_application_window_new_tab(GTK_WIDGET(window), session_file);

	// Create property actions
	static const gchar * properties[] = {
		"model-number",
		"font-family",
		"dynamic-font-spacing",
		"trace",
	};

	for(ix = 0; ix < G_N_ELEMENTS(properties); ix++) {

		GAction * action = G_ACTION(v3270_property_action_new(terminal,properties[ix]));

		if(!g_action_get_name(action)) {
			g_warning("Window property action %s is unnamed",properties[ix]);
		} else {
			g_action_map_add_action(G_ACTION_MAP(window),action);
		}
	}

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

 void pw3270_application_generic_activated(GSimpleAction * action, GVariant G_GNUC_UNUSED(*parameter), gpointer G_GNUC_UNUSED(application)) {
	g_message("Generic action %s was activated",g_action_get_name(G_ACTION(action)));
 }

 GtkWidget * pw3270_application_window_get_active_terminal(GtkWidget *widget) {
 	return PW3270_APPLICATION_WINDOW(widget)->terminal;
 }

 void pw3270_application_window_set_active_terminal(GtkWidget *widget, GtkWidget *terminal) {

 	pw3270ApplicationWindow * window = PW3270_APPLICATION_WINDOW(widget);

 	if(window->terminal == terminal)
		return;

	if(terminal && GTK_IS_V3270(terminal)) {

		window->terminal = terminal;

		// Store the active terminal widget.
		gtk_widget_grab_default(terminal);
		debug("Terminal %p is now default",terminal);

		// Change window title
		g_autofree gchar * title = v3270_get_session_title(terminal);
		gtk_window_set_title(GTK_WINDOW(window), title);

		pw3270_window_set_subtitle(GTK_WIDGET(window), v3270_is_connected(terminal) ? _("Connected to host") : _("Disconnected from host"));

	} else {

		terminal = NULL;
		pw3270_window_set_subtitle(GTK_WIDGET(window), _("Disconnected from host"));

	}

	// Update actions
	size_t ix;
	gchar ** actions = g_action_group_list_actions(G_ACTION_GROUP(window));

	for(ix = 0; actions[ix]; ix++) {

//		debug("%s",actions[ix]);

		GAction * action = g_action_map_lookup_action(G_ACTION_MAP(window), actions[ix]);

		if(action && V3270_IS_ACTION(action)) {
			v3270_action_set_terminal_widget(action,terminal);
		}

	}

	g_strfreev(actions);

 }

 GSettings *pw3270_application_window_settings_new() {

 	// Get settings
	g_autofree gchar * path = g_strconcat("/apps/" PACKAGE_NAME "/", g_get_application_name(), "/window/",NULL);
	debug("path=%s",path);

#ifdef DEBUG

	GError * error = NULL;
	GSettingsSchemaSource * source =
		g_settings_schema_source_new_from_directory(
			".",
			NULL,
			TRUE,
			&error
		);

	g_assert_no_error(error);

	GSettingsSchema * schema =
		g_settings_schema_source_lookup(
			source,
			"br.com.bb." PACKAGE_NAME ".window",
			TRUE);

	debug("schema %s=%p path=%s","br.com.bb." PACKAGE_NAME ".window",schema,path);

	GSettings * settings = g_settings_new_full(schema, NULL, path);

	g_settings_schema_source_unref(source);

#else

	GSettings * settings = g_settings_new_with_path("br.com.bb." PACKAGE_NAME ".window", path);

#endif // DEBUG

	return settings;

 }

