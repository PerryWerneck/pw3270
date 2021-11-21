/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2008 Banco do Brasil S.A.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "private.h"
#include <pw3270.h>
#include <pw3270/application.h>
#include <pw3270/actions.h>
#include <pw3270/keypad.h>
#include <stdlib.h>

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

	GSettings	* settings;
	GList		* keypads;
	gchar 		* logfile;
	GSList		* plugins;		///< @brief Handlers of the loaded plugins.
	PW3270_UI_STYLE	ui_style;

};

static void 	startup(GApplication * application);
static void 	activate(GApplication * application);
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

static void window_added(GtkApplication *application, GtkWindow *window) {

	GTK_APPLICATION_CLASS(pw3270Application_parent_class)->window_added(application,window);

	void (*call)(GtkWindow *window);

	GSList * item;
	for(item = PW3270_APPLICATION(application)->plugins; item; item = g_slist_next(item)) {
		if(g_module_symbol((GModule *) item->data, "pw3270_plugin_window_added", (gpointer *) &call)) {
			call(window);
		}
	}

}

static void window_removed(GtkApplication *application, GtkWindow *window) {

	debug("%s(%p)",__FUNCTION__,window);

	void (*call)(GtkWindow *window);

	GSList * item;
	for(item = PW3270_APPLICATION(application)->plugins; item; item = g_slist_next(item)) {
		if(g_module_symbol((GModule *) item->data, "pw3270_plugin_window_removed", (gpointer *) &call)) {
			call(window);
		}
	}

	GTK_APPLICATION_CLASS(pw3270Application_parent_class)->window_removed(application,window);

}

static void g_log_to_lib3270(const gchar *log_domain,GLogLevelFlags G_GNUC_UNUSED(log_level),const gchar *message,gpointer G_GNUC_UNUSED(user_data)) {
	debug("%s",message);
	lib3270_write_log(
		NULL,
		log_domain ? log_domain : "gtk",
		"%s",
		message
	);
}

static void pw3270Application_class_init(pw3270ApplicationClass *klass) {

	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	g_log_set_default_handler(g_log_to_lib3270,NULL);

	object_class->get_property = get_property;
	object_class->set_property = set_property;
	object_class->finalize = finalize;

	{
		GtkApplicationClass *application_class = GTK_APPLICATION_CLASS(klass);

		application_class->window_added = window_added;
		application_class->window_removed = window_removed;

	}

	{
		GApplicationClass * application_class = G_APPLICATION_CLASS(klass);

		application_class->startup = startup;
		application_class->activate = activate;
		application_class->open = pw3270_application_open;

	}

	props[PROP_UI_STYLE] =
	    g_param_spec_uint(
	        "ui-style",
	        _("UI Type"),
	        _("The code of the User interface type"),
	        PW3270_UI_STYLE_CLASSICAL,
	        PW3270_UI_STYLE_AUTOMATIC,
#ifdef _WIN32
	        PW3270_UI_STYLE_CLASSICAL,
#else
	        PW3270_UI_STYLE_AUTOMATIC,
#endif // _WIN32
	        G_PARAM_READABLE|G_PARAM_WRITABLE
	    );


	g_object_class_install_properties(object_class, NUM_PROPERTIES, props);

}

static gboolean on_user_interface(const gchar G_GNUC_UNUSED(*option), const gchar *value, gpointer G_GNUC_UNUSED(dunno), GError **error) {

	g_autoptr(GSettings) app_settings = pw3270_application_settings_new();
	if(!app_settings) {
		g_warning("Can't get application settings");
		return FALSE;
	}

	g_autoptr(GSettings) win_settings = pw3270_application_window_settings_new();
	if(!win_settings) {
		g_warning("Can't get window settings");
		return FALSE;
	}

	if(!g_ascii_strcasecmp(value,"gnome")) {

		g_settings_set_uint(app_settings,"ui-style",PW3270_UI_STYLE_GNOME);
		g_settings_set_boolean(win_settings,"toolbar-visible",TRUE);
		g_settings_set_boolean(win_settings,"menubar-visible",FALSE);

	} else if(!g_ascii_strcasecmp(value,"classic")) {

		g_settings_set_uint(app_settings,"ui-style",PW3270_UI_STYLE_CLASSICAL);
		g_settings_set_boolean(win_settings,"toolbar-visible",TRUE);
		g_settings_set_boolean(win_settings,"menubar-visible",TRUE);

	} else if(!g_ascii_strcasecmp(value,"default")) {

		g_settings_set_uint(app_settings,"ui-style",PW3270_UI_STYLE_AUTOMATIC);

	} else {

		g_set_error(
		    error,
		    g_quark_from_static_string(G_STRINGIFY(PRODUCT_NAME)),
		    EINVAL,
		    _( "\"%s\" is not a valid user interface name" ), value
		);

	}

	return TRUE;

}

static gboolean on_logfile(const gchar G_GNUC_UNUSED(*option), const gchar *value, gpointer G_GNUC_UNUSED(dunno), GError G_GNUC_UNUSED(**error)) {
	pw3270_application_set_log_filename(g_application_get_default(),value);
	return TRUE;
}

static void pw3270Application_init(pw3270Application *app) {

	static GOptionEntry cmd_options[] = {

		{ "user-interface", 'U', 0, G_OPTION_ARG_CALLBACK,	&on_user_interface, N_( "Set the user-interface type" ),  NULL },
		{ "logfile",		'l', 0, G_OPTION_ARG_CALLBACK,	&on_logfile, N_( "Set default log file name" ),  NULL },
		{ NULL }

	};

	g_application_add_main_option_entries(G_APPLICATION(app), cmd_options);

#ifdef _WIN32
	app->ui_style = PW3270_UI_STYLE_CLASSICAL;
#else
	app->ui_style = PW3270_UI_STYLE_AUTOMATIC;
#endif // _WIN32

	// Get settings
	app->settings = pw3270_application_settings_new();

	// Bind properties
	if(app->settings) {

		g_object_ref_sink(G_OBJECT(app->settings));

#ifdef _WIN32
		{
			// https://stackoverflow.com/questions/37035936/how-to-get-native-windows-decorations-on-gtk3-on-windows-7-and-msys2
			int gtk_csd = g_settings_get_int(app->settings,"gtk-csd");
			if(gtk_csd != -1) {
				g_autofree gchar * env = g_strdup_printf("GTK_CSD=%d",gtk_csd);
				putenv(env);
			}
		}
#endif // _WIN32

		g_settings_bind(app->settings, "ui-style", app, "ui-style", G_SETTINGS_BIND_DEFAULT);
	}

	// Get plugins.
	{
#ifdef _WIN32
		lib3270_autoptr(char) path = lib3270_build_data_filename("plugins",NULL);
#else
		const gchar * path = G_STRINGIFY(LIBDIR) G_DIR_SEPARATOR_S G_STRINGIFY(PRODUCT_NAME) "-plugins";
#endif // _WIN32

		if(g_file_test(path,G_FILE_TEST_IS_DIR)) {

			g_message("Loading plugins from %s",path);

			GError	* err   = NULL;
			GDir	* dir 	= g_dir_open(path,0,&err);

			if(dir) {

				const gchar *name;
				while((name = g_dir_read_name(dir)) != NULL) {

					g_autofree gchar *filename = g_build_filename(path,name,NULL);

					if(g_str_has_suffix(filename,G_MODULE_SUFFIX)) {

						g_message("Loading %s",filename);

						GModule *handle = g_module_open(filename,G_MODULE_BIND_LOCAL);

						if(handle) {

							app->plugins = g_slist_append(app->plugins,handle);

						} else {

							g_warning("Can't load %s: %s",filename,g_module_error());

						}

					}

				}

				g_dir_close(dir);
			}

			if(err) {

				g_warning("Can't load plugins from %s: %s",path,err->message);
				g_error_free(err);

			}

		}

	}

}


static void finalize(GObject *object) {

	pw3270Application * application = PW3270_APPLICATION(object);

	if(application->plugins) {
#pragma GCC diagnostic push
#ifdef _WIN32
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif // _WIN32
		g_slist_free_full(application->plugins,(GDestroyNotify) g_module_close);
#pragma GCC diagnostic pop
		application->plugins = NULL;
	}

	if(application->settings) {
		g_object_unref(application->settings);
		application->settings = NULL;
	}

	lib3270_set_log_handler(NULL,NULL,NULL);

	if(application->logfile) {
		g_free(application->logfile);
		application->logfile = NULL;
	}

	g_list_free_full(application->keypads,g_object_unref);

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

	size_t ix;

	pw3270Application * app = PW3270_APPLICATION(application);

	G_APPLICATION_CLASS(pw3270Application_parent_class)->startup(application);

	GSettings *settings = pw3270_application_get_settings(application);

	//
	// Common actions
	//
	GAction * actions[] = {
		pw3270_about_action_new(),
		pw3270_preferences_action_new(),
		pw3270_quit_action_new()
	};

	for(ix = 0; ix < G_N_ELEMENTS(actions); ix++) {
		g_action_map_add_action(G_ACTION_MAP(application),actions[ix]);
	}

	//
	// Open session actions.
	//
	if(g_settings_get_boolean(settings,"allow-open-session-actions")) {
		g_action_map_add_action(G_ACTION_MAP(application),pw3270_open_session_action_new());
	}

	//
	// New tab actions
	//
	if(g_settings_get_boolean(settings,"allow-new-tab-actions")) {

		GAction * new_tab_actions[] = {
			pw3270_open_tab_action_new(),
			pw3270_new_tab_action_new()
		};

		for(ix = 0; ix < G_N_ELEMENTS(new_tab_actions); ix++) {
			g_action_map_add_action(G_ACTION_MAP(application),new_tab_actions[ix]);
		}

	}

	//
	// New window actions
	//
	if(g_settings_get_boolean(settings,"allow-new-window-actions")) {

		GAction * new_window_actions[] = {
			pw3270_open_window_action_new(),
			pw3270_new_window_action_new()
		};

		for(ix = 0; ix < G_N_ELEMENTS(new_window_actions); ix++) {
			g_action_map_add_action(G_ACTION_MAP(application),new_window_actions[ix]);
		}

	}

	//
	// Setup application menus
	//
	GtkBuilder * builder;
#ifdef DEBUG
	builder = gtk_builder_new_from_file("ui/application.xml");
#else
	{
		lib3270_autoptr(char) build_file = lib3270_build_data_filename("ui","application.xml",NULL);
		builder = gtk_builder_new_from_file(build_file);
	}
#endif // DEBUG


	//
	// Load keypad models
	//
	{
		lib3270_autoptr(char) keypad_path = lib3270_build_data_filename("keypad",NULL);

		g_autoptr(GError) error = NULL;
		g_autoptr(GDir) dir = g_dir_open(keypad_path,0,&error);

		if(dir) {

			const gchar *name = g_dir_read_name(dir);
			while(!error && name) {
				g_autofree gchar * path = g_build_filename(keypad_path,name,NULL);
				app->keypads = pw3270_keypad_model_new_from_xml(app->keypads,path);
				name = g_dir_read_name(dir);
			}

		}

		if(error) {
			g_message("Can't read %s: %s",keypad_path,error->message);
		}
	}

	if(gtk_application_prefers_app_menu(GTK_APPLICATION(application)))
		gtk_application_set_app_menu(GTK_APPLICATION (application), G_MENU_MODEL(gtk_builder_get_object (builder, "app-menu")));

	gtk_application_set_menubar(GTK_APPLICATION (application), G_MENU_MODEL(gtk_builder_get_object (builder, "menubar")));

	pw3270_load_placeholders(application, builder);

	g_object_unref(builder);

}

void activate(GApplication *application) {

	GtkWidget * window = pw3270_application_window_new(GTK_APPLICATION(application),NULL);

	// Present the new window
	pw3270_window_set_current_page(window,0);
	gtk_window_present(GTK_WINDOW(window));

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

GSList * pw3270_application_get_plugins(GApplication *app) {

	g_return_val_if_fail(PW3270_IS_APPLICATION(app),NULL);
	return PW3270_APPLICATION(app)->plugins;

}

void pw3270_application_plugin_foreach(GApplication *app, GFunc func, gpointer user_data) {

	g_return_if_fail(PW3270_IS_APPLICATION(app));

	GSList * item;
	for(item = PW3270_APPLICATION(app)->plugins; item; item = g_slist_next(item)) {
		func(item->data,user_data);
	}

}

void pw3270_application_plugin_call(GApplication *app, const gchar *method, gpointer user_data) {

	g_return_if_fail(PW3270_IS_APPLICATION(app));

	int (*call)(GtkWidget *);

	GSList * item;
	for(item = PW3270_APPLICATION(app)->plugins; item; item = g_slist_next(item)) {
		if(g_module_symbol((GModule *) item->data, method, (gpointer *) &call)) {
			call(user_data);
		}
	}

}


GList * pw3270_application_get_keypad_models(GApplication *app) {
	g_return_val_if_fail(PW3270_IS_APPLICATION(app),NULL);
	return PW3270_APPLICATION(app)->keypads;
}

static int loghandler(const H3270 G_GNUC_UNUSED(*hSession), pw3270Application *app, const char *module, int G_GNUC_UNUSED(code), const char *message) {

	if(!app->logfile) {
		return -1;
	}

	FILE *f = fopen(app->logfile,"a");

	if(f) {
		time_t ltime = time(0);

	   char timestamp[80];
#ifdef HAVE_LOCALTIME_R
		struct tm tm;
		strftime(timestamp, 79, "%x %X", localtime_r(&ltime,&tm));
#else
		strftime(timestamp, 79, "%x %X", localtime(&ltime));
#endif // HAVE_LOCALTIME_R

		fprintf(f,"%s %s\t%s\n",timestamp,module,message);

		fclose(f);
	}

	return 0;
}

void pw3270_application_set_log_filename(GApplication *app, const gchar *filename) {

	g_return_if_fail(PW3270_IS_APPLICATION(app));

	pw3270Application * application = PW3270_APPLICATION(app);

	if(application->logfile) {
		g_free(application->logfile);
		application->logfile = NULL;
	}

	if(filename) {
		application->logfile = g_strdup(filename);
		lib3270_set_log_handler(NULL,(LIB3270_LOG_HANDLER) loghandler, app);
	} else {
		lib3270_set_log_handler(NULL,NULL,NULL);
	}

}

