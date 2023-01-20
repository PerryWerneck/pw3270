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

#include "../private.h"
#include <pw3270/window.h>
#include <pw3270/actions.h>
#include <v3270/settings.h>
#include <v3270/dialogs.h>
#include <v3270/colorscheme.h>
#include <pw3270/application.h>

static GtkWidget * factory(V3270SimpleAction *action, GtkWidget *terminal);

GAction * pw3270_action_session_properties_new(void) {

	V3270SimpleAction * action = v3270_dialog_action_new(factory);

	action->name = "session.properties";
	action->icon_name = "preferences-other";
	action->label = _("Session preferences");
	action->tooltip = _("Change the preferences for the active session");

	return G_ACTION(action);
}

GtkWidget * factory(V3270SimpleAction *action, GtkWidget *terminal) {

	size_t ix;

	GApplication *application = g_application_get_default();

	GtkWidget * dialog = v3270_settings_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dialog), action->label);

	// Host settings is conditional.
	if(pw3270_application_get_boolean(application,"allow-host-settings",TRUE)) {
		gtk_container_add(GTK_CONTAINER(dialog), v3270_host_settings_new());
	}

	// Add commom settings.
	{

		GtkWidget * elements[] = {
			v3270_color_settings_new(),
			v3270_font_settings_new(),
			v3270_accelerator_settings_new(),
			v3270_clipboard_settings_new()
		};

		for(ix = 0; ix < G_N_ELEMENTS(elements); ix++) {
			gtk_container_add(GTK_CONTAINER(dialog), elements[ix]);
		}

	}

	pw3270_application_plugin_call(
	    application,
	    "pw3270_plugin_set_session_properties",
	    dialog
	);

	// Setup dialog box
	gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(gtk_widget_get_toplevel(terminal)));
	gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);

	v3270_settings_dialog_set_terminal_widget(dialog, terminal);

	g_signal_connect(dialog,"close",G_CALLBACK(gtk_widget_destroy),NULL);
	g_signal_connect(dialog,"response",G_CALLBACK(v3270_setttings_dialog_response),terminal);

	gtk_widget_show_all(dialog);
	return dialog;

}
