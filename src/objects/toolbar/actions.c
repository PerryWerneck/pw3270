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

 GtkWidget * pw3270_toolbar_insert_action(GtkWidget *toolbar, GAction *action, gint pos) {

	debug("toolbar=%p action=%p",toolbar,action);

	g_return_val_if_fail(PW3270_IS_ACTION(action) && PW3270_IS_TOOLBAR(toolbar),NULL);

	const gchar * icon_name = pw3270_action_get_icon_name(action);
	if(!icon_name) {
		g_message("Action doesn't have an icon");
		return NULL;
	}

	debug("%s - %s",icon_name,pw3270_action_get_label(action));

	GtkToolItem * item = gtk_tool_button_new(
								gtk_image_new_from_icon_name(icon_name,GTK_ICON_SIZE_LARGE_TOOLBAR),
								pw3270_action_get_label(action)
							);

	gtk_tool_button_set_use_underline(GTK_TOOL_BUTTON(item),TRUE);

	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, pos);
	gtk_widget_show_all(GTK_WIDGET(item));

	const gchar * tooltip = pw3270_action_get_tooltip(action);
	if(tooltip)
		gtk_widget_set_tooltip_markup(GTK_WIDGET(item),tooltip);

	return GTK_WIDGET(item);
 }
