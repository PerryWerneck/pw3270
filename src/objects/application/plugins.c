/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2008 Banco do Brasil S.A.
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

#include "private.h"

void pw3270_load_plugins_from_path(pw3270Application *app, const char *path) {

	if(g_file_test(path,G_FILE_TEST_IS_DIR)) {

		g_message("Loading plugins from %s",path);

		GError	* err   = NULL;
		GDir	* dir 	= g_dir_open(path,0,&err);

		if(dir) {

			const gchar *name;
			while((name = g_dir_read_name(dir)) != NULL) {

				g_autofree gchar *filename = g_build_filename(path,name,NULL);

				if(g_str_has_suffix(filename,G_MODULE_SUFFIX)) {

					g_message("Loading %s",filename);

					GModule *handle = g_module_open(filename,G_MODULE_BIND_LOCAL);

					if(handle) {

						app->plugins = g_slist_append(app->plugins,handle);

					} else {

						g_warning("Can't load %s: %s",filename,g_module_error());

					}

				}

			}

			g_dir_close(dir);
		}

		if(err) {

			g_warning("Can't load plugins from %s: %s",path,err->message);
			g_error_free(err);

		}

	} else {

		g_warning("Can't load plugins from %s: %s",path,"Invalid path");

	}

}
