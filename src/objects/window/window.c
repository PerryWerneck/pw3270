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
 #include <pw3270/keypad.h>
 #include <v3270/settings.h>
 #include <v3270/keyfile.h>

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

	if(window->keypads) {
		g_list_free(window->keypads);
		window->keypads = NULL;
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

#ifdef DEBUG
	{
		gtk_icon_theme_append_search_path(
			gtk_icon_theme_get_default(),
			"./icons"
		);
	}
#else
	{
		lib3270_autoptr(char) path = lib3270_build_data_filename("icons",NULL);
		if(g_file_test(path,G_FILE_TEST_IS_DIR)) {
			gtk_icon_theme_append_search_path(
				gtk_icon_theme_get_default(),
				path
			);
		}
	}
#endif // DEBUG


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

 static void save_keypad_state(GtkWidget *keypad, GtkWidget *window, gboolean visible) {

 	GtkWidget * terminal = pw3270_application_window_get_active_terminal(window);
 	if(!terminal)
		return;

	GKeyFile * keyfile = v3270_key_file_get(terminal);
	if(!keyfile)
		return;

	g_key_file_set_boolean(
		keyfile,
		"keypads",
		gtk_widget_get_name(keypad),
		visible
	);

	v3270_emit_save_settings(terminal,NULL);

 }

 static void keypad_hide(GtkWidget *keypad, GtkWidget *window) {
	save_keypad_state(keypad,window,FALSE);
 }

 static void keypad_show(GtkWidget *keypad, GtkWidget *window) {
	save_keypad_state(keypad,window,TRUE);
 }

 static GtkWidget * setup_keypad(pw3270ApplicationWindow *window, GObject * model) {

	GtkWidget * widget = pw3270_keypad_get_from_model(model);

	if(!widget) {
		return NULL;
	}

	window->keypads = g_list_prepend(window->keypads,widget);

	const gchar * name = pw3270_keypad_model_get_name(model);

	gtk_widget_set_name(widget,name);

	g_signal_connect(widget,"hide",G_CALLBACK(keypad_hide),window);
	g_signal_connect(widget,"show",G_CALLBACK(keypad_show),window);

	g_autofree gchar * action_name = g_strconcat("keypad.",name,NULL);

	GPropertyAction * action =
			g_property_action_new(
				action_name,
				widget,
				"visible"
			);

	g_action_map_add_action(
		G_ACTION_MAP(window),
		G_ACTION(action)
	);

	return widget;

 }

 static void pw3270ApplicationWindow_init(pw3270ApplicationWindow *widget) {

	// Get settings
	g_autoptr(GSettings) settings = pw3270_application_window_settings_new();

	// Override defaults
	{
		// https://gitlab.gnome.org/GNOME/gtk/-/blob/gtk-3-24/gtk/gtksettings.c
		GtkSettings *settings = gtk_widget_get_settings(GTK_WIDGET (widget));
		g_object_set(settings,"gtk-menu-bar-accel","",NULL);
	}

	// Setup defaults
	widget->state.width = 800;
	widget->state.height = 500;
	widget->state.is_maximized = 0;
	widget->state.is_fullscreen = 0;

	// Create contents
	GtkBox * container = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL,0));

	widget->notebook = GTK_NOTEBOOK(gtk_notebook_new());
	gtk_notebook_set_scrollable(widget->notebook,TRUE);
	gtk_notebook_set_show_tabs(widget->notebook,FALSE);
	gtk_notebook_set_show_border(widget->notebook, FALSE);
	gtk_notebook_set_group_name(widget->notebook,PACKAGE_NAME ":Terminals");

	/*
	{
		// Create new tab action widget
		GtkWidget * new_tab = gtk_button_new_from_icon_name("tab-new-symbolic",GTK_ICON_SIZE_LARGE_TOOLBAR);
		gtk_button_set_relief(GTK_BUTTON(new_tab),GTK_RELIEF_NONE);
		gtk_actionable_set_action_name(GTK_ACTIONABLE(new_tab),g_intern_static_string("app.new.tab"));

		gtk_widget_set_margin_start(new_tab,6);
		gtk_widget_set_margin_end(new_tab,6);
		gtk_widget_set_margin_bottom(new_tab,0);
		gtk_widget_set_valign(new_tab,GTK_ALIGN_END);

		gtk_button_set_image_position(GTK_BUTTON(new_tab),GTK_POS_BOTTOM);
		gtk_widget_show_all(new_tab);
		gtk_notebook_set_action_widget(widget->notebook,new_tab,GTK_PACK_START);
	}
	*/

	// Create boxes
	GtkBox * hBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0));
	GtkBox * vBox = GTK_BOX(gtk_box_new(GTK_ORIENTATION_VERTICAL,0));

	gtk_widget_show(GTK_WIDGET(hBox));
	gtk_widget_show(GTK_WIDGET(vBox));

	// Create toolbar
	{
		widget->toolbar = GTK_TOOLBAR(pw3270_toolbar_new());

		g_action_map_add_action(
			G_ACTION_MAP(widget),
			G_ACTION(g_property_action_new("toolbar", widget->toolbar, "visible"))
		);

		switch(g_settings_get_int(settings,"toolbar-position")) {
		case 1:
			gtk_orientable_set_orientation(GTK_ORIENTABLE(widget->toolbar),GTK_ORIENTATION_HORIZONTAL);
			gtk_box_pack_end(container,GTK_WIDGET(widget->toolbar),FALSE,TRUE,0);
			break;

		case 2:
			gtk_orientable_set_orientation(GTK_ORIENTABLE(widget->toolbar),GTK_ORIENTATION_VERTICAL);
			gtk_box_pack_end(hBox,GTK_WIDGET(widget->toolbar),FALSE,TRUE,0);
			break;

		case 3:
			gtk_orientable_set_orientation(GTK_ORIENTABLE(widget->toolbar),GTK_ORIENTATION_VERTICAL);
			gtk_box_pack_start(hBox,GTK_WIDGET(widget->toolbar),FALSE,TRUE,0);
			break;

		default:
			gtk_orientable_set_orientation(GTK_ORIENTABLE(widget->toolbar),GTK_ORIENTATION_HORIZONTAL);
			gtk_box_pack_start(container,GTK_WIDGET(widget->toolbar),FALSE,TRUE,0);
			break;

		}

		g_settings_bind(
			settings,
			"toolbar-visible",
			widget->toolbar,
			"visible",
			G_SETTINGS_BIND_DEFAULT
		);

		g_settings_bind(
			settings,
			"toolbar-icon-type",
			widget->toolbar,
			"icon-type",
			G_SETTINGS_BIND_DEFAULT
		);

		g_settings_bind(
			settings,
			"toolbar-style",
			widget->toolbar,
			"style",
			G_SETTINGS_BIND_DEFAULT
		);

		g_settings_bind(
			settings,
			"toolbar-icon-size",
			widget->toolbar,
			"icon-size",
			G_SETTINGS_BIND_DEFAULT
		);

	}

	gtk_box_pack_start(container,GTK_WIDGET(hBox),TRUE,TRUE,0);

	//
	// Create and pack keypads?
	//
	{
		GList * keypads = pw3270_application_get_keypad_models(g_application_get_default());
		GList * keypad;

		// Add top Keypads
		for(keypad = keypads;keypad;keypad = g_list_next(keypad)) {

			if(pw3270_keypad_get_position(G_OBJECT(keypad->data)) == KEYPAD_POSITION_TOP) {
				gtk_box_pack_start(
					vBox,
					setup_keypad(widget, G_OBJECT(keypad->data)),
					FALSE,FALSE,0
				);
			}
		}


		// Add left keypads
		for(keypad = keypads;keypad;keypad = g_list_next(keypad)) {

			if(pw3270_keypad_get_position(G_OBJECT(keypad->data)) == KEYPAD_POSITION_LEFT) {
				gtk_box_pack_start(
					hBox,
					setup_keypad(widget, G_OBJECT(keypad->data)),
					FALSE,FALSE,0
				);
			}
		}

		// Add center notebook
		gtk_box_pack_start(vBox,GTK_WIDGET(widget->notebook),TRUE,TRUE,0);
		gtk_box_pack_start(hBox,GTK_WIDGET(vBox),TRUE,TRUE,0);

		// Add bottom keypads
		for(keypad = keypads;keypad;keypad = g_list_next(keypad)) {

			if(pw3270_keypad_get_position(G_OBJECT(keypad->data)) == KEYPAD_POSITION_BOTTOM) {
				gtk_box_pack_end(
					vBox,
					setup_keypad(widget, G_OBJECT(keypad->data)),
					FALSE,FALSE,0
				);
			}
		}


		// Add right keypads
		for(keypad = keypads;keypad;keypad = g_list_next(keypad)) {

			if(pw3270_keypad_get_position(G_OBJECT(keypad->data)) == KEYPAD_POSITION_RIGHT) {
				gtk_box_pack_end(
					hBox,
					setup_keypad(widget, G_OBJECT(keypad->data)),
					FALSE,FALSE,0
				);
			}
		}

	}

	gtk_widget_show_all(GTK_WIDGET(widget->notebook));

	gtk_widget_show(GTK_WIDGET(container));
	gtk_container_add(GTK_CONTAINER(widget),GTK_WIDGET(container));

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

			pw3270_action_save_session_preferences_new(),

			pw3270_file_transfer_action_new(),

			pw3270_action_window_close_new(),

			pw3270_action_connect_new(),

			v3270_pfkey_action_new(),
			v3270_pakey_action_new(),

			pw3270_action_save_desktop_icon_new(),

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
	// Bind properties
	//
	g_action_map_add_action(
		G_ACTION_MAP(widget),
		G_ACTION(g_property_action_new("menubar", widget, "show-menubar"))
	);

	g_settings_bind(
		settings,
		"toolbar-action-names",
		widget->toolbar,
		"action-names",
		G_SETTINGS_BIND_DEFAULT
	);


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
	pw3270_load_placeholders(G_APPLICATION(application), builder);

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

#ifdef DEBUG
		PW3270_UI_STYLE style = PW3270_UI_STYLE_AUTOMATIC;
#else
		PW3270_UI_STYLE style = pw3270_application_get_ui_style(G_APPLICATION(application));
#endif // DEBUG

		if(style == PW3270_UI_STYLE_AUTOMATIC) {

#ifdef G_OS_UNIX
            style = PW3270_UI_STYLE_GNOME;
			g_settings_set_boolean(settings,"menubar-visible",FALSE);
			g_settings_set_int(settings,"header-icon-type",1);
#else
            style = PW3270_UI_STYLE_CLASSICAL;
			g_settings_set_boolean(settings,"menubar-visible",TRUE);
			g_settings_set_int(settings,"header-icon-type",0);
#endif // G_OS_UNIX

			g_settings_set_boolean(settings,"toolbar-visible",TRUE);

			pw3270_application_set_ui_style(G_APPLICATION(application),style);

		}


		if(style == PW3270_UI_STYLE_GNOME) {

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

	}

	// Setup default position and size
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);

	// Create terminal widget
	GtkWidget * terminal = pw3270_application_window_new_tab(GTK_WIDGET(window), session_file);

	// Create property actions
	static const struct Property {
		LIB3270_ACTION_GROUP group;
		const gchar *name;
	} properties[] = {
		{
			.name = "model-number",
			.group = LIB3270_ACTION_GROUP_OFFLINE
		},
		{
			.name = "font-family",
			.group = LIB3270_ACTION_GROUP_NONE
		},
		{
			.name = "dynamic-font-spacing",
			.group = LIB3270_ACTION_GROUP_NONE
		},
		{
			.name = "trace",
			.group = LIB3270_ACTION_GROUP_NONE
		},
	};

	for(ix = 0; ix < G_N_ELEMENTS(properties); ix++) {

		GAction * action = G_ACTION(v3270_property_action_new(terminal,properties[ix].name,properties[ix].group));

		if(!g_action_get_name(action)) {
			g_warning("Window property action %s is unnamed",properties[ix].name);
		} else {
			g_action_map_add_action(G_ACTION_MAP(window),action);
		}
	}

	pw3270_application_window_set_active_terminal(GTK_WIDGET(window),terminal);

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

 GtkWidget * pw3270_application_window_get_active_terminal(GtkWidget *widget) {
 	return PW3270_APPLICATION_WINDOW(widget)->terminal;
 }

 void pw3270_application_window_set_active_terminal(GtkWidget *widget, GtkWidget *terminal) {

	size_t ix;

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

		// Setup keypads
		if(window->keypads) {

			GKeyFile * keyfile = v3270_key_file_get(terminal);

			if(keyfile) {

				GList * keypad;
				for(keypad = window->keypads; keypad; keypad = g_list_next(keypad)) {

					GtkWidget *kWidget = GTK_WIDGET(keypad->data);
					if(g_key_file_get_boolean(keyfile,"keypads",gtk_widget_get_name(kWidget),NULL)) {
						gtk_widget_show(kWidget);
					} else {
						gtk_widget_hide(kWidget);
					}
				}

			}

		}

	} else {

		terminal = NULL;
		pw3270_window_set_subtitle(GTK_WIDGET(window), _("Disconnected from host"));

	}

	// Update window actions
	{
		gchar ** actions = g_action_group_list_actions(G_ACTION_GROUP(window));

		for(ix = 0; actions[ix]; ix++) {

			GAction * action = g_action_map_lookup_action(G_ACTION_MAP(window), actions[ix]);

			if(action && V3270_IS_ACTION(action)) {
				v3270_action_set_terminal_widget(action,terminal);
			}

		}

		g_strfreev(actions);
	}

	// Update application actions
	{
		GtkApplication * application = GTK_APPLICATION(gtk_window_get_application(GTK_WINDOW(window)));

		if(application) {

			gchar ** actions = g_action_group_list_actions(G_ACTION_GROUP(application));

			for(ix = 0; actions[ix]; ix++) {

				GAction * action = g_action_map_lookup_action(G_ACTION_MAP(application), actions[ix]);
				if(action && V3270_IS_ACTION(action)) {
					v3270_action_set_terminal_widget(action,terminal);
				}

			}

		}
	}

 }

 GSettings *pw3270_application_window_settings_new() {

 	// Get settings
	g_autofree gchar * path = g_strconcat("/apps/" PACKAGE_NAME "/", g_get_application_name(), "/window/",NULL);
//	debug("path=%s",path);

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

//	debug("schema %s=%p path=%s","br.com.bb." PACKAGE_NAME ".window",schema,path);

	GSettings * settings = g_settings_new_full(schema, NULL, path);

	g_settings_schema_source_unref(source);

#else

	GSettings * settings = g_settings_new_with_path("br.com.bb." PACKAGE_NAME ".window", path);

#endif // DEBUG

	return settings;

 }

 GList * pw3270_application_window_get_keypads(GtkWidget *window) {

	g_return_val_if_fail(PW3270_IS_APPLICATION_WINDOW(window),NULL);
	return PW3270_APPLICATION_WINDOW(window)->keypads;

 }
