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

 #include <config.h>
 #include <pw3270.h>

 gchar * pw3270_build_data_path(const char *name) {

 		g_autofree gchar * pkgdir = g_win32_get_package_installation_directory_of_module(NULL);

		gchar * path = g_build_filename(pkgdir,name,NULL);
		if(g_file_test(path,G_FILE_TEST_IS_DIR)) {
			return path;
		}
		g_free(path);

		path = g_build_filename(pkgdir,"share",G_STRINGIFY(PRODUCT_NAME),name,NULL);
		if(g_file_test(path,G_FILE_TEST_IS_DIR)) {
			return path;
		}

		g_free(path);
		g_message("Cant find path for '%s'",path);
		return NULL;

 }

 gchar * pw3270_build_data_filename(const char *filename) {

 		g_autofree gchar * pkgdir = g_win32_get_package_installation_directory_of_module(NULL);

		gchar * path = g_build_filename(pkgdir,filename,NULL);
		if(g_file_test(path,G_FILE_TEST_IS_REGULAR)) {
			return path;
		}
		g_free(path);

		path = g_build_filename(pkgdir,"share",G_STRINGIFY(PRODUCT_NAME),filename,NULL);
		if(g_file_test(path,G_FILE_TEST_IS_REGULAR)) {
			return path;
		}

		g_free(path);
		g_error("Cant find '%s'",filename);
		return NULL;
 }
