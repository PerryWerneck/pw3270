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

#ifdef _WIN32
	#include <winsock2.h>
	#include <windows.h>
#endif // _WIN32

 #include <pw3270.h>
 #include <pw3270/settings.h>
 #include <lib3270.h>
 #include <lib3270/log.h>
 #include <pw3270/actions.h>

 struct _PW3270SettingsActions {
  	GtkGrid parent;
	GtkWidget * views[3];
	GtkTreeModel * model;
 };

 struct _PW3270SettingsActionsClass {
 	GtkGridClass parent;

 	int dunno;
 };

 G_DEFINE_TYPE(PW3270SettingsActions, PW3270SettingsActions, GTK_TYPE_GRID);

 static void PW3270SettingsActions_class_init(PW3270SettingsActionsClass *klass) {

 }

 static void PW3270SettingsActions_init(PW3270SettingsActions *grid) {

	size_t ix;

 	static const struct View {
		const gchar *label;
		const gchar *tooltip;
	} views[G_N_ELEMENTS(grid->views)] = {
		{
			.label = N_("Start"),
			.tooltip = N_("Items packed from the start to the end")
		},

		{
			.label = N_("Available"),
			.tooltip = N_("List of the available and unpacked actions")
		},

		{
			.label = N_("End"),
			.tooltip = N_("Items packed from the end to the start")
		}
 	};

	gtk_grid_set_row_homogeneous(GTK_GRID(grid),FALSE);
	gtk_grid_set_row_spacing(GTK_GRID(grid),12);
 	gtk_grid_set_column_spacing(GTK_GRID(grid),6);

 	{
 		// Create views
 		GtkTreeSelection * selection;
 		GtkWidget *box;

		for(ix = 0; ix < G_N_ELEMENTS(grid->views); ix++) {

			// Create label.
			GtkWidget * label = gtk_label_new(gettext(views[ix].label));
			gtk_widget_set_tooltip_markup(label,gettext(views[ix].tooltip));
			//gtk_label_set_xalign(GTK_LABEL(label),0);
			gtk_widget_set_hexpand(label,TRUE);
			gtk_widget_set_vexpand(label,FALSE);

			gtk_grid_attach(
				GTK_GRID(grid),
				label,
				(ix*2),0,
				1,1
			);

			// Create view
			grid->views[ix] = pw3270_action_view_new();
			gtk_widget_set_hexpand(grid->views[ix],TRUE);
			gtk_widget_set_vexpand(grid->views[ix],TRUE);

			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(grid->views[ix]));
			gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

			GtkWidget * box = gtk_scrolled_window_new(NULL,NULL);
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(box),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
			gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(box),GTK_SHADOW_ETCHED_IN);
			gtk_container_add(GTK_CONTAINER(box),grid->views[ix]);

			gtk_grid_attach(
				GTK_GRID(grid),
				box,
				(ix*2),1,
				1,5
			);

		}

 	}

 	// Create buttons
 	{
		GtkWidget *buttons[] = {
			pw3270_action_view_move_button_new(grid->views[1],grid->views[0],"go-previous"),
			pw3270_action_view_move_button_new(grid->views[1],grid->views[2],"go-next"),
			pw3270_action_view_move_button_new(grid->views[0],grid->views[1],"go-next"),
			pw3270_action_view_move_button_new(grid->views[2],grid->views[1],"go-previous")
		};

		gtk_grid_attach(GTK_GRID(grid),buttons[0],1,2,1,1);
		gtk_grid_attach(GTK_GRID(grid),buttons[1],3,2,1,1);

		gtk_grid_attach(GTK_GRID(grid),buttons[2],1,3,1,1);
		gtk_grid_attach(GTK_GRID(grid),buttons[3],3,3,1,1);

 	}

 }

 GtkWidget * pw3270_settings_actions_new() {

	return GTK_WIDGET(g_object_new(
			GTK_TYPE_PW3270_SETTINGS_ACTIONS,
			NULL
		));

 }

 Pw3270ActionList * pw3270_settings_action_load(GtkWidget *widget, Pw3270ActionList *action_list, const gchar *action_names) {

	PW3270SettingsActions *editor = (PW3270SettingsActions *) widget;

	static const unsigned short columns[] = { 0, 2 };
	unsigned short column;
	size_t action;

	gchar **views = g_strsplit(action_names,":",-1);

	for(column = 0; column < G_N_ELEMENTS(columns); column++) {

		if(!views[column])
			break;

		gchar ** actions = g_strsplit(views[column],",",-1);

		for(action = 0; actions[action];action++) {
			action_list = pw3270_action_list_move_action(
								action_list,
								actions[action],
								editor->views[columns[column]]
							);
		}

		g_strfreev(actions);
    }

   	g_strfreev(views);

	pw3270_action_view_set_actions(editor->views[1], action_list);

	return action_list;
 }

 /*
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


*/
