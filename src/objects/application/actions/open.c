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
 #include <pw3270.h>
 #include <pw3270/application.h>
 #include <pw3270/actions.h>

 static GtkWidget * session_dialog_new(PW3270Action *action, GtkApplication *application) {

	GtkWidget *	dialog =
		gtk_file_chooser_dialog_new(
				action->label,
				gtk_application_get_active_window(application),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				_("Open Session"), GTK_RESPONSE_OK,
				_("Cancel"),GTK_RESPONSE_CANCEL,
				NULL
		);

	gtk_file_chooser_set_pw3270_filters(GTK_FILE_CHOOSER(dialog));
	gtk_widget_show_all(dialog);

	return dialog;
 }

 static void open_window(GtkWidget *dialog, gint response_id, GtkApplication *application) {

 	if(response_id == GTK_RESPONSE_OK) {

		g_autofree gchar * file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

		if(file_name) {
			GtkWidget *window = pw3270_application_window_new(application, file_name);
			pw3270_window_set_current_page(window,0);
			gtk_window_present(GTK_WINDOW(window));
		}

 	}

 	gtk_widget_destroy(dialog);

 }

 static void open_tab(GtkWidget *dialog, gint response_id, GtkApplication *application) {

 	if(response_id == GTK_RESPONSE_OK) {

		g_autofree gchar * file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

		if(file_name) {

			pw3270_application_window_new_tab(GTK_WIDGET(gtk_application_get_active_window(GTK_APPLICATION(application))), file_name);

		}

 	}

 	gtk_widget_destroy(dialog);

 }

 static GtkWidget * open_window_factory(PW3270Action *action, GtkApplication *application) {

	GtkWidget * dialog = session_dialog_new(action,application);
	g_signal_connect(dialog,"response",G_CALLBACK(open_window),application);
	return dialog;

 }

 static GtkWidget * open_tab_factory(PW3270Action *action, GtkApplication *application) {

	GtkWidget * dialog = session_dialog_new(action,application);
	g_signal_connect(dialog,"response",G_CALLBACK(open_tab),application);
	return dialog;

 }

 GAction * pw3270_open_window_action_new() {

	PW3270Action * action = pw3270_dialog_action_new(open_window_factory);

 	action->name = "open.session.window";
	action->label = _( "Open in new window" );
	action->tooltip = _( "Open session in New window" );
	action->icon_name = "window-new";

	return G_ACTION(action);
 }

 GAction * pw3270_open_tab_action_new() {

	PW3270Action * action = pw3270_dialog_action_new(open_tab_factory);

 	action->name = "open.session.tab";
	action->label = _( "Open in new tab" );
	action->tooltip = _( "Open session in New Tab" );
	action->icon_name = "tab-new";

	return G_ACTION(action);
 }

