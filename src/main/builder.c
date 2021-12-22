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

 #include "private.h"

 static GMenu * get_keypad_menu(GApplication *application) {

	GList * keypads = pw3270_application_get_keypad_models(application);

	if(!keypads)
		return NULL;

	GMenu * menu = g_menu_new();

	// Create keypad items.
	GList *item;
	for(item = keypads; item; item = g_list_next(item)) {
		GObject * model = G_OBJECT(item->data);
		g_autofree gchar * action_name = g_strconcat("win.keypad.",pw3270_keypad_model_get_name(model),NULL);
		g_menu_append(menu,pw3270_keypad_model_get_label(model),action_name);
	}

	return menu;

 }

 GtkBuilder * pw3270_application_builder_new(GApplication *application) {

#if !defined(DEBUG)

	lib3270_autoptr(char) filename = lib3270_build_data_filename(G_STRINGIFY(PRODUCT_NAME) ".ui.xml",NULL);

#elif defined(G_OS_UNIX)

	static const char * filename = "ui/linux.ui.xml";

#else

	#error Cant determine platform based UI definition

#endif // DEBUG

	GtkBuilder * builder = gtk_builder_new_from_file(filename);

	//
	// Load placeholders
	//

	GObject * placeholder;
	size_t ix;

	//
	// Load fonts
	//
	placeholder = gtk_builder_get_object(builder, "font-select-placeholder");

	if(placeholder && G_IS_MENU(placeholder)) {

		GMenu * font_menu = G_MENU(placeholder);

		gint n_families;
		PangoFontFamily **families;
		pango_context_list_families(gdk_pango_context_get_for_screen(gdk_screen_get_default()),&families, &n_families);

		for(ix=0; ix < (size_t) n_families; ix++) {
			if(!pango_font_family_is_monospace(families[ix]))
				continue;

			const gchar * family = pango_font_family_get_name(families[ix]);
			g_autofree gchar * detailed_action = g_strconcat("win.font-family::",family,NULL);
			g_menu_append(font_menu,family,detailed_action);

		}

	}

	//
	// View options
	//
	GMenu * keypad_menu = get_keypad_menu(application);

	if(keypad_menu) {

		static const gchar * placeholders[] = {
			"view-menu-placeholder",
			"view-when-offline-placeholder",
			"view-when-online-placeholder"
		};

		for(ix = 0; ix < G_N_ELEMENTS(placeholders); ix++) {

			placeholder = gtk_builder_get_object(builder, placeholders[ix]);

			if(placeholder && G_IS_MENU(placeholder)) {
				g_menu_append_item(G_MENU(placeholder), g_menu_item_new_submenu(_("Keypads"),G_MENU_MODEL(keypad_menu)));
			}

		}

		g_object_unref(keypad_menu);
	}

	return builder;
 }

