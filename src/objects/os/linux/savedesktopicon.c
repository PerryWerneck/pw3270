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
  * @brief Implement Linux version of the save desktop icon action.
  *
  */

 #include <v3270.h>
 #include <pw3270.h>
 #include <pw3270/application.h>
 #include <v3270/actions.h>
 #include <v3270/keyfile.h>
 #include <v3270/settings.h>
 #include <lib3270.h>
 #include <lib3270/log.h>
 #include <lib3270/properties.h>
 #include <pw3270/tools.h>

 static GtkWidget * factory(V3270SimpleAction *action, GtkWidget *terminal);
 static void response(GtkWidget *dialog, gint response_id, GtkWidget *terminal);

 static const struct _entry {

	const gchar * key;
	const gchar * label;
	const gchar * tooltip;
	const gchar * default_value;
	gint margin_top;
	gint width;

 } entries[] = {

	// 0 = Shortcut name
	{
		.key = "Name",
		.label = N_("Shortcut name"),
		.default_value = G_STRINGIFY(PRODUCT_NAME),
		.width = 20,
	},

	// 1 = Shortcut file
	{
		.label = N_("Shortcut file"),
		.tooltip = N_("Path for the new shortcut"),
		.width = 40,
	},

	// 2 = Session name
	{
		.label = N_("Session name"),
		.margin_top = 12,
		.tooltip = N_("The session name used in the window/tab title (empty for default)"),
		.width = 15,
	},

	// 3 = Session file
	{
		.label = N_("Session file"),
		.tooltip = N_("The file with the session preferences for this shortcut"),
		.width = 40,
	},

	// 4 = Generic name.
	{
		.key = "GenericName",
		.margin_top = 12,
		.label = N_("Generic name"),
		.default_value = G_STRINGIFY(PRODUCT_NAME),
		.width = 20,
	},

	{
		.key = "Comment",
		.label = N_("Comment"),
		.default_value = N_("IBM 3270 Terminal emulator"),
		.width = 30,
	}

 };

 GAction * pw3270_action_save_desktop_icon_new(void) {

	V3270SimpleAction * action = v3270_dialog_action_new(factory);

	action->name = "save.launcher";
	action->label = _("Save desktop icon");
	action->tooltip = _("Create a desktop icon for the current session");

	return G_ACTION(action);

 }

 static gchar * get_filename(GtkWidget *terminal) {

 	g_autofree gchar * defname = v3270_keyfile_get_default_filename();
	const gchar * current = v3270_key_file_get_filename(terminal);

	// If is not the default name, return it.
	if(strcmp(defname,current)) {
		return g_strdup(current);
	}

	// It's the default one, create a new one on the user_config dir
	g_autofree gchar * config_path = v3270_key_file_get_default_path(terminal);

	// Use the hostname
	const char * hostname = lib3270_host_get_name(v3270_get_session(terminal));
	if(!hostname) {
		hostname = G_STRINGIFY(PRODUCT_NAME);
	}

	// Build the filename
	gchar *filename = g_strconcat(config_path,G_DIR_SEPARATOR_S,hostname,".3270",NULL);

	unsigned int index = 0;
	while(g_file_test(filename,G_FILE_TEST_EXISTS)) {
		g_free(filename);
		filename = g_strdup_printf("%s" G_DIR_SEPARATOR_S "%s.%u.3270",config_path,hostname,++index);
	}

	return filename;

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

		if(entries[ix].margin_top) {
			gtk_widget_set_margin_top(label,entries[ix].margin_top);
			gtk_widget_set_margin_top(inputs[ix],entries[ix].margin_top);
		}

		if(entries[ix].default_value) {
			gtk_entry_set_text(GTK_ENTRY(inputs[ix]),gettext(entries[ix].default_value));
		}

		if(entries[ix].tooltip) {
			gtk_widget_set_tooltip_markup(GTK_WIDGET(inputs[ix]),gettext(entries[ix].tooltip));
		}

		gtk_entry_set_width_chars(GTK_ENTRY(inputs[ix]),entries[ix].width);
		gtk_widget_set_hexpand(inputs[ix],FALSE);
		gtk_widget_set_vexpand(inputs[ix],FALSE);

		gtk_grid_attach(grid,inputs[ix],1,ix,entries[ix].width,1);

	}

	g_autofree gchar * filename = g_strdup_printf("%s/" G_STRINGIFY(PRODUCT_NAME) ".desktop",g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP));

	// 1 = Shortcut filename
	{
		gtk_entry_set_text(GTK_ENTRY(inputs[1]),filename);
		gtk_entry_bind_to_filechooser(
			inputs[1],
			GTK_FILE_CHOOSER_ACTION_SAVE,
			_("Save to shortcut file"),
			NULL,
			"*.desktop",
			_("Standard desktop files")
		);

	}

	// 2 = Session name
	{
		gchar * session_name = g_strdup(v3270_get_session_name(terminal));
		gchar * ptr = strchr(session_name,':');
		if(ptr)
			*ptr = 0;

		if(strcmp(session_name,G_STRINGIFY(PRODUCT_NAME)))
			gtk_entry_set_text(GTK_ENTRY(inputs[2]),session_name);

	}

	// 3 = Session filename
	{
		g_autofree gchar * session_filename = get_filename(terminal);
		gtk_entry_set_text(GTK_ENTRY(inputs[3]),session_filename);

		gtk_entry_bind_to_filechooser(
			inputs[3],
			GTK_FILE_CHOOSER_ACTION_SAVE,
			_("Save to session filename"),
			NULL,
			"*.3270",
			_("3270 session files")
		);

	}

	// 4 = Generic name
	gtk_entry_set_placeholder_text(GTK_ENTRY(inputs[4]),v3270_get_url(terminal));
	gtk_entry_set_text(GTK_ENTRY(inputs[4]),v3270_get_url(terminal));
	gtk_entry_set_input_hints(GTK_ENTRY(inputs[4]),GTK_INPUT_HINT_SPELLCHECK);

	gtk_widget_show_all(GTK_WIDGET(grid));
	return dialog;
 }

 static void apply(GtkWidget *dialog, GtkWidget *terminal) {

	GError * error = NULL;
	size_t ix;

	static const char * key_file_data =
		"[Desktop Entry]\n" \
		"Icon=" G_STRINGIFY(PRODUCT_NAME) "\n"  \
		"Terminal=false\n" \
		"Type=Application\n" \
		"StartupNotify=true\n" \
		"Categories=GTK;GNOME;TerminalEmulator\n" \
		"OnlyShowIn=GNOME;Unity\n";

	GKeyFile * keyfile = g_key_file_new();
	g_key_file_load_from_data(keyfile,key_file_data,-1,G_KEY_FILE_NONE,NULL);

#ifdef DEBUG
	{
		g_autofree gchar * dbg_data = g_key_file_to_data(keyfile,NULL,NULL);
		debug("\n%s\n",dbg_data);
	}
#endif // DEBUG


	GtkWidget ** inputs = g_object_get_data(G_OBJECT(dialog),"inputs");
	debug("dialog=%p inputs=%p",dialog,inputs);

	for(ix = 0; ix < G_N_ELEMENTS(entries); ix++) {
		if(entries[ix].key) {
			debug("inputs[%u]=%p",(unsigned int) ix, inputs[ix]);
			g_key_file_set_string(keyfile,"Desktop Entry",entries[ix].key,gtk_entry_get_text(GTK_ENTRY(inputs[ix])));
		}
	}

	// Save keyfile
	v3270_key_file_save_to_file(
		terminal,
		gtk_entry_get_text(GTK_ENTRY(inputs[3])),
		&error
	);

	// Get program file name
	// https://stackoverflow.com/questions/4517425/how-to-get-program-path
	if(!error) {
		char buffer[4096];
		g_autofree gchar * pidfile = g_strdup_printf("/proc/%d/exe", getpid());

		int bytes = readlink(pidfile,buffer,4095);

		if(bytes >= 0)
			buffer[bytes] = '\0';

		g_autofree gchar * exec_line =
								g_strconcat(
									buffer,
									" \"",gtk_entry_get_text(GTK_ENTRY(inputs[3])),"\"",
									NULL
								);

		g_key_file_set_string(keyfile,"Desktop Entry","Exec",exec_line);

	}

	// Save shortcude
	g_key_file_save_to_file(keyfile,gtk_entry_get_text(GTK_ENTRY(inputs[1])),&error);

	g_key_file_free(keyfile);

	if(error) {

		g_message("%s",error->message);
		g_error_free(error);

	} else {

		// Set session name (after save to avoid changes on the old session file).
		v3270_set_session_name(terminal,gtk_entry_get_text(GTK_ENTRY(inputs[2])));
		v3270_emit_save_settings(terminal,NULL);

	}


}

void response(GtkWidget *dialog, gint response_id, GtkWidget *terminal) {

	debug("%s(%d)",__FUNCTION__,response_id);

	gtk_widget_hide(dialog);
	if(response_id == GTK_RESPONSE_APPLY) {
		apply(dialog,terminal);
	}

	gtk_widget_destroy(dialog);

}


