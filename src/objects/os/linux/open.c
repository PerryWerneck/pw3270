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

#include <lib3270.h>
#include <lib3270/log.h>
#include <pw3270/application.h>
#include <pw3270/window.h>
#include <v3270.h>
#include <v3270/keyfile.h>

static gchar * v3270_keyfile_find(const gchar *name) {
	//
	// It can be a session file, scans for it
	//
	const gchar * paths[] = {
		g_get_user_special_dir(G_USER_DIRECTORY_DOCUMENTS),
		g_get_user_config_dir()
	};

	static const gchar *subdirs[] = {
		"3270",
		G_STRINGIFY(PRODUCT_NAME),
		PACKAGE_NAME
	};

	size_t path, subdir;

	g_autofree gchar * filename = g_strconcat(name,".3270",NULL);

	for(path = 0; path < G_N_ELEMENTS(paths); path++) {

		// Try subdirs.
		for(subdir = 0; subdir < G_N_ELEMENTS(subdirs); subdir++) {

			gchar * fullpath = g_build_filename(paths[path],subdirs[subdir],filename,NULL);

			debug("Searching for \"%s\"",fullpath);

			if(g_file_test(fullpath,G_FILE_TEST_IS_REGULAR)) {
				return fullpath;
			}
			g_free(fullpath);

		}

		// Try path.
		{
			gchar * fullpath = g_build_filename(paths[path],filename,NULL);

			debug("Searching for \"%s\"",fullpath);

			if(g_file_test(fullpath,G_FILE_TEST_IS_REGULAR)) {
				return fullpath;
			}
			g_free(fullpath);
		}

	}

	return NULL;

}

/// @brief Open session file
static void open(GtkApplication *application, GtkWindow **window, const gchar *filename) {

	g_message("Opening '%s'",filename);

	if(*window) {

		// Already open a window, open in new tab.
		pw3270_application_window_new_tab(GTK_WIDGET(*window), filename);

	} else {
		// It's a new window
		*window = GTK_WINDOW(pw3270_application_window_new(application, filename));

	}

}

void pw3270_application_open_file(GtkApplication *application, GtkWindow **window, GFile *file) {

	g_autofree gchar * scheme = g_file_get_uri_scheme(file);

	if(g_ascii_strcasecmp(scheme,"file") == 0) {

		// It's a file scheme.
		if(g_file_query_exists(file,NULL)) {

			// The file exists, load it.
			g_autofree gchar *filename = g_file_get_path(file);
			open(application,window,filename);

		} else {

			// Search for file.
			g_autofree gchar * basename = g_file_get_basename(file);
			g_autofree gchar * filename = v3270_keyfile_find(basename);

			if(filename) {
				open(application,window,filename);
			} else {
				g_warning("Cant find session '%s'",basename);
			}

		}

	} else if(g_ascii_strcasecmp(scheme,"tn3270") == 0 || g_ascii_strcasecmp(scheme,"tn3270s") == 0) {

		g_autofree gchar * uri = g_file_get_uri(file);
		size_t sz = strlen(uri);

		if(sz > 0 && uri[sz-1] == '/')
			uri[sz-1] = 0;

		g_message("Opening '%s' with default settings",uri);

		if(!*window) {
			*window = GTK_WINDOW(pw3270_application_window_new(application, NULL));
		} else {
			pw3270_application_window_new_tab(GTK_WIDGET(*window), NULL);
		}

		GtkWidget * terminal = pw3270_application_window_get_active_terminal(GTK_WIDGET(*window));
		v3270_set_default_session(terminal);
		v3270_set_url(terminal,uri);

	} else {

		g_warning("Don't know how to handle '%s' scheme",scheme);

	}

}
