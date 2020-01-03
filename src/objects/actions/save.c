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

 static gchar * get_session_file_name(GtkWidget *terminal, const gchar *title) {

	gchar * filename = NULL;

	GtkWidget *	dialog =
		gtk_file_chooser_dialog_new(
				title,
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
		if(current_file)
			gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),current_file);
	}

	gtk_widget_show_all(dialog);
	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	}

	gtk_widget_destroy(dialog);

	return filename;
 }

 static void activate(GAction G_GNUC_UNUSED(*action), GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal) {

	debug("%s",__FUNCTION__);

	g_autofree gchar * filename = get_session_file_name(terminal, _("Save session properties"));

	if(!filename)
		return;



 }

 GAction * pw3270_action_save_session_as_new(void) {

	pw3270SimpleAction * action = pw3270_simple_action_new();

	action->parent.name = "save.session.as";
	action->parent.activate = activate;
	action->label = N_("Save As");
	action->icon_name = "document-save-as";
	action->tooltip = N_("Save session properties to file");

	return G_ACTION(action);

 }
