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
	g_application_quit(G_APPLICATION(app));
	return FALSE;

}
#endif // G_OS_UNIX

int main (int argc, char **argv) {

	int status = -1;

#ifdef _WIN32
	debug("Process %s running on pid %u\n",argv[0],(unsigned int) GetCurrentProcessId());
#endif // _WIN32

	GtkApplication *app;

	// Setup locale
#ifdef LC_ALL
	setlocale( LC_ALL, "" );
#endif

#ifdef _WIN32
	{
		g_autofree gchar * appdir = g_win32_get_package_installation_directory_of_module(NULL);
		g_autofree gchar * locdir = g_build_filename(appdir,"locale",NULL);
		debug("Locale from \"%s\"\n",locdir);
		bindtextdomain( G_STRINGIFY(PRODUCT_NAME), locdir );
	}
#endif // _WIN32

	bind_textdomain_codeset(G_STRINGIFY(PRODUCT_NAME), "UTF-8");
	textdomain(G_STRINGIFY(PRODUCT_NAME));

	// Setup and start application.
	g_set_application_name(G_STRINGIFY(PRODUCT_NAME));
	app = pw3270_application_new(G_STRINGIFY(APPLICATION_ID),G_APPLICATION_HANDLES_OPEN);

#ifdef G_OS_UNIX
	g_unix_signal_add(SIGTERM, (GSourceFunc) quit_signal, app);
#endif // G_OS_UNIX

	status = g_application_run(G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	debug("%s ends with RC=%d",__FUNCTION__,status);
	return status;

}



