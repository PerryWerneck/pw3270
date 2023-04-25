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
 * @brief Misc tools for pw3270 application.
 *
 */

#include "private.h"
#include <pw3270.h>
#include <pw3270/application.h>
#include <pw3270/settings.h>
#include <pw3270/window.h>

/*---[ Implement ]----------------------------------------------------------------------------------*/

GtkBuilder * pw3270_application_get_builder(const gchar *name) {

#ifdef DEBUG
	g_autofree gchar * filename = g_build_filename("ui",name,NULL);
#else
	lib3270_autoptr(char) filename = lib3270_build_data_filename("ui",name,NULL);
#endif // DEBUG

	return gtk_builder_new_from_file(filename);
}

void gtk_container_remove_all(GtkContainer *container) {

	GList * children = gtk_container_get_children(container);
	GList * item;

	for(item = children; item; item = g_list_next(item)) {
		gtk_container_remove(container,GTK_WIDGET(item->data));
	}

	g_list_free(children);

}

void gtk_file_chooser_set_pw3270_filters(GtkFileChooser *chooser) {

	static const struct Filter {
		const gchar * name;
		const gchar * pattern;
	} filters[] = {
		{
			.name = N_("TN3270 Session Files"),
			.pattern = "*.3270"
		},
		{
			.name = N_("All files"),
			.pattern = "*.*"
		}
	};

	size_t ix;

	for(ix = 0; ix < G_N_ELEMENTS(filters); ix++) {
		GtkFileFilter *filter = gtk_file_filter_new();
		gtk_file_filter_add_pattern (filter, filters[ix].pattern);
		gtk_file_filter_set_name(filter, filters[ix].name);
		gtk_file_chooser_add_filter(chooser,filter);
	}

}

GtkWidget * pw3270_get_active_terminal() {

	GApplication *app = g_application_get_default();
	g_return_val_if_fail(GTK_IS_APPLICATION(app),NULL);

	GtkWindow * window = gtk_application_get_active_window(GTK_APPLICATION(app));

	return pw3270_application_window_get_active_terminal(GTK_WIDGET(window));

}

H3270 * pw3270_get_active_session() {

	GApplication *app = g_application_get_default();

	g_return_val_if_fail(GTK_IS_APPLICATION(app),NULL);

	GtkWindow * window = gtk_application_get_active_window(GTK_APPLICATION(app));

	return pw3270_window_get_session_handle(GTK_WIDGET(window));

}
