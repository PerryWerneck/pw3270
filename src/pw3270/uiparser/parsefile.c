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
 * Este programa está nomeado como parser.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

 #include <stdlib.h>
 #include "private.h"

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 static const struct _element_builders
 {
 	const gchar		* name;
 	enum ui_element	  id;
	GObject 		* (*create)(GMarkupParseContext *context,GtkAction *action,struct parser *info,const gchar **names, const gchar **values, GError **error);
	void			  (*end)(GMarkupParseContext *context,GObject *widget,struct parser *info,GError **error);
 } element_builder[] =
 {
 	{ "menubar",		UI_ELEMENT_MENUBAR,		ui_create_menubar,		ui_end_menubar		},
 	{ "menu",			UI_ELEMENT_MENU,		ui_create_menu,			ui_end_menu			},
 	{ "menuitem",		UI_ELEMENT_MENUITEM,	ui_create_menuitem,		ui_end_menuitem		},
 	{ "separator",		UI_ELEMENT_SEPARATOR,	ui_create_separator,	ui_end_separator	},
 	{ "toolbar",		UI_ELEMENT_TOOLBAR,		ui_create_toolbar,		ui_end_toolbar		},
 	{ "toolitem",		UI_ELEMENT_TOOLITEM,	ui_create_toolitem,		ui_end_toolitem		},
 	{ "accelerator",	UI_ELEMENT_ACCELERATOR,	ui_create_accelerator,	ui_end_accelerator	},
 	{ "popup",			UI_ELEMENT_POPUP,		ui_create_popup,		ui_end_popup		},
 	{ "script",			UI_ELEMENT_SCRIPT,		ui_create_script,		ui_end_script		},
 	{ "scroll",			UI_ELEMENT_SCROLL,		ui_create_scroll,		ui_end_scroll		},
 	{ "keypad",			UI_ELEMENT_KEYPAD,		ui_create_keypad,		ui_end_keypad		},
 };

/*--[ Implement ]------------------------------------------------------------------------------------*/

 const gchar * ui_get_attribute(const gchar *key, const gchar **name, const gchar **value)
 {
 	int f;

 	for(f=0;name[f];f++)
 	{
 		if(!g_ascii_strcasecmp(key,name[f]))
			return value[f];
 	}

 	return NULL;
 }

 gboolean ui_get_bool_attribute(const gchar *key, const gchar **name, const gchar **value, gboolean def)
 {
	const gchar *val = ui_get_attribute(key,name,value);

	if(!val)
		return def;

 	if(!g_ascii_strcasecmp(val,"yes") || atoi(val))
		return TRUE;

	return FALSE;

 }


 GObject * ui_get_element(struct parser *info, GtkAction *action, enum ui_element id, const gchar **names, const gchar **values, GError **error)
 {
 	GObject		* obj;
	const gchar	* name = NULL;
	gchar		* key;

	if(action)
		name = gtk_action_get_name(action);

	if(!name)
		name = ui_get_attribute("name",names,values);

	if(!name)
	{
		*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s", _( "Can't parse unnamed element"));
		return NULL;
	}

//	trace("%s - %s",__FUNCTION__,name);
	key = g_utf8_strdown(name,-1);
	obj = g_hash_table_lookup(info->element_list[id],key);
	g_free(key);

	return obj;

 }

 GObject * ui_insert_element(struct parser *info, GtkAction *action, enum ui_element id, const gchar **names, const gchar **values, GObject *widget, GError **error)
 {
	const gchar * name = NULL;
	gchar		* key;

	if(action)
		name = gtk_action_get_name(action);

	if(!name)
		name = ui_get_attribute("name",names,values);

	if(!name)
	{
		*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s", _( "Can't parse unnamed element"));
		g_object_unref(widget);
		return NULL;
	}

	key = g_utf8_strdown(name,-1);
	gtk_widget_set_name(GTK_WIDGET(widget),key);
	g_object_ref(G_OBJECT(widget));
	g_hash_table_insert(info->element_list[id],key,widget);

	return widget;
 }

 static GtkAction * get_action(const gchar *name, struct parser *info, const gchar **names, const gchar **values, GError **error)
 {
 	GtkAction *action;

 	if(!g_ascii_strcasecmp(name,"quit"))
	{
		action = g_hash_table_lookup(info->actions,name);
		if(!action)
		{
			action = gtk_action_new(name,NULL,NULL,GTK_STOCK_QUIT);
			g_signal_connect(action,"activate",G_CALLBACK(gtk_main_quit), NULL);
		}
	}
 	else
	{
		action = ui_get_action(info->center_widget,name,info->actions,names,values,error);
	}

	if(action)
		ui_action_set_options(action,info,names,values,error);

	return action;
 }

 static void element_start(GMarkupParseContext *context, const gchar *element_name, const gchar **names,const gchar **values, struct parser *info, GError **error)
 {
 	enum ui_element   id		= (enum ui_element) -1;
 	GtkAction		* action	= NULL;
 	const gchar		* name		= NULL;
 	GtkWidget		* widget	= NULL;
 	int				  f;

	name = ui_get_attribute("platform",names,values);
	if(name)
	{
#if defined(WIN32)
		static const gchar *platname = "windows";
#elif defined(linux)
		static const gchar *platname = "linux";
#elif defined( __APPLE__ )
		static const gchar *platname = "apple";
#else
		static const gchar *platname = "none";
#endif
		gchar **plat = g_strsplit(name,",",-1);
		info->disabled = 1;

		for(f=0;info->disabled && plat[f];f++)
		{
			if(!g_ascii_strcasecmp(plat[f],platname))
				info->disabled = 0;
		}

		g_strfreev(plat);

		if(info->disabled)
			return;
	}

	for(f=0;f<G_N_ELEMENTS(element_builder);f++)
	{
		if(!g_ascii_strcasecmp(element_name,element_builder[f].name))
		{
			id = f;
			break;
		}
	}

	if(id == (enum ui_element) -1)
		return;

	name = ui_get_attribute("action",names,values);
	if(name)
	{
		// Setup action
		action = get_action(name,info,names,values,error);
	}
	else if(ui_get_attribute("group",names,values))
	{
		name = ui_get_attribute("name",names,values);

		if(!name)
		{
			// Invalid unnamed element
			*error = g_error_new(ERROR_DOMAIN,EINVAL,_( "Can't accept unnamed %s"),element_name);
			return;
		}
		action = get_action(name,info,names,values,error);
	}

	if(*error)
		return;

	if(element_builder[id].id < UI_ELEMENT_COUNT)
	{
		// Active element
		widget = GTK_WIDGET(ui_get_element(info,action,element_builder[id].id,names,values,error));
		if(*error)
			return;
	}

	if(!widget)
		widget = GTK_WIDGET(element_builder[id].create(context,action,info,names,values,error));

	if(widget)
	{
		g_object_set_data(G_OBJECT(widget),"parent",info->element);
		info->element = G_OBJECT(widget);
		info->action  = action;

#if GTK_CHECK_VERSION(2,18,0)
		gtk_widget_set_visible(widget,ui_get_bool_attribute("visible",names,values,TRUE));
#else
		if(ui_get_bool_attribute("visible",names,values,TRUE))
			gtk_widget_show(widget);
		else
			gtk_widget_hide(widget);
#endif // GTK(2,18,0)

	}
	else
	{
		info->action = NULL;
	}
 }

 static void element_end(GMarkupParseContext *context, const gchar *element_name, struct parser *info, GError **error)
 {
 	GtkWidget *widget = GTK_WIDGET(info->element);
 	int f;

	if(info->disabled)
	{
		info->disabled = 0;
		return;
	}

	for(f=0;f<G_N_ELEMENTS(element_builder);f++)
	{
		if(!g_ascii_strcasecmp(element_name,element_builder[f].name))
		{
			element_builder[f].end(context,info->element,info,error);
			break;
		}
	}

 	if(!widget)
		return;

	info->element = G_OBJECT(g_object_get_data(G_OBJECT(widget),"parent"));
	g_object_set_data(G_OBJECT(widget),"parent",NULL);
 }

 int ui_parse_file(struct parser *info, const gchar *filename)
 {
	static const GMarkupParser parser =
	{
		(void (*)(GMarkupParseContext *, const gchar *, const gchar **, const gchar **, gpointer, GError **))
				element_start,
		(void (*)(GMarkupParseContext *, const gchar *, gpointer, GError **))
				element_end,
//		(void (*)(GMarkupParseContext *, const gchar *, gsize, gpointer, GError **))
		NULL,

//		(void (*)(GMarkupParseContext *context, const gchar *passthrough_text, gsize text_len,  gpointer user_data,GError **error))
		NULL,

//		(void (*)(GMarkupParseContext *, GError *, gpointer))
		NULL

	};

	int rc = 0;
 	GMarkupParseContext * context;
 	GError				* error		= NULL;
 	gchar				* text 		= NULL;

	// Load file
	if(!g_file_get_contents(filename,&text,NULL,&error))
	{
		GtkWidget	* dialog;
		gchar		* name = g_path_get_basename(filename);

		dialog = gtk_message_dialog_new(	NULL,
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_WARNING,
											GTK_BUTTONS_OK,
											_(  "Can't load %s" ), name);

		g_free(name);

		gtk_window_set_title(GTK_WINDOW(dialog), _( "Can't parse UI" ) );

		if(error && error->message)
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", error->message);

		g_error_free(error);

		gtk_dialog_run(GTK_DIALOG (dialog));
		gtk_widget_destroy(dialog);

		return -1;
	}

	context = g_markup_parse_context_new(&parser,G_MARKUP_TREAT_CDATA_AS_TEXT|G_MARKUP_PREFIX_ERROR_POSITION,info,NULL);

	if(!g_markup_parse_context_parse(context,text,strlen(text),&error))
	{
		GtkWidget	* dialog;
		gchar		* name = g_path_get_basename(filename);

		dialog = gtk_message_dialog_new(	NULL,
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_WARNING,
											GTK_BUTTONS_OK,
											_(  "Parse error in %s" ), name);

		g_free(name);

		gtk_window_set_title(GTK_WINDOW(dialog), _( "Can't parse UI" ) );

		if(error && error->message)
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", error->message);

		g_error_free(error);

		gtk_dialog_run(GTK_DIALOG (dialog));
		gtk_widget_destroy(dialog);

		rc = -1;
	}

	g_markup_parse_context_free(context);

	g_free(text);

//	trace("%s exits with rc=%d",__FUNCTION__,rc);
//	info->disabled	= 0;
	if(info->element)
	{
		g_object_set_data(G_OBJECT(info->element),"parent",NULL);
		info->element = NULL;
	}
	return rc;
 }

