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
 * Este programa está nomeado como mainwindow.c e possui - linhas de código.
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

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 enum action_group
 {
	ACTION_GROUP_DEFAULT,
	ACTION_GROUP_ONLINE,
	ACTION_GROUP_OFFLINE,
	ACTION_GROUP_SELECTION,
	ACTION_GROUP_CLIPBOARD,
	ACTION_GROUP_FILETRANSFER,
	ACTION_GROUP_PASTE,

	ACTION_GROUP_MAX
 };

 enum popup_group
 {
 	POPUP_DEFAULT,
 	POPUP_ONLINE,
 	POPUP_OFFLINE,
 	POPUP_SELECTION,

 	POPUP_MAX
 };

 static const gchar *groupname[ACTION_GROUP_MAX+1] = {	"default",
														"online",
														"offline",
														"selection",
														"clipboard",
														"filetransfer",
														"paste",
														NULL
														};

 static const gchar *popupname[POPUP_MAX+1] = {			"default",
														"online",
														"offline",
														"selection",
														NULL
														};

 static const gchar *actionname[ACTION_COUNT+1] = {		"pastenext",
														"reselect",
														"setfullscreen",
														"resetfullscreen"
 													};
/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void toggle_changed(GtkWidget *widget, LIB3270_TOGGLE id, gboolean toggled, const gchar *name, GtkWindow *toplevel)
 {
 	gchar *nm = g_ascii_strdown(name,-1);
	set_boolean_to_config("toggle",nm,toggled);
	g_free(nm);

	if(id == LIB3270_TOGGLE_FULL_SCREEN)
	{
		if(toggled)
			gtk_window_fullscreen(GTK_WINDOW(toplevel));
		else
			gtk_window_unfullscreen(GTK_WINDOW(toplevel));
	}

	#warning TODO: Update toggle actions.

 }

 static gboolean window_state_event(GtkWidget *window, GdkEventWindowState *event, GtkWidget *widget)
 {
	gboolean	  fullscreen = event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN ? TRUE : FALSE;
	GtkAction	**action = (GtkAction **) g_object_get_data(G_OBJECT(window),"named_actions");

	// Update fullscreen toggles
	if(action[ACTION_FULLSCREEN])
		gtk_action_set_visible(action[ACTION_FULLSCREEN],!fullscreen);

	if(action[ACTION_UNFULLSCREEN])
		gtk_action_set_visible(action[ACTION_UNFULLSCREEN],fullscreen);

	lib3270_set_toggle(v3270_get_session(widget),LIB3270_TOGGLE_FULL_SCREEN,fullscreen);

	return 0;
 }

 static void disconnected(GtkWidget *widget, GtkActionGroup **group)
 {
	gtk_action_group_set_sensitive(group[ACTION_GROUP_ONLINE],FALSE);
	gtk_action_group_set_sensitive(group[ACTION_GROUP_OFFLINE],TRUE);
 }

 static void connected(GtkWidget *widget, GtkActionGroup **group)
 {
	gtk_action_group_set_sensitive(group[ACTION_GROUP_ONLINE],TRUE);
	gtk_action_group_set_sensitive(group[ACTION_GROUP_OFFLINE],FALSE);
 }

 static void update_config(GtkWidget *widget, const gchar *name, const gchar *value)
 {
 	set_string_to_config("terminal",name,"%s",value);
 }

 static void update_model(GtkWidget *widget, guint id, const gchar *name)
 {
 	trace("Widget %p changed to %s (id=%d)",widget,name,id);
	set_integer_to_config("terminal","model",id);
 }

 static gboolean popup_menu(GtkWidget *widget, gboolean selected, gboolean online, GdkEventButton *event, GtkWidget **popup)
 {
 	GtkWidget *menu = NULL;

	if(!online)
		menu = popup[POPUP_OFFLINE];
	else if(selected && popup[POPUP_SELECTION])
		menu = popup[POPUP_SELECTION];
	else if(popup[POPUP_ONLINE])
		menu = popup[POPUP_ONLINE];
	else
		menu = popup[POPUP_DEFAULT];

 	trace("Popup %p on widget %p online=%s selected=%s",menu,widget,online ? "Yes" : "No", selected ? "Yes" : "No");

	if(!menu)
		return FALSE;

	trace("Showing popup \"%s\"",gtk_widget_get_name(menu));

	gtk_widget_show_all(menu);
	gtk_menu_set_screen(GTK_MENU(menu), gtk_widget_get_screen(widget));
	gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event->button,event->time);

 	return TRUE;
 }

 static void selecting(GtkWidget *widget, gboolean on, GtkActionGroup **group)
 {
	GtkAction **action = (GtkAction **) g_object_get_data(G_OBJECT(widget),"named_actions");
	gtk_action_group_set_sensitive(group[ACTION_GROUP_SELECTION],on);

	if(action[ACTION_RESELECT])
		gtk_action_set_sensitive(action[ACTION_RESELECT],!on);

 }

 static void setup_input_method(GtkWidget *widget, GtkWidget *obj)
 {
	GtkWidget *menu	= gtk_menu_new();
	gtk_im_multicontext_append_menuitems((GtkIMMulticontext *) GTK_V3270(obj)->input_method,GTK_MENU_SHELL(menu));
	gtk_widget_show_all(menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(widget),menu);
 }

 static void set_screen_size(GtkCheckMenuItem *item, GtkWidget *widget)
 {
	if(gtk_check_menu_item_get_active(item))
	{
		trace("screen model on widget %p changes to %d",widget,(int) g_object_get_data(G_OBJECT(item),"mode_3270"));
		lib3270_set_model(v3270_get_session(widget),(int) g_object_get_data(G_OBJECT(item),"mode_3270"));
	}
 }

 static void setup_screen_sizes(GtkWidget *widget, GtkWidget *obj)
 {
 	static const gchar 	* text[]	= { "80x24", "80x32", "80x43", "132x27" };
	GtkWidget			* menu		= gtk_menu_new();
 	int			  		  model		= lib3270_get_model(v3270_get_session(obj))-2;
	GSList 				* group		= NULL;
	GtkWidget			* item;
 	int					  f;

	gtk_widget_set_sensitive(widget,TRUE);

	for(f=0;f<G_N_ELEMENTS(text);f++)
	{
		gchar * name = g_strdup_printf( _( "Model %d (%s)"),f+2,text[f]);

		item = gtk_radio_menu_item_new_with_label(group,name);
		g_free(name);

		g_object_set_data(G_OBJECT(item),"mode_3270",(gpointer) (f+2));

		group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));

		gtk_widget_show(item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

		if(f == model)
			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),TRUE);

		g_signal_connect(G_OBJECT(item),"toggled",G_CALLBACK(set_screen_size),(gpointer) obj);

	}

	gtk_widget_show_all(menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(widget),menu);
 }


 GtkWidget * create_main_window(void)
 {
 	static const UI_WIDGET_SETUP widget_setup[] =
 	{
 		{ "inputmethod",	setup_input_method	},
 		{ "screensizes",	setup_screen_sizes	},
 		{ "fontselect",		setup_font_list		},
 		{ NULL,				NULL				}
 	};

 	static const struct _widget_config
 	{
 		const gchar *key;
 		void 		(*set)(GtkWidget *widget, const gchar *str);
 	} widget_config[] =
 	{
 		{ "colors", 		v3270_set_colors		},
 		{ "font-family",	v3270_set_font_family	}
 	};

	GtkWidget 		* window;
	GtkWidget		* terminal	= v3270_new();
	H3270			* host 		= v3270_get_session(terminal);
	gchar			* path		= build_data_filename("ui",NULL);
	GtkActionGroup	**group;
	GtkAction 		**action;
	GtkWidget		**popup;
	int			  	  f;

	// Initialize terminal config
	for(f=0;f<G_N_ELEMENTS(widget_config);f++)
	{
		gchar *str = get_string_from_config("terminal",widget_config[f].key,NULL);
		widget_config[f].set(terminal,str);
		if(str)
			g_free(str);
	}
	lib3270_set_model(host,get_integer_from_config("terminal","model",2));

	for(f=0;f<LIB3270_TOGGLE_COUNT;f++)
	{
		gchar *nm = g_ascii_strdown(lib3270_get_toggle_name(f),-1);
		lib3270_set_toggle(host,f,get_boolean_from_config("toggle",nm,lib3270_get_toggle(host,f)));
		g_free(nm);
	}

	// Create window
	window = ui_parse_xml_folder(path,groupname,popupname,actionname,terminal,widget_setup);
	group  = g_object_get_data(G_OBJECT(window),"action_groups");
	popup  = g_object_get_data(G_OBJECT(window),"popup_menus");
	action = (GtkAction **) g_object_get_data(G_OBJECT(window),"named_actions");

	g_object_set_data(G_OBJECT(terminal),"named_actions",action);

	// Setup action groups
	gtk_action_group_set_sensitive(group[ACTION_GROUP_SELECTION],FALSE);
	gtk_action_group_set_sensitive(group[ACTION_GROUP_CLIPBOARD],FALSE);
	gtk_action_group_set_sensitive(group[ACTION_GROUP_FILETRANSFER],FALSE);
	gtk_action_group_set_sensitive(group[ACTION_GROUP_PASTE],FALSE);
	disconnected(terminal, (gpointer) group);

	// Setup actions
	if(action[ACTION_FULLSCREEN])
		gtk_action_set_visible(action[ACTION_FULLSCREEN],!lib3270_get_toggle(host,LIB3270_TOGGLE_FULL_SCREEN));

	if(action[ACTION_UNFULLSCREEN])
		gtk_action_set_visible(action[ACTION_UNFULLSCREEN],lib3270_get_toggle(host,LIB3270_TOGGLE_FULL_SCREEN));

	if(action[ACTION_PASTENEXT])
		gtk_action_set_sensitive(action[ACTION_PASTENEXT],FALSE);

	if(action[ACTION_RESELECT])
		gtk_action_set_sensitive(action[ACTION_RESELECT],FALSE);

	// Connect window signals
	g_signal_connect(window,"window_state_event",G_CALLBACK(window_state_event),terminal);

	// Connect widget signals
	g_signal_connect(terminal,"toggle_changed",G_CALLBACK(toggle_changed),window);
	g_signal_connect(terminal,"disconnected",G_CALLBACK(disconnected),group);
	g_signal_connect(terminal,"connected",G_CALLBACK(connected),group);
	g_signal_connect(terminal,"update_config",G_CALLBACK(update_config),0);
	g_signal_connect(terminal,"model_changed",G_CALLBACK(update_model),0);
	g_signal_connect(terminal,"selecting",G_CALLBACK(selecting),group);
	g_signal_connect(terminal,"popup",G_CALLBACK(popup_menu),popup);

	g_free(path);
	gtk_widget_grab_focus(terminal);

	if(lib3270_get_toggle(host,LIB3270_TOGGLE_FULL_SCREEN))
		gtk_window_fullscreen(GTK_WINDOW(window));

#ifdef DEBUG
	lib3270_testpattern(host);
#endif

 	return window;
 }




