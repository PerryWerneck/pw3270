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

 G_DEFINE_TYPE(pw3270ApplicationWindow, pw3270ApplicationWindow, GTK_TYPE_APPLICATION_WINDOW);

 static void destroy(GtkWidget *widget) {

	debug("%s(%p)",__FUNCTION__,widget);

	// Update actions
	size_t ix;
	gchar ** actions = g_action_group_list_actions(G_ACTION_GROUP(widget));

	for(ix = 0; actions[ix]; ix++) {
		GAction * action = g_action_map_lookup_action(G_ACTION_MAP(widget), actions[ix]);

		if(action && PW3270_IS_ACTION(action)) {
			pw3270_action_set_terminal_widget(action,NULL);
		}

	}

	g_strfreev(actions);

	GTK_WIDGET_CLASS(pw3270ApplicationWindow_parent_class)->destroy(widget);

 }

 static void pw3270ApplicationWindow_class_init(pw3270ApplicationWindowClass *klass) {

	GtkWidgetClass * widget = GTK_WIDGET_CLASS(klass);
	widget->destroy = destroy;


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
			pw3270_set_host_action_new(),
			pw3270_set_color_action_new(),
			pw3270_session_preferences_action_new(),
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
		static const gchar *actions[] = {

			"win.copy",
			"win.paste",
			"win.select-all",
			"separator",
			"win.connect",
			"win.disconnect",
			"separator",
			"win.set.colors",
			"win.preferences",
			"win.file.transfer",
			"win.print",
			"app.quit"

		};

		size_t ix;

		for(ix = 0; ix < G_N_ELEMENTS(actions); ix++) {
			pw3270_toolbar_insert_action(GTK_WIDGET(widget->toolbar),actions[ix],-1);
		}

	}

 }

 GtkWidget * pw3270_application_window_new(GtkApplication * application) {

	const gchar * title = _( "IBM 3270 Terminal emulator" );

	g_autoptr(GSettings) settings = pw3270_application_get_settings(G_APPLICATION(application));

	g_return_val_if_fail(GTK_IS_APPLICATION(application), NULL);
	pw3270ApplicationWindow * window =
		g_object_new(
			PW3270_TYPE_APPLICATION_WINDOW,
			"application", application,
			NULL);

	if(PW3270_IS_APPLICATION(gtk_window_get_application(GTK_WINDOW(window)))) {

		GtkBuilder * builder = gtk_builder_new_from_file("ui/window.xml");

		switch(pw3270_application_get_ui_style(G_APPLICATION(application))) {
		case PW3270_UI_STYLE_CLASSICAL:
			{
				gtk_window_set_title(GTK_WINDOW(window), title);

			}
			break;

		case PW3270_UI_STYLE_GNOME:
			{
				// Create header bar
				GtkHeaderBar * header = GTK_HEADER_BAR(gtk_header_bar_new());
				gtk_window_set_titlebar(GTK_WINDOW(window), GTK_WIDGET(header));
				gtk_header_bar_set_show_close_button(header,TRUE);

				gtk_header_bar_set_title(header,title);
				if(settings)
					g_settings_bind(settings, "has-subtitle", header, "has-subtitle", G_SETTINGS_BIND_DEFAULT);
				else
					gtk_header_bar_set_has_subtitle(header,TRUE);

				// Create gear button
				// https://wiki.gnome.org/Initiatives/GnomeGoals/GearIcons
				GtkWidget *	gear_menu = pw3270_setup_image_button(gtk_menu_button_new(),"open-menu-symbolic");
				gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(gear_menu), G_MENU_MODEL(gtk_builder_get_object (builder, "gear-menu")));
				gtk_header_bar_pack_end(header, gear_menu);

				// Create "new tab" bar
				GtkWidget * new_tab_button = pw3270_setup_image_button(gtk_button_new(),"tab-new-symbolic");
				gtk_actionable_set_action_name(GTK_ACTIONABLE(new_tab_button),"app.new.tab");
				gtk_header_bar_pack_start(header, new_tab_button);

				// Show the new header
				gtk_widget_show_all(GTK_WIDGET(header));
			}
			break;

		default:
			g_warning("Unexpected UI");

		}

		g_object_unref(builder);

	}

	// Setup and show main window
	gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(window),TRUE);

	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_window_set_default_size (GTK_WINDOW (window), 800, 500);

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

