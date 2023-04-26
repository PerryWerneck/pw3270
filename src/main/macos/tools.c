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
 #include <sys/syslimits.h>
 #include <CoreFoundation/CFBundle.h>
 #include <CoreFoundation/CFURL.h>

 static gchar * get_path_from_bundle(const char *name) {

	size_t szBuffer = PATH_MAX;
	char buffer[PATH_MAX+1];
	memset(buffer,0,PATH_MAX+1]);

	CFBundleRef mainBundle = CFBundleGetMainBundle();

	if (mainBundle) {

		CFURLRef url = CFBundleCopyBundleURL(mainBundle);

		if (url) {

			CFURLGetFileSystemRepresentation(url, true, (UInt8 *) buffer, szBuffer);
			CFRelease(url);

			gchar * path = g_build_filename(buffer,name,NULL);

			if(access(path,R_OK) == 0) {
				return path;
			}

			g_free(path);

		}

	}

	return NUL;

 }

 gchar * pw3270_build_data_path(const char *name) {

		gchar * path = get_path_from_bundle(name);

		if(path) {
			if(g_file_test(path,G_FILE_TEST_IS_DIR)) {
				return path;
			}
			g_free(path);
		}

		g_message("Cant find path for '%s'",path);
		return NULL;

 }

 gchar * pw3270_build_data_filename(const char *filename) {

		gchar * path = get_path_from_bundle(name);

		if(path) {
			if(g_file_test(path,G_FILE_TEST_IS_REGULAR)) {
				return path;
			}
			g_free(path);
		}

		g_error("Cant find '%s'",filename);
		return NULL;
 }

