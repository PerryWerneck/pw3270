/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) <2008> <Banco do Brasil S.A.>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
		g_autofree gchar * pkgdir = g_win32_get_package_installation_directory_of_module(NULL);
		{
			g_autofree gchar * appdir = g_build_filename(pkgdir,"locale",NULL);
			if(g_file_test(appdir,G_FILE_TEST_IS_DIR)) {
				bindtextdomain( PACKAGE_NAME, appdir );
			} else {
				g_autofree gchar * sysdir = g_build_filename(pkgdir,"share","locale",NULL);
				if(g_file_test(sysdir,G_FILE_TEST_IS_DIR)) {
					bindtextdomain( PACKAGE_NAME, sysdir );
				}
			}

		}
	}
#endif // _WIN32

	bind_textdomain_codeset(PACKAGE_NAME, "UTF-8");
	textdomain(PACKAGE_NAME);

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



