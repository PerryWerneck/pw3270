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

 static void load(const gchar *path, GtkWidget *widget)
 {
	GDir		* dir;
 	const gchar	* name;
 	GError		* err	= NULL;
 	GList		* lst	= NULL;

	trace("Loading plugins from %s",path);

	dir = g_dir_open(path,0,&err);
	if(!dir)
	{
		g_warning("%s",err->message);
		g_error_free(err);
		return;
	}

	name = g_dir_read_name(dir);
	while(name)
	{
		gchar *filename = g_build_filename(path,name,NULL);

		if(g_str_has_suffix(filename,G_MODULE_SUFFIX))
		{
			GModule *handle = g_module_open(filename,G_MODULE_BIND_LOCAL);
			if(handle)
			{
				int (*init)(GtkWidget *);

				if(g_module_symbol(handle, "pw3270_plugin_init", (gpointer) &init))
				{
					if(init(widget))
					{
						// Plugin init fails
						g_module_close(handle);
					}
					else
					{
						// Plugin init is ok, save handle
						lst = g_list_append(lst,handle);
					}
				}
				else
				{
					// No plugin init warn and save it anyway
					g_warning("No pw3270_plugin_init() method in %s",filename);
					lst = g_list_append(lst,handle);
				}
			}
		}
		g_free(filename);
		name = g_dir_read_name(dir);
	}

	g_dir_close(dir);

	if(lst)
	{
		// At least one plugin was loaded, save handle, start it
		GList *l	= g_list_first(lst);
		int f;

		nPlugin = g_list_length(lst);
		g_message("%d plugin(s) loaded",nPlugin);
		hPlugin = g_malloc0(nPlugin * sizeof(GModule *));

		for(f=0;f<nPlugin && l;f++)
		{
			void (*start)(GtkWidget *);

			hPlugin[f] = (GModule *) l->data;

			l = g_list_next(l);

			if(g_module_symbol(hPlugin[f], "pw3270_plugin_start", (gpointer) &start))
				start(widget);
		}

		g_list_free(lst);
	}

 }

 G_GNUC_INTERNAL void init_plugins(GtkWidget *widget)
 {
#ifdef DEBUG
	load("." G_DIR_SEPARATOR_S "plugins", widget);
#endif

 }

 G_GNUC_INTERNAL void deinit_plugins(GtkWidget *widget)
 {
 	int f;

	if(!hPlugin)
		return;

	trace("Unloading %d plugin(s)",nPlugin);

	for(f=0;f<nPlugin;f++)
	{
		void (*stop)(GtkWidget *);

		if(g_module_symbol(hPlugin[f], "pw3270_plugin_stop", (gpointer) &stop))
			stop(widget);
	}

	for(f=0;f<nPlugin;f++)
	{
		void (*deinit)(GtkWidget *);

		if(g_module_symbol(hPlugin[f], "pw3270_plugin_deinit", (gpointer) &deinit))
			deinit(widget);

		g_module_close(hPlugin[f]);
	}

	g_free(hPlugin);
	hPlugin = NULL;
	nPlugin	= 0;
 }
