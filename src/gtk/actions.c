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

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void lib3270_action(GtkAction *action, GtkWidget *widget)
{
	int	(*call)(H3270 *h) = (int (*)(H3270 *h)) g_object_get_data(G_OBJECT(action),"lib3270_call");
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);
	call(GTK_V3270(widget)->host);
}

static void connect_action(GtkAction *action, GtkWidget *widget)
{
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);
	v3270_connect(widget,g_object_get_data(G_OBJECT(action),"host"));
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
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);
	v3270_copy_clipboard(GTK_V3270(widget));
}


void ui_connect_action(GtkAction *action, GtkWidget *widget, const gchar *name, const gchar *id)
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
	#define DECLARE_LIB3270_FKEY_ACTION( name )					/* name */

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
		{ "copy", 		copy_action			},
		{ "disconnect", disconnect_action	},
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

	lib3270_set_toggle(GTK_V3270(widget)->host,toggle,gtk_toggle_action_get_active(action));
}

void ui_connect_toggle(GtkAction *action, GtkWidget *widget, const gchar *name, const gchar *id)
{
	LIB3270_TOGGLE toggle = lib3270_get_toggle_id(id);

	if(toggle != -1)
	{
		g_object_set_data(G_OBJECT(action),"toggle_id",(gpointer) toggle);
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),(lib3270_get_toggle(GTK_V3270(widget)->host,toggle) != 0));
		g_signal_connect(action,"toggled",G_CALLBACK(lib3270_toggle_action),widget);
		return;
	}

	trace("Action %s with toggle %s on widget %p",gtk_action_get_name(action),id,widget);

	// Not found, disable action
	gtk_action_set_sensitive(action,FALSE);
}

static void action_pfkey(GtkAction *action, GtkWidget *widget)
{
	trace("Action %s activated on widget %p key=%d",gtk_action_get_name(action),widget,g_object_get_data(G_OBJECT(action),"pfkey"));
	lib3270_pfkey(GTK_V3270(widget)->host,(int) g_object_get_data(G_OBJECT(action),"pfkey"));
}

void ui_connect_pfkey(GtkAction *action, GtkWidget *widget, const gchar *name, const gchar *id)
{
	g_object_set_data(G_OBJECT(action),"pfkey",(gpointer) atoi(id));
	g_signal_connect(action,"activate",G_CALLBACK(action_pfkey),widget);
}

static void action_pakey(GtkAction *action, GtkWidget *widget)
{
	trace("Action %s activated on widget %p key=%d",gtk_action_get_name(action),widget,g_object_get_data(G_OBJECT(action),"pakey"));
	lib3270_pakey(GTK_V3270(widget)->host,(int) g_object_get_data(G_OBJECT(action),"pakey"));
}

static void action_fullscreen(GtkAction *action, GtkWidget *widget)
{
	lib3270_set_toggle(GTK_V3270(widget)->host,LIB3270_TOGGLE_FULL_SCREEN,1);
}

static void action_reselect(GtkAction *action, GtkWidget *widget)
{
	lib3270_reselect(GTK_V3270(widget)->host);
}

static void action_unfullscreen(GtkAction *action, GtkWidget *widget)
{
	lib3270_set_toggle(GTK_V3270(widget)->host,LIB3270_TOGGLE_FULL_SCREEN,0);
}

void ui_connect_pakey(GtkAction *action, GtkWidget *widget, const gchar *name, const gchar *id)
{
	g_object_set_data(G_OBJECT(action),"pakey",(gpointer) atoi(id));
	g_signal_connect(action,"activate",G_CALLBACK(action_pakey),widget);
}

void ui_connect_index_action(GtkAction *action, GtkWidget *widget, int ix, GtkAction **lst)
{
	trace("action(%d): %p",ix,action);

	switch(ix)
	{
	case ACTION_PASTENEXT:
		break;

	case ACTION_FULLSCREEN:
		g_signal_connect(action,"activate",G_CALLBACK(action_fullscreen),widget);
		break;

	case ACTION_UNFULLSCREEN:
		g_signal_connect(action,"activate",G_CALLBACK(action_unfullscreen),widget);
		break;

	case ACTION_RESELECT:
		g_signal_connect(action,"activate",G_CALLBACK(action_reselect),widget);
		break;

	default:
		g_warning("Action \"%s\" has unexpected id %d",gtk_action_get_name(action),ix);
		gtk_action_set_sensitive(action,FALSE);
	}
}
