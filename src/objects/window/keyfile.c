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
 #include <glib.h>
 #include <glib/gstdio.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <lib3270.h>
 #include <lib3270/log.h>
 #include <v3270.h>
 #include <v3270/settings.h>
 #include <v3270/keyfile.h>
 #include <v3270/actions.h>
 #include <lib3270/properties.h>
 #include <string.h>
 #include <stdlib.h>

 struct _V3270KeyFile
 {
 	gboolean	  changed;		///< @brief Save file?
	GKeyFile	* key_file;
	gchar		  filename[1];
 };

 static V3270KeyFile * v3270_get_session_descriptor(GtkWidget *terminal) {

 	return (V3270KeyFile *) g_object_get_data(G_OBJECT(terminal),"session-descriptor");

 }

 static void close_keyfile(V3270KeyFile * session) {

 	if(session->key_file) {

		if(session->changed) {
			g_message("Saving file %s",session->filename);
			g_key_file_save_to_file(session->key_file,session->filename,NULL);
			session->changed = FALSE;
		} else {
			g_message("Closing file %s",session->filename);
		}

		g_key_file_free(session->key_file);
		session->key_file = NULL;
 	}

 	g_free(session);
 }

 V3270KeyFile * v3270_key_file_open(GtkWidget *terminal, const gchar *filename, GError **error) {

	g_return_val_if_fail(GTK_IS_V3270(terminal),FALSE);
	g_return_val_if_fail(*error == NULL,FALSE);

	V3270KeyFile * new_session = (V3270KeyFile *) g_malloc0(sizeof(struct _V3270KeyFile) + strlen(filename));
	V3270KeyFile * old_session = g_object_get_data(G_OBJECT(terminal),"session-descriptor");

	// Clone session
	if(old_session) {
		*new_session = *old_session;
	}

	strcpy(new_session->filename,filename);
	new_session->key_file = g_key_file_new();
	g_object_set_data_full(G_OBJECT(terminal),"session-descriptor",new_session,(GDestroyNotify) close_keyfile);

	// Load file
	if(g_file_test(new_session->filename,G_FILE_TEST_IS_REGULAR)) {

		// Found session file, open it.
        if(!g_key_file_load_from_file(new_session->key_file,new_session->filename,G_KEY_FILE_NONE,error)) {
			g_warning("Can't load \"%s\"",new_session->filename);
        } else {
			g_message("Loading session preferences from %s",new_session->filename);
        }

	} else {

		// No session file, load the defaults (if available) and save file
		lib3270_autoptr(char) default_settings = lib3270_build_data_filename("defaults.conf",NULL);

		if(g_file_test(default_settings,G_FILE_TEST_IS_REGULAR)) {
			if(!g_key_file_load_from_file(new_session->key_file,default_settings,G_KEY_FILE_NONE,error)) {
				g_warning("Can't load \"%s\"",default_settings);
			} else {
				g_message("Loading session preferences from %s",default_settings);
			}
		} else {
#ifdef DEBUG
			g_message("Can't find default settings file \"%s\"",default_settings);
#else
			g_warning("Can't find default settings file \"%s\"",default_settings);
#endif // DEBUG
		}

	}

	if(*error) {
		g_warning("Error '%s' loading keyfile", (*error)->message);
		return new_session;
	}

	// Got key file, load it.
	v3270_load_key_file(terminal,new_session->key_file,NULL);
	v3270_accelerator_map_load_key_file(terminal,new_session->key_file,NULL);

	if(g_key_file_has_group(new_session->key_file,"environment")) {

		// Has environment group, set values.
		gchar **keys = g_key_file_get_keys(new_session->key_file,"environment",NULL,NULL);

		if(keys) {
			size_t ix;
			for(ix=0;keys[ix];ix++) {
				g_autofree gchar * value = g_key_file_get_string(new_session->key_file,"environment",keys[ix],NULL);
				if(value) {
#ifdef _WIN32
					g_autofree gchar * env = g_strconcat(keys[ix],"=",value,NULL);
					putenv(env);
#else
					if(setenv(keys[ix],value,1)) {
						g_warning("Can't set \"%s\" to \"%s\"",keys[ix],value);
					}
#endif // _WIN32
				}
			}

			g_strfreev(keys);
		}
	}

	return new_session;
}

void v3270_key_file_close(GtkWidget *terminal) {

	V3270KeyFile *session = g_object_get_data(G_OBJECT(terminal),"session-descriptor");

 	if(session->key_file) {

		if(session->changed) {
			g_message("Saving file %s",session->filename);
			g_key_file_save_to_file(session->key_file,session->filename,NULL);
			session->changed = FALSE;
		} else {
			g_message("Closing file %s",session->filename);
		}

		g_key_file_free(session->key_file);
		session->key_file = NULL;
 	}

 }

 GKeyFile * v3270_key_file_get(GtkWidget *terminal) {
	return v3270_get_session_descriptor(terminal)->key_file;
 }

 void v3270_key_file_save_to_file(GtkWidget * terminal, const gchar *filename, GError **error) {

	if(error && *error)
		return;

	V3270KeyFile * new_session = (V3270KeyFile *) g_malloc0(sizeof(struct _V3270KeyFile) + strlen(filename));
	V3270KeyFile * old_session = g_object_get_data(G_OBJECT(terminal),"session-descriptor");

 	if(old_session) {
		*new_session = *old_session;
	}

	strcpy(new_session->filename,filename);
	new_session->key_file = g_key_file_new();

	g_object_set_data_full(G_OBJECT(terminal),"session-descriptor",new_session,(GDestroyNotify) close_keyfile);
	v3270_key_file_save(terminal,error);

 }

 void v3270_key_file_save(GtkWidget *terminal, GError **error) {

	if(error && *error)
		return;

	V3270KeyFile *session = v3270_get_session_descriptor(terminal);

	session->changed = FALSE;

	debug("%s: terminal=%p session=%p key-file=%p)",__FUNCTION__,terminal,session,session->key_file);

	v3270_to_key_file(terminal,session->key_file,"terminal");
	v3270_accelerator_map_to_key_file(terminal, session->key_file, "accelerators");

	g_key_file_save_to_file(session->key_file,session->filename,NULL);

 }

 /// @brief Search standard paths.
 gchar * v3270_key_file_get_default_path(GtkWidget *terminal) {

	size_t folder;
	const gchar *folders[] = {
		g_get_user_data_dir(),
		g_get_user_special_dir(G_USER_DIRECTORY_DOCUMENTS),
		g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP)
	};

	size_t application;
	const gchar *applications[] = {
		G_STRINGIFY(PRODUCT_NAME),
		PACKAGE_NAME,
		"3270",
		"tn3270"
	};

	for(folder = 0; folder < G_N_ELEMENTS(folders); folder++) {

		if(!(folders[folder] && g_file_test(folders[folder],G_FILE_TEST_IS_DIR)))
			continue;

		for(application = 0; application < G_N_ELEMENTS(applications); application++) {

			gchar * appdir = g_build_filename(folder[folders],application[applications],NULL);

			debug("Testing for \"%s\"",appdir);
			if(g_file_test(appdir,G_FILE_TEST_IS_DIR)) {
				return appdir;
			}
			g_free(appdir);

		}

	}

	// Not found, try the current session path.
	const gchar * filename = v3270_key_file_get_filename(terminal);
	debug("Testing for \"%s\"",filename);
	if(filename
			&& g_file_test(filename,G_FILE_TEST_IS_REGULAR)
			&& g_str_has_prefix(filename,g_get_user_data_dir())
			&& !g_str_has_prefix(filename,g_get_user_config_dir())
		) {
		return g_path_get_dirname(filename);
	}

	return g_strdup(g_get_user_special_dir(G_USER_DIRECTORY_DOCUMENTS));

 }

 gchar * v3270_keyfile_get_default_filename(void) {

	gchar * filename = g_build_filename(g_get_user_config_dir(),"default.3270",NULL);

	g_autofree gchar * compatible = g_build_filename(g_get_user_config_dir(),G_STRINGIFY(PRODUCT_NAME) ".conf",NULL);
	if(g_file_test(compatible,G_FILE_TEST_IS_REGULAR))
		g_rename(compatible,filename);

	return filename;
 }

 gchar * v3270_key_file_build_filename(GtkWidget *terminal) {

  	g_autofree gchar * defname = v3270_keyfile_get_default_filename();
	const gchar * current = v3270_key_file_get_filename(terminal);

	// If is not the default name, return it.
	if(current && *current && g_file_test(current,G_FILE_TEST_IS_REGULAR) && strcmp(defname,current)) {
		return g_strdup(current);
	}

	g_autofree gchar * folder = v3270_key_file_get_default_path(terminal);

	const char * hostname = lib3270_host_get_name(v3270_get_session(terminal));
	debug("Hostname=\"%s\"",hostname);

	gchar * name = g_strconcat(folder,G_DIR_SEPARATOR_S,(hostname ? hostname : "session"),".3270",NULL);
	unsigned int index = 0;
	while(g_file_test(name,G_FILE_TEST_EXISTS)) {
		g_free(name);
		name = g_strdup_printf("%s" G_DIR_SEPARATOR_S "%s.%u.3270",folder,hostname,++index);
	}

	debug("%s returns \"%s\"",__FUNCTION__,name);

	return name;
 }

 const gchar * v3270_key_file_get_filename(GtkWidget *terminal) {

 	V3270KeyFile *session = v3270_get_session_descriptor(terminal);

 	if(session && *session->filename)
		return session->filename;

	return NULL;

 }

 void v3270_key_file_set_boolean(GtkWidget *terminal, const gchar *group_name, const gchar *key, gboolean value) {

	V3270KeyFile *session = v3270_get_session_descriptor(terminal);
	g_key_file_set_boolean(session->key_file,group_name ? group_name : "terminal",key,value);
	session->changed = TRUE;

}

 gboolean v3270_key_file_can_write(GtkWidget *widget) {

#if defined(DEBUG)

	return TRUE;

#else

	const V3270KeyFile * descriptor = v3270_get_session_descriptor(widget);

	if(!(descriptor && *descriptor->filename))
		return FALSE;

	if(g_access(descriptor->filename,W_OK))
		return FALSE;

#ifdef _WIN32
	return TRUE;
#else
	return !g_str_has_prefix(descriptor->filename,g_get_user_config_dir());
#endif // _WIN32

#endif // DEBUG

 }



