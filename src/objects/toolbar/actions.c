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
 #include <pw3270/actions.h>

 static GtkWidget * pw3270_tool_button_new(GAction *action) {

	const gchar * action_name = g_action_get_name(action);
	debug("action=%s enabled=%s type=%s",action_name,g_action_get_enabled(action) ? "Yes" : "No", g_action_get_parameter_type(action));

	if(PW3270_IS_ACTION(action)) {

		// It's a pw3270 action, get attributes from it.

		const gchar * icon_name = pw3270_action_get_icon_name(action);
		if(!icon_name) {
			g_message("Action doesn't have an icon");
			return NULL;
		}

		debug("%s - %s",icon_name,pw3270_action_get_label(action));

		GtkToolItem * item = gtk_tool_button_new(
									NULL,
									pw3270_action_get_label(action)
								);

		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(item),icon_name);

		const gchar * tooltip = pw3270_action_get_tooltip(action);
		if(tooltip)
			gtk_widget_set_tooltip_markup(GTK_WIDGET(item),tooltip);

		return GTK_WIDGET(item);

	}

	//
	// Check for my Actions.
	//

	static const struct _actions {
		const gchar * name;
		const gchar * icon;
		const gchar * label;
		const gchar * tooltip;
	} actions[] = {

		{
			.name = "win.connect",
			.icon = "gtk-connect",
			.label = N_("Connect"),
			.tooltip = N_("Connect to host")
		},

		{
			.name = "win.close",
			.icon = "window-close",
			.label = N_("Close"),
			.tooltip = N_("Close window")
		}

	};

    size_t ix;
    for(ix = 0; ix < G_N_ELEMENTS(actions); ix++) {

		if(!g_ascii_strcasecmp(action_name,actions[ix].name)) {

			GtkToolItem * item = gtk_tool_button_new(
										gtk_image_new_from_icon_name(actions[ix].icon,GTK_ICON_SIZE_LARGE_TOOLBAR),
										gettext(actions[ix].label)
									);

			if(actions[ix].tooltip)
				gtk_widget_set_tooltip_markup(GTK_WIDGET(item),gettext(actions[ix].tooltip));

			return GTK_WIDGET(item);
		}

    }

	g_warning("Action \"%s\" can't be inserted on toolbar",action_name);

	return NULL;

 }

 static void clicked(GtkToolButton G_GNUC_UNUSED(*toolbutton), GAction *action) {

 	if(g_action_get_enabled(action)) {
		g_action_activate(action,NULL);
 	}
#ifdef DEBUG
	else {
		debug("Action \"%s\" is disabled",g_action_get_name(action));
	}
#endif // DEBUG

 }

 static void notify(GAction *action, GParamSpec *pspec, GtkWidget *item) {
 	if(!strcmp(pspec->name,"enabled"))
		gtk_widget_set_sensitive(item,g_action_get_enabled(action));
 }

 GtkWidget * pw3270_toolbar_insert_action(GtkWidget *toolbar, GAction *action, gint pos) {

	g_return_val_if_fail(PW3270_IS_TOOLBAR(toolbar),NULL);

	GtkWidget * item = pw3270_tool_button_new(action);

	if(!item)
		return NULL;

	gtk_tool_button_set_use_underline(GTK_TOOL_BUTTON(item),TRUE);
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(item), pos);
	gtk_widget_show_all(GTK_WIDGET(item));
	gtk_widget_set_sensitive(GTK_WIDGET(item),g_action_get_enabled(action));

	g_signal_connect(G_OBJECT(item),"clicked",G_CALLBACK(clicked),action);
	g_signal_connect(G_OBJECT(action),"notify",G_CALLBACK(notify),item);

	return item;

 }

 GtkWidget * pw3270_toolbar_insert_action_by_name(GtkWidget *toolbar, const gchar *name, gint pos) {

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

	if(window) {

		GAction *action = g_action_map_lookup_action(G_ACTION_MAP(window), name);

		if(action)
			return pw3270_toolbar_insert_action(toolbar, action, pos);

		g_warning("Can't find action \"%s\"",name);

	}

	return NULL;
 }

