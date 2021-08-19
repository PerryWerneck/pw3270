/* SPDX-License-Identifier: LGPL-3.0-or-later */

/*
 * Copyright (C) 2021 Perry Werneck <perry.werneck@gmail.com>
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

 #ifndef GETTEXT_PACKAGE
	#define GETTEXT_PACKAGE PACKAGE_NAME
 #endif

 #include <libintl.h>
 #include <glib/gi18n.h>
 #include <gtk/gtk.h>

 #include <pw3270/application.h>
 #include <pw3270/window.h>

 static GSettings * settings_new(const gchar *schema_id) {

 	GSettings *settings = NULL;

#if defined(DEBUG)
	{
		GError * error = NULL;
		GSettingsSchemaSource * source =
			g_settings_schema_source_new_from_directory(
				".",
				NULL,
				TRUE,
				&error
			);

		if(error) {
			g_warning("Error loading '%s': %s","gschemas.compiled",error->message);
			g_error_free(error);
			return NULL;
		}

		GSettingsSchema * schema =
			g_settings_schema_source_lookup(
				source,
				schema_id,
				TRUE
			);

		settings = g_settings_new_full(schema, NULL, NULL);

		g_settings_schema_source_unref(source);
	}
#elif defined(_WIN32)
	{
		g_autofree gchar * appdir = g_win32_get_package_installation_directory_of_module(NULL);
		g_autofree gchar * filename = g_build_filename(appdir,"gschemas.compiled",NULL);

		if(g_file_test(filename,G_FILE_TEST_IS_REGULAR)) {

			GError * error = NULL;
			g_autofree gchar *dirname = g_path_get_dirname(filename);

			GSettingsSchemaSource * source =
				g_settings_schema_source_new_from_directory(
					dirname,
					NULL,
					TRUE,
					&error
				);

			if(error) {
				g_warning("Error loading '%s': %s",filename,error->message);
				g_error_free(error);
				return NULL;
			}

			GSettingsSchema * schema =
				g_settings_schema_source_lookup(
					source,
					schema_id,
					TRUE);

			debug("schema %s=%p","br.com.bb." PACKAGE_NAME,schema);

			g_message("Loading '%s'",filename);
			settings = g_settings_new_full(schema, NULL, NULL);

			g_settings_schema_source_unref(source);

		} else {

			g_message("Loading '%s' from default path","gschemas.compiled");
			settings = g_settings_new(schema_id);

		}
	}
#else

	g_message("Loading '%s' from default path","gschemas.compiled");
	settings = g_settings_new(schema_id);

#endif // DEBUG

	if(!settings) {
		g_warning("Error creating settings");
	}

	return settings;
 }

 GSettings * pw3270_application_settings_new() {
	return settings_new("br.com.bb." G_STRINGIFY(PRODUCT_NAME));
 }

 GSettings * pw3270_application_window_settings_new() {
	return settings_new("br.com.bb." G_STRINGIFY(PRODUCT_NAME) ".window");
 }
