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

 #include <config.h>
 #include "../private.h"
 #include <v3270/settings.h>
 #include <lib3270/log.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 void load_terminal_settings(GtkWidget *widget)
 {
	GError *error = NULL;
	g_autofree gchar * name = g_strconcat(g_get_application_name(),".conf",NULL);

#ifdef DATADIR
	//
	// Search the application DATADIR
	//
	{
		g_autofree gchar *filename = g_build_filename(DATAROOTDIR,G_STRINGIFY(PRODUCT_NAME),name,NULL);

		if(g_file_test(filename,G_FILE_TEST_IS_REGULAR))
		{
			GKeyFile *key_file = g_key_file_new();

			g_key_file_load_from_file(key_file,filename,G_KEY_FILE_NONE,&error);

			if(error)
			{
				g_warning("Can't load \"%s\": %s",filename,error->message);
				g_error_free(error);
				error = NULL;
			}
			else
			{
				g_message("Loading system settings from %s",filename);
				v3270_load_key_file(widget,key_file,"terminal");
			}

			g_key_file_free(key_file);

		}

	}
#endif // DATADIR

	//
	// Get from user datadir
	//
	{
		g_autofree gchar *filename = g_build_filename(g_get_user_config_dir(),name,NULL);

		if(g_file_test(filename,G_FILE_TEST_IS_REGULAR))
		{
			GKeyFile *key_file = g_key_file_new();

			g_key_file_load_from_file(key_file,filename,G_KEY_FILE_NONE,&error);

			if(error)
			{
				g_warning("Can't load \"%s\": %s",filename,error->message);
				g_error_free(error);
				error = NULL;
			}
			else
			{
				g_message("Loading user settings from %s",filename);
				v3270_load_key_file(widget,key_file,"terminal");
			}

			g_key_file_free(key_file);

		}

	}

 }

