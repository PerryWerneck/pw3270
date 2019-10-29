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
 #include <v3270/settings.h>
 #include <v3270/dialogs.h>

 static void on_response(GtkDialog *dialog, gint response_id, GtkWidget *settings) {

	v3270_settings_on_dialog_response(dialog,response_id,settings);

	if(response_id == GTK_RESPONSE_APPLY) {
		v3270_reconnect(v3270_settings_get_terminal_widget(settings));
	}

	gtk_widget_destroy(GTK_WIDGET(dialog));

 }

 void pw3270_window_set_host_activated(GSimpleAction G_GNUC_UNUSED(* action), GVariant G_GNUC_UNUSED(*parameter), gpointer window) {

	debug("%s",__FUNCTION__);

	if(!PW3270_IS_APPLICATION_WINDOW(window))
		return;

	GtkWidget * terminal = gtk_window_get_default_widget(GTK_WINDOW(window));
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

		gtk_window_set_default_size(GTK_WINDOW(dialog), 700, 150);

		g_signal_connect(dialog,"close",G_CALLBACK(gtk_widget_destroy),NULL);
		g_signal_connect(dialog,"response",G_CALLBACK(on_response),settings);

		gtk_widget_show_all(dialog);
	}

 }

