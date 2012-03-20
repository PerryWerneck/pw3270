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
	GObject 		* (*create)(GtkAction *action,struct parser *info,const gchar **names, const gchar **values, GError **error);
	void			  (*end)(GObject *widget,struct parser *info,GError **error);
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
 };

/*--[ Implement ]------------------------------------------------------------------------------------*/

 const gchar * ui_get_attribute(const gchar *key, const gchar **name, const gchar **value)
 {
 	int f;

 	for(f=0;name[f];f++)
 	{
 		if(!g_strcasecmp(key,name[f]))
			return value[f];
 	}

 	return NULL;
 }

 gboolean ui_get_bool_attribute(const gchar *key, const gchar **name, const gchar **value, gboolean def)
 {
	const gchar *val = ui_get_attribute(key,name,value);

	if(!val)
		return def;

 	if(!g_strcasecmp(val,"yes") || atoi(val))
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

//	trace("%s - %s",__FUNCTION__,name);
	return widget;
 }

 static GtkAction * get_action(const gchar *name, struct parser *info, const gchar **names, const gchar **values, GError **error)
 {
 	GtkAction *action;

 	if(!g_strcasecmp(name,"quit"))
	{
		action = g_hash_table_lookup(info->actions,name);
		if(!action)
		{
			action = gtk_action_new(name,NULL,NULL,NULL);
			g_signal_connect(action,"activate",G_CALLBACK(gtk_main_quit), NULL);
		}
	}
 	else
	{
		action = ui_get_action(info->center_widget,name,info->actions,names,values,error);
	}

	if(!action)
		return action;

/*
	const gchar		* target   	= NULL;
	const gchar		* direction	= ui_get_attribute("direction",names,values);
	const gchar		* id		= ui_get_attribute("id",names,values);
	GtkAction		* action;
	gchar			* nm;
	void			  (*connect)(GtkAction *action, GtkWidget *widget, const gchar *name, const gchar *id)		= ui_connect_action;
	GtkAction		* (*create)(const gchar *,const gchar *,const gchar *,const gchar *)						= gtk_action_new;

	if(!g_strcasecmp(name,"toggle"))
	{
		nm 		= g_strconcat(name,id,NULL);
		create	= (GtkAction * (*)(const gchar *,const gchar *,const gchar *,const gchar *)) gtk_toggle_action_new;
		connect = ui_connect_toggle;
	}
	else if(!g_strcasecmp(name,"move"))
	{
		target = ui_get_attribute("target",names,values);

		if(!(target && direction))
		{
			*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s",_("Move action needs target & direction attributes" ));
			return NULL;
		}

		nm = g_strconcat((flags & 0x80) ? "select" : "move",target,direction, NULL);

	}
	else if(!g_strcasecmp(name,"toggleset"))
	{
		nm = g_strconcat("set",id,NULL);
	}
	else if(!g_strcasecmp(name,"togglereset"))
	{
		nm = g_strconcat("reset",id,NULL);
	}
	else if(!g_strcasecmp(name,"pfkey"))
	{
		nm = g_strdup_printf("pf%02d",atoi(id ? id : "0"));
		connect = ui_connect_pfkey;
	}
	else if(!g_strcasecmp(name,"pakey"))
	{
		nm = g_strdup_printf("pa%02d",atoi(id ? id : "0"));
		connect = ui_connect_pakey;
	}
	else
	{
		nm = g_strdup(name);
	}

	action = g_hash_table_lookup(info->actions,nm);

	if(action)
	{
		g_free(nm);
	}
	else
	{
		int ix = -1;

		action = GTK_ACTION(create(nm,NULL,NULL,NULL));
		g_hash_table_insert(info->actions,nm,action);

		if(info->actionname)
		{
			int f;
			for(f=0;info->actionname[f] && ix < 0;f++)
			{
				if(!g_strcasecmp(nm,info->actionname[f]))
					ix = f;
			}
		}

		if(ix >= 0)
			ui_connect_index_action(info->action[ix] = action,info->center_widget,ix,info->action);
		else if(target)
			ui_connect_target_action(action,info->center_widget,target,flags,error);
		else if(g_strcasecmp(name,"quit"))
			connect(action,info->center_widget,name,id);
		else
			g_signal_connect(action,"activate",G_CALLBACK(gtk_main_quit), NULL);

	}
*/
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

	if(info->disabled)
	{
		info->disabled++;
		trace("%s: <%s> disabled=%d",__FUNCTION__,element_name,info->disabled);
		return;
	}

	for(f=0;f<G_N_ELEMENTS(element_builder);f++)
	{
		if(!g_strcasecmp(element_name,element_builder[f].name))
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
		widget = GTK_WIDGET(element_builder[id].create(action,info,names,values,error));

	if(widget)
	{
		g_object_set_data(G_OBJECT(widget),"parent",info->element);
		info->element = G_OBJECT(widget);
		gtk_widget_set_visible(widget,ui_get_bool_attribute("visible",names,values,TRUE));
	}
	else
	{
		info->disabled++;
	}
 }

 static void element_end(GMarkupParseContext *context, const gchar *element_name, struct parser *info, GError **error)
 {
 	GtkWidget *widget = GTK_WIDGET(info->element);
 	int f;

	if(info->disabled)
	{
		info->disabled--;
//		trace("%s: <%s> disabled=%d",__FUNCTION__,element_name,info->disabled);
		return;
	}

	for(f=0;f<G_N_ELEMENTS(element_builder);f++)
	{
		if(!g_strcasecmp(element_name,element_builder[f].name))
		{
			element_builder[f].end(info->element,info,error);
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

	context = g_markup_parse_context_new(&parser,G_MARKUP_TREAT_CDATA_AS_TEXT,info,NULL);

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
	info->disabled	= 0;
	if(info->element)
	{
		g_object_set_data(G_OBJECT(info->element),"parent",NULL);
		info->element = NULL;
	}
	return rc;
 }

