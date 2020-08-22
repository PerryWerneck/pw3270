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
 * References:
 *
 * https://fossies.org/linux/gtk+/examples/plugman.c
 *
 */

 #include "../private.h"
 #include <pw3270/application.h>
 #include <pw3270/actions.h>

 static void quit_activated(GAction G_GNUC_UNUSED(*action), GVariant G_GNUC_UNUSED(*parameter), GtkApplication *application) {

	g_print("Exiting application\n");

	/*
	GList *list = gtk_application_get_windows(GTK_APPLICATION(application));

	while(list) {

		GtkWidget * window = GTK_WIDGET(list->data);
		list = list->next;

		gtk_widget_destroy(window);

	}
	*/

	g_application_quit(G_APPLICATION(application));

 }

 GAction * pw3270_quit_action_new() {

	PW3270Action * action = pw3270_action_new();

	action->name = "quit";
	action->label = _( "Quit" );
	action->tooltip = _( "Close all windows and exit application" );
	action->icon_name = "gtk-quit";
	action->activate = quit_activated;

 	return G_ACTION(action);
 }

 static void new_tab_activated(GAction G_GNUC_UNUSED(*action), GVariant G_GNUC_UNUSED(*parameter), GtkApplication *application) {

 	debug("%s",__FUNCTION__);
 	pw3270_application_window_new_tab(GTK_WIDGET(gtk_application_get_active_window(GTK_APPLICATION(application))), NULL);

 }

 GAction * pw3270_new_tab_action_new() {

	PW3270Action * action = pw3270_action_new();

	action->name = "new.tab";
	action->label = _( "New tab" );
	action->tooltip = _( "New tab with default session" );
	action->icon_name = "tab-new";
	action->activate = new_tab_activated;

 	return G_ACTION(action);
 }

 static void new_window_activated(GAction G_GNUC_UNUSED(*action), GVariant G_GNUC_UNUSED(*parameter), GtkApplication *application) {

 	debug("%s",__FUNCTION__);
 	g_application_activate(G_APPLICATION(application));

 }

 GAction * pw3270_new_window_action_new() {

	PW3270Action * action = pw3270_action_new();

	action->name = "new.window";
	action->label = _( "New window" );
	action->tooltip = _( "New window with default session" );
	action->icon_name = "window-new";
	action->activate = new_window_activated;

 	return G_ACTION(action);
 }

