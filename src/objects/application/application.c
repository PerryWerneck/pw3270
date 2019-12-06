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
 * References:
 *
 * https://fossies.org/linux/gtk+/examples/plugman.c
 *
 */

 #include "private.h"
 #include <pw3270.h>
 #include <pw3270/application.h>
 #include <pw3270/actions.h>

 enum {
	PROP_ZERO,
	PROP_UI_STYLE,

	NUM_PROPERTIES
 };

 static GParamSpec * props[NUM_PROPERTIES];

 struct _pw3270ApplicationClass {
 	GtkApplicationClass parent_class;
 };

 struct _pw3270Application {
 	GtkApplication parent;

 	GSettings * settings;

 	PW3270_UI_STYLE	ui_style;

 };

 static void 		startup(GApplication * application);
 static void 		activate(GApplication * application);
 static void 		open(GApplication * application, GFile **files, gint n_files, const gchar *hint);
 static void		finalize(GObject *object);

 G_DEFINE_TYPE(pw3270Application, pw3270Application, GTK_TYPE_APPLICATION);

 static void get_property(GObject *object, guint prop_id, GValue *value, GParamSpec G_GNUC_UNUSED(*pspec)) {

	switch (prop_id) {
	case PROP_UI_STYLE:
		g_value_set_uint(value,pw3270_application_get_ui_style(G_APPLICATION(object)));
		break;

    default:
      g_assert_not_reached ();
    }

 }

 static void set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec G_GNUC_UNUSED(*pspec)) {

	switch (prop_id) {
	case PROP_UI_STYLE:
		pw3270_application_set_ui_style(G_APPLICATION(object),g_value_get_uint(value));
		break;

    default:
      g_assert_not_reached ();
    }

 }

 static void pw3270Application_class_init(pw3270ApplicationClass *klass) {

	GApplicationClass *application_class = G_APPLICATION_CLASS(klass);
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->get_property = get_property;
	object_class->set_property = set_property;
 	object_class->finalize = finalize;

	application_class->startup = startup;
	application_class->activate = activate;
	application_class->open = open;

	props[PROP_UI_STYLE] =
		g_param_spec_uint(
			"ui-style",
			_("UI Type"),
			_("The code of the User interface type"),
			PW3270_UI_STYLE_CLASSICAL,
			PW3270_UI_STYLE_GNOME,
#ifdef _WIN32
			PW3270_UI_STYLE_CLASSICAL,
#else
			PW3270_UI_STYLE_GNOME,
#endif // _WIN32
			G_PARAM_READABLE|G_PARAM_WRITABLE
		);


	g_object_class_install_properties(object_class, NUM_PROPERTIES, props);

 }

 static void pw3270Application_init(pw3270Application *app) {

#ifdef _WIN32
	app->ui_style = PW3270_UI_STYLE_CLASSICAL;
#else
	app->ui_style = PW3270_UI_STYLE_GNOME;
#endif // _WIN32

	// Get settings
	{
		g_autofree gchar * path = g_strconcat("/apps/" PACKAGE_NAME "/", g_get_application_name(),"/",NULL);
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
				"br.com.bb." PACKAGE_NAME,
				TRUE);

		g_settings_schema_source_unref(source);

		app->settings = g_settings_new_full(schema, NULL, path);

#else

		app->settings = g_settings_new_with_path("br.com.bb." PACKAGE_NAME, path);

#endif // DEBUG

	}

	// Bind properties
	if(app->settings) {
		g_object_ref_sink(G_OBJECT(app->settings));
		g_settings_bind(app->settings, "ui-style", app, "ui-style", G_SETTINGS_BIND_DEFAULT);
	}

 }

 static void finalize(GObject *object) {

 	pw3270Application * application = PW3270_APPLICATION(object);

 	if(application->settings) {
		g_object_unref(application->settings);
		application->settings = NULL;
 	}


 	G_OBJECT_CLASS(pw3270Application_parent_class)->finalize(object);

 }

 GtkApplication * pw3270_application_new(const gchar *application_id, GApplicationFlags flags) {

	return g_object_new(
				PW3270_TYPE_APPLICATION,
				"application-id", application_id,
				"flags", flags,
				NULL);

 }

 void startup(GApplication *application) {

	G_APPLICATION_CLASS(pw3270Application_parent_class)->startup(application);

	//
	// Setup application default actions.
	//
	static GActionEntry actions[] = {
		{
			.name = "about",
			.activate = pw3270_application_about_activated,
		},

		{
			.name = "preferences",
			.activate = pw3270_application_preferences_activated,
		},

		{
			.name = "quit",
			.activate = pw3270_application_quit_activated,
		},

		{
			.name = "new.tab",
			.activate = pw3270_application_new_tab_activated,
		},

		{
			.name = "new.window",
			.activate = pw3270_application_new_window_activated,
		},

		{
			.name = "open",
			.activate = pw3270_application_generic_activated,
		},

		{
			.name = "open.tab",
			.activate = pw3270_application_generic_activated,
		},

		{
			.name = "open.window",
			.activate = pw3270_application_generic_activated,
		},

	};

	g_action_map_add_action_entries(
		G_ACTION_MAP(application),
		actions,
		G_N_ELEMENTS(actions),
		application
	);

	//
	// Setup application menus
	//
	GtkBuilder * builder = gtk_builder_new_from_file("ui/application.xml");

	if(gtk_application_prefers_app_menu(GTK_APPLICATION(application)))
		gtk_application_set_app_menu(GTK_APPLICATION (application), G_MENU_MODEL(gtk_builder_get_object (builder, "app-menu")));

	if(pw3270_application_get_ui_style(application) == PW3270_UI_STYLE_CLASSICAL)
		gtk_application_set_menubar(GTK_APPLICATION (application), G_MENU_MODEL(gtk_builder_get_object (builder, "menubar")));

	pw3270_load_placeholders(builder);

	g_object_unref(builder);

 }

 void activate(GApplication *application) {

	size_t ix;

	GtkWidget * window = pw3270_application_window_new(GTK_APPLICATION(application));

	// Create terminal widget & associated widget
	GtkWidget * terminal = pw3270_terminal_new(window);

	// Create property actions
	static const gchar * properties[] = {
		"model-number",
		"font-family",
		"dynamic-font-spacing",
		"trace",
	};

	for(ix = 0; ix < G_N_ELEMENTS(properties); ix++) {

		g_action_map_add_action(
			G_ACTION_MAP(window),
			v3270_property_action_new(terminal,properties[ix])
		);

	}

	// Create conditional actions.
	static const struct {
		const gchar * label;
		const gchar * tooltip;
		const gchar * action_name;
		const gchar * property_name;
		void (*activate)(GAction *action, GVariant *parameter, GtkWidget *terminal);
	} conditional_actions[] = {
		{
			.label = N_("Save copy"),
			.action_name = "save_copy",
			.property_name = "has_copy",
			.activate = pw3270_application_save_copy_activated
		},
		{
			.label = N_("Print copy"),
			.action_name = "print_copy",
			.property_name = "has_copy",
			.activate = pw3270_application_print_copy_activated
		}
	};

	for(ix = 0; ix < G_N_ELEMENTS(conditional_actions); ix++) {

		pw3270SimpleAction * action = PW3270_SIMPLE_ACTION(v3270_conditional_action_new(terminal,conditional_actions[ix].property_name));

		action->parent.name	= conditional_actions[ix].action_name;
		action->label =  conditional_actions[ix].label;
		action->tooltip = conditional_actions[ix].tooltip;
		PW3270_ACTION(action)->activate = conditional_actions[ix].activate;

		g_action_map_add_action(
			G_ACTION_MAP(window),
			G_ACTION(action)
		);

	}

	// Present the new window
	pw3270_window_set_current_page(window,0);
	gtk_window_present(GTK_WINDOW(window));

 }

 void open(GApplication *application, GFile **files, gint n_files, const gchar G_GNUC_UNUSED(*hint)) {

	GtkWindow * window = gtk_application_get_active_window(GTK_APPLICATION(application));

	debug("%s was called with %d files (active_window=%p)", __FUNCTION__, n_files, window);

	if(!window)
		window = GTK_WINDOW(pw3270_application_window_new(GTK_APPLICATION(application)));

	// Add tabs to the window
	gint file;
	gint last = -1;

	for(file = 0; file < n_files; file++) {
		last = pw3270_window_append_page(GTK_WIDGET(window), files[file]);
	}

	gtk_window_present(window);

	if(last != -1)
		pw3270_window_set_current_page(GTK_WIDGET(window),last);

 }

 void pw3270_application_set_ui_style(GApplication *app, PW3270_UI_STYLE type) {

 	g_return_if_fail(PW3270_IS_APPLICATION(app));

	pw3270Application * application = PW3270_APPLICATION(app);

	if(application->ui_style == type)
		return;

	application->ui_style = type;
	g_object_notify_by_pspec(G_OBJECT(app), props[PROP_UI_STYLE]);

 }

 PW3270_UI_STYLE pw3270_application_get_ui_style(GApplication *app) {

 	g_return_val_if_fail(PW3270_IS_APPLICATION(app),PW3270_UI_STYLE_CLASSICAL);
    return PW3270_APPLICATION(app)->ui_style;

 }

 GSettings * pw3270_application_get_settings(GApplication *app) {

	g_return_val_if_fail(PW3270_IS_APPLICATION(app),NULL);
	return PW3270_APPLICATION(app)->settings;

 }
