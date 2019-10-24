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
 #include <pw3270/application.h>

 struct _pw3270ApplicationClass {
 	GtkApplicationClass parent_class;
 };

 struct _pw3270Application {
 	GtkApplication parent;
 };

 static void 		  startup(GApplication * application);
 static void 		  activate(GApplication * application);
 static void 		  open(GApplication * application, GFile **files, gint n_files, const gchar *hint);

 G_DEFINE_TYPE(pw3270Application, pw3270Application, GTK_TYPE_APPLICATION);

 static void pw3270Application_class_init(pw3270ApplicationClass *klass) {

	GApplicationClass *application_class = G_APPLICATION_CLASS(klass);

	application_class->startup = startup;
	application_class->activate = activate;
	application_class->open = open;

 }

 static void pw3270Application_init(pw3270Application *app) {


 }

 GtkApplication * pw3270_application_new(const gchar *application_id, GApplicationFlags flags) {

	return g_object_new(
				PW3270_TYPE_APPLICATION,
				"application-id", application_id,
				"flags", G_APPLICATION_HANDLES_OPEN,
				NULL);

 }

 static void action_activated(GSimpleAction * action, GVariant *parameter, gpointer application) {

	debug("%s",__FUNCTION__);

 }

 void startup(GApplication *application) {

	G_APPLICATION_CLASS(pw3270Application_parent_class)->startup(application);

	//
	// Setup application default actions.
	//
	static GActionEntry app_entries[] = {
		{
			.name = "about",
			.activate = action_activated,
		},
		{
			.name = "preferences",
			.activate = action_activated,
		},

		{
			.name = "quit",
			.activate = pw3270_application_quit_activated,
		},

		{
			.name = "new_tab",
			.activate = pw3270_application_new_tab_activated,
		},

		{
			.name = "new_window",
			.activate = pw3270_application_new_window_activated,
		}

	};

	g_action_map_add_action_entries(
		G_ACTION_MAP(application),
		app_entries,
		G_N_ELEMENTS(app_entries),
		application
	);

	//
	// Setup application menus
	//
	GtkBuilder * builder = gtk_builder_new_from_file("ui/application.xml");
	gtk_application_set_app_menu(GTK_APPLICATION (application), G_MENU_MODEL(gtk_builder_get_object (builder, "app-menu")));

	if(pw3270_application_get_ui_type(application) == PW3270_UI_STYLE_CLASSICAL)
		gtk_application_set_menubar(GTK_APPLICATION (application), G_MENU_MODEL(gtk_builder_get_object (builder, "menubar")));

	g_object_unref(builder);

 }

 void activate(GApplication *application) {

	GtkWidget * window = pw3270_application_window_new(GTK_APPLICATION(application));

	// Create terminal widget
	pw3270_terminal_new(window);

	// Present the new window
	gtk_window_present(GTK_WINDOW(window));
	pw3270_window_set_current_page(window,0);

 }

 void open(GApplication *application, GFile **files, gint n_files, const gchar *hint) {

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

 PW3270_UI_TYPE pw3270_application_get_ui_type(GApplication *app) {
 	return PW3270_UI_STYLE_GNOME;
 }
