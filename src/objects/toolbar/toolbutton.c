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

 static const struct Button {
	const gchar * name;
	const gchar * icon_name;
	const gchar * label;
	const gchar * tooltip;
 } buttons[] = {
	{
		.name = "connect",
		.icon_name = "gtk-connect",
		.label = N_("Connect"),
		.tooltip = N_("Connect to host")
	},

	{
		.name = "close",
		.icon_name = "window-close",
		.label = N_("Close"),
		.tooltip = N_("Close window")
	},

	{
		.name = "preferences",
		.icon_name = "preferences-other",
		.label = N_("Preferences"),
	}

 };

 static const struct Button * get_button_info(const gchar *name) {

	size_t ix;
	const gchar * ptr = strchr(name,'.');

	if(ptr)
		ptr++;
	else
		ptr = name;

	for(ix = 0; ix < G_N_ELEMENTS(buttons); ix++) {

		if(!g_ascii_strcasecmp(ptr,buttons[ix].name)) {
			return &buttons[ix];
		}

	}

	return NULL;
 }

 static GtkWidget * setup_button(GtkWidget *button, const gchar *action_name) {

	if(button) {
		gtk_tool_button_set_use_underline(GTK_TOOL_BUTTON(button),TRUE);
		gtk_widget_show_all(GTK_WIDGET(button));
	}

	debug("%s(%s)=%p",__FUNCTION__,action_name,button);

	return button;

 }

 GtkWidget * pw3270_tool_button_new_from_action_name(const gchar * action_name) {

	// Do I have button info?
	GtkToolItem * button = NULL;

	const struct Button * info = get_button_info(action_name);

	if(info) {

		button = gtk_tool_button_new(
						NULL,
						info->label
					);

		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button),info->icon_name);

		if(info->tooltip)
			gtk_widget_set_tooltip_markup(GTK_WIDGET(button),info->tooltip);


	} else {

		g_warning("No toolbar info for action \"%s\"",action_name);

	}

	return setup_button(GTK_WIDGET(button),action_name);

 }

 GtkWidget * pw3270_tool_button_new(GAction *action) {

	const gchar * action_name = g_action_get_name(action);
	GtkToolItem	* button = NULL;

	if(PW3270_IS_ACTION(action)) {

		// It's a pw3270 action, get attributes from it.

		const gchar * icon_name = pw3270_action_get_icon_name(action);
		if(!icon_name) {
			g_message("Action doesn't have an icon");
			return NULL;
		}

		button = gtk_tool_button_new(
						NULL,
						pw3270_action_get_label(action)
					);

		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(button),icon_name);

		const gchar * tooltip = pw3270_action_get_tooltip(action);
		if(tooltip)
			gtk_widget_set_tooltip_markup(GTK_WIDGET(button),tooltip);

		return setup_button(GTK_WIDGET(button),action_name);

	}

	return pw3270_tool_button_new_from_action_name(action_name);

 }

