/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270.
 *
 * Copyright (C) <2008> <Banco do Brasil S.A.>
 *
 * Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
 * os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
 * Free Software Foundation.
 *
 * Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
 * GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
 * A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
 * obter mais detalhes.
 *
 * Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como - e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#include "private.h"
#include <pw3270.h>
#include <pw3270/toolbar.h>
#include <pw3270/application.h>
#include <pw3270/actions.h>

void pw3270_window_set_header_action_names(GtkWidget *window, const gchar *action_names) {

	GtkWidget * header = gtk_window_get_titlebar(GTK_WINDOW(window));

	if(!(header && GTK_IS_HEADER_BAR(header)))
		return;

	gtk_container_remove_all(GTK_CONTAINER(header));

	if(action_names && *action_names) {

		size_t ix;
		gchar ** header_blocks = g_strsplit(action_names,":",-1);

		g_autoptr(GtkBuilder) builder = pw3270_application_builder_new(g_application_get_default());

		if(!gtk_application_prefers_app_menu(GTK_APPLICATION(g_application_get_default()))) {

			// No application menu, add view and help sections to open menu.
			g_menu_append_section(
			    G_MENU(gtk_builder_get_object(builder,"open-menu")),
			    NULL,
			    G_MENU_MODEL(gtk_builder_get_object(builder,"help-menu-placeholder"))
			);

			g_menu_append_submenu(
			    G_MENU(gtk_builder_get_object(builder,"preferences-menu")),
			    _("View"),
			    G_MENU_MODEL(gtk_builder_get_object(builder,"view-menu-placeholder"))
			);

		}

		if(g_strv_length(header_blocks) >= 2) {

			gchar ** elements;
			GtkWidget * button;

			// First the left side actions.
			elements = g_strsplit(header_blocks[0],",",-1);
			for(ix=0; elements[ix]; ix++) {
				button = pw3270_header_button_new_from_builder(GTK_WIDGET(window),builder,elements[ix]);
				if(button) {
					g_object_set_data(G_OBJECT(button),"header-position-id",GINT_TO_POINTER(0));
					gtk_header_bar_pack_start(GTK_HEADER_BAR(header), button);
				}
			}
			g_strfreev(elements);

			// And then, the right side actions;
			elements = g_strsplit(header_blocks[1],",",-1);
			for(ix=0; elements[ix]; ix++) {
				button = pw3270_header_button_new_from_builder(GTK_WIDGET(window),builder,elements[ix]);
				if(button) {
					g_object_set_data(G_OBJECT(button),"header-position-id",GINT_TO_POINTER(1));
					gtk_header_bar_pack_end(GTK_HEADER_BAR(header), button);
				}
			}
			g_strfreev(elements);

		}

		g_strfreev(header_blocks);

	}

}

static void on_sensitive(GtkWidget G_GNUC_UNUSED(*button), GParamSpec G_GNUC_UNUSED(*spec), GtkWidget G_GNUC_UNUSED(*widget)) {

	gboolean sensitive;
	g_object_get(button, "sensitive", &sensitive, NULL);
	gtk_widget_set_visible(button,sensitive);

}

static GAction * get_action_from_name(GtkWidget *widget, const gchar *action_name) {

	if(g_str_has_prefix(action_name,"app.")) {
		return g_action_map_lookup_action(G_ACTION_MAP(g_application_get_default()),action_name+4);
	}

	return g_action_map_lookup_action(G_ACTION_MAP(widget),action_name+4);
}

GtkWidget * pw3270_header_button_new_from_builder(GtkWidget *widget, GtkBuilder * builder, const gchar *action_name) {

	GtkWidget * button = NULL;
	g_autoptr(GSettings) settings = pw3270_application_window_settings_new();
	gboolean symbolic = g_settings_get_int(settings,"header-icon-type") == 1;

	if(g_str_has_prefix(action_name,"menu.")) {

		// It's a menu
		g_autofree gchar * icon_name = g_strconcat(action_name+5,"-symbolic",NULL);

		button = gtk_menu_button_new();
		gtk_button_set_image(GTK_BUTTON(button),gtk_image_new_from_icon_name(icon_name,GTK_ICON_SIZE_MENU));

		gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(button), G_MENU_MODEL(gtk_builder_get_object(builder, action_name+5)));
		gtk_widget_set_visible(button,TRUE);


	} else {

		GAction * action = get_action_from_name(widget,action_name);

		if(!action) {
			g_warning("Can't find action %s",action_name);
			return NULL;
		}

		button = gtk_button_new_from_action(action,GTK_ICON_SIZE_BUTTON,symbolic);

		gtk_actionable_set_action_name(GTK_ACTIONABLE(button),action_name);
		gtk_widget_set_visible(button,g_action_get_enabled(action));

		g_autofree gchar * tooltip = g_action_get_tooltip(action);
		if(tooltip)
			gtk_widget_set_tooltip_markup(GTK_WIDGET(button),tooltip);

	}

	g_signal_connect(button, "notify::sensitive", G_CALLBACK(on_sensitive), widget);
	gtk_widget_set_focus_on_click(button,FALSE);
	gtk_widget_set_can_focus(button,FALSE);
	gtk_widget_set_can_default(button,FALSE);
	gtk_widget_set_name(button,action_name);

	return button;
}

gchar * pw3270_window_get_action_names(GtkWidget *window) {

	GtkWidget * header = gtk_window_get_titlebar(GTK_WINDOW(window));

	if(!(header && GTK_IS_HEADER_BAR(header)))
		return g_strdup("win.disconnect,win.reconnect,win.file.transfer,win.print:menu.open-menu");

	GString * str = g_string_new("");

	GList * children = gtk_container_get_children(GTK_CONTAINER(header));
	GList * item;

	int id;
	for(id = 0; id < 2; id++) {

		gboolean sep = FALSE;

		for(item = children; item; item = g_list_next(item)) {

			if(GPOINTER_TO_INT(g_object_get_data(G_OBJECT(item->data),"header-position-id")) != id)
				continue;

			if(sep)
				g_string_append(str,",");

			if(GTK_IS_MENU_BUTTON(item->data)) {
				g_string_append(str,gtk_widget_get_name(GTK_WIDGET(item->data)));
			} else if(GTK_IS_ACTIONABLE(item->data)) {
				g_string_append(str,gtk_actionable_get_action_name(GTK_ACTIONABLE(item->data)));
			}

			sep = TRUE;
		}

		if(!id)
			g_string_append(str,":");

	}

	g_list_free(children);

	return g_string_free(str,FALSE);
}
