/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob
 * o nome G3270.
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
 * Este programa está nomeado como testprogram.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <v3270.h>
 #include <v3270/filetransfer.h>
 #include <v3270/ftprogress.h>
 #include <stdlib.h>

 /*---[ Implement ]----------------------------------------------------------------------------------*/

static gboolean popup_menu(GtkWidget *widget, gboolean selected, gboolean online, GdkEventButton *event, gpointer user_data) {

	if(!online)
		return FALSE;

	GtkWidget * dialog = v3270ft_new();

	v3270ft_load(dialog,"transfer.xml");

	gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(gtk_widget_get_toplevel(widget)));

	do {

		gtk_widget_show_all(dialog);

		switch(gtk_dialog_run(GTK_DIALOG(dialog))) {
		case GTK_RESPONSE_APPLY:
		case GTK_RESPONSE_OK:
		case GTK_RESPONSE_YES:
			gtk_widget_hide(dialog);
			v3270ft_transfer(dialog,v3270_get_session(widget));
			break;

		case GTK_RESPONSE_CANCEL:
		case GTK_RESPONSE_NO:
		case GTK_RESPONSE_DELETE_EVENT:
			v3270ft_remove_all(dialog);
			break;

		default:
			g_warning("Unexpected response from v3270ft");
		}

	} while(v3270ft_get_length(dialog) > 0);

	gtk_widget_destroy(dialog);

	return TRUE;

}

static void activate(GtkApplication* app, gpointer user_data) {

	const gchar * search[]	= {

		g_get_user_config_dir(),
 		g_get_user_data_dir(),
 		g_get_home_dir(),
 		NULL

	};

	GtkWidget	* window	= gtk_application_window_new(app);
	GtkWidget	* terminal	= v3270_new();
	gchar 		* filename	= NULL;

	// Setup terminal
	GKeyFile 	* conf		= g_key_file_new();

	g_key_file_load_from_dirs(conf,"pw3270.conf",(const gchar **) search, &filename,G_KEY_FILE_NONE,NULL);
	g_message("Configuration from %s",filename);

	// v3270_set_from_keyfile(terminal,conf);

	g_key_file_unref(conf);

	v3270_connect(terminal);

	g_signal_connect(terminal,"popup",G_CALLBACK(popup_menu),NULL);

	// Setup and show window
	gtk_window_set_title(GTK_WINDOW(window), "Window");
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_window_set_default_size (GTK_WINDOW (window), 800, 500);
	gtk_container_add(GTK_CONTAINER(window),terminal);
	gtk_widget_show_all (window);

}

int main (int argc, char **argv) {

  GtkApplication *app;
  int status;

  app = gtk_application_new ("br.com.bb.pw3270",G_APPLICATION_FLAGS_NONE);

  g_signal_connect (app, "activate", G_CALLBACK(activate), NULL);

  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  g_message("rc=%d",status);
  return status;
}

