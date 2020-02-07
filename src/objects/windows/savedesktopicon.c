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
  * @brief Implement Windows version of the save desktop icon action.
  *
  * References:
  *
  * <https://stackoverflow.com/questions/3906974/how-to-programmatically-create-a-shortcut-using-win32>
  * <https://docs.microsoft.com/pt-br/windows/win32/shell/links?redirectedfrom=MSDN>
  *
  */

 #include <winsock2.h>
 #include <windows.h>

 #include <v3270.h>
 #include <pw3270.h>
 #include <pw3270/application.h>
 #include <v3270/actions.h>
 #include <lib3270.h>
 #include <lib3270/log.h>

 static GtkWidget * factory(V3270SimpleAction *action, GtkWidget *terminal);
 static void response(GtkWidget *dialog, gint response_id, GtkWidget *terminal);

 static const struct _entry {

	const gchar * label;
	const gchar * tooltip;
	gint width;

 } entries[] = {

	{
		.label = N_("File name"),
		.width = 40,
	},

	{
		.label = N_("Launcher name"),
		.width = 20,
	}

 };

 GAction * pw3270_action_save_desktop_icon_new(void) {

	V3270SimpleAction * action = v3270_dialog_action_new(factory);

	action->name = "save.launcher";
	action->label = _("Save desktop icon");
	action->tooltip = _("Create a desktop icon for the current session");

	return G_ACTION(action);

 }

 GtkWidget * factory(V3270SimpleAction *action, GtkWidget *terminal) {

	size_t ix;

	gboolean use_header;
	g_object_get(gtk_settings_get_default(), "gtk-dialogs-use-header", &use_header, NULL);

	GtkWidget *	dialog =
		GTK_WIDGET(g_object_new(
			GTK_TYPE_DIALOG,
			"use-header-bar", (use_header ? 1 : 0),
			NULL
		));

	gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
	gtk_window_set_title(GTK_WINDOW(dialog),action->label);

	gtk_dialog_add_buttons(
		GTK_DIALOG(dialog),
		_("_Cancel"), GTK_RESPONSE_CANCEL,
		_("_Save"), GTK_RESPONSE_APPLY,
		NULL
	);

	g_signal_connect(dialog,"response",G_CALLBACK(response),terminal);

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

	for(ix = 0; ix < G_N_ELEMENTS(entries); ix++) {

		GtkWidget * label = gtk_label_new(gettext(entries[ix].label));
		gtk_label_set_xalign(GTK_LABEL(label),1);
		gtk_grid_attach(grid,label,0,ix,1,1);

		inputs[ix] = gtk_entry_new();
		debug("inputs[%u]=%p",(unsigned int) ix, inputs[ix]);

		gtk_entry_set_width_chars(GTK_ENTRY(inputs[ix]),entries[ix].width);
//		gtk_entry_set_max_width_chars(GTK_ENTRY(inputs[ix]),entries[ix].n_chars);
		gtk_widget_set_hexpand(inputs[ix],FALSE);
		gtk_widget_set_vexpand(inputs[ix],FALSE);

		gtk_grid_attach(grid,inputs[ix],1,ix,entries[ix].width,1);

	}

	/*

	gtk_entry_set_text(GTK_ENTRY(inputs[0]),filename);

	gtk_entry_set_placeholder_text(GTK_ENTRY(inputs[1]),G_STRINGIFY(PRODUCT_NAME));
	gtk_entry_set_text(GTK_ENTRY(inputs[1]),G_STRINGIFY(PRODUCT_NAME));

	gtk_entry_set_placeholder_text(GTK_ENTRY(inputs[2]),G_STRINGIFY(PRODUCT_NAME));
	gtk_entry_set_text(GTK_ENTRY(inputs[2]),G_STRINGIFY(PRODUCT_NAME));

	gtk_entry_set_placeholder_text(GTK_ENTRY(inputs[3]),v3270_get_url(terminal));
	gtk_entry_set_text(GTK_ENTRY(inputs[3]),v3270_get_url(terminal));
	gtk_entry_set_input_hints(GTK_ENTRY(inputs[3]),GTK_INPUT_HINT_SPELLCHECK);

	*/

	gtk_widget_show_all(GTK_WIDGET(grid));
	return dialog;
 }

 void response(GtkWidget *dialog, gint response_id, GtkWidget *terminal) {

	debug("%s(%d)",__FUNCTION__,response_id);

	if(response_id == GTK_RESPONSE_APPLY) {

	}

 	gtk_widget_destroy(dialog);

}
