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
  * @brief Implement PW3270 "open" actions.
  *
  */

 #include "../private.h"
 #include <v3270.h>
 #include <pw3270/application.h>

 static void set_file_filters(GtkFileChooser *chooser) {

	static const struct Filter {
		const gchar * name;
		const gchar * pattern;
	} filters[] = {
		{
			.name = N_("TN3270 Session Files"),
			.pattern = "*.3270"
		},
		{
			.name = N_("All files"),
			.pattern = "*.*"
		}
	};

	size_t ix;

	for(ix = 0; ix < G_N_ELEMENTS(filters); ix++) {
		GtkFileFilter *filter = gtk_file_filter_new();
		gtk_file_filter_add_pattern (filter, filters[ix].pattern);
		gtk_file_filter_set_name(filter, filters[ix].name);
		gtk_file_chooser_add_filter(chooser,filter);
	}

 }

 static gchar * get_session_file_name(GtkApplication *application, const gchar *title) {

	gchar * filename = NULL;

	GtkWidget *	dialog =
		gtk_file_chooser_dialog_new(
				title,
				gtk_application_get_active_window(application),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				_("Open Session"), GTK_RESPONSE_OK,
				_("Cancel"),GTK_RESPONSE_CANCEL,
				NULL
		);

	set_file_filters(GTK_FILE_CHOOSER(dialog));

	gtk_widget_show_all(dialog);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	}

	gtk_widget_destroy(dialog);

	return filename;
 }

 void pw3270_application_open_activated(GSimpleAction * action, GVariant *parameter, gpointer application) {

 	debug("%s",__FUNCTION__);
 	g_simple_action_set_enabled(action,FALSE);

 	g_simple_action_set_enabled(action,TRUE);
 }

  void pw3270_application_open_tab_activated(GSimpleAction * action, GVariant *parameter, gpointer application) {

 	debug("%s",__FUNCTION__);
 	g_simple_action_set_enabled(action,FALSE);
 	g_autofree gchar * session_file_name = get_session_file_name(GTK_APPLICATION(application),_("Open session in new tab"));


 	g_simple_action_set_enabled(action,TRUE);
 }

 void pw3270_application_open_window_activated(GSimpleAction * action, GVariant *parameter, gpointer application) {

 	debug("%s",__FUNCTION__);
 	g_simple_action_set_enabled(action,FALSE);
 	g_autofree gchar * session_file_name = get_session_file_name(GTK_APPLICATION(application),_("Open session in new window"));


 	g_simple_action_set_enabled(action,TRUE);
 }

