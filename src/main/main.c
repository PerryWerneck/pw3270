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
 * Este programa está nomeado como main.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 /**
  * @brief PW3270 main aplication Entry point
  *
  */

 #include "private.h"
 #include <pw3270/application.h>
 #include <lib3270.h>
 #include <lib3270/log.h>
 #include <locale.h>
 #include <stdlib.h>

 #ifdef G_OS_UNIX
	#include <glib-unix.h>
 #endif // G_OS_UNIX

/*---[ Implement ]----------------------------------------------------------------------------------*/

#ifdef G_OS_UNIX
static gboolean	quit_signal(GtkApplication *app) {

	debug("%s",__FUNCTION__);
	g_message("Terminating by signal");

	/*
	GList *list = gtk_application_get_windows(GTK_APPLICATION(application));

	while(list) {

		GtkWidget * window = GTK_WIDGET(list->data);
		list = list->next;

		gtk_widget_destroy(window);

	}
	*/

	g_application_quit(G_APPLICATION(app));
	return FALSE;
}
#endif // G_OS_UNIX

static void g_log_to_lib3270(const gchar *log_domain,GLogLevelFlags G_GNUC_UNUSED(log_level),const gchar *message,gpointer G_GNUC_UNUSED(user_data)) {
	lib3270_write_log(NULL,log_domain ? log_domain : G_STRINGIFY(PRODUCT_NAME),"%s",message);
}

int main (int argc, char **argv) {

	GtkApplication *app;
	int status;

	// Setup locale
#ifdef LC_ALL
	setlocale( LC_ALL, "" );
#endif

#ifdef _WIN32
	{
		g_autofree gchar * appdir = g_win32_get_package_installation_directory_of_module(NULL);
		g_autofree gchar * locdir = g_build_filename(appdir,"locale",NULL);
		bindtextdomain( PACKAGE_NAME, locdir );
	}
#endif // _WIN32

	g_log_set_default_handler(g_log_to_lib3270,NULL);

	bind_textdomain_codeset(PACKAGE_NAME, "UTF-8");
	textdomain(PACKAGE_NAME);

	// Setup and start application.

	g_set_application_name(G_STRINGIFY(PRODUCT_NAME));
	app = pw3270_application_new("br.com.bb." G_STRINGIFY(PRODUCT_NAME),G_APPLICATION_HANDLES_OPEN);

#ifdef G_OS_UNIX
	// Termination
	g_unix_signal_add(SIGTERM, (GSourceFunc) quit_signal, app);
#endif // G_OS_UNIX

	status = g_application_run(G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	return status;

}



