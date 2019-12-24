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
 #include <pw3270/settings.h>
 #include <pw3270/actions.h>

 typedef struct _ToolbarSettingsPage {
	Pw3270SettingsPage parent;
	GtkWidget * views[2];
	GtkWidget * buttons[2];

 } ToolbarSettingsPage;

 static void load(Pw3270SettingsPage *pg, GtkApplication *application, GSettings *settings) {

 	size_t ix;

 	debug("%s",__FUNCTION__);

 	// Populate views
	Pw3270ActionList * action_list = pw3270_action_list_new(application);

    // Load current values.
    g_autofree gchar * action_names = g_settings_get_string(settings,"toolbar-action-names");

 	gchar ** actions = g_strsplit(action_names,",",-1);

	for(ix = 0; actions[ix]; ix++) {

		if(g_ascii_strcasecmp(actions[ix],"separator")) {

			// It's an action
			action_list = pw3270_action_list_move_action(action_list,actions[ix],((ToolbarSettingsPage *) pg)->views[0]);

		} else {

			// It's a separator
			pw3270_action_view_append(((ToolbarSettingsPage *) pg)->views[0], _( "Separator"), NULL, "separator", 2);

		}

	}

	g_strfreev(actions);

	// Load available actions.
	pw3270_action_view_set_actions(((ToolbarSettingsPage *) pg)->views[1], action_list);
	pw3270_action_view_append(((ToolbarSettingsPage *) pg)->views[1], _( "Separator"), NULL, "separator", 1);

	pw3270_action_list_free(action_list);


 }

 static void apply(Pw3270SettingsPage *pg, GtkApplication G_GNUC_UNUSED(*application), GSettings *settings) {

 	debug("%s",__FUNCTION__);

 	g_autofree gchar * action_names = pw3270_action_view_get_action_names(((ToolbarSettingsPage *) pg)->views[0]);
	g_settings_set_string(settings,"toolbar-action-names",action_names);

 	debug("[%s]",action_names);

 }

 static void selection_changed(GtkTreeSelection *selection, GtkWidget *button) {
	gtk_widget_set_sensitive(button,gtk_tree_selection_count_selected_rows(selection) > 0);
 }

 void toolbar_insert(GtkButton G_GNUC_UNUSED(*button), ToolbarSettingsPage *settings) {
 	debug("%s(%p)",__FUNCTION__,settings);
 	pw3270_action_view_move_selected(settings->views[1],settings->views[0]);
 }

 void toolbar_remove(GtkButton G_GNUC_UNUSED(*button), ToolbarSettingsPage *settings) {
 	debug("%s(%p)",__FUNCTION__,settings);
 	pw3270_action_view_move_selected(settings->views[0],settings->views[1]);
 }

 Pw3270SettingsPage * pw3270_toolbar_settings_new() {

	size_t ix;
	GtkTreeSelection * selection;

	ToolbarSettingsPage * page = g_new0(ToolbarSettingsPage,1);

	page->parent.load = load;
	page->parent.apply = apply;
	page->parent.label = _("Toolbar");
	page->parent.title = _("Setup toolbar");

	page->parent.widget = gtk_grid_new();
	gtk_grid_set_row_homogeneous(GTK_GRID(page->parent.widget),FALSE);

	// Create views
	static const gchar *labels[G_N_ELEMENTS(page->views)] = {
		N_("Selected"),
		N_("Available")
	};

	for(ix = 0; ix < G_N_ELEMENTS(page->views); ix++) {

		GtkWidget * label = gtk_label_new(gettext(labels[ix]));
		gtk_label_set_xalign(GTK_LABEL(label),0);

		page->views[ix] = pw3270_action_view_new();

		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(page->views[ix]));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);

		gtk_grid_attach(
			GTK_GRID(page->parent.widget),
			label,
			ix * 3,0,2,1
		);

		GtkWidget * box = gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(box),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
		gtk_container_add(GTK_CONTAINER(box),page->views[ix]);

		gtk_grid_attach(
			GTK_GRID(page->parent.widget),
			box,
			ix * 3,1,2,4
		);

	}

	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(page->views[0]),TRUE);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(gtk_tree_view_get_model(GTK_TREE_VIEW(page->views[1]))), GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID, GTK_SORT_ASCENDING);

	// Create buttons
	static const gchar * icon_names[G_N_ELEMENTS(page->buttons)] = {
		"go-next",
		"go-previous"
	};

	GtkWidget * box = gtk_button_box_new(GTK_ORIENTATION_VERTICAL);
	gtk_widget_set_hexpand(box,FALSE);
	gtk_widget_set_vexpand(box,FALSE);

	for(ix = 0; ix < G_N_ELEMENTS(icon_names); ix++) {
		page->buttons[ix] = gtk_button_new_from_icon_name(icon_names[ix],GTK_ICON_SIZE_DND);

		gtk_widget_set_focus_on_click(page->buttons[ix],FALSE);
		gtk_button_set_relief(GTK_BUTTON(page->buttons[ix]),GTK_RELIEF_NONE);
		gtk_widget_set_sensitive(page->buttons[ix],FALSE);

		g_signal_connect(
			gtk_tree_view_get_selection(GTK_TREE_VIEW(page->views[ix])),
			"changed",
			G_CALLBACK(selection_changed),
			page->buttons[ix]
		);

	}

	gtk_box_pack_start(GTK_BOX(box),page->buttons[0],FALSE,FALSE,0);
	gtk_box_pack_end(GTK_BOX(box),page->buttons[1],FALSE,FALSE,0);

	g_signal_connect(
		page->buttons[0],
		"clicked",
		G_CALLBACK(toolbar_remove),
		page
	);

	g_signal_connect(
		page->buttons[1],
		"clicked",
		G_CALLBACK(toolbar_insert),
		page
	);

	gtk_grid_attach(
		GTK_GRID(page->parent.widget),
		box,
		2,2,1,1
	);

	return (Pw3270SettingsPage *) page;
 }
