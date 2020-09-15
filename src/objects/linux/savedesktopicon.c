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
 #include <lib3270.h>
 #include <lib3270/log.h>

 static GtkWidget * factory(V3270SimpleAction *action, GtkWidget *terminal);
 static void response(GtkWidget *dialog, gint response_id, GtkWidget *terminal);

/*

[Desktop Entry]
GenericName=pw3270
Name=pw3270
Comment=Comment
Exec=/usr/bin/sisbb
Icon=pw3270
Terminal=false
Type=Application
StartupNotify=true
Categories=GTK;GNOME;TerminalEmulator
OnlyShowIn=GNOME;Unity
X-Desktop-File-Install-Version=0.23
*/

 static const struct _entry {

	const gchar * key;
	const gchar * label;
	const gchar * tooltip;
	const gchar * default_value;
	gint width;
//	gint n_chars;

 } entries[] = {

	{
		.label = N_("File name"),
		.width = 40,
	},

	{
		.key = "Name",
		.label = N_("Launcher name"),
		.default_value = G_STRINGIFY(PRODUCT_NAME),
		.width = 20,
	},

	{
		.key = "GenericName",
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

		if(entries[ix].default_value) {
			gtk_entry_set_text(GTK_ENTRY(inputs[ix]),gettext(entries[ix].default_value));
		}

		gtk_entry_set_width_chars(GTK_ENTRY(inputs[ix]),entries[ix].width);
//		gtk_entry_set_max_width_chars(GTK_ENTRY(inputs[ix]),entries[ix].n_chars);
		gtk_widget_set_hexpand(inputs[ix],FALSE);
		gtk_widget_set_vexpand(inputs[ix],FALSE);

		gtk_grid_attach(grid,inputs[ix],1,ix,entries[ix].width,1);

	}

	g_autofree gchar * filename = g_strdup_printf("%s/" G_STRINGIFY(PRODUCT_NAME) ".desktop",g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP));

	gtk_entry_set_text(GTK_ENTRY(inputs[0]),filename);

	/*
	gtk_entry_set_placeholder_text(GTK_ENTRY(inputs[1]),G_STRINGIFY(PRODUCT_NAME));
	gtk_entry_set_text(GTK_ENTRY(inputs[1]),G_STRINGIFY(PRODUCT_NAME));

	gtk_entry_set_placeholder_text(GTK_ENTRY(inputs[2]),G_STRINGIFY(PRODUCT_NAME));
	gtk_entry_set_text(GTK_ENTRY(inputs[2]),G_STRINGIFY(PRODUCT_NAME));
	*/

	gtk_entry_set_placeholder_text(GTK_ENTRY(inputs[3]),v3270_get_url(terminal));
	gtk_entry_set_text(GTK_ENTRY(inputs[3]),v3270_get_url(terminal));
	gtk_entry_set_input_hints(GTK_ENTRY(inputs[3]),GTK_INPUT_HINT_SPELLCHECK);

	gtk_widget_show_all(GTK_WIDGET(grid));
	return dialog;
 }

 void response(GtkWidget *dialog, gint response_id, GtkWidget *terminal) {

	debug("%s(%d)",__FUNCTION__,response_id);

	if(response_id == GTK_RESPONSE_APPLY) {

		static const char * key_file_data =
		"[Desktop Entry]\n" \
		"Icon=" G_STRINGIFY(PRODUCT_NAME) "\n"  \
		"Terminal=false\n" \
		"Type=Application\n" \
		"StartupNotify=true\n" \
		"Categories=GTK;GNOME;TerminalEmulator\n" \
		"OnlyShowIn=GNOME;Unity\n";

		GError * error = NULL;
		size_t ix;

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

		// Get session filename
		/*
		const gchar * session_file = v3270_get_session_filename(terminal);

		if(!session_file) {

			// No session file, create one.

			// Check for configdir
			g_autofree gchar * configdir = g_build_filename(g_get_user_config_dir(),G_STRINGIFY(PRODUCT_NAME),"sessions",NULL);
			g_mkdir_with_parents(configdir,0755);

			// Create a base name
			g_autofree gchar * basename = g_path_get_basename(gtk_entry_get_text(GTK_ENTRY(inputs[0])));

			gchar *ptr = strrchr(basename,'.');
			if(ptr)
				*ptr = 0;

			ix = time(NULL);
			gchar * new_session_file = g_strdup_printf("%s/%s.3270",configdir,basename);
			while(!g_file_test(new_session_file,G_FILE_TEST_EXISTS)) {
				g_free(new_session_file);
				new_session_file = g_strdup_printf("%s/%s_%08lx.3270",configdir,basename,(unsigned long) ix++);
			}

			g_message("Saving session to %s",new_session_file);
			v3270_set_session_filename(terminal,new_session_file);
			g_free(new_session_file);

		}
		*/

		// Get program file name
		// https://stackoverflow.com/questions/4517425/how-to-get-program-path
		{
			char buffer[4096];
			g_autofree gchar * pidfile = g_strdup_printf("/proc/%d/exe", getpid());

			int bytes = readlink(pidfile,buffer,4095);

			if(bytes >= 0)
				buffer[bytes] = '\0';

			g_autofree gchar * exec_line = g_strdup_printf("%s \"%s\"",buffer,v3270_key_file_get_file_name(terminal));
			g_key_file_set_string(keyfile,"Desktop Entry","Exec",exec_line);

		}

		g_key_file_save_to_file(keyfile,gtk_entry_get_text(GTK_ENTRY(inputs[0])),&error);

		if(error) {

			g_message("%s",error->message);


			g_error_free(error);

		} else {

			g_message("File \"%s\" was saved",gtk_entry_get_text(GTK_ENTRY(inputs[0])));

		}


		g_key_file_free(keyfile);
	}

 	gtk_widget_destroy(dialog);

}
