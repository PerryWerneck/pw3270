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
#include <pw3270/actions.h>

GtkWidget * pw3270_toolbar_insert_action(GtkWidget *toolbar, const gchar *name, gint pos) {

	g_return_val_if_fail(PW3270_IS_TOOLBAR(toolbar),NULL);

	if(!g_ascii_strcasecmp(name,"")) {

		GtkToolItem * item = gtk_separator_tool_item_new();

		gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(item),FALSE);
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, pos);
		gtk_widget_show_all(GTK_WIDGET(item));

		return GTK_WIDGET(item);

	} else if(!g_ascii_strcasecmp(name,"separator")) {

		GtkToolItem * item = gtk_separator_tool_item_new();

		gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(item),TRUE);
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, pos);
		gtk_widget_show_all(GTK_WIDGET(item));

		return GTK_WIDGET(item);

	}

	GtkWidget * window = gtk_widget_get_toplevel(toolbar);

	if(window && G_IS_ACTION_MAP(window)) {

		GtkToolItem * item = NULL;
		GAction *action = g_action_map_lookup_action(G_ACTION_MAP(window), name);

		if(!action) {
			const gchar *ptr = strchr(name,'.');
			if(ptr) {
				action = g_action_map_lookup_action(G_ACTION_MAP(window), ptr+1);
			}
		}

		if(!action) {
			action = g_action_map_lookup_action(G_ACTION_MAP(g_application_get_default()),name);
		}

		if(!action) {
			const gchar *ptr = strchr(name,'.');
			if(ptr) {
				action = g_action_map_lookup_action(G_ACTION_MAP(g_application_get_default()), ptr+1);
			}
		}

//		debug("%s(%s)=%p",__FUNCTION__,name,action);

		if(!action) {
			g_warning("Can't find action \"%s\"",name);
			return NULL;
		}

		item = gtk_tool_button_new_from_action(
		           action,
		           GTK_ICON_SIZE_LARGE_TOOLBAR,
		           pw3270_toolbar_get_icon_type(GTK_TOOLBAR(toolbar)) == 1
		       );

		if(item) {

			gtk_widget_show_all(GTK_WIDGET(item));

			if(action && g_action_get_parameter_type(action) == G_VARIANT_TYPE_STRING) {
				g_autofree gchar * detailed = g_strconcat(name,"::",NULL);
				gtk_actionable_set_detailed_action_name(GTK_ACTIONABLE(item),detailed);
			} else {
				gtk_actionable_set_action_name(GTK_ACTIONABLE(item),name);
			}

			gtk_toolbar_insert(GTK_TOOLBAR(toolbar),item,pos);
			return GTK_WIDGET(item);

		} else {

			g_warning("Can't create toolbar item for action \"%s\"",name);

		}

	}

	return NULL;
}

