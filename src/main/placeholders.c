/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob
 * o nome G3270.
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
 * Este programa está nomeado como main.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

/**
 * @brief PW3270 Placeholders management.
 *
 */

#include "private.h"
#include <pw3270/application.h>
#include <pw3270/keypad.h>
#include <lib3270.h>
#include <lib3270/log.h>

/*---[ Implement ]----------------------------------------------------------------------------------*/

static GMenu * get_keypad_menu(GApplication *application) {

	GList * keypads = pw3270_application_get_keypad_models(application);

	if(!keypads)
		return NULL;

	GMenu * menu = g_menu_new();

	// Create keypad items.
	GList *item;
	for(item = keypads; item; item = g_list_next(item)) {
		GObject * model = G_OBJECT(item->data);
		g_autofree gchar * action_name = g_strconcat("win.keypad.",pw3270_keypad_model_get_name(model),NULL);
		g_menu_append(menu,pw3270_keypad_model_get_label(model),action_name);
	}

	return menu;

}

void pw3270_load_placeholders(GApplication *application, GtkBuilder * builder) {

	GObject * placeholder;
	size_t ix;

	//
	// Load fonts
	//
	placeholder = gtk_builder_get_object(builder, "font-select-placeholder");

	if(placeholder && G_IS_MENU(placeholder)) {

		GMenu * font_menu = G_MENU(placeholder);

		gint n_families;
		PangoFontFamily **families;
		pango_context_list_families(gdk_pango_context_get_for_screen(gdk_screen_get_default()),&families, &n_families);

		for(ix=0; ix < (size_t) n_families; ix++) {
			if(!pango_font_family_is_monospace(families[ix]))
				continue;

			const gchar * family = pango_font_family_get_name(families[ix]);
			g_autofree gchar * detailed_action = g_strconcat("win.font-family::",family,NULL);
			g_menu_append(font_menu,family,detailed_action);

		}

	}

	//
	// View options
	//
	GMenu * keypad_menu = get_keypad_menu(application);

	if(keypad_menu) {

		static const gchar * placeholders[] = {
			"view-menu-placeholder",
			"view-when-offline-placeholder",
			"view-when-online-placeholder"
		};

		for(ix = 0; ix < G_N_ELEMENTS(placeholders); ix++) {

			placeholder = gtk_builder_get_object(builder, placeholders[ix]);

			if(placeholder && G_IS_MENU(placeholder)) {
				g_menu_append_item(G_MENU(placeholder), g_menu_item_new_submenu(_("Keypads"),G_MENU_MODEL(keypad_menu)));
			}

		}

		g_object_unref(keypad_menu);
	}

}
