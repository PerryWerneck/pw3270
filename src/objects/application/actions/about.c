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

	GtkAboutDialog	* dialog = GTK_ABOUT_DIALOG(gtk_about_dialog_new());

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
		g_autofree gchar * version = g_strdup_printf(
											_("Version %s-%s"),
											PACKAGE_VERSION,
#ifdef PACKAGE_RELEASE
											PACKAGE_RELEASE
#else
											G_STRINGIFY(BUILD_DATE)
#endif // PACKAGE_RELEASE
										);

		gtk_about_dialog_set_version(dialog,version);
	}

	// Set comments
	{
		g_autofree gchar * comments =

		g_strdup_printf(
				_( "3270 terminal emulator for %s." ),
#if defined(__MINGW64__)
				_( "64 bits Windows" )
#elif defined(__MINGW32__)
				_( "32 bits Windows" )
#elif defined(linux) && defined(__i386__)
				_( "32 bits Linux" )
#elif defined(linux) && defined(__x86_64__)
				_( "64 bits Linux" )
#else
				"GTK"
#endif
				);
		gtk_about_dialog_set_comments(dialog, comments);
	}

	// Set maintainers
	{
		static const gchar *maintainers[] = {
			"Perry Werneck <perry.werneck@gmail.com>",
			NULL
		};

		static const gchar *apple[] = {
			"Andre Breves <andre.breves@gmail.com>",
			NULL
		};

		static const gchar *references[]	=
		{
			"Paul Mattes <Paul.Mattes@usa.net>",
			"Georgia Tech Research Corporation (GTRC)",
			"and others",
			NULL
		};

		static const gchar *contributors[] = {
			"Erico Mendonça <erico.mendonca@suse.com>",
			NULL
		};

		gtk_about_dialog_add_credit_section(dialog, _("Maintainers"),	maintainers);
		gtk_about_dialog_add_credit_section(dialog, _("Apple version"), apple);
		gtk_about_dialog_add_credit_section (dialog, _("Contributors"), contributors);

		gtk_about_dialog_add_credit_section(dialog, _("Based on X3270 from"), references);

	}

	gtk_about_dialog_set_copyright(dialog, "Copyright © 2008 Banco do Brasil S.A." );

#ifdef _WIN32

	lib3270_autoptr(char) license = lib3270_build_data_filename(_("LICENSE"),NULL);

	if(g_file_test(license, G_FILE_TEST_IS_REGULAR)) {

		g_autofree gchar * text = NULL;

		if(g_file_get_contents(license,&text,NULL,NULL)) {
			gtk_about_dialog_set_license(dialog, text );
			gtk_about_dialog_set_wrap_license(dialog,TRUE);
		}

	} else {
		gtk_about_dialog_set_license_type(dialog,GTK_LICENSE_GPL_3_0);
	}

#else
	gtk_about_dialog_set_license_type(dialog,GTK_LICENSE_GPL_3_0);
#endif // _WIN32

//	gtk_about_dialog_set_website(dialog,NC_("ProjectURL","https://portal.softwarepublico.gov.br/social/pw3270/"));
//	gtk_about_dialog_set_website_label(dialog,NC_("ProjectURLLabel","Brazilian Public Software Portal" ));

	gtk_about_dialog_set_website(dialog,_("https://github.com/PerryWerneck/pw3270"));
	gtk_about_dialog_set_website_label(dialog,_("View this project on github"));

	gtk_about_dialog_set_translator_credits(dialog,_("translator-credits"));

	gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);

	g_signal_connect(dialog,"response",G_CALLBACK(gtk_widget_destroy),NULL);
	gtk_widget_show_all(GTK_WIDGET(dialog));

	// Call plugins
	pw3270_application_plugin_call(
		g_application_get_default(),
		"pw3270_plugin_set_about_dialog",
		dialog
	);

	return GTK_WIDGET(dialog);

 }

 GAction * pw3270_about_action_new() {

 	PW3270Action * action = pw3270_dialog_action_new(factory);

 	action->name = "about";
 	action->label = _("About PW3270");
 	action->icon_name = "help-about";

	return G_ACTION(action);
 }
