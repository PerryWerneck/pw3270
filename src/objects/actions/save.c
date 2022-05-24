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

/**
 * @brief Implement PW3270 save actions.
 *
 */

#include "private.h"
#include <v3270.h>
#include <v3270/keyfile.h>
#include <pw3270.h>
#include <pw3270/application.h>
#include <v3270/tools.h>
#include <v3270/settings.h>

static GtkWidget * factory(V3270SimpleAction *action, GtkWidget *terminal);
static void response(GtkWidget *dialog, gint response_id, GtkWidget *terminal);

static const struct Entry {

	const gchar *label;
	const gchar *tooltip;
	const gint width;

} entries[] = {

	// 0 = Session name
	{
		.label = N_("Session name"),
		.tooltip = N_("The session name used in the window/tab title (empty for default)"),
		.width = 15,
	},

	// 1 = Session file
	{
		.label = N_("Session file"),
		.tooltip = N_("The file to save the current session preferences"),
		.width = 40,
	}

};

GAction * pw3270_action_save_session_preferences_new(void) {

	V3270SimpleAction * action = v3270_dialog_action_new(factory);

	action->name = "save.session.preferences";
	action->label = _("Save session preferences");
	action->icon_name = "document-save-as";
	action->tooltip = _("Save current session preferences to file");

	return G_ACTION(action);

}

GtkWidget * factory(V3270SimpleAction *action, GtkWidget *terminal) {

	// Create dialog
	gboolean use_header;
	g_object_get(gtk_settings_get_default(), "gtk-dialogs-use-header", &use_header, NULL);

	GtkWidget *	dialog =
	    GTK_WIDGET(g_object_new(
	                   GTK_TYPE_DIALOG,
	                   "use-header-bar", (use_header ? 1 : 0),
	                   NULL
	               ));


	gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(gtk_widget_get_toplevel(terminal)));
	gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
	gtk_window_set_title(GTK_WINDOW(dialog),action->label);

	gtk_dialog_add_buttons(
	    GTK_DIALOG(dialog),
	    _("_Cancel"), GTK_RESPONSE_CANCEL,
	    _("_Save"), GTK_RESPONSE_APPLY,
	    NULL
	);


	// Create entry fields
	GtkWidget ** inputs = g_new0(GtkWidget *,G_N_ELEMENTS(entries));
	g_object_set_data_full(G_OBJECT(dialog),"inputs",inputs,g_free);
	debug("Dialog=%p inputs=%p",dialog,inputs);

	GtkGrid * grid = GTK_GRID(gtk_grid_new());

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),GTK_WIDGET(grid),TRUE,TRUE,0);

	// https://developer.gnome.org/hig/stable/visual-layout.html.en
	gtk_container_set_border_width(GTK_CONTAINER(grid),18);
	gtk_grid_set_row_spacing(GTK_GRID(grid),6);
	gtk_grid_set_column_spacing(GTK_GRID(grid),12);

	// https://developer.gnome.org/hig/stable/visual-layout.html.en
	// gtk_box_set_spacing(GTK_BOX(content_area),18);

	size_t ix;
	for(ix = 0; ix < G_N_ELEMENTS(entries); ix++) {

		GtkWidget * label = gtk_label_new(gettext(entries[ix].label));
		gtk_label_set_xalign(GTK_LABEL(label),1);
		gtk_grid_attach(grid,label,0,ix,1,1);

		inputs[ix] = gtk_entry_new();

		if(entries[ix].tooltip) {
			gtk_widget_set_tooltip_markup(GTK_WIDGET(inputs[ix]),gettext(entries[ix].tooltip));
		}

		gtk_entry_set_width_chars(GTK_ENTRY(inputs[ix]),entries[ix].width);
		gtk_widget_set_hexpand(inputs[ix],FALSE);
		gtk_widget_set_vexpand(inputs[ix],FALSE);

		gtk_grid_attach(grid,inputs[ix],1,ix,entries[ix].width,1);

	}

	// Load current session name
	{
		g_autofree gchar * session_name = g_strdup(v3270_get_session_name(terminal));
		gchar *ptr = strrchr(session_name,':');
		if(ptr)
			*ptr = 0;
		gtk_entry_set_text(GTK_ENTRY(inputs[0]),session_name);
	}

	// Load current file name
	{
		g_autofree gchar * session_filename = v3270_key_file_build_filename(terminal);
		gtk_entry_set_text(GTK_ENTRY(inputs[1]),session_filename);

		gtk_entry_bind_to_filechooser(
		    inputs[1],
		    GTK_FILE_CHOOSER_ACTION_SAVE,
		    _("Save session preferences"),
		    NULL,
		    "*.3270",
		    _("3270 session files")
		);

	}

	g_signal_connect(dialog,"response",G_CALLBACK(response),terminal);

	gtk_widget_show_all(GTK_WIDGET(grid));
	return dialog;

}

void response(GtkWidget *dialog, gint response_id, GtkWidget *terminal) {

	if(response_id == GTK_RESPONSE_APPLY) {

		GtkWidget ** inputs = g_object_get_data(G_OBJECT(dialog),"inputs");
		gtk_widget_hide(dialog);

		GError * error = NULL;
		v3270_key_file_save_to_file(
		    terminal,
		    gtk_entry_get_text(GTK_ENTRY(inputs[1])),
		    &error
		);

		if(error) {

			g_message("%s",error->message);
			g_error_free(error);

		} else {

			// Set session name (after save to avoid changes on the old session file).
			v3270_set_session_name(terminal,gtk_entry_get_text(GTK_ENTRY(inputs[0])));
			v3270_emit_save_settings(terminal,NULL);

		}

	}


	gtk_widget_destroy(dialog);

}
