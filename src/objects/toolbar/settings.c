/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes paul.mattes@case.edu), de emulação de terminal 3270 para acesso a
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
#include <pw3270/settings.h>
#include <pw3270/toolbar.h>
#include <pw3270/actions.h>
#include <pw3270/window.h>
#include <v3270/dialogs.h>

static void load(GtkWidget *widget, GSettings *settings, PW3270SettingsPage *page);
static void apply(GtkWidget *widget, GSettings *settings, PW3270SettingsPage *page);

/*--[ Constants ]------------------------------------------------------------------------------------*/

static const struct _comboboxes {
	const gchar * name;			///< @brief The gsettings name.
	const gchar * label;		///< @brief The combo name.
	guint left;
	guint top;
} comboboxes[] = {

	{
		.left = 0,
		.top = 0,
		.name = "toolbar-icon-size",
		.label = N_("Icon Size"),
	},

	{
		.left = 3,
		.top = 0,
		.name = "toolbar-icon-type",
		.label = N_("Icon Style")
	},

	{
		.left = 0,
		.top = 1,
		.name = "toolbar-style",
		.label = N_("Toolbar Style")
	},

	{
		.left = 3,
		.top = 1,
		.name = "toolbar-position",
		.label = N_("Toolbar position")
	}

};

struct _PW3270SettingsPage {
	GtkWidget * views[2];
	GtkWidget * combos[G_N_ELEMENTS(comboboxes)];
};

/*--[ Implement ]------------------------------------------------------------------------------------*/

GtkWidget * pw3270_toolbar_settings_new() {

	size_t ix;

	// Create page widget.
	PW3270Settings 	* settings = pw3270_settings_new();
	settings->label = _("Toolbar");
	settings->title = _("Setup toolbar");
	settings->apply = apply;
	settings->load = load;

	// Create private data.
	PW3270SettingsPage * page = settings->settings = g_new0(PW3270SettingsPage,1);

	//
	// Create views
	//
	{
		GtkTreeSelection * selection;

		GtkGrid * grid = GTK_GRID(gtk_grid_new());
		gtk_grid_set_row_homogeneous(grid,FALSE);

		gtk_grid_attach(
		    GTK_GRID(settings),
		    v3270_dialog_section_new(_("Itens"), _("Select the toolbar itens"), GTK_WIDGET(grid)),
		    0,0,4,3
		);

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
			    grid,
			    label,
			    ix * 3,0,2,1
			);

			GtkWidget * box = gtk_scrolled_window_new(NULL,NULL);
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(box),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
			gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(box),GTK_SHADOW_ETCHED_IN);
			gtk_container_add(GTK_CONTAINER(box),page->views[ix]);

			gtk_grid_attach(
			    grid,
			    box,
			    ix * 3,1,2,4
			);

		}

		gtk_tree_view_set_reorderable(GTK_TREE_VIEW(page->views[0]),TRUE);
		pw3270_action_view_order_by_label(page->views[1]);

		// Create buttons
		GtkWidget * box = gtk_button_box_new(GTK_ORIENTATION_VERTICAL);
		gtk_widget_set_hexpand(box,FALSE);
		gtk_widget_set_vexpand(box,FALSE);

		GtkWidget * buttons[] = {
			pw3270_action_view_move_button_new(page->views[0],page->views[1],"go-next"),
			pw3270_action_view_move_button_new(page->views[1],page->views[0],"go-previous"),
		};

		gtk_box_pack_start(GTK_BOX(box),buttons[0],FALSE,FALSE,0);
		gtk_box_pack_end(GTK_BOX(box),buttons[1],FALSE,FALSE,0);

		gtk_grid_attach(
		    grid,
		    box,
		    2,2,1,1
		);

	}

	//
	// Create Combos.
	//
	{
		GtkGrid * grid = GTK_GRID(gtk_grid_new());

		gtk_grid_attach(
		    GTK_GRID(settings),
		    v3270_dialog_section_new(_("Style & Position"), _("Setup the toolbar style and position"), GTK_WIDGET(grid)),
		    0,5,4,1
		);

		// https://developer.gnome.org/hig/stable/visual-layout.html.en
		gtk_grid_set_row_spacing(grid,6);
		gtk_grid_set_column_spacing(grid,12);
		gtk_widget_set_hexpand(GTK_WIDGET(grid),TRUE);

		GtkCellRenderer * renderer	= gtk_cell_renderer_text_new();

		for(ix = 0; ix < G_N_ELEMENTS(page->combos); ix++) {

			GtkTreeModel *model = pw3270_model_from_name(comboboxes[ix].name);

			GtkWidget * label = gtk_label_new(gettext(comboboxes[ix].label));
			gtk_label_set_xalign(GTK_LABEL(label),1);

			gtk_grid_attach(grid,label,comboboxes[ix].left,comboboxes[ix].top,1,1);

			page->combos[ix] = gtk_combo_box_new_with_model(model);
			gtk_widget_set_hexpand(page->combos[ix],FALSE);

			gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(page->combos[ix]), renderer, TRUE);
			gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(page->combos[ix]), renderer, "text", 0, NULL);

			gtk_grid_attach(grid,page->combos[ix],comboboxes[ix].left+1,comboboxes[ix].top,2,1);

		}

		{
			GtkWidget * label = gtk_label_new("");
			gtk_label_set_markup(GTK_LABEL(label),_("<small><b>The toolbar position only takes effect after restart</b></small>"));
			g_object_set(G_OBJECT(label),"margin-top",12,NULL);
			gtk_grid_attach(grid,label,0,3,6,1);
		}

	}

	return GTK_WIDGET(settings);
}

void load(GtkWidget *widget, GSettings *settings, PW3270SettingsPage *page) {

	size_t ix;

	if(!G_IS_SETTINGS(settings)) {
		g_warning("The settings object is not valid, disabling dialog to avoid segfaults");
		gtk_widget_set_sensitive(widget,FALSE);
		return;
	}

	// Populate views
	Pw3270ActionList * action_list = pw3270_action_list_new(GTK_APPLICATION(g_application_get_default()));

	// Load current values.
	g_autofree gchar * action_names = g_settings_get_string(settings,"toolbar-action-names");

	gchar ** actions = g_strsplit(action_names,",",-1);

	for(ix = 0; actions[ix]; ix++) {

		if(g_ascii_strcasecmp(actions[ix],"separator")) {

			// It's an action
			action_list = pw3270_action_list_move_action(action_list,actions[ix],page->views[0]);

		} else {

			// It's a separator
			pw3270_action_view_append(page->views[0], _( "Separator"), NULL, "separator", 2);

		}

	}

	g_strfreev(actions);

	// Load available actions.
	pw3270_action_view_set_actions(page->views[1], action_list);
	pw3270_action_view_append(page->views[1], _( "Separator"), NULL, "separator", PW3270_ACTION_VIEW_FLAG_ALLOW_ADD);

	pw3270_action_list_free(action_list);

	GtkTreeIter	iter;
	for(ix = 0; ix < G_N_ELEMENTS(page->combos); ix++) {

		pw3270_model_get_iter_from_value(
		    gtk_combo_box_get_model(GTK_COMBO_BOX(page->combos[ix])),
		    &iter,
		    (guint) g_settings_get_int(settings,comboboxes[ix].name)
		);

		gtk_combo_box_set_active_iter(GTK_COMBO_BOX(page->combos[ix]),&iter);

	}

}

void apply(GtkWidget G_GNUC_UNUSED(*widget), GSettings *settings, PW3270SettingsPage *page) {

	size_t ix;

	g_autofree gchar * action_names = pw3270_action_view_get_action_names(page->views[0]);
	g_settings_set_string(settings,"toolbar-action-names",action_names);

	GtkTreeIter	iter;
	for(ix = 0; ix < G_N_ELEMENTS(page->combos); ix++) {

		if(gtk_combo_box_get_active_iter(GTK_COMBO_BOX(page->combos[ix]),&iter)) {

			g_settings_set_int(
			    settings,
			    comboboxes[ix].name,
			    (gint) pw3270_model_get_value_from_iter(gtk_combo_box_get_model(GTK_COMBO_BOX(page->combos[ix])),&iter)
			);

		}

	}

}

