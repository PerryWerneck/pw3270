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

 /// @brief Create a button associated with the action.
 GtkWidget * pw3270_action_button_new(GAction *action, const gchar *action_name) {

	g_return_val_if_fail(PW3270_IS_ACTION(action),NULL);

	const gchar * icon_name = pw3270_action_get_icon_name(action);

	GtkWidget *image;
	if(g_str_has_prefix(icon_name,"gtk-")) {
		image = gtk_image_new_from_icon_name(icon_name,GTK_ICON_SIZE_BUTTON);
	} else {
		g_autofree gchar * symbolic_name = g_strconcat(icon_name,"-symbolic",NULL);
		image = gtk_image_new_from_icon_name(symbolic_name,GTK_ICON_SIZE_BUTTON);
	}


	// If fails, use the regular icon.
	if(!image) {
		debug("***************** %s",icon_name);
	}

	if(!image) {
		g_warning("Can't create button for icon \"%s\"",icon_name);
		return NULL;
	}

	GtkWidget * button = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(button), image);

	gtk_actionable_set_action_name(GTK_ACTIONABLE(button),action_name ? action_name : g_action_get_name(action));
	gtk_widget_set_visible(button,g_action_get_enabled(action));

	gtk_widget_set_can_focus(button,FALSE);
	gtk_widget_set_can_default(button,FALSE);
	gtk_widget_set_focus_on_click(button,FALSE);

	const gchar * tooltip = pw3270_action_get_tooltip(action);
	if(tooltip)
		gtk_widget_set_tooltip_markup(button,tooltip);

	return button;

 }

