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
 #include <pw3270.h>
 #include <pw3270/application.h>
 #include <pw3270/actions.h>
 #include <pw3270/settings.h>
 #include <pw3270/toolbar.h>


//	gtk_window_set_title(GTK_WINDOW(dialog),action->label);

 static GtkWidget * factory(PW3270Action * action, GtkApplication *application) {

	size_t ix;
	GtkWindow * window = gtk_application_get_active_window(application);
	GtkWidget * dialog = pw3270_settings_dialog_new(G_ACTION(action));

	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_attached_to(GTK_WINDOW(dialog), GTK_WIDGET(window));
	gtk_window_set_transient_for(GTK_WINDOW(dialog),window);

	GtkWidget * pages[] = {
		pw3270_toolbar_settings_new()
	};

	for(ix = 0; ix < G_N_ELEMENTS(pages); ix++) {
		gtk_container_add(GTK_CONTAINER(dialog),pages[ix]);
	}

	gtk_widget_show_all(dialog);

	return dialog;

 }

 GAction * pw3270_preferences_action_new() {

 	PW3270Action * action = pw3270_dialog_action_new(factory);

 	action->name = "preferences";
 	action->label = _("Application preferences");
 	action->icon_name = "preferences-system";

	return G_ACTION(action);
 }

