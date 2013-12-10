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
 * Este programa está nomeado como window.c e possui - linhas de código.
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
#include <lib3270/popup.h>
#include <lib3270/actions.h>

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 struct _pw3270
 {
	GtkWindow		  parent;
 	GtkWidget		* terminal;
 };

 struct _pw3270Class
 {
	GtkWindowClass parent_class;

	int dummy;
 };

 G_DEFINE_TYPE(pw3270, pw3270, GTK_TYPE_WINDOW);

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

/*--[ Implement ]------------------------------------------------------------------------------------*/

#if GTK_CHECK_VERSION(3,0,0)
 static void pw3270_destroy(GtkWidget *widget)
#else
 static void pw3270_destroy(GtkObject *widget)
#endif
 {
	pw3270 * window = GTK_PW3270(widget);

	trace("%s %p",__FUNCTION__,widget);

 	if(window->terminal)
		v3270_disconnect(window->terminal);

 }

 static gboolean window_state_event(GtkWidget *window, GdkEventWindowState *event, GtkWidget *widget)
 {
	gboolean	  fullscreen	= event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN ? TRUE : FALSE;
	GtkAction	**action		= (GtkAction **) g_object_get_data(G_OBJECT(widget),"named_actions");

	// Update fullscreen toggles
	if(action[ACTION_FULLSCREEN])
		gtk_action_set_visible(action[ACTION_FULLSCREEN],!fullscreen);

	if(action[ACTION_UNFULLSCREEN])
		gtk_action_set_visible(action[ACTION_UNFULLSCREEN],fullscreen);

	lib3270_set_toggle(v3270_get_session(widget),LIB3270_TOGGLE_FULL_SCREEN,fullscreen);

	save_window_state_to_config("window", "toplevel", event->new_window_state);

	return 0;
 }

 static gboolean configure_event(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
 {
 	GdkWindowState CurrentState = gdk_window_get_state(gtk_widget_get_window(widget));

	if( !(CurrentState & (GDK_WINDOW_STATE_FULLSCREEN|GDK_WINDOW_STATE_MAXIMIZED|GDK_WINDOW_STATE_ICONIFIED)) )
		save_window_size_to_config("window","toplevel",widget);

	return 0;
 }

 static int popup_handler(H3270 *session, void *terminal, LIB3270_NOTIFY type, const char *title, const char *msg, const char *fmt, va_list args)
 {
 	GtkWidget *widget = NULL;

 	if(session && terminal && GTK_IS_V3270(terminal))
		widget = GTK_WIDGET(terminal);

 	if(fmt)
	{
		gchar *text = g_strdup_vprintf(fmt,args);
		v3270_popup_message(GTK_WIDGET(widget),type,title,msg,text);
		g_free(text);
	}
	else
	{
		v3270_popup_message(GTK_WIDGET(widget),type,title,msg,NULL);
	}
	return 0;
 }

 static void pw3270_class_init(pw3270Class *klass)
 {
#if GTK_CHECK_VERSION(3,0,0)
	GtkWidgetClass	* widget_class	= GTK_WIDGET_CLASS(klass);
	widget_class->destroy = pw3270_destroy;
#else
	{
		GtkObjectClass *object_class = (GtkObjectClass*) klass;
		object_class->destroy = pw3270_destroy;
	}
#endif // GTK3

	configuration_init();
	lib3270_set_popup_handler(popup_handler);

 }

 GtkWidget * pw3270_new(const gchar *host, const gchar *systype, unsigned short colors)
 {
 	GtkWidget * widget	= g_object_new(GTK_TYPE_PW3270, NULL);

	#warning Reimplementar

	/*
	if(systype)
	{
		set_string_to_config("host","uri","%s",systype);
		pw3270_set_session_options(widget,pw3270_options_by_hosttype(systype));
	}
	else
	{
		gchar *ptr = get_string_from_config("host","systype","S390");
		pw3270_set_session_options(widget,pw3270_options_by_hosttype(ptr));
		g_free(ptr);
	}

	*/

	if(colors)
		set_integer_to_config("host","colortype",colors);
	else
		colors = get_integer_from_config("host","colortype",16);

	pw3270_set_session_color_type(widget,colors);

	if(host)
	{
		set_string_to_config("host","uri","%s",host);
		pw3270_connect(widget,host);
	}
	else
	{
		gchar *ptr = get_string_from_config("host","uri","");

		if(*ptr)
		{
			if(pw3270_get_toggle(widget,LIB3270_TOGGLE_CONNECT_ON_STARTUP))
				pw3270_connect(widget,ptr);
			else
				pw3270_set_host(widget,ptr);
		}
		g_free(ptr);
	}

	v3270_set_scaled_fonts(GTK_PW3270(widget)->terminal,get_boolean_from_config("terminal","sfonts",FALSE));

 	return widget;
 }

 void pw3270_connect(GtkWidget *widget, const gchar *uri)
 {
 	g_return_if_fail(GTK_IS_PW3270(widget));
 	v3270_connect(GTK_PW3270(widget)->terminal,uri);
 }

 const gchar * pw3270_set_host(GtkWidget *widget, const gchar *uri)
 {
 	g_return_if_fail(GTK_IS_PW3270(widget));
 	return v3270_set_host(GTK_PW3270(widget)->terminal,uri);
 }

 const gchar * pw3270_get_hostname(GtkWidget *widget)
 {
 	g_return_val_if_fail(GTK_IS_PW3270(widget),"");
 	return v3270_get_hostname(GTK_PW3270(widget)->terminal);
 }

 gboolean pw3270_get_toggle(GtkWidget *widget, LIB3270_TOGGLE ix)
 {
 	g_return_val_if_fail(GTK_IS_PW3270(widget),FALSE);
 	return v3270_get_toggle(GTK_PW3270(widget)->terminal,ix);
 }

 H3270 * pw3270_get_session(GtkWidget *widget)
 {
 	g_return_val_if_fail(GTK_IS_PW3270(widget),NULL);
 	return v3270_get_session(GTK_PW3270(widget)->terminal);
 }

 const gchar * pw3270_get_session_name(GtkWidget *widget)
 {
 	if(widget && GTK_IS_PW3270(widget))
		return v3270_get_session_name(GTK_PW3270(widget)->terminal);
	return g_get_application_name();
 }

 static void update_window_title(GtkWidget *window)
 {
	gchar			* title;
	GtkWidget		* widget = GTK_PW3270(window)->terminal;

	if(v3270_is_connected(widget))
	{
		const gchar *host = v3270_get_hostname(widget);

		if(host && *host)
			title = g_strdup_printf("%s - %s",v3270_get_session_name(widget),host);
		else
			title = g_strdup_printf("%s",v3270_get_session_name(widget));
	}
	else
	{
		title = g_strdup_printf(_( "%s - Disconnected" ),v3270_get_session_name(widget));
	}

	gtk_window_set_title(GTK_WINDOW(window),title);
	g_free(title);
 }

 LIB3270_EXPORT void pw3270_set_session_name(GtkWidget *widget, const gchar *name)
 {
 	g_return_if_fail(GTK_IS_PW3270(widget));
	v3270_set_session_name(GTK_PW3270(widget)->terminal,name);
	update_window_title(widget);
 }

 LIB3270_EXPORT void pw3270_set_session_options(GtkWidget *widget, LIB3270_OPTION options)
 {
 	g_return_if_fail(GTK_IS_PW3270(widget));
	v3270_set_session_options(GTK_PW3270(widget)->terminal,options);
 }

 LIB3270_EXPORT int pw3270_set_session_color_type(GtkWidget *widget, unsigned short colortype)
 {
 	g_return_val_if_fail(GTK_IS_PW3270(widget),EFAULT);
	return v3270_set_session_color_type(GTK_PW3270(widget)->terminal,colortype);
 }

 static void chktoplevel(GtkWidget *window, GtkWidget **widget)
 {
 	if(*widget)
		return;

	if(GTK_IS_PW3270(window))
		*widget = window;
 }

 LIB3270_EXPORT GtkWidget * pw3270_get_toplevel(void)
 {
	GtkWidget	* widget	= NULL;
	GList		* lst 		= gtk_window_list_toplevels();

	g_list_foreach(lst, (GFunc) chktoplevel, &widget);

	g_list_free(lst);
	return widget;
 }

 LIB3270_EXPORT GtkWidget * pw3270_get_terminal_widget(GtkWidget *widget)
 {
 	if(!widget)
		widget = pw3270_get_toplevel();
 	g_return_val_if_fail(GTK_IS_PW3270(widget),NULL);
 	return GTK_PW3270(widget)->terminal;
 }

 static void setup_input_method(GtkWidget *widget, GtkWidget *obj)
 {
	GtkWidget *menu	= gtk_menu_new();
	gtk_im_multicontext_append_menuitems((GtkIMMulticontext *) v3270_get_im_context(obj) ,GTK_MENU_SHELL(menu));
	gtk_widget_show_all(menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(widget),menu);
 }

 static void set_screen_size(GtkCheckMenuItem *item, GtkWidget *widget)
 {
	if(gtk_check_menu_item_get_active(item))
	{
		trace("screen model on widget %p changes to %d",widget,GPOINTER_TO_INT(g_object_get_data(G_OBJECT(item),"mode_3270")));
		lib3270_set_model(v3270_get_session(widget),GPOINTER_TO_INT(g_object_get_data(G_OBJECT(item),"mode_3270")));
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

		g_object_set_data(G_OBJECT(item),"mode_3270",GINT_TO_POINTER((f+2)));

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

 static void pastenext(GtkWidget *widget, gboolean on, GtkAction **action)
 {
#if GTK_CHECK_VERSION(3,10,0)
	g_simple_action_set_enabled(G_SIMPLE_ACTION(action[ACTION_PASTENEXT]),on);
#else
	gtk_action_set_sensitive(action[ACTION_PASTENEXT],on);
#endif // GTK(3,10)
 }

 static void disconnected(GtkWidget *terminal, GtkWidget * window)
 {
	GtkActionGroup	**group		= g_object_get_data(G_OBJECT(window),"action_groups");
	GtkWidget		**keypad	= g_object_get_data(G_OBJECT(window),"keypads");

	if(group)
	{
		gtk_action_group_set_sensitive(group[ACTION_GROUP_PASTE],FALSE);
		gtk_action_group_set_sensitive(group[ACTION_GROUP_ONLINE],FALSE);
		gtk_action_group_set_sensitive(group[ACTION_GROUP_OFFLINE],TRUE);
	}

	if(keypad)
	{
		while(*keypad)
			gtk_widget_set_sensitive(*(keypad++),FALSE);
	}

	update_window_title(window);
 }

 static void connected(GtkWidget *terminal, const gchar *host, GtkWidget * window)
 {
	GtkActionGroup	**group		= g_object_get_data(G_OBJECT(window),"action_groups");
	GtkWidget		**keypad	= g_object_get_data(G_OBJECT(window),"keypads");

	trace("%s(%s)",__FUNCTION__,host ? host : "NULL");

	if(group)
	{
		gtk_action_group_set_sensitive(group[ACTION_GROUP_ONLINE],TRUE);
		gtk_action_group_set_sensitive(group[ACTION_GROUP_OFFLINE],FALSE);
		gtk_action_group_set_sensitive(group[ACTION_GROUP_PASTE],TRUE);
	}

	if(keypad)
	{
		while(*keypad)
			gtk_widget_set_sensitive(*(keypad++),TRUE);
	}

	set_string_to_config("host","uri","%s",host);

	update_window_title(window);

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

 static void selecting(GtkWidget *widget, gboolean on, GtkActionGroup **group)
 {
	GtkAction **action = (GtkAction **) g_object_get_data(G_OBJECT(widget),"named_actions");
	gtk_action_group_set_sensitive(group[ACTION_GROUP_SELECTION],on);

#if GTK_CHECK_VERSION(3,10,0)
	if(action[ACTION_RESELECT])
		g_simple_action_set_enabled(G_SIMPLE_ACTION(action[ACTION_RESELECT]),!on);
#else
	if(action[ACTION_RESELECT])
		gtk_action_set_sensitive(action[ACTION_RESELECT],!on);
#endif // GTK(3,10)

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

 static void has_text(GtkWidget *widget, gboolean on, GtkActionGroup **group)
 {
	gtk_action_group_set_sensitive(group[ACTION_GROUP_CLIPBOARD],on);
 }

 static void print_all(GtkWidget *widget, GtkWidget *window)
 {
	pw3270_print(widget, NULL, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, PW3270_SRC_ALL);
 }

 static void toggle_changed(GtkWidget *widget, LIB3270_TOGGLE id, gboolean toggled, const gchar *name, GtkWindow *toplevel)
 {
	GtkAction **list = (GtkAction **) g_object_get_data(G_OBJECT(widget),"toggle_actions");
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

	if(list[id])
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(list[id]),toggled);

 }

 static gboolean field_clicked(GtkWidget *widget, gboolean connected, V3270_OIA_FIELD field, GdkEventButton *event, GtkWidget *window)
 {
	trace("%s: %s field=%d event=%p window=%p",__FUNCTION__,connected ? "Connected" : "Disconnected", field, event, window);

	if(!connected)
		return FALSE;

	if(field == V3270_OIA_SSL)
	{
		v3270_popup_security_dialog(widget);
		trace("%s: Show SSL connection info dialog",__FUNCTION__);
		return TRUE;
	}


	return FALSE;
 }

 static void pw3270_init(pw3270 *widget)
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

	int f;
	GtkAction	**action = g_new0(GtkAction *,ACTION_COUNT);
	H3270 		* host;

	trace("%s(%p)",__FUNCTION__,widget);

	// Initialize terminal widget
	widget->terminal = v3270_new();
	host = v3270_get_session(widget->terminal);

	for(f=0;f<G_N_ELEMENTS(widget_config);f++)
	{
		gchar *str = get_string_from_config("terminal",widget_config[f].key,NULL);
//		trace("str=%p strlen=%d",str,strlen(str));
		widget_config[f].set(widget->terminal,str);
		if(str)
			g_free(str);
	}
	lib3270_set_model(v3270_get_session(widget->terminal),get_integer_from_config("terminal","model",2));

	for(f=0;f<LIB3270_TOGGLE_COUNT;f++)
	{
		gchar *nm = g_ascii_strdown(lib3270_get_toggle_name(f),-1);
		lib3270_set_toggle(host,f,get_boolean_from_config("toggle",nm,lib3270_get_toggle(host,f)));
		g_free(nm);
	}

	g_object_set_data_full(G_OBJECT(widget->terminal),"toggle_actions",g_new0(GtkAction *,LIB3270_TOGGLE_COUNT),g_free);
	g_object_set_data_full(G_OBJECT(widget->terminal),"named_actions",(gpointer) action, (GDestroyNotify) g_free);

	// Load UI
	{
		gchar *path = pw3270_build_filename(GTK_WIDGET(widget),"ui",NULL);

		if(ui_parse_xml_folder(GTK_WINDOW(widget),path,groupname,popupname,widget->terminal,widget_setup))
		{
			g_free(path);
			gtk_widget_set_sensitive(widget->terminal,FALSE);
			return;
		}

		g_free(path);
	}


	// Setup actions
	{
		GtkWidget		**popup = g_object_get_data(G_OBJECT(widget),"popup_menus");
		GtkActionGroup	**group	= g_object_get_data(G_OBJECT(widget),"action_groups");

		// Setup action groups
		gtk_action_group_set_sensitive(group[ACTION_GROUP_SELECTION],FALSE);
		gtk_action_group_set_sensitive(group[ACTION_GROUP_CLIPBOARD],FALSE);
		gtk_action_group_set_sensitive(group[ACTION_GROUP_FILETRANSFER],FALSE);
		gtk_action_group_set_sensitive(group[ACTION_GROUP_PASTE],FALSE);

		disconnected(widget->terminal, GTK_WIDGET(widget));

		// Setup actions
		if(action[ACTION_FULLSCREEN])
			gtk_action_set_visible(action[ACTION_FULLSCREEN],!lib3270_get_toggle(host,LIB3270_TOGGLE_FULL_SCREEN));

		if(action[ACTION_UNFULLSCREEN])
			gtk_action_set_visible(action[ACTION_UNFULLSCREEN],lib3270_get_toggle(host,LIB3270_TOGGLE_FULL_SCREEN));

		if(action[ACTION_PASTENEXT])
		{
			gtk_action_set_sensitive(action[ACTION_PASTENEXT],FALSE);
			g_signal_connect(widget->terminal,"pastenext",G_CALLBACK(pastenext),action);
		}

		if(action[ACTION_RESELECT])
			gtk_action_set_sensitive(action[ACTION_RESELECT],FALSE);


		// Connect action signals
		g_signal_connect(widget->terminal,"disconnected",G_CALLBACK(disconnected),widget);
		g_signal_connect(widget->terminal,"connected",G_CALLBACK(connected),widget);
		g_signal_connect(widget->terminal,"update_config",G_CALLBACK(update_config),0);
		g_signal_connect(widget->terminal,"model_changed",G_CALLBACK(update_model),0);
		g_signal_connect(widget->terminal,"selecting",G_CALLBACK(selecting),group);
		g_signal_connect(widget->terminal,"popup",G_CALLBACK(popup_menu),popup);
		g_signal_connect(widget->terminal,"has_text",G_CALLBACK(has_text),group);

	}

	// Connect widget signals
	g_signal_connect(widget->terminal,"field_clicked",G_CALLBACK(field_clicked),widget);
	g_signal_connect(widget->terminal,"toggle_changed",G_CALLBACK(toggle_changed),widget);
	g_signal_connect(widget->terminal,"print",G_CALLBACK(print_all),widget);

	// Connect window signals
	g_signal_connect(widget,"window_state_event",G_CALLBACK(window_state_event),widget->terminal);
	g_signal_connect(widget,"configure_event",G_CALLBACK(configure_event),widget->terminal);

	// Finish setup
#ifdef DEBUG
	lib3270_testpattern(host);
#endif

	trace("%s ends",__FUNCTION__);
	gtk_window_set_focus(GTK_WINDOW(widget),widget->terminal);

 }

