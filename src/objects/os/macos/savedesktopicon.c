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

/**
 * @brief Implement MacOS version of the save desktop icon action.
 *
 */

#include <v3270.h>
#include <pw3270.h>
#include <pw3270/application.h>
#include <v3270/actions.h>
#include <v3270/keyfile.h>
#include <v3270/settings.h>
#include <lib3270.h>
#include <lib3270/log.h>
#include <lib3270/properties.h>
#include <v3270/tools.h>

static GtkWidget * factory(V3270SimpleAction *action, GtkWidget *terminal);

GAction * pw3270_action_save_session_shortcut_new(void) {

	V3270SimpleAction * action = v3270_dialog_action_new(factory);

	action->name = "save.launcher";
	action->label = _("Save session shortcut");
	action->tooltip = _("Create shortcut for the current session");

	return G_ACTION(action);

}

GtkWidget * factory(V3270SimpleAction *action, GtkWidget *terminal) {

	GtkWidget * dialog =
		gtk_message_dialog_new(
			GTK_WINDOW(gtk_widget_get_toplevel(terminal)),
			GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,
			GTK_BUTTONS_OK,
			_("This action is not available in this platform")
		);

	g_signal_connect(dialog,"close",G_CALLBACK(gtk_widget_destroy),NULL);
	g_signal_connect(dialog,"response",G_CALLBACK(gtk_widget_destroy),NULL);

	return dialog;
}

static void apply(GtkWidget *dialog, GtkWidget *terminal) {

}

