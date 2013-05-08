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
 * Este programa está nomeado como config.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

 #include <gtk/gtk.h>
 #include "common.h"
 #include <stdarg.h>
 #include <glib/gstdio.h>

#ifdef WIN32

	#include <windows.h>
	// #define HAVE_WIN_REGISTRY 1

	#ifndef KEY_WOW64_64KEY
		#define KEY_WOW64_64KEY 0x0100
	#endif // KEY_WOW64_64KEY

	#ifndef KEY_WOW64_32KEY
		#define KEY_WOW64_32KEY	0x0200
	#endif // KEY_WOW64_64KEY

#endif // WIN32

/*--[ Globals ]--------------------------------------------------------------------------------------*/

#ifdef HAVE_WIN_REGISTRY

 	static const gchar	* registry_path = "SOFTWARE";

#else

	static GKeyFile		* program_config = NULL;
 	static const gchar	* mask = "%s" G_DIR_SEPARATOR_S "%s.conf";

#endif // HAVE_WIN_REGISTRY

/*--[ Implement ]------------------------------------------------------------------------------------*/

#ifdef HAVE_WIN_REGISTRY

 enum REG_KEY
 {
	REG_KEY_USER,
	REG_KEY_SYSTEM,
	REG_KEY_INEXISTENT
 };

 static enum REG_KEY registry_query(const gchar *group, const gchar *key, HKEY *hKey)
 {
 	static HKEY	  predefined[] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };
	gchar		* path = g_strdup_printf("%s\\%s\\%s",registry_path,g_get_application_name(),group);
	int			  f;

	for(f=0;f<G_N_ELEMENTS(predefined);f++)
	{
		if(RegOpenKeyEx(predefined[f],path,0,KEY_READ,hKey) == ERROR_SUCCESS)
		{
			if(RegQueryValueExA(*hKey,key,NULL,NULL,NULL,NULL) == ERROR_SUCCESS)
			{
				trace("Key[%s\%s] found at id %d",path,key,f);
				g_free(path);
				return f;
			}
			RegCloseKey(*hKey);
		}
	}

	trace("Key[%s\%s] not found",path,key,f);
	g_free(path);

	return -1;
 }

 static BOOL registry_open_key(const gchar *group, const gchar *key, REGSAM samDesired, HKEY *hKey)
 {
 	static HKEY	  predefined[] = { HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE };
 	int			  f;
	gchar		* path = g_strdup_printf("%s\\%s\\%s",registry_path,g_get_application_name(),group);

	for(f=0;f<G_N_ELEMENTS(predefined);f++)
	{
		if(RegOpenKeyEx(predefined[f],path,0,samDesired,hKey) == ERROR_SUCCESS)
		{
			g_free(path);
			return TRUE;
		}
	}

	g_free(path);

	return FALSE;
 }

 void registry_foreach(HKEY parent, const gchar *name,void (*cbk)(const gchar *key, const gchar *val, gpointer *user_data), gpointer *user_data)
 {
	HKEY hKey = 0;

	if(RegOpenKeyEx(parent,name,0,KEY_READ,&hKey) == ERROR_SUCCESS)
	{
		#define MAX_KEY_LENGTH 255
		#define MAX_VALUE_NAME 16383

		TCHAR    	pName[MAX_KEY_LENGTH];
		DWORD 		cName	= MAX_KEY_LENGTH;
		int			ix		= 0;

		while(RegEnumValue(hKey,ix++,pName,&cName,NULL,NULL,NULL,NULL) == ERROR_SUCCESS)
		{
			BYTE data[4097];
			unsigned long datatype;
			unsigned long datalen 	= 4096;

			if(RegQueryValueExA(hKey,pName,NULL,&datatype,data,&datalen) == ERROR_SUCCESS)
			{
				data[datalen+1] = 0;
				cbk(pName,(const gchar *) data, user_data);
			}
			cName = MAX_KEY_LENGTH;
		}
		RegCloseKey(hKey);
	}
 }

 void registry_set_double(HKEY hKey, const gchar *key, gdouble value)
 {
	// Reference: http://git.gnome.org/browse/glib/tree/glib/gkeyfile.c
	gchar result[G_ASCII_DTOSTR_BUF_SIZE];
	g_ascii_dtostr (result, sizeof (result), value);

	RegSetValueEx(hKey,key,0,REG_SZ,(const BYTE *) result,strlen(result)+1);
 }

 gboolean registry_get_double(HKEY hKey, const gchar *key, gdouble *value)
 {
//	GError			* error = NULL;
	BYTE			  data[4096];
	unsigned long	  datatype;
	unsigned long	  datalen 	= sizeof(data);
	gchar 			* end_of_valid_d;

	if(RegQueryValueExA(hKey,key,NULL,&datatype,data,&datalen) != ERROR_SUCCESS)
		return FALSE;

	data[datalen] = 0;

	* value = g_ascii_strtod((const gchar *) data, &end_of_valid_d);

	if(*end_of_valid_d != '\0' || end_of_valid_d == ((gchar *) data))
	{
		g_warning("Key %s on registry isnt a valid double value",key);
		return FALSE;
	}

 	return TRUE;
 }



#else
 static gchar * search_for_ini(void)
 {
 	static const gchar * (*dir[])(void) =
 	{
 		g_get_user_config_dir,
 		g_get_user_data_dir,
 		g_get_home_dir,

	};
 	gchar *filename;
 	int f;

	const gchar * const *sysconfig;

#ifdef DEBUG
	filename = g_strdup_printf(mask,".",g_get_application_name());
	trace("Checking for %s",filename);
	if(g_file_test(filename,G_FILE_TEST_IS_REGULAR))
		return filename;
	g_free(filename);
#endif

 	for(f=0;f<G_N_ELEMENTS(dir);f++)
	{
		filename = g_strdup_printf(mask,dir[f](),g_get_application_name());
		trace("Checking for %s",filename);
		if(g_file_test(filename,G_FILE_TEST_IS_REGULAR))
			return filename;
		g_free(filename);
	}

	sysconfig = g_get_system_config_dirs();
 	for(f=0;sysconfig[f];f++)
	{
		filename = g_strdup_printf(mask,sysconfig[f],g_get_application_name());
		trace("Checking for %s",filename);
		if(g_file_test(filename,G_FILE_TEST_IS_REGULAR))
			return filename;
		g_free(filename);
	}

	sysconfig = g_get_system_data_dirs();
 	for(f=0;sysconfig[f];f++)
	{
		filename = g_strdup_printf(mask,sysconfig[f],g_get_application_name());
		trace("Checking for %s",filename);
		if(g_file_test(filename,G_FILE_TEST_IS_REGULAR))
			return filename;
		g_free(filename);
	}

 	return g_strdup_printf(mask,g_get_user_config_dir(),g_get_application_name());
 }
#endif  //  #ifdef HAVE_WIN_REGISTRY

 gboolean get_boolean_from_config(const gchar *group, const gchar *key, gboolean def)
 {
#ifdef HAVE_WIN_REGISTRY
	gboolean	ret 		= def;
	HKEY 		hKey;

	if(registry_query(group,key,&hKey) != REG_KEY_INEXISTENT)
	{
		DWORD			  data;
		unsigned long	  datalen	= sizeof(data);
		unsigned long	  datatype;

		if(RegQueryValueExA(hKey,key,NULL,&datatype,(BYTE *) &data,&datalen) == ERROR_SUCCESS)
		{
			if(datatype == REG_DWORD)
				ret = data ? TRUE : FALSE;
			else
				g_warning("Unexpected registry data type in %s\\%s\\%s\\%s",registry_path,g_get_application_name(),group,key);
		}

		RegCloseKey(hKey);
	}

	return ret;

#else

	if(program_config)
	{
		GError		* err = NULL;
		gboolean	  val = g_key_file_get_boolean(program_config,group,key,&err);
		if(err)
			g_error_free(err);
		else
			return val;
	}
#endif // HAVE_WIN_REGISTRY

	return def;
 }

 gint get_integer_from_config(const gchar *group, const gchar *key, gint def)
 {
#ifdef HAVE_WIN_REGISTRY

	HKEY key_handle;

	if(registry_open_key(group,key,KEY_READ,&key_handle))
	{
		DWORD			data;
		gint			ret = def;
		unsigned long	datalen	= sizeof(data);
		unsigned long	datatype;

		if(RegQueryValueExA(key_handle,key,NULL,&datatype,(BYTE *) &data,&datalen) == ERROR_SUCCESS)
		{
			if(datatype == REG_DWORD)
				ret = (gint) data;
			else
				g_warning("Unexpected registry data type in %s\\%s\\%s\\%s",registry_path,g_get_application_name(),group,key);
		}

		RegCloseKey(key_handle);

		return ret;

	}

#else

	if(program_config)
	{
		GError	* err = NULL;
		gint	  val = g_key_file_get_integer(program_config,group,key,&err);
		if(err)
			g_error_free(err);
		else
			return val;
	}
#endif // HAVE_WIN_REGISTRY

	return def;
 }


 gchar * get_string_from_config(const gchar *group, const gchar *key, const gchar *def)
 {
#ifdef HAVE_WIN_REGISTRY

	HKEY key_handle;
	unsigned long	  datalen 	= 4096;
	unsigned long	  datatype;
	gchar 			* ret		= NULL;
	BYTE			* data;

	if(!registry_open_key(group,key,KEY_READ,&key_handle))
	{
		if(def)
			return g_strdup(def);
		return NULL;
	}

	data = (BYTE *) g_malloc0(datalen+2);

	if(RegQueryValueExA(key_handle,key,NULL,&datatype,data,&datalen) == ERROR_SUCCESS)
	{
		data[datalen+1] = 0;
		ret = g_strdup((const gchar *) data);
		trace("datalen=%d",datalen);
	}
	else if(def)
	{
		ret = g_strdup(def);
	}

	g_free(data);

	RegCloseKey(key_handle);

	return ret;

#else

	if(program_config)
	{
		gchar *ret = g_key_file_get_string(program_config,group,key,NULL);
		if(ret)
			return ret;
	}

	if(def)
		return g_strdup(def);

	return NULL;

#endif // HAVE_WIN_REGISTRY
 }

void configuration_init(void)
{
#ifndef HAVE_WIN_REGISTRY
	gchar *filename = search_for_ini();

	if(program_config)
		g_key_file_free(program_config);

	program_config = g_key_file_new();

	if(filename)
	{
		g_key_file_load_from_file(program_config,filename,G_KEY_FILE_NONE,NULL);
		g_free(filename);
	}

#endif // HAVE_WIN_REGISTRY
}

static void set_string(const gchar *group, const gchar *key, const gchar *fmt, va_list args)
{
	gchar * value = g_strdup_vprintf(fmt,args);

#ifdef HAVE_WIN_REGISTRY

	gchar * path = g_strdup_printf("%s\\%s\\%s",registry_path,g_get_application_name(),group);
 	HKEY	hKey;
 	DWORD	disp;

	if(RegCreateKeyEx(HKEY_CURRENT_USER,path,0,NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,&disp) == ERROR_SUCCESS)
	{
		RegSetValueEx(hKey,key,0,REG_SZ,(const BYTE *) value,strlen(value)+1);
		RegCloseKey(hKey);
	}

	g_free(path);

#else

	g_key_file_set_string(program_config,group,key,value);

#endif

	g_free(value);
}

void set_string_to_config(const gchar *group, const gchar *key, const gchar *fmt, ...)
{
 	va_list args;
	va_start(args, fmt);
	set_string(group,key,fmt,args);
	va_end(args);
}

void set_boolean_to_config(const gchar *group, const gchar *key, gboolean val)
{
#ifdef HAVE_WIN_REGISTRY

 	HKEY	hKey;
 	DWORD	disp;
	gchar * path = g_strdup_printf("%s\\%s\\%s",registry_path,g_get_application_name(),group);

	trace("Creating key %s",path);
	if(RegCreateKeyEx(HKEY_CURRENT_USER,path,0,NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,&disp) == ERROR_SUCCESS)
	{
		DWORD	value = val ? 1 : 0;
		LONG	rc = RegSetValueEx(hKey, key, 0, REG_DWORD,(const BYTE *) &value,sizeof(value));

		SetLastError(rc);

		if(rc != ERROR_SUCCESS)
		{
			gchar *msg = g_win32_error_message(GetLastError());
			g_warning("Error \"%s\" when setting key HKCU\\%s\\%s",msg,path,key);
			g_free(msg);
		}
		RegCloseKey(hKey);
	}
	else
	{
		gchar *msg = g_win32_error_message(GetLastError());
		g_warning("Error \"%s\" when creating key HKCU\\%s",msg,path);
		g_free(msg);
	}

	g_free(path);

#else

	if(program_config)
		g_key_file_set_boolean(program_config,group,key,val);

#endif // HAVE_WIN_REGISTRY

}

void set_integer_to_config(const gchar *group, const gchar *key, gint val)
{
#ifdef HAVE_WIN_REGISTRY

 	HKEY	hKey;
 	DWORD	disp;
	gchar * path = g_strdup_printf("%s\\%s\\%s",registry_path,g_get_application_name(),group);

	trace("Creating key %s",path);
	if(RegCreateKeyEx(HKEY_CURRENT_USER,path,0,NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,&disp) == ERROR_SUCCESS)
	{
		DWORD	value = (DWORD) val;
		LONG	rc = RegSetValueEx(hKey, key, 0, REG_DWORD,(const BYTE *) &value,sizeof(value));

		SetLastError(rc);

		if(rc != ERROR_SUCCESS)
		{
			gchar *msg = g_win32_error_message(GetLastError());
			g_warning("Error \"%s\" when setting key HKCU\\%s\\%s",msg,path,key);
			g_free(msg);
		}
		RegCloseKey(hKey);
	}
	else
	{
		gchar *msg = g_win32_error_message(GetLastError());
		g_warning("Error \"%s\" when creating key HKCU\\%s",msg,path);
		g_free(msg);
	}

	g_free(path);

#else

	if(program_config)
		g_key_file_set_integer(program_config,group,key,val);

#endif // HAVE_WIN_REGISTRY

}

void configuration_deinit(void)
{
#ifdef HAVE_WIN_REGISTRY

#else

	gchar *text;

	if(!program_config)
		return;

	text = g_key_file_to_data(program_config,NULL,NULL);

	if(text)
	{
		gchar *filename = g_strdup_printf(mask,g_get_user_config_dir(),g_get_application_name());

		trace("Saving configuration in \"%s\"",filename);

		g_mkdir_with_parents(g_get_user_config_dir(),S_IRUSR|S_IWUSR);

		g_file_set_contents(filename,text,-1,NULL);

		g_free(text);
	}


	g_key_file_free(program_config);
	program_config = NULL;

#endif // HAVE_WIN_REGISTRY
}

gchar * build_data_filename(const gchar *first_element, ...)
{
	va_list args;
	gchar	*path;

	va_start(args, first_element);
	path = filename_from_va(first_element,args);
	va_end(args);
	return path;
}

gchar * filename_from_va(const gchar *first_element, va_list args)
{
	static const gchar	* datadir	= NULL;
	const gchar *		  appname[]	= { g_get_application_name(), PACKAGE_NAME };
	GString			 	* result	= NULL;
	const gchar			* element;

	if(datadir)
		result = g_string_new(datadir);

#if defined( HAVE_WIN_REGISTRY )

	if(!result)
	{
		// No predefined datadir, search registry
		int p;

		for(p=0;p<G_N_ELEMENTS(appname) && !result;p++)
		{
			gchar	* path	= g_strconcat("Software\\",appname[p],NULL);
			HKEY	  hKey	= 0;
			LONG	  rc 	= 0;

			// Note: This could be needed: http://support.microsoft.com/kb/556009
			// http://msdn.microsoft.com/en-us/library/windows/desktop/aa384129(v=vs.85).aspx

			rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,path,0,KEY_QUERY_VALUE,&hKey);
			SetLastError(rc);

			if(rc == ERROR_SUCCESS)
			{
				char			data[4096];
				unsigned long	datalen	= sizeof(data);		// data field length(in), data returned length(out)
				unsigned long	datatype;					// #defined in winnt.h (predefined types 0-11)

				rc = RegQueryValueExA(hKey,"datadir",NULL,&datatype,(LPBYTE) data,&datalen);
				if(rc == ERROR_SUCCESS)
				{
					result = g_string_new(g_strchomp(data));
				}
				else
				{
					gchar *msg = g_win32_error_message(rc);
#ifndef DEBUG
					g_message("Error \"%s\" when getting application datadir from registry",msg);
#endif // !DEBUG
					g_free(msg);
				}
				RegCloseKey(hKey);
			}
			else
			{
				gchar *msg = g_win32_error_message(rc);
#ifndef DEBUG
				g_message("Error \"%s\" when opening datadir key from registry",msg);
#endif // !DEBUG
				g_free(msg);
			}

			g_free(path);
		}
	}
#endif // HAVE_WIN_REGISTRY

	if(!result)
	{
		// Search for application folder on system data dirs
		const gchar * const * dir = g_get_system_data_dirs();
		int p;

		for(p=0;p<G_N_ELEMENTS(appname) && !datadir;p++)
		{
			int f;

			for(f=0;dir[f] && !datadir;f++)
			{
				gchar *name = g_build_filename(dir[f],appname[p],NULL);
//				trace("Searching for %s: %s",name,g_file_test(name,G_FILE_TEST_IS_DIR) ? "Ok" : "Not found");
				if(g_file_test(name,G_FILE_TEST_IS_DIR))
					result = g_string_new(datadir = name);
				else
					g_free(name);
			}
		}

	}

#ifdef DEBUG
	if(!result)
	{
		int f;
		gchar *dir = g_get_current_dir();

		for(f=0;f<2 && dir;f++)
		{
			gchar *ptr = dir;
			dir = g_path_get_dirname(ptr);
			g_free(ptr);
		}

		if(dir)
		{
			gchar *name = g_build_filename(dir,"ui",NULL);
			if(g_file_test(name,G_FILE_TEST_IS_DIR))
				result = g_string_new(dir);
			g_free(name);
			g_free(dir);
		}

	}
#endif // DEBUG

	if(!result)
	{
		gchar *dir = g_get_current_dir();
		result = g_string_new(dir);
		g_free(dir);
		g_warning("Unable to find application datadir, using %s",result->str);
	}

	for(element = first_element;element;element = va_arg(args, gchar *))
    {
    	g_string_append_c(result,G_DIR_SEPARATOR);
    	g_string_append(result,element);
    }

	return g_string_free(result, FALSE);
}

#ifdef HAVE_WIN_REGISTRY
gboolean get_registry_handle(const gchar *group, HKEY *hKey, REGSAM samDesired)
{
	gboolean	  ret;
	gchar		* path = g_strdup_printf("%s\\%s\\%s",registry_path,g_get_application_name(),group);
 	DWORD		  disp;

	if(RegCreateKeyEx(HKEY_CURRENT_USER,path,0,NULL,REG_OPTION_NON_VOLATILE,samDesired,NULL,hKey,&disp) == ERROR_SUCCESS)
		ret = TRUE;
	else
		ret = FALSE;

	g_free(path);

	return ret;
}
#else
GKeyFile * get_application_keyfile(void)
{
	if(!program_config)
		configuration_init();
	return program_config;
}
#endif // HAVE_WIN_REGISTRY

 static const struct _WindowState
 {
        const char *name;
        GdkWindowState flag;
        void (*activate)(GtkWindow *);
 } WindowState[] =
 {
        { "Maximized",	GDK_WINDOW_STATE_MAXIMIZED,	gtk_window_maximize             },
        { "Iconified",	GDK_WINDOW_STATE_ICONIFIED,	gtk_window_iconify              },
        { "Sticky",		GDK_WINDOW_STATE_STICKY,	gtk_window_stick                }
 };

void save_window_state_to_config(const gchar *group, const gchar *key, GdkWindowState CurrentState)
{
#if defined( HAVE_WIN_REGISTRY )

        gchar * path = g_strdup_printf("%s\\%s\\%s\\%s",registry_path,g_get_application_name(),group,key);

        HKEY    hKey;
        DWORD   disp;

		if(RegCreateKeyEx(HKEY_CURRENT_USER,path,0,NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,&disp) == ERROR_SUCCESS)
		{
			int f;
			for(f=0;f<G_N_ELEMENTS(WindowState);f++)
			{
				DWORD value = (CurrentState & WindowState[f].flag) ? 1 : 0;
//				trace("%s=%s",WindowState[f].name,value ? "Yes" : "No");
				RegSetValueEx(hKey, WindowState[f].name, 0, REG_DWORD,(const BYTE *) &value,sizeof(value));
			}

			RegCloseKey(hKey);
		}

		g_free(path);

#else
		int			  f;
		GKeyFile	* conf 	= get_application_keyfile();
		gchar		* id	= g_strconcat(group,".",key,NULL);

		for(f=0;f<G_N_ELEMENTS(WindowState);f++)
			g_key_file_set_boolean(conf,id,WindowState[f].name,CurrentState & WindowState[f].flag);

		g_free(id);

#endif // HAVE_WIN_REGISTRY
}

void save_window_size_to_config(const gchar *group, const gchar *key, GtkWidget *hwnd)
{
#if defined( HAVE_WIN_REGISTRY )

        gchar * path = g_strdup_printf("%s\\%s\\%s\\%s",registry_path,g_get_application_name(),group,key);

        HKEY    hKey;
        DWORD   disp;

		if(RegCreateKeyEx(HKEY_CURRENT_USER,path,0,NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,&disp) == ERROR_SUCCESS)
		{
			int pos[2];

			gtk_window_get_size(GTK_WINDOW(hwnd),&pos[0],&pos[1]);

			RegSetValueEx(hKey, "Size", 0, REG_BINARY,(const BYTE *) pos,sizeof(pos));

			RegCloseKey(hKey);
		}

		g_free(path);

#else
		int 		  pos[2];
		GKeyFile	* conf 	= get_application_keyfile();
		gchar		* id	= g_strconcat(group,".",key,NULL);

        gtk_window_get_size(GTK_WINDOW(hwnd),&pos[0],&pos[1]);
        g_key_file_set_integer_list(conf,id,"size",pos,2);

		g_free(id);

#endif // HAVE_WIN_REGISTRY
}

#if defined( HAVE_WIN_REGISTRY )
static void restore_window_from_regkey(GtkWidget *hwnd, HKEY hKey, const gchar *path)
{
	int 			f;
	int 			pos[2];
	unsigned long	datalen;
	unsigned long	datatype;


	datalen = sizeof(pos);
	if(RegQueryValueExA(hKey,"Size",NULL,&datatype,(BYTE *) pos,&datalen) == ERROR_SUCCESS)
	{
		if(datatype == REG_BINARY && datalen == sizeof(pos))
		{
			gtk_window_resize(GTK_WINDOW(hwnd),pos[0],pos[1]);
		}
		else
		{
			g_warning("Unexpected registry data in %s\\Size",path);
		}
	}


	for(f=0;f<G_N_ELEMENTS(WindowState);f++)
	{
		DWORD			data;

		datalen       = sizeof(data);

		if(RegQueryValueExA(hKey,WindowState[f].name,NULL,&datatype,(BYTE *) &data,&datalen) == ERROR_SUCCESS)
		{
			if(datatype == REG_DWORD)
			{
				if(data)
					WindowState[f].activate(GTK_WINDOW(hwnd));

			}
			else
			{
				g_warning("Unexpected registry data type in %s\\%s",path,WindowState[f].name);
			}
		}
	}


}
#endif // HAVE_WIN_REGISTRY

void restore_window_from_config(const gchar *group, const gchar *key, GtkWidget *hwnd)
{
#if defined( HAVE_WIN_REGISTRY )

	gchar * path = g_strdup_printf("%s\\%s\\%s\\%s",registry_path,g_get_application_name(),group,key);
	HKEY    hKey;

	if(RegOpenKeyEx(HKEY_CURRENT_USER,path,0,KEY_READ,&hKey) == ERROR_SUCCESS)
	{
		// Load user settings
		restore_window_from_regkey(hwnd,hKey,path);
		RegCloseKey(hKey);
	}
	else if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,path,0,KEY_READ,&hKey) == ERROR_SUCCESS)
	{
		// Load system defaults
		restore_window_from_regkey(hwnd,hKey,path);
		RegCloseKey(hKey);
	}

	g_free(path);

#else
	gchar		* id	= g_strconcat(group,".",key,NULL);
	GKeyFile	* conf 	= get_application_keyfile();

	if(g_key_file_has_key(conf,id,"size",NULL))
	{
		gsize	  sz	= 2;
		gint	* vlr	=  g_key_file_get_integer_list(conf,id,"size",&sz,NULL);
		int		  f;

		if(vlr)
		{
			gtk_window_resize(GTK_WINDOW(hwnd),vlr[0],vlr[1]);
			g_free(vlr);
		}

		for(f=0;f<G_N_ELEMENTS(WindowState);f++)
		{
			if(g_key_file_get_boolean(conf,id,WindowState[f].name,NULL))
				WindowState[f].activate(GTK_WINDOW(hwnd));
		}

	}

	g_free(id);

#endif // HAVE_WIN_REGISTRY

}

