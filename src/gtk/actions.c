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
 * Este programa está nomeado como actions.c e possui - linhas de código.
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
 #include "uiparser/parser.h"
 #include "v3270/v3270.h"
 #include <lib3270/actions.h>
 #include <lib3270/selection.h>
 #include <stdlib.h>

 #define ERROR_DOMAIN g_quark_from_static_string(PACKAGE_NAME)
 #define TOGGLE_GDKDEBUG LIB3270_TOGGLE_COUNT+1

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void lib3270_action(GtkAction *action, GtkWidget *widget)
{
	int	(*call)(H3270 *h) = (int (*)(H3270 *h)) g_object_get_data(G_OBJECT(action),"lib3270_call");
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);
	call(v3270_get_session(widget));
}

static void connect_action(GtkAction *action, GtkWidget *widget)
{
	gchar *host = (gchar *) g_object_get_data(G_OBJECT(action),"host");

	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);

	if(host)
	{
		v3270_connect(widget,host);
		return;
	}

	host = get_string_from_config("host","uri","");
	if(*host)
	{
		v3270_connect(widget,host);
		g_free(host);
		return;
	}
	g_free(host);

	hostname_action(action,widget);
}

static void nop_action(GtkAction *action, GtkWidget *widget)
{
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);
}

static void disconnect_action(GtkAction *action, GtkWidget *widget)
{
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);
	v3270_disconnect(widget);
}

static void activate_action(GtkAction *action, GtkWidget *widget)
{
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);
	gtk_widget_activate(widget);
}

static void reload_action(GtkAction *action, GtkWidget *widget)
{
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);
	v3270_reload(widget);
}

static void copy_action(GtkAction *action, GtkWidget *widget)
{
	V3270_SELECT_FORMAT	  mode = V3270_SELECT_TEXT;
	const gchar 		* str = (const gchar *) g_object_get_data(G_OBJECT(action),"format");

	if(str)
	{
		static const struct _format
		{
			V3270_SELECT_FORMAT	  mode;
			const gchar			* name;
		} format[] =
		{
			{ V3270_SELECT_TEXT,	"text"		},
			{ V3270_SELECT_TABLE,	"table"		},
		};

		int f;

		for(f=0;f<G_N_ELEMENTS(format);f++)
		{
			if(!g_strcasecmp(format[f].name,str))
			{
				mode = format[f].mode;
				break;
			}
		}
	}

	trace("Action %s activated on widget %p mode=%d",gtk_action_get_name(action),widget,(int) mode);
	v3270_copy(widget,mode);
}

static void append_action(GtkAction *action, GtkWidget *widget)
{
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);
	v3270_copy_append(widget);
}

static void paste_clipboard_action(GtkAction *action, GtkWidget *widget)
{
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);
	v3270_paste(widget);
}

static void paste_next_action(GtkAction *action, GtkWidget *widget)
{
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);
	lib3270_pastenext(v3270_get_session(widget));
}

static void connect_standard_action(GtkAction *action, GtkWidget *widget, const gchar *name)
{
	#undef DECLARE_LIB3270_ACTION
	#undef DECLARE_LIB3270_CLEAR_SELECTION_ACTION
	#undef DECLARE_LIB3270_KEY_ACTION
	#undef DECLARE_LIB3270_CURSOR_ACTION
	#undef DECLARE_LIB3270_FKEY_ACTION

	#define DECLARE_LIB3270_ACTION( name )						{ #name, lib3270_ ## name			},
	#define DECLARE_LIB3270_CLEAR_SELECTION_ACTION( name )		{ #name, lib3270_ ## name			},
	#define DECLARE_LIB3270_KEY_ACTION( name )					{ #name, lib3270_ ## name			},
	#define DECLARE_LIB3270_CURSOR_ACTION( name )				{ #name, lib3270_cursor_ ## name	},
	#define DECLARE_LIB3270_FKEY_ACTION( name )					// name

	static const struct _lib3270_action
	{
		const gchar * name;
		int			  (*call)(H3270 *h);
	} lib3270_entry[] =
	{
		#include <lib3270/action_table.h>
	};

	static const struct _gtk_action
	{
		const gchar * name;
		void		  (*call)(GtkAction *action, GtkWidget *widget);
	}
	gtk_action[] =
	{
		{ "activate", 	activate_action		},
		{ "reload",		reload_action		},
		{ "connect", 	connect_action		},
		{ "disconnect", disconnect_action	},
		{ "hostname",	hostname_action		},
		{ "editcolors",	editcolors_action	},
		{ "about",		about_dialog_action	},
	};

	int f;

	// Search for lib3270 predefined actions
	for(f=0;f<G_N_ELEMENTS(lib3270_entry);f++)
	{
		if(!g_strcasecmp(name,lib3270_entry[f].name))
		{
			g_object_set_data(G_OBJECT(action),"lib3270_call",lib3270_entry[f].call);
			g_signal_connect(action,"activate",G_CALLBACK(lib3270_action),widget);
			return;
		}
	}

	// Search for application actions
	for(f=0;f<G_N_ELEMENTS(gtk_action);f++)
	{
		if(!g_strcasecmp(name,gtk_action[f].name))
		{
			g_signal_connect(action,"activate",G_CALLBACK(gtk_action[f].call),widget);
			return;
		}
	}

	if(!g_strcasecmp(name,"screensizes"))
		return;

	// Not-found, disable action
	gtk_action_set_sensitive(action,FALSE);
}


static void lib3270_toggle_action(GtkToggleAction *action,GtkWidget *widget)
{
	LIB3270_TOGGLE toggle = (LIB3270_TOGGLE) g_object_get_data(G_OBJECT(action),"toggle_id");

	trace("Action %s toggled on widget %p (id=%d)",gtk_action_get_name(GTK_ACTION(action)),widget,(int) toggle);

	if(toggle == TOGGLE_GDKDEBUG)
		gdk_window_set_debug_updates(gtk_toggle_action_get_active(action));
	else if(toggle < LIB3270_TOGGLE_COUNT)
		lib3270_set_toggle(v3270_get_session(widget),toggle,gtk_toggle_action_get_active(action));
}

static void selection_move_action(GtkAction *action, GtkWidget *widget)
{
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);
	lib3270_move_selection(v3270_get_session(widget),(LIB3270_DIRECTION) g_object_get_data(G_OBJECT(action),"direction"));
}

static void cursor_move_action(GtkAction *action, GtkWidget *widget)
{
	int flags = (int) g_object_get_data(G_OBJECT(action),"move_flags");
	trace("Action %s activated on widget %p flags=%04x",gtk_action_get_name(action),widget,(unsigned short) g_object_get_data(G_OBJECT(action),"move_flags"));
	lib3270_move_cursor(v3270_get_session(widget),(LIB3270_DIRECTION) (flags & 0x03), (flags & 0x80) );
}

static void connect_move_action(GtkAction *action, GtkWidget *widget, const gchar *target, unsigned short flags, GError **error)
{
	if(!target)
	{
		gtk_action_set_sensitive(action,FALSE);
		return;
	}

	if(!g_strcasecmp(target,"selection"))
	{
		g_object_set_data(G_OBJECT(action),"direction",(gpointer) (flags & 3));
		g_signal_connect(action,"activate",G_CALLBACK(selection_move_action),widget);
	}
	else if(!g_strcasecmp(target,"cursor"))
	{
		g_object_set_data(G_OBJECT(action),"move_flags",(gpointer) ((int) flags));
		g_signal_connect(action,"activate",G_CALLBACK(cursor_move_action),widget);
	}
	else
	{
		*error = g_error_new(	ERROR_DOMAIN,
								ENOENT,
								_( "Unexpected target \"%s\""),
								target);
	}

}

static void action_pfkey(GtkAction *action, GtkWidget *widget)
{
	trace("Action %s activated on widget %p key=%p",gtk_action_get_name(action),widget,g_object_get_data(G_OBJECT(action),"pfkey"));
	lib3270_pfkey(v3270_get_session(widget),(int) g_object_get_data(G_OBJECT(action),"pfkey"));
}

static void action_pakey(GtkAction *action, GtkWidget *widget)
{
	trace("Action %s activated on widget %p key=%p",gtk_action_get_name(action),widget,g_object_get_data(G_OBJECT(action),"pakey"));
	lib3270_pakey(v3270_get_session(widget),(int) g_object_get_data(G_OBJECT(action),"pakey"));
}

static void action_set_toggle(GtkAction *action, GtkWidget *widget)
{
	LIB3270_TOGGLE id = (LIB3270_TOGGLE) g_object_get_data(G_OBJECT(action),"toggle_id");
	trace("Action %s activated on widget %p toggle=%d",gtk_action_get_name(action),widget,id);
	lib3270_set_toggle(v3270_get_session(widget),id,1);
}

static void action_reset_toggle(GtkAction *action, GtkWidget *widget)
{
	LIB3270_TOGGLE id = (LIB3270_TOGGLE) g_object_get_data(G_OBJECT(action),"toggle_id");
	trace("Action %s activated on widget %p toggle=%d",gtk_action_get_name(action),widget,id);
	lib3270_set_toggle(v3270_get_session(widget),id,0);
}

static void action_select_all(GtkAction *action, GtkWidget *widget)
{
	lib3270_selectall(v3270_get_session(widget));
}

static void action_select_field(GtkAction *action, GtkWidget *widget)
{
	lib3270_selectfield(v3270_get_session(widget));
}

static void action_select_none(GtkAction *action, GtkWidget *widget)
{
	v3270_unselect(widget);
}

static void action_select_last(GtkAction *action, GtkWidget *widget)
{
	lib3270_reselect(v3270_get_session(widget));
}

static int id_from_array(const gchar *key, const gchar **array, GError **error)
{
	int f;

	if(!key)
		return -1;

	for(f = 0;array[f];f++)
	{
		if(!g_strcasecmp(key,array[f]))
			return f;
	}

	*error = g_error_new(ERROR_DOMAIN,EINVAL, _("Unexpected or invalid attribute value \"%s\""), key );

	return -1;
}

static int get_attribute_id(const gchar *name, const gchar *key, gchar **nm, const gchar **src, const gchar **names, const gchar **values, GError **error)
{
 	const gchar *attr = ui_get_attribute(key,names,values);
 	int			 id;

 	if(!attr)
	{
		*error = g_error_new(ERROR_DOMAIN,EINVAL,_("Attribute \"%s\" is invalid or undefined" ), key);
		return -1;
	}

	id = id_from_array(attr,src,error);
	if(id >= 0)
	{
		if(*nm)
			g_free(*nm);
		*nm = g_strconcat(name,attr,NULL);
		return id;
	}

	if(!*error)
		*error = g_error_new(ERROR_DOMAIN,EINVAL,_("Unexpected or invalid %s attribute: \"%s\"" ), key, attr);

	return -1;
}

static int setup_block_action(const gchar *name, const gchar *attr, GError **error)
{
	int id = -1;

	if(!attr)
	{
		*error = g_error_new(ERROR_DOMAIN,EINVAL,_("%s action needs src attribute" ), name);
	}
	else
	{
		static const gchar * src[] = { "all", "selected", "copy", NULL };

		id = id_from_array(attr,src,error);
		if(id < 0)
		{
			*error = g_error_new(ERROR_DOMAIN,EINVAL,_("Unexpected or invalid src attribute: \"%s\"" ), attr);
		}
	}
	return id;
}

GtkAction * ui_get_action(GtkWidget *widget, const gchar *name, GHashTable *hash, const gchar **names, const gchar **values, GError **error)
{
	static const gchar *actionname[ACTION_COUNT] = {	"pastenext",
														"reselect",
														"setfullscreen",
														"resetfullscreen"
 													};
 	GtkAction		* action 		= NULL;
	GtkAction 		**toggle_action	= (GtkAction **) g_object_get_data(G_OBJECT(widget),"toggle_actions");
	const gchar		* direction		= ui_get_attribute("direction",names,values);
	unsigned short	  flags			= 0;
	const GCallback * callback		= NULL;
	const gchar		* attr;
	int				  id			= 0;
	gchar			* nm			= NULL;
	int				  f;

	enum _action_type
	{
		ACTION_TYPE_DEFAULT,
		ACTION_TYPE_TOGGLE,
		ACTION_TYPE_MOVE,
		ACTION_TYPE_PFKEY,
		ACTION_TYPE_PAKEY,
		ACTION_TYPE_SET,
		ACTION_TYPE_RESET,
		ACTION_TYPE_TABLE,

	} action_type = ACTION_TYPE_DEFAULT;

	if(direction)
	{
		static const gchar *dirname[] = { "up", "down", "left", "right" };
		int f;

		for(f=0;f<G_N_ELEMENTS(dirname);f++)
		{
			if(!g_strcasecmp(direction,dirname[f]))
			{
				flags |= f;
				break;
			}
		}
	}

	if(ui_get_bool_attribute("selecting",names,values,FALSE))
		flags |= 0x80;

	// Build action name & type
	if(!g_strcasecmp(name,"toggle"))
	{
		action_type	= ACTION_TYPE_TOGGLE;
		attr		= ui_get_attribute("id",names,values);
		if(!attr)
			attr = ui_get_attribute("toggle",names,values);

		if(g_strcasecmp(attr,"gdkdebug"))
		{
			id = lib3270_get_toggle_id(attr);
			if(id < 0)
			{
				*error = g_error_new(ERROR_DOMAIN,EINVAL,_("%s action needs a valid toggle name" ), name);
				return NULL;
			}
		}
		else
		{
			id = TOGGLE_GDKDEBUG;
		}
		nm 	= g_strconcat(name,attr,NULL);
	}
	else if(!g_strcasecmp(name,"move"))
	{
		action_type	= ACTION_TYPE_MOVE;
		attr		= ui_get_attribute("target",names,values);

		if(!(attr && direction))
		{
			*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s",_("Move action needs target & direction attributes" ));
			return NULL;
		}

		nm = g_strconcat((flags & 0x80) ? "select" : "move",attr,direction, NULL);

	}
	else if(!g_strcasecmp(name,"paste"))
	{
		static const GCallback cbk[] = {	G_CALLBACK(paste_clipboard_action),
											G_CALLBACK(paste_next_action),
											G_CALLBACK(paste_file_action)
										};
		static const gchar 		* src[] = { "clipboard", "next", "file", NULL };

		callback	= cbk;
		action_type	= ACTION_TYPE_TABLE;

		id = get_attribute_id(name,"src",&nm,src,names,values,error);
		if(id < 0)
			return NULL;

	}
	else if(!g_strcasecmp(name,"copy"))
	{
		static const GCallback cbk[] =	{	G_CALLBACK(copy_action),
											G_CALLBACK(append_action)
											};
		callback	= cbk;
		action_type	= ACTION_TYPE_TABLE;
		id 			= ui_get_bool_attribute("append",names,values,FALSE) ? 1 : 0;
		nm 			= g_strconcat(id == 0 ? "copy" : "append",ui_get_attribute("format",names,values),NULL);
	}
	else if(!g_strcasecmp(name,"select"))
	{
		static const gchar * src[] = 	{	"all", "field", "none", "last", NULL };

		static const GCallback cbk[] =	{	G_CALLBACK(action_select_all),
											G_CALLBACK(action_select_field),
											G_CALLBACK(action_select_none),
											G_CALLBACK(action_select_last)
										};
		callback	= cbk;
		action_type	= ACTION_TYPE_TABLE;
		id = get_attribute_id(name,"target",&nm,src,names,values,error);
		if(id < 0)
			return NULL;
	}
	else if(!g_strcasecmp(name,"save"))
	{
		static const GCallback cbk[] = {	G_CALLBACK(save_all_action),
											G_CALLBACK(save_selected_action),
											G_CALLBACK(save_copy_action)
											};

		callback	= cbk;
		action_type	= ACTION_TYPE_TABLE;
		attr 		= ui_get_attribute("src",names,values);
		id			= setup_block_action(name,attr,error);
		if(*error)
			return NULL;
		nm = g_strconcat(name,attr,NULL);
	}
	else if(!g_strcasecmp(name,"print"))
	{
		static const GCallback cbk[] = {	G_CALLBACK(print_all_action),
											G_CALLBACK(print_selected_action),
											G_CALLBACK(print_copy_action)
											};

		callback	= cbk;
		action_type	= ACTION_TYPE_TABLE;
		attr 		= ui_get_attribute("src",names,values);
		id			= setup_block_action(name,attr,error);
		if(*error)
			return NULL;
		nm = g_strconcat(name,attr,NULL);
	}
	else if(!g_strcasecmp(name,"set"))
	{
		action_type	= ACTION_TYPE_SET;
		attr		= ui_get_attribute("toggle",names,values);
		id			= lib3270_get_toggle_id(attr);
		if(id < 0)
		{
			*error = g_error_new(ERROR_DOMAIN,EINVAL,_("%s action needs a valid toggle name" ), name);
			return NULL;
		}
		nm 	 = g_strconcat("set",attr,NULL);
	}
	else if(!g_strcasecmp(name,"reset"))
	{
		action_type	= ACTION_TYPE_RESET;
		attr		= ui_get_attribute("toggle",names,values);
		id			= lib3270_get_toggle_id(attr);
		if(id < 0)
		{
			*error = g_error_new(ERROR_DOMAIN,EINVAL,_("%s action needs a valid toggle name" ), name);
			return NULL;
		}
		nm = g_strconcat("reset",attr,NULL);
	}
	else if(!g_strcasecmp(name,"pfkey"))
	{
		action_type = ACTION_TYPE_PFKEY;
		attr 		= ui_get_attribute("id",names,values);
		if(!attr)
		{
			*error = g_error_new(ERROR_DOMAIN,EINVAL,_("%s action needs a valid id attribute" ),name);
			return NULL;
		}
		id   = atoi(attr);
		nm   = g_strdup_printf("pf%02d",id);
	}
	else if(!g_strcasecmp(name,"pakey"))
	{
		action_type = ACTION_TYPE_PAKEY;
		attr 		= ui_get_attribute("id",names,values);
		if(!attr)
		{
			*error = g_error_new(ERROR_DOMAIN,EINVAL,_("%s action needs a valid id attribute" ),name);
			return NULL;
		}
		id   = atoi(attr);
		nm   = g_strdup_printf("pa%02d",id);
	}
	else
	{
		attr = ui_get_attribute("name",names,values);
		nm = g_strdup(attr ? attr : name);
	}

	// Check if action is available
	action = g_hash_table_lookup(hash,nm);
	if(action)
	{
		g_free(nm);
		return action;
	}

	// Not available, create a new one
	switch(action_type)
	{
	case ACTION_TYPE_DEFAULT:
		action = gtk_action_new(nm,NULL,NULL,NULL);
		connect_standard_action(action,widget,name);
		break;

	case ACTION_TYPE_TOGGLE:
		action = GTK_ACTION(gtk_toggle_action_new(nm,NULL,NULL,NULL));
		if(id < LIB3270_TOGGLE_COUNT)
			toggle_action[id] = action;
		g_object_set_data(G_OBJECT(action),"toggle_id",(gpointer) id);
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),(lib3270_get_toggle(v3270_get_session(widget),id) != 0));
		g_signal_connect(action,"toggled",G_CALLBACK(lib3270_toggle_action),widget);
		break;

	case ACTION_TYPE_MOVE:
		action = gtk_action_new(nm,NULL,NULL,NULL);
		connect_move_action(action,widget,attr,flags,error);
		break;

	case ACTION_TYPE_PFKEY:
		action = gtk_action_new(nm,NULL,NULL,NULL);
		g_object_set_data(G_OBJECT(action),"pfkey",(gpointer) id);
		g_signal_connect(action,"activate",G_CALLBACK(action_pfkey),widget);
		break;

	case ACTION_TYPE_PAKEY:
		action = gtk_action_new(nm,NULL,NULL,NULL);
		g_object_set_data(G_OBJECT(action),"pakey",(gpointer) id);
		g_signal_connect(action,"activate",G_CALLBACK(action_pakey),widget);
		break;

	case ACTION_TYPE_SET:
		action = gtk_action_new(nm,NULL,NULL,NULL);
		g_object_set_data(G_OBJECT(action),"toggle_id",(gpointer) id);
		g_signal_connect(action,"activate",G_CALLBACK(action_set_toggle),widget);
		break;

	case ACTION_TYPE_RESET:
		action = gtk_action_new(nm,NULL,NULL,NULL);
		g_object_set_data(G_OBJECT(action),"toggle_id",(gpointer) id);
		g_signal_connect(action,"activate",G_CALLBACK(action_reset_toggle),widget);
		break;

	case ACTION_TYPE_TABLE:
		action = gtk_action_new(nm,NULL,NULL,NULL);
		g_signal_connect(action,"activate",callback[id],widget);
		break;
	}

	for(f=0;f<ACTION_COUNT;f++)
	{
		if(!g_strcasecmp(actionname[f],nm))
		{
			GtkAction **named_action = (GtkAction **) g_object_get_data(G_OBJECT(widget),"named_actions");
			named_action[f] = action;
			break;
		}
	}

	g_hash_table_insert(hash,nm,action);

	return action;
}

