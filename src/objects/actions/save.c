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
 #include <pw3270.h>
 #include <pw3270/application.h>


 static GtkWidget * factory(pw3270SimpleAction *action, GtkWidget *terminal);
 static void response(GtkWidget *dialog, gint response_id, GtkWidget *terminal);

 GAction * pw3270_action_save_session_as_new(void) {

	pw3270SimpleAction * action = pw3270_dialog_action_new(factory);

	action->parent.name = "save.session.as";
	action->label = _("Save As");
	action->icon_name = "document-save-as";
	action->tooltip = _("Save session properties");

	return G_ACTION(action);

 }

 GtkWidget * factory(pw3270SimpleAction *action, GtkWidget *terminal) {

	GtkWidget *	dialog =
		gtk_file_chooser_dialog_new(
				action->tooltip,
				GTK_WINDOW(gtk_widget_get_toplevel(terminal)),
				GTK_FILE_CHOOSER_ACTION_SAVE,
				_("Save"), GTK_RESPONSE_OK,
				_("Cancel"),GTK_RESPONSE_CANCEL,
				NULL
		);

	gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
	gtk_file_chooser_set_pw3270_filters(GTK_FILE_CHOOSER(dialog));

	if(terminal) {
		const gchar * current_file = v3270_get_session_filename(terminal);
		if(current_file && g_file_test(current_file,G_FILE_TEST_IS_REGULAR) && !g_str_has_prefix(current_file,g_get_user_config_dir()))
			gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),current_file);
	}

	g_signal_connect(dialog,"close",G_CALLBACK(gtk_widget_destroy),NULL);
	g_signal_connect(dialog,"response",G_CALLBACK(response),terminal);

	return dialog;
 }

 void response(GtkWidget *dialog, gint response_id, GtkWidget *terminal) {

	debug("%s(%d)",__FUNCTION__,response_id);

	g_autofree gchar * filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

	gtk_widget_destroy(dialog);

	if(response_id == GTK_RESPONSE_OK) {
		v3270_set_session_filename(terminal, filename);
	}

 }
