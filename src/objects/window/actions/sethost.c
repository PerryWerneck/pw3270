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

 #include "../private.h"
 #include <pw3270/window.h>
 #include <pw3270/actions.h>
 #include <v3270/settings.h>
 #include <v3270/dialogs.h>

 static void activate(GAction G_GNUC_UNUSED(*action), GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal);

 GAction * pw3270_set_host_action_new(void) {

	pw3270Action * action = PW3270_ACTION(pw3270_action_new_from_lib3270(lib3270_action_get_by_name("reconnect")));

	action->activate = activate;
	action->name = "set.host";

	return G_ACTION(action);

 }

 static void on_response(GtkDialog *dialog, gint response_id, GtkWidget *settings) {

	v3270_settings_on_dialog_response(dialog,response_id,settings);

	if(response_id == GTK_RESPONSE_APPLY) {
		v3270_reconnect(v3270_settings_get_terminal_widget(settings));
	}

	gtk_widget_destroy(GTK_WIDGET(dialog));

 }

 void activate(GAction G_GNUC_UNUSED(*action), GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal) {

	if(!GTK_IS_V3270(terminal))
		return;

	GtkWidget * settings = v3270_host_select_new();
	GtkWidget * dialog =
		v3270_settings_dialog_new(
			terminal,
			settings
		);

	if(dialog) {

		v3270_dialog_setup(dialog,_("Setup host"),_("C_onnect"));
		gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

		g_signal_connect(dialog,"close",G_CALLBACK(gtk_widget_destroy),NULL);
		g_signal_connect(dialog,"response",G_CALLBACK(on_response),settings);

		gtk_widget_show_all(dialog);
	}

 }

