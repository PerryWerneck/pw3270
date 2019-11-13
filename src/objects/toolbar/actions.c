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

	if(window) {

		GtkToolItem * item = NULL;
		GAction *action = g_action_map_lookup_action(G_ACTION_MAP(window), name);

		if(!action) {
			const gchar *ptr = strchr(name,'.');

			if(ptr)
				action = g_action_map_lookup_action(G_ACTION_MAP(window), ptr+1);
		}

		if(action) {
			debug("Creating button \"%s\" from action \"%s\"",name,g_action_get_name(G_ACTION(action)));
			item = GTK_TOOL_ITEM(pw3270_tool_button_new(action));
		} else {
			debug("Creating button \"%s\" from action name",name);
			item = GTK_TOOL_ITEM(pw3270_tool_button_new_from_action_name(name));
		}

		if(item) {

			if(action && g_action_get_parameter_type(action) == G_VARIANT_TYPE_STRING) {
				g_autofree gchar * detailed = g_strconcat(name,"::",NULL);
				gtk_actionable_set_detailed_action_name(GTK_ACTIONABLE(item),detailed);
			} else {
				gtk_actionable_set_action_name(GTK_ACTIONABLE(item),name);
			}

			gtk_toolbar_insert(GTK_TOOLBAR(toolbar),item,pos);
			return GTK_WIDGET(item);
		}

	}

	return NULL;
 }

