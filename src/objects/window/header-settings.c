/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes paul.mattes@case.edu), de emulação de terminal 3270 para acesso a
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
#include <pw3270/actions.h>
#include <pw3270/settings.h>
#include <pw3270/application.h>
#include <v3270/dialogs.h>

static void load(GtkWidget *widget, GSettings *settings, PW3270SettingsPage *page);
static void apply(GtkWidget *widget, GSettings *settings, PW3270SettingsPage *page);

/*--[ Constants ]------------------------------------------------------------------------------------*/

struct _PW3270SettingsPage {
	GtkWidget * editor;
};

/*--[ Implement ]------------------------------------------------------------------------------------*/

GtkWidget * pw3270_header_settings_new() {

	// Create page widget.
	PW3270Settings 	* settings = pw3270_settings_new();
	settings->label = _("Title bar");
	settings->title = _("Setup title bar");
	settings->apply = apply;
	settings->load = load;

	// Create private data.
	PW3270SettingsPage * page = settings->settings = g_new0(PW3270SettingsPage,1);

	page->editor = pw3270_settings_actions_new();

	gtk_grid_attach(
	    GTK_GRID(settings),
	    v3270_dialog_section_new(_("Title bar actions"), _("Change the position of the title bar icons"), page->editor),
	    0,0,4,3
	);


	gtk_widget_show_all(GTK_WIDGET(settings));
	return GTK_WIDGET(settings);
}

void load(GtkWidget *widget, GSettings *settings, PW3270SettingsPage *page) {

	if(!G_IS_SETTINGS(settings)) {
		g_warning("The settings object is not valid, disabling dialog to avoid segfaults");
		gtk_widget_set_sensitive(widget,FALSE);
		return;
	}

	// Get avaliable actions.
	Pw3270ActionList * action_list = pw3270_action_list_new(GTK_APPLICATION(g_application_get_default()));

	// Add standard menus
	{
		static const struct menu {
			const gchar * action_name;
			const gchar * label;
			const gchar * icon_name;
		} menus[] = {
			{
				.action_name = "menu.open-menu",
				.label = N_("Application menu"),
				.icon_name = "open-menu-symbolic"
			}
		};

		size_t ix;

		for(ix = 0; ix < G_N_ELEMENTS(menus); ix++) {

			GError *error = NULL;

			GdkPixbuf * pixbuf = gtk_icon_theme_load_icon(
			                         gtk_icon_theme_get_default(),
			                         menus[ix].icon_name,
			                         GTK_ICON_SIZE_MENU,
			                         GTK_ICON_LOOKUP_GENERIC_FALLBACK,
			                         &error
			                     );

			if(error) {
				g_warning("%s",error->message);
				g_error_free(error);
				error = NULL;
			}

			action_list = pw3270_action_list_append(
			                  action_list,
			                  gettext(menus[ix].label),
			                  pixbuf,
			                  menus[ix].action_name,
			                  PW3270_ACTION_VIEW_ALLOW_MOVE
			              );
		}

	}

	// Load settings
	g_autofree gchar * action_names = g_settings_get_string(settings,"header-action-names");

	action_list = pw3270_settings_action_set(page->editor, action_list, action_names);

	pw3270_action_list_free(action_list);
}

void apply(GtkWidget G_GNUC_UNUSED(*widget), GSettings *settings, PW3270SettingsPage *page) {

	g_autofree gchar * action_names = pw3270_settings_action_get(page->editor);
	g_settings_set_string(settings,"header-action-names",action_names);

}

