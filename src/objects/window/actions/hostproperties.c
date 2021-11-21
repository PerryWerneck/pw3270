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

#include "../private.h"
#include <pw3270/window.h>
#include <pw3270/actions.h>
#include <v3270/settings.h>
#include <v3270/dialogs.h>

/*
static GtkWidget * factory(V3270SimpleAction *action, GtkWidget *terminal);

GAction * pw3270_action_host_properties_new(void) {

	V3270SimpleAction * action = v3270_dialog_action_new(factory);

	action->name = "host.properties";
	action->group.id = LIB3270_ACTION_GROUP_OFFLINE;
	action->icon_name = "network-server";
	action->label = N_("Host properties");

	return G_ACTION(action);
}

GtkWidget * factory(V3270SimpleAction G_GNUC_UNUSED(*action), GtkWidget *terminal) {

	GtkWidget 		* dialog = v3270_settings_dialog_new();
	V3270Settings	* settings = GTK_V3270_SETTINGS(v3270_host_settings_new());

	if(settings->title)
		gtk_window_set_title(GTK_WINDOW(dialog), settings->title);

	gtk_container_add(GTK_CONTAINER(dialog), GTK_WIDGET(settings));

	gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(gtk_widget_get_toplevel(terminal)));
	gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);

	v3270_settings_dialog_set_terminal_widget(dialog, terminal);

	g_signal_connect(dialog,"response",G_CALLBACK(v3270_setttings_dialog_response),settings);

	gtk_widget_show_all(dialog);
	return dialog;

}
*/

