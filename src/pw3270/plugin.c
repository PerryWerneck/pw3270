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
 * Este programa está nomeado como plugin.c e possui - linhas de código.
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
#include "pw3270/plugin.h"

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 static guint 	  nPlugin	= 0;	/**< Number of active plugins */
 static GModule	**hPlugin	= NULL;	/**< Plugin handles */

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void load(const gchar *path)
 {
	GDir		* dir;
 	const gchar	* name;
 	GError		* err	= NULL;
 	GList		* lst	= NULL;
#ifdef WIN32
	UINT 		  errorMode;
#endif // WIN32


	trace("Loading plugins from %s",path);

	if(!g_file_test(path,G_FILE_TEST_IS_DIR))
		return;

	dir = g_dir_open(path,0,&err);
	if(!dir)
	{
		g_warning("%s",err->message);
		g_error_free(err);
		return;
	}

#ifdef WIN32
	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms680621(v=vs.85).aspx
	errorMode = SetErrorMode(1);
#endif // WIN32

	name = g_dir_read_name(dir);
	while(name)
	{
		gchar *filename = g_build_filename(path,name,NULL);

//		trace("%s is %s",filename,g_str_has_suffix(filename,G_MODULE_SUFFIX) ? "valid" : "invalid");

		if(g_str_has_suffix(filename,G_MODULE_SUFFIX))
		{
			GModule *handle = g_module_open(filename,G_MODULE_BIND_LOCAL);

			if(!handle)
			{
				g_message("Error \"%s\" loading %s",g_module_error(),filename);
/*
			    gchar       * text  = g_strdup(g_module_error());
			    gchar       * name  = g_path_get_basename(filename);
                GtkWidget   * popup = gtk_message_dialog_new_with_markup(
                                                    NULL,
                                                    GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,
                                                    _( "Can't load plugin %s" ),name);

                gtk_window_set_title(GTK_WINDOW(popup),_("Can't load plugin"));

				g_message("Error \"%s\" loading %s",text,filename);

                gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(popup),_( "<b>%s</b>\n" ),text);

                gtk_dialog_run(GTK_DIALOG(popup));
                gtk_widget_destroy(popup);

				g_free(text);
				g_free(name);
*/
			}
			else
			{
				int (*init)();

				if(g_module_symbol(handle, "pw3270_plugin_init", (gpointer) &init))
				{
					if(init())
					{
						// Plugin init fails
						g_module_close(handle);
					}
					else
					{
						// Plugin init is ok, save handle
						lst = g_list_append(lst,handle);
						trace("%s=%p",filename,handle);
					}
				}
				else
				{
					// No plugin init warn and save it anyway
					lst = g_list_append(lst,handle);
				}
			}
		}
		g_free(filename);
		name = g_dir_read_name(dir);
	}

#ifdef WIN32
	SetErrorMode(errorMode);
#endif // WIN32

	g_dir_close(dir);

	if(lst)
	{
		// At least one plugin was loaded, save handle
		GList *l	= g_list_first(lst);
		int f;

		nPlugin = g_list_length(lst);
		g_message("%d plugin%s loaded",nPlugin,nPlugin > 1 ? "s" : "");
		hPlugin = g_malloc0(nPlugin * sizeof(GModule *));

		for(f=0;f<nPlugin && l;f++)
		{
			hPlugin[f] = (GModule *) l->data;
			l = g_list_next(l);
		}
		g_list_free(lst);
	}

 }

 LIB3270_EXPORT void pw3270_init_plugins(void)
 {
#if defined( DEBUG )

	gchar * dir  = g_get_current_dir();
	gchar * path = g_build_filename(dir,"plugins",NULL);

	trace("%s testing [%s]",__FUNCTION__,path);

	if(!g_file_test(path,G_FILE_TEST_IS_DIR))
	{
		g_free(path);

		path = g_build_filename(dir,".bin","Debug","plugins",NULL);
		trace("%s testing [%s]",__FUNCTION__,path);

		if(!g_file_test(path,G_FILE_TEST_IS_DIR))
		{
			g_free(path);
			path = pw3270_build_filename(NULL,"plugins",NULL);
			trace("%s using [%s]",__FUNCTION__,path);
		}
	}

	load(path);

	g_free(path);
	g_free(dir);

#elif defined( WIN32 )

	gchar * path = pw3270_build_filename(NULL,"plugins",NULL);
	load(path);
	g_free(path);

#else

	const gchar * appname[]	= { g_get_application_name(), PACKAGE_NAME };
	int f;

	for(f=0;f<G_N_ELEMENTS(appname);f++)
	{
		gchar *path = g_strdup_printf("%s" G_DIR_SEPARATOR_S "%s-plugins",LIBDIR,appname[f]);

		if(g_file_test(path,G_FILE_TEST_IS_DIR))
		{
			load(path);
			g_free(path);
			return;
		}

		g_free(path);
	}

#endif
 }

 LIB3270_EXPORT void pw3270_start_plugins(GtkWidget *widget)
 {
	int f;

	for(f=0;f<nPlugin;f++)
	{
		int (*start)(GtkWidget *);

		if(g_module_symbol(hPlugin[f], "pw3270_plugin_start", (gpointer) &start))
			start(widget);
	}

 }

 LIB3270_EXPORT void pw3270_stop_plugins(GtkWidget *widget)
 {
	int f;

	for(f=0;f<nPlugin;f++)
	{
		int (*stop)(GtkWidget *);

		if(g_module_symbol(hPlugin[f], "pw3270_plugin_stop", (gpointer) &stop))
			stop(widget);
	}

 }

 LIB3270_EXPORT void pw3270_deinit_plugins(void)
 {
 	int f;

	if(!hPlugin)
		return;

	trace("Unloading %d plugin(s)",nPlugin);
	for(f=0;f<nPlugin;f++)
	{
		void (*deinit)(void);

		if(g_module_symbol(hPlugin[f], "pw3270_plugin_deinit", (gpointer) &deinit))
			deinit();

		g_module_close(hPlugin[f]);
	}

	g_free(hPlugin);
	hPlugin = NULL;
	nPlugin	= 0;
 }

 LIB3270_EXPORT int pw3270_setup_plugin_action(GtkAction *action, GtkWidget *widget, const gchar *name)
 {
 	int		  f;
 	gchar	* fname;

	trace("%s hPlugin=%p",__FUNCTION__,hPlugin);
	if(!hPlugin)
		return ENOENT;

	// Search for plugin setup calls
	fname = g_strdup_printf("pw3270_setup_action_%s",name);
	trace("Searching for \"%s\"",fname);

	for(f=0;f<nPlugin;f++)
	{
		int (*setup)(GtkAction *action, GtkWidget *widget);

		if(g_module_symbol(hPlugin[f], fname, (gpointer) &setup))
		{
			trace("%s=%p",fname,setup);
			g_free(fname);
			return setup(action,widget);
		}
	}
	g_free(fname);

	// Search for activation callbacks
	fname = g_strdup_printf("pw3270_action_%s_activated",name);
	trace("Searching for \"%s\"",fname);

	for(f=0;f<nPlugin;f++)
	{
		void (*call)(GtkAction *action, GtkWidget *widget);

		if(g_module_symbol(hPlugin[f], fname, (gpointer) &call))
		{
			trace("%s=%p",fname,call);
			g_signal_connect(action,"activate",G_CALLBACK(call),widget);
			g_free(fname);
			return 0;
		}
	}
	g_free(fname);

	return ENOENT;

 }
