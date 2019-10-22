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
 #include <pw3270/window.h>
 #include <pw3270/toolbar.h>
 #include <v3270.h>
 #include <v3270/trace.h>
 #include <lib3270/log.h>

 /*---[ Implement ]----------------------------------------------------------------------------------*/

 static void activate(GtkApplication* app, G_GNUC_UNUSED gpointer user_data) {

	GtkWidget * window = pw3270_application_window_new(app);

	// Create terminal widget
	pw3270_terminal_new(window);
	pw3270_terminal_new(window);

	// Setup and show main window
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_window_set_default_size (GTK_WINDOW (window), 800, 500);
	gtk_widget_show_all(window);

}

GtkWidget * pw3270_toolbar_new(void) {

	static const struct _item {
		const gchar *icon;
		const gchar *label;
	} itens[] = {

		{
			"gtk-connect",
			"_Connect"
		},

		{
			"gtk-disconnect",
			"_Disconnect"
		}

	};

	GtkWidget * toolbar = gtk_toolbar_new();
	size_t item;

	for(item = 0; item < G_N_ELEMENTS(itens); item++) {

		GtkToolItem * button = gtk_tool_button_new(gtk_image_new_from_icon_name(itens[item].icon,GTK_ICON_SIZE_LARGE_TOOLBAR),itens[item].label);
		gtk_tool_button_set_use_underline(GTK_TOOL_BUTTON(button),TRUE);
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), button, -1);

	}

	return toolbar;
}

static void preferences_activated(GSimpleAction * action, GtkApplication *application) {

	debug("%s",__FUNCTION__);

}

static void quit_activated(GSimpleAction * action, GVariant *parameter, gpointer application) {

	debug("%s",__FUNCTION__);

}

void startup(GtkApplication *application, gpointer user_data) {

	static GActionEntry app_entries[] = {
		{
			"app.about",
			quit_activated,
			NULL,
			NULL,
			NULL
		},
		{
			"app.help",
			quit_activated,
			NULL,
			NULL,
			NULL
		},
		{
			"app.preferences",
			preferences_activated,
			NULL,
			NULL,
			NULL
		},
		{
			"app.quit",
			quit_activated,
			NULL,
			NULL,
			NULL
		}
	};

	g_action_map_add_action_entries(
		G_ACTION_MAP(application),
		app_entries,
		G_N_ELEMENTS(app_entries),
		application
	);

	GtkBuilder * builder = gtk_builder_new_from_file("application.ui");

	debug("Builder: %p",builder);

	GMenuModel * app_menu = G_MENU_MODEL(gtk_builder_get_object(builder, "app-menu"));
	debug("app-menu: %p", app_menu);
	gtk_application_set_app_menu(application, app_menu);


	// gtk_application_set_menubar(application, G_MENU_MODEL(gtk_builder_get_object(builder, "app-menubar")));

	g_object_unref(builder);
}

int main (int argc, char **argv) {

  GtkApplication *app;
  int status;

  app = gtk_application_new ("br.com.bb.pw3270",G_APPLICATION_FLAGS_NONE);

  g_signal_connect (app, "activate", G_CALLBACK(activate), NULL);
  g_signal_connect (app, "startup", G_CALLBACK(startup), NULL);

  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  g_message("rc=%d",status);

  return 0;

}


