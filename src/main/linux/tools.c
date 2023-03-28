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

 gchar * pw3270_build_data_filename(const char *filename) {

		gchar * path = g_build_filename(G_STRINGIFY(DATADIR),filename,NULL);

		printf("\n\n\n%s\n\n\n",path);

		if(g_file_test(path,G_FILE_TEST_IS_REGULAR)) {
			return path;
		}
		g_free(path);

		g_error("Cant find '%s'",filename);
		return NULL;
 }

