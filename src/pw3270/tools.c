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
 * Este programa está nomeado como tools.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "private.h"
 #include <lib3270/charset.h>
 #include <lib3270/toggle.h>

#ifdef _WIN32
	BOOL WINAPI DllMain(HANDLE hinst, DWORD dwcallpurpose, LPVOID lpvResvd);

	static int libpw3270_loaded(void);
	static int libpw3270_unloaded(void);

#else
	int libpw3270_loaded(void) __attribute__((constructor));
	int libpw3270_unloaded(void) __attribute__((destructor));
#endif

/*--[ Implement ]------------------------------------------------------------------------------------*/

#ifdef _WIN32

BOOL WINAPI DllMain(HANDLE hinst, DWORD dwcallpurpose, LPVOID lpvResvd)
{
//	Trace("%s - Library %s",__FUNCTION__,(dwcallpurpose == DLL_PROCESS_ATTACH) ? "Loaded" : "Unloaded");

    switch(dwcallpurpose)
    {
	case DLL_PROCESS_ATTACH:
			libpw3270_loaded();
			break;

	case DLL_PROCESS_DETACH:
			libpw3270_unloaded();
			break;
    }

    return TRUE;
}

#endif // _WIN32

int libpw3270_loaded(void)
{
	trace("%s",__FUNCTION__);
	return 0;
}

int libpw3270_unloaded(void)
{
	trace("%s",__FUNCTION__);
	configuration_deinit();
	return 0;
}


LIB3270_EXPORT gchar * pw3270_build_filename(GtkWidget *widget, const gchar *first_element, ...)
{
	va_list args;
	gchar	*path;

	va_start(args, first_element);
	path = filename_from_va(first_element,args);
	va_end(args);
	return path;
}

LIB3270_EXPORT void pw3270_save_window_size(GtkWidget *widget, const gchar *name)
{
	trace("%s",__FUNCTION__);
	save_window_size_to_config("window", name, widget);
}

LIB3270_EXPORT void pw3270_restore_window(GtkWidget *widget, const gchar *name)
{
	restore_window_from_config("window", name, widget);
}

LIB3270_EXPORT gint pw3270_get_integer(GtkWidget *widget, const gchar *group, const gchar *key, gint def)
{
	return get_integer_from_config(group, key, def);
}

LIB3270_EXPORT gchar * pw3270_get_string(GtkWidget *widget, const gchar *group, const gchar *key, const gchar *def)
{
	return get_string_from_config(group, key, def);
}

LIB3270_EXPORT void pw3270_set_integer(GtkWidget *widget, const gchar *group, const gchar *key, gint val)
{
	set_integer_to_config(group, key, val);
}

LIB3270_EXPORT void pw3270_set_string(GtkWidget *widget, const gchar *group, const gchar *key, const gchar *val)
{
	set_string_to_config(group, key, val);
}

LIB3270_EXPORT gint pw3270_get_boolean(GtkWidget *widget, const gchar *group, const gchar *key, gint def)
{
	return get_boolean_from_config(group, key, def);
}

LIB3270_EXPORT void pw3270_set_boolean(GtkWidget *widget, const gchar *group, const gchar *key, gint val)
{
	set_boolean_to_config(group, key, val);
}

LIB3270_EXPORT gboolean pw3270_set_toggle_by_name(GtkWidget *widget, const gchar *name, gboolean flag)
{
	H3270 			* hSession	= pw3270_get_session(widget);
	LIB3270_TOGGLE	  id		= lib3270_get_toggle_id(name);

	trace("%s(%s) id=%u",__FUNCTION__,name,id);

	if(!hSession || id == (LIB3270_TOGGLE) -1)
		return FALSE;

	lib3270_set_toggle(hSession,id,(int) flag);
	return TRUE;
}

/*
LIB3270_EXPORT gchar * pw3270_get_filename(GtkWidget *widget, const gchar *group, const gchar *key, GtkFileFilter **filter, const gchar *title)
{

	gchar * filename = pw3270_get_string(widget,group,key,NULL);

#if defined(_WIN32)

	GThread 	* thd;
	struct file	  fl;
	GtkWidget	* dialog	= gtk_widget_get_toplevel(widget);
	GdkWindow	* win		= gtk_widget_get_window(GTK_WIDGET(dialog));

	gtk_widget_set_sensitive(GTK_WIDGET(dialog),FALSE);

	memset(&fl,0,sizeof(fl));
	fl.ofn.lStructSize		= sizeof(fl.ofn);
	fl.ofn.hwndOwner		= GDK_WINDOW_HWND(win);
	fl.ofn.lpstrFile		= fl.szName;

	fl.ofn.lpstrTitle		= title;

	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
	// use the contents of szFile to initialize itself.
	fl.ofn.lpstrFile[0] 	= '\0';

	fl.ofn.nMaxFile 		= sizeof(fl.szName);

	// Monta lista de arquivos.
//	va_list		  args;
//	size_t		  ix		= 0;

	fl.ofn.lpstrFilter = (char *) g_malloc0(4096);

	va_start (args, filter);
	while(filter) {

		filter = gettext(filter);
		size_t sz = strlen(filter)+1;

		if(ix+sz > 4095)
			break;

		debug("%s",filter);

		memcpy(((char *) fl.ofn.lpstrFilter)+ix,filter,sz);
		ix += sz;
		filter = va_arg(args, const char *);
	}
	va_end (args);
	debug("%s",fl.ofn.lpstrFilter);
	fl.ofn.nFilterIndex		= 1;

	fl.ofn.lpstrInitialDir	= NULL;
	fl.ofn.nMaxFileTitle	= 0;

	// Guarda o valor atual
	if(filename)
	{
		strncpy(fl.szName,filename,fl.ofn.nMaxFile);
		g_free(filename);
		filename = NULL;
	}

	fl.action = GTK_FILE_CHOOSER_ACTION_OPEN;

	thd = g_thread_new("GetFileName",(GThreadFunc) select_file, &fl);

	fl.enabled = TRUE;
	while(fl.enabled) {
		g_main_context_iteration(NULL,TRUE);
	}

	g_thread_unref(thd);

	if(fl.ok) {
		filename = g_strdup(fl.szName);
	}

	g_free( ((char *) fl.ofn.lpstrFilter) );
	gtk_widget_set_sensitive(GTK_WIDGET(dialog),TRUE);

#else

	GtkWidget 	* dialog = gtk_file_chooser_dialog_new(	title,
														GTK_WINDOW(gtk_widget_get_toplevel(widget)),
														GTK_FILE_CHOOSER_ACTION_OPEN,
														GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
														GTK_STOCK_EXECUTE,	GTK_RESPONSE_ACCEPT,
														NULL );

	if(filter)
	{
		int f;

		for(f=0;filter[f];f++)
			gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),filter[f]);
	}

	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(dialog),FALSE);

	if(filename)
	{
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),filename);
		g_free(filename);
		filename = NULL;
	}

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
	}

	gtk_widget_destroy(dialog);

#endif

	if(filename && *filename)
	{
		pw3270_set_string(widget,group,key,filename);
	}

	return filename;
}
*/

LIB3270_EXPORT gchar * pw3270_get_datadir(const gchar *first_element, ...)
{
	va_list args;
	gchar	*path;

	va_start(args, first_element);
	path = filename_from_va(first_element,args);
	va_end(args);
	return path;
}

LIB3270_EXPORT void pw3270_remap_from_xml(GtkWidget *widget, const gchar *name)
{
	v3270_set_remap_filename(pw3270_get_terminal_widget(widget),name);
}

LIB3270_EXPORT void pw3270_set_host_charset(GtkWidget *widget, const gchar *name)
{
	H3270 * hSession	= pw3270_get_session(widget);

	if(!(hSession && name))
		return;

	if(!lib3270_set_host_charset(hSession,name))
		return;

	// Charset setup failed, notify user
	GtkWidget	* dialog = gtk_message_dialog_new(	GTK_WINDOW(widget),
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_ERROR,
													GTK_BUTTONS_OK,
													"%s", _( "Can't set host charset" ) );


	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),_( "There's no available settings for charset \"%s\"" ),name);
	gtk_window_set_title(GTK_WINDOW(dialog),_( "Charset error" ));

	g_signal_connect(dialog,"close",G_CALLBACK(gtk_widget_destroy),NULL);
	g_signal_connect(dialog,"response",G_CALLBACK(gtk_widget_destroy),NULL);
	gtk_widget_show_all(dialog);

	/*
	gtk_dialog_run(GTK_DIALOG (dialog));
	gtk_widget_destroy(dialog);
	*/

}

LIB3270_EXPORT void pw3270_set_action_state(GtkAction *action, gboolean on)
{

 	if(!action)
	{
		return;
	}
#if GTK_CHECK_VERSION(3,10,0)
	else if(G_IS_SIMPLE_ACTION(action))
	{
		g_simple_action_set_enabled(G_SIMPLE_ACTION(action),on);
	}
#endif // GTK(3,10)
	else
	{
		gtk_action_set_sensitive(action,on);
	}

 }

 #if ! GLIB_CHECK_VERSION(2,44,0)

	// Reference: https://github.com/ImageMagick/glib/blob/master/glib/glib-autocleanups.h
	LIB3270_EXPORT void pw3270_autoptr_cleanup_generic_gfree(void *p)
	{
		void **pp = (void**)p;
		g_free (*pp);
	}

 #endif // ! GLIB(2,44,0)
