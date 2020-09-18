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
 #include <pw3270/actions.h>
 #include <pw3270/settings.h>
 #include <pw3270/application.h>
 #include <v3270/dialogs.h>

 static void load(GtkWidget *widget, PW3270SettingsPrivate *settings);
 static void apply(GtkWidget *widget, PW3270SettingsPrivate *settings);

/*--[ Constants ]------------------------------------------------------------------------------------*/

  struct _PW3270SettingsPrivate {
	GtkWidget * views[3];
	GtkTreeModel * model;
 };

 static const struct _views {
	const char * label;
	gint left;
	gint top;
	gint height;
 } views[] = {
	{
		.label = N_("Left"),
		.left = 0,
		.top = 0,
		.height = 4
	},
	{
		.label = N_("Right"),
		.left = 0,
		.top = 6,
		.height = 4
	},
	{
		.label = N_("Available"),
		.left = 2,
		.top = 0,
		.height = 10
	}
 };

 /*--[ Implement ]------------------------------------------------------------------------------------*/

 static void remove_from_left(GtkButton G_GNUC_UNUSED(*button), PW3270SettingsPrivate *settings) {
 	debug("%s(%p)",__FUNCTION__,settings);
 	pw3270_action_view_move_selected(settings->views[0],settings->views[2]);
 }

 static void add_to_left(GtkButton G_GNUC_UNUSED(*button), PW3270SettingsPrivate *settings) {
 	debug("%s(%p)",__FUNCTION__,settings);
 	pw3270_action_view_move_selected(settings->views[2],settings->views[0]);
 }

 static void remove_from_right(GtkButton G_GNUC_UNUSED(*button), PW3270SettingsPrivate *settings) {
 	debug("%s(%p)",__FUNCTION__,settings);
 	pw3270_action_view_move_selected(settings->views[1],settings->views[2]);
 }

 static void add_to_right(GtkButton G_GNUC_UNUSED(*button), PW3270SettingsPrivate *settings) {
 	debug("%s(%p)",__FUNCTION__,settings);
 	pw3270_action_view_move_selected(settings->views[2],settings->views[1]);
 }

 GtkWidget * pw3270_header_settings_new() {

 	size_t ix;

	// Create page widget.
	PW3270Settings 	* settings = pw3270_settings_new();
	settings->label = _("Title bar");
	settings->title = _("Setup title bar");
	settings->apply = apply;
	settings->load = load;

	// Create private data.
	PW3270SettingsPrivate * page = settings->settings = g_new0(PW3270SettingsPrivate,1);

	// Create dialog grid
	GtkGrid * grid = GTK_GRID(gtk_grid_new());
	gtk_grid_set_row_homogeneous(grid,FALSE);
	gtk_grid_set_row_spacing(GTK_GRID(grid),12);
 	gtk_grid_set_column_spacing(GTK_GRID(grid),6);

	gtk_grid_attach(
		GTK_GRID(settings),
		v3270_dialog_section_new(_("Itens"), _("Select the title bar itens"), GTK_WIDGET(grid)),
		0,0,4,3
	);

	//
	// Create views
	//
	{
		GtkTreeSelection * selection;

		for(ix = 0; ix < G_N_ELEMENTS(page->views); ix++) {

			GtkWidget * label = gtk_label_new(gettext(views[ix].label));
			gtk_label_set_xalign(GTK_LABEL(label),0);

			gtk_grid_attach(
				grid,
				label,
				views[ix].left,
				views[ix].top,
				1,1
			);

			page->views[ix] = pw3270_action_view_new();

			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(page->views[ix]));
			gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

			GtkWidget * box = gtk_scrolled_window_new(NULL,NULL);
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(box),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
			gtk_container_add(GTK_CONTAINER(box),page->views[ix]);

			gtk_grid_attach(
				grid,
				box,
				views[ix].left,
				views[ix].top+1,
				1,
				views[ix].height
			);

		}

		gtk_tree_view_set_reorderable(GTK_TREE_VIEW(page->views[0]),TRUE);
		gtk_tree_view_set_reorderable(GTK_TREE_VIEW(page->views[1]),TRUE);
		pw3270_action_view_order_by_label(page->views[2]);

	}

	// Create buttons
	{
		GtkWidget *buttons[] = {
			pw3270_action_view_extract_button_new(page->views[0],"go-next"),
			pw3270_action_view_extract_button_new(page->views[2],"go-previous"),
			pw3270_action_view_extract_button_new(page->views[1],"go-next"),
			pw3270_action_view_extract_button_new(page->views[2],"go-previous")
		};

		g_signal_connect(
			buttons[0],
			"clicked",
			G_CALLBACK(remove_from_left),
			page
		);

		g_signal_connect(
			buttons[1],
			"clicked",
			G_CALLBACK(add_to_left),
			page
		);

		g_signal_connect(
			buttons[2],
			"clicked",
			G_CALLBACK(remove_from_right),
			page
		);

		g_signal_connect(
			buttons[3],
			"clicked",
			G_CALLBACK(add_to_right),
			page
		);


		for(ix = 0; ix < 2; ix++) {
			GtkWidget * box = gtk_button_box_new(GTK_ORIENTATION_VERTICAL);
			gtk_widget_set_hexpand(box,FALSE);
			gtk_widget_set_vexpand(box,FALSE);

			gtk_box_pack_start(GTK_BOX(box),buttons[ix*2],FALSE,FALSE,0);
			gtk_box_pack_end(GTK_BOX(box),buttons[(ix*2)+1],FALSE,FALSE,0);

			gtk_grid_attach(
				grid,
				box,
				views[ix].left+1,
				views[ix].top+2,
				1,
				2
			);


		}

	}


	gtk_widget_show_all(GTK_WIDGET(settings));
	return GTK_WIDGET(settings);
 }

 void load(GtkWidget *widget, PW3270SettingsPrivate *page) {

	size_t view, action;
	g_autoptr(GSettings) settings = pw3270_application_window_settings_new();

 	// Populate views
	Pw3270ActionList * action_list = pw3270_action_list_new(GTK_APPLICATION(g_application_get_default()));

	// Add standard menus
	{
		static const struct menu {
			const gchar * action_name;
			const gchar * label;
			const gchar * icon_name;
		} menus[] = {
			{
				.action_name = "menu.open-menu",
				.label = N_("Application menu"),
				.icon_name = "open-menu-symbolic"
			}
		};

		size_t ix;

		for(ix = 0; ix < G_N_ELEMENTS(menus); ix++) {

			GError *error = NULL;

			GdkPixbuf * pixbuf = gtk_icon_theme_load_icon(
										gtk_icon_theme_get_default(),
										menus[ix].icon_name,
										GTK_ICON_SIZE_MENU,
										GTK_ICON_LOOKUP_GENERIC_FALLBACK,
										&error
									);

			if(error) {
				g_warning(error->message);
				g_error_free(error);
				error = NULL;
			}

			action_list = pw3270_action_list_append(
								action_list,
								gettext(menus[ix].label),
								pixbuf,
								menus[ix].action_name,
								PW3270_ACTION_VIEW_ALLOW_MOVE
							);
		}

	}

	// Load settings
    g_autofree gchar * action_names = g_settings_get_string(settings,"header-action-names");
    gchar **views = g_strsplit(action_names,":",-1);

    for(view = 0; view < 2; view++) {

		if(!views[view])
			break;

		gchar ** actions = g_strsplit(views[view],",",-1);

		for(action = 0; actions[action];action++) {
			action_list = pw3270_action_list_move_action(action_list,actions[action],page->views[view]);
		}

		g_strfreev(actions);
    }

   	g_strfreev(views);

	pw3270_action_view_set_actions(page->views[2], action_list);

	pw3270_action_list_free(action_list);

 }

 void apply(GtkWidget *widget, PW3270SettingsPrivate *page) {

  	g_autofree gchar * left_names = pw3270_action_view_get_action_names(page->views[0]);
 	g_autofree gchar * right_names = pw3270_action_view_get_action_names(page->views[1]);
 	g_autofree gchar * action_names = g_strconcat(left_names,":",right_names,NULL);

 	debug("Header actions: [%s]",action_names);

	g_autoptr(GSettings) settings = pw3270_application_window_settings_new();
	g_settings_set_string(settings,"header-action-names",action_names);

 }

