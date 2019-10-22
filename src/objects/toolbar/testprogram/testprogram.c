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

 #include <config.h>
 #include <pw3270/toolbar.h>
 #include <v3270.h>
 #include <v3270/trace.h>
 #include <lib3270/log.h>

 /*---[ Implement ]----------------------------------------------------------------------------------*/

 static void activate(GtkApplication* app, G_GNUC_UNUSED gpointer user_data) {

	GtkWidget	* window	= gtk_application_window_new(app);
	GtkWidget	* terminal	= v3270_new();
	GtkWidget	* vBox		= gtk_box_new(GTK_ORIENTATION_VERTICAL,2);
	GtkWidget	* notebook	= gtk_notebook_new();
	GtkWidget	* toolbar	= pw3270_toolbar_new();

	{
		// Setup toolbar
		static const char * action_names[] = {
			"reconnect",
			"disconnect",
			"selectall",
			"unselect"
		};

        size_t ix;

        for(ix = 0; ix < G_N_ELEMENTS(action_names); ix++) {
			pw3270_toolbar_insert_lib3270_action(toolbar, lib3270_action_get_by_name(action_names[ix]), -1);
        }

	}

	gtk_box_pack_start(GTK_BOX(vBox),toolbar,FALSE,TRUE,0);
	gtk_box_pack_start(GTK_BOX(vBox),notebook,TRUE,TRUE,0);

	// Create Terminal window
	{
		gtk_widget_set_can_default(terminal,TRUE);

		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),terminal,gtk_label_new("Terminal"));

#ifdef _WIN32
		v3270_set_font_family(terminal,"Droid Sans Mono");
#endif // _WIN32

	}

	// Create trace window
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),v3270_trace_new(terminal),gtk_label_new("Trace"));

	// Setup and show main window
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_window_set_default_size (GTK_WINDOW (window), 800, 500);
	gtk_container_add(GTK_CONTAINER(window),vBox);
	gtk_widget_show_all(window);

	gtk_widget_grab_focus(terminal);

}

int main (int argc, char **argv) {

  GtkApplication *app;
  int status;

  app = gtk_application_new ("br.com.bb.pw3270",G_APPLICATION_FLAGS_NONE);

  g_signal_connect (app, "activate", G_CALLBACK(activate), NULL);

  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  g_message("rc=%d",status);

  return 0;

}


