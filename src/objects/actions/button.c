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

 /**
  * @brief Implement PW3270 action button.
  *
  */

 #include "private.h"
 #include <pw3270/actions.h>

 GtkWidget * gtk_button_new_from_action(GAction *action, GtkIconSize icon_size) {

	if(!action)
		return NULL;

	g_autofree gchar * icon_name = g_action_get_icon_name(action);
	if(icon_name)
		return gtk_button_new_from_icon_name(icon_name,icon_size);

	GdkPixbuf * pixbuf = g_action_get_pixbuf(action, GTK_ICON_SIZE_BUTTON, GTK_ICON_LOOKUP_GENERIC_FALLBACK);

	if(pixbuf) {

		GtkWidget * button = gtk_button_new();

		GtkWidget * image = gtk_image_new_from_pixbuf(pixbuf);
		gtk_widget_show_all(image);

		gtk_button_set_image(GTK_BUTTON(button),image);

		return button;
	}


	return NULL;
 }

 GtkToolItem * gtk_tool_button_new_from_action(GAction *action, GtkIconSize icon_size) {

	if(!action)
		return NULL;

	g_autofree gchar * tooltip = g_action_get_tooltip(action);
	g_autofree gchar * label = g_action_get_label(action);
	debug("%s(%s).label=%s",__FUNCTION__,g_action_get_name(action),label);
	if(!label)
		return NULL;

	g_autofree gchar * icon_name = g_action_get_icon_name(action);
	debug("%s(%s).icon_name=%s",__FUNCTION__,g_action_get_name(action),icon_name);
	if(icon_name) {

		// Has icon name
		GtkToolItem * item = gtk_tool_button_new(NULL,label);

		gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(item),icon_name);

		if(tooltip)
			gtk_widget_set_tooltip_markup(GTK_WIDGET(item),tooltip);

		return item;
	}

	GdkPixbuf * pixbuf = g_action_get_pixbuf(action, icon_size, GTK_ICON_LOOKUP_GENERIC_FALLBACK);

	if(pixbuf) {

		GtkToolItem * item = gtk_tool_button_new(NULL,label);

		GtkWidget * image = gtk_image_new_from_pixbuf(pixbuf);
		gtk_widget_show_all(image);

		gtk_tool_button_set_icon_widget(GTK_TOOL_BUTTON(item),image);

		if(tooltip)
			gtk_widget_set_tooltip_markup(GTK_WIDGET(item),tooltip);

		return item;
	}

	return NULL;

 }

