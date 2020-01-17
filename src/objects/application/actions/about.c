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
 #include <pw3270/actions.h>
 #include <pw3270/application.h>

 static GtkWidget * factory(PW3270Action G_GNUC_UNUSED(*action), GtkApplication G_GNUC_UNUSED(*application)) {

 	static const gchar *authors[]	=
 	{
 		"Perry Werneck <perry.werneck@gmail.com>",
		"Paul Mattes <Paul.Mattes@usa.net>",
		"Georgia Tech Research Corporation (GTRC)",
		"and others",
		NULL
	};

	static const gchar *license		=
	N_( "This program is free software; you can redistribute it and/or "
		"modify it under the terms of the GNU General Public License as "
 		"published by the Free Software Foundation; either version 2 of the "
		"License, or (at your option) any later version.\n\n"
		"This program is distributed in the hope that it will be useful, "
		"but WITHOUT ANY WARRANTY; without even the implied warranty of "
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
		"GNU General Public License for more details.\n\n"
		"You should have received a copy of the GNU General Public License "
		"along with this program; if not, write to the Free Software "
		"Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02111-1307 "
		"USA" );

	GtkAboutDialog	* dialog = GTK_ABOUT_DIALOG(gtk_about_dialog_new());

	// Associate with the window
	{
		GtkWindow * window = gtk_application_get_active_window(GTK_APPLICATION(application));
		if(window) {
			gtk_window_set_transient_for(GTK_WINDOW(dialog), window);
		}
	}

	// Get application logo
	{
		lib3270_autoptr(char) logo = lib3270_build_data_filename(G_STRINGIFY(PRODUCT_NAME) "-logo.png",NULL);

		if(g_file_test(logo,G_FILE_TEST_EXISTS)) {
			GError 		* error	= NULL;
			GdkPixbuf	* pix	= gdk_pixbuf_new_from_file(logo,&error);

			gtk_about_dialog_set_logo(dialog,pix);

			if(pix) {

				g_object_unref(pix);

			} else {

				g_warning("Can't load %s: %s",logo,error->message);
				g_error_free(error);

			}

		} else {

			g_message("%s: %s",logo,strerror(ENOENT));

		}

	}

	// Set version
	{
#ifdef PACKAGE_RELEASE
		g_autofree gchar * version = g_strdup_printf(_("Version %s-%s"),PACKAGE_VERSION,G_STRINGIFY(PACKAGE_RELEASE));
#else
		g_autofree gchar * version = g_strdup_printf(_("Version %s-%s"),PACKAGE_VERSION,G_STRINGIFY(BUILD_DATE));
#endif // PACKAGE_REVISION

		gtk_about_dialog_set_version(dialog,version);
	}

	// Set comments
	{
		g_autofree gchar * comments = g_strdup_printf(_( "3270 terminal emulator for GTK %d.%d" ),GTK_MAJOR_VERSION,GTK_MINOR_VERSION);
		gtk_about_dialog_set_comments(dialog, comments);
	}

	gtk_about_dialog_set_copyright(dialog, "Copyright © 2008 Banco do Brasil S.A." );
	gtk_about_dialog_set_license(dialog, gettext( license ) );
	gtk_about_dialog_set_wrap_license(dialog,TRUE);
	gtk_about_dialog_set_website(dialog,"https://portal.softwarepublico.gov.br/social/pw3270/");
	gtk_about_dialog_set_website_label(dialog,_( "Brazilian Public Software Portal" ));
	gtk_about_dialog_set_authors(dialog,authors);
	gtk_about_dialog_set_translator_credits(dialog,_("translator-credits"));

	gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);

	g_signal_connect(dialog,"response",G_CALLBACK(gtk_widget_destroy),NULL);
	gtk_widget_show_all(GTK_WIDGET(dialog));

	return GTK_WIDGET(dialog);

 }

 GAction * pw3270_about_action_new() {

 	PW3270Action * action = pw3270_dialog_action_new(factory);

 	action->name = "about";
 	action->label = _("About PW3270");
 	action->icon_name = "help-about";

	return G_ACTION(action);
 }
