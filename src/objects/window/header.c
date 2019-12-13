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

 static void on_sensitive(GtkWidget *button, GParamSpec *spec, GtkWidget *widget) {

	gboolean sensitive;
	g_object_get(button, "sensitive", &sensitive, NULL);
	gtk_widget_set_visible(button,sensitive);

 }

 GtkWidget * pw3270_header_button_new_from_builder(GtkWidget *widget, GtkBuilder * builder, const gchar *action_name) {

	GtkWidget * button = NULL;

	if(g_str_has_prefix(action_name,"menu.")) {

		// It's a menu
		g_autofree gchar * icon_name = g_strconcat(action_name+5,"-symbolic",NULL);
		button = pw3270_setup_image_button(gtk_menu_button_new(),icon_name);
		gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(button), G_MENU_MODEL(gtk_builder_get_object(builder, action_name+5)));
		gtk_widget_set_visible(button,TRUE);

	} else if(g_str_has_prefix(action_name,"app.")) {

		// It's an application action

	} else if(g_str_has_prefix(action_name,"win.")) {

		// It's a window action.
		GAction * action = g_action_map_lookup_action(G_ACTION_MAP(widget),action_name+4);
		const gchar * icon_name = pw3270_action_get_icon_name(action);
        if(action && icon_name) {
			button = pw3270_setup_image_button(gtk_menu_button_new(),icon_name);
			gtk_actionable_set_action_name(GTK_ACTIONABLE(button),action_name);
			gtk_widget_set_visible(button,g_action_get_enabled(action));


        }

	}

	if(button) {

		g_signal_connect(button, "notify::sensitive", G_CALLBACK(on_sensitive), widget);
		gtk_widget_set_focus_on_click(button,FALSE);
		gtk_widget_set_can_focus(button,FALSE);

	}

	return button;
 }
