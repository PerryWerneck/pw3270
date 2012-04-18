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
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como main.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

#include "globals.h"
#include "v3270/v3270.h"
#include "v3270/accessible.h"
#include <stdlib.h>

/*--[ Statics ]--------------------------------------------------------------------------------------*/

 static GtkWidget *toplevel = NULL;

/*--[ Implement ]------------------------------------------------------------------------------------*/

static int initialize(void)
{
	const gchar * msg = gtk_check_version(GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);

	if(msg)
	{
		// Invalid GTK version, notify user
		int rc;

		GtkWidget *dialog = gtk_message_dialog_new(	NULL,
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_WARNING,
													GTK_BUTTONS_OK_CANCEL,
													_( "%s requires GTK version %d.%d.%d" ),PACKAGE_NAME,GTK_MAJOR_VERSION,GTK_MINOR_VERSION,GTK_MICRO_VERSION );


		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s",msg);
		gtk_window_set_title(GTK_WINDOW(dialog),_( "GTK Version mismatch" ));
		gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);

        rc = gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);

        if(rc != GTK_RESPONSE_OK)
			return EINVAL;
	}

	return 0;
}

static void toplevel_setup(GtkWindow *window)
{
 	gchar * name		= g_strdup_printf("%s.png",g_get_application_name());
	gchar * filename 	= build_data_filename(name,NULL);

	gtk_window_set_type_hint(window,GDK_WINDOW_TYPE_HINT_NORMAL);
	gtk_window_set_position(window,GTK_WIN_POS_CENTER);
	gtk_window_set_role(window,"toplevel");

	// Set default icon
	if(g_file_test(filename,G_FILE_TEST_EXISTS))
	{
		GError * error = NULL;

		trace("Loading default icon from %s",filename);

		if(!gtk_window_set_default_icon_from_file(filename,&error))
		{
			g_warning("Error %s loading default icon from %s",error->message,filename);
			g_error_free(error);
		}
	}

	g_free(filename);
	g_free(name);
}

int main(int argc, char *argv[])
{
	static const gchar	* appname	= PACKAGE_NAME;
	static const gchar	* host		= NULL;
	int 				  rc 		= 0;

	// Process command-line options
	{
		static const GOptionEntry app_options[] =
		{
			{ "appname",	'a', 0, G_OPTION_ARG_STRING,	&appname,	N_( "Application name" ),	PACKAGE_NAME	},
			{ "host",		'h', 0, G_OPTION_ARG_STRING,	&host,		N_( "Host to connect"),		NULL			},
			{ NULL }
		};

		GOptionContext	* options	= g_option_context_new (_("- 3270 Emulator for Gtk"));
		GError			* error		= NULL;

		g_option_context_add_main_entries(options, app_options, NULL);

		gtk_init(&argc, &argv);

		if(!g_option_context_parse( options, &argc, &argv, &error ))
		{
			GtkWidget *dialog = gtk_message_dialog_new(	NULL,
														GTK_DIALOG_DESTROY_WITH_PARENT,
														GTK_MESSAGE_ERROR,
														GTK_BUTTONS_CANCEL,
														"%s", _(  "Option parsing failed." ));

			gtk_window_set_title(GTK_WINDOW(dialog),_( "Parse error" ));
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", error->message);

			gtk_dialog_run(GTK_DIALOG (dialog));
			gtk_widget_destroy(dialog);

			g_error_free(error);

			return -1;
		}
	}

	g_set_application_name(appname);

	rc = initialize();
	if(!rc)
	{
		configuration_init();

		toplevel = pw3270_new(host);

		toplevel_setup(GTK_WINDOW(toplevel));

		gtk_window_present(GTK_WINDOW(toplevel));

		if(pw3270_get_toggle(toplevel,LIB3270_TOGGLE_FULL_SCREEN))
			gtk_window_fullscreen(GTK_WINDOW(toplevel));

		gtk_main();

		configuration_deinit();
	}

	return rc;
}

