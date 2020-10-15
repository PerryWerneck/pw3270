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
 * Este programa está nomeado como - e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "private.h"
 #include <pw3270/application.h>
 #include <pw3270/settings.h>

 #define GTK_TOOLBAR_DEFAULT_STYLE ((GtkToolbarStyle) -1)

 static gboolean popup_context_menu(GtkToolbar *toolbar, gint x, gint y, gint button_number);
 static void finalize(GObject *object);
 static void get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
 static void set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);

 enum {
	PROP_NONE,
	PROP_ACTION_NAMES,
	PROP_ICON_SIZE,
	PROP_ICON_TYPE,
	PROP_STYLE
 };

 enum {
	TOOLBAR_MENU_STYLE,
	TOOLBAR_MENU_ICON_SIZE,
	TOOLBAR_MENU_ICON_TYPE
 };

 static const gchar * toolbar_menus[] = {
	"toolbar-style",
	"toolbar-icon-size",
	"toolbar-icon-type"
 };

 struct _pw3270ToolBar {
 	GtkToolbar parent;
	GtkToolbarStyle style;
	int icon_type;

	GtkWidget *menu;
 	GtkWidget *submenu[G_N_ELEMENTS(toolbar_menus)];

 };

 struct _pw3270ToolBarClass {

	GtkToolbarClass parent_class;


 };

 G_DEFINE_TYPE(pw3270ToolBar, pw3270ToolBar, GTK_TYPE_TOOLBAR);

 static void pw3270ToolBar_class_init(pw3270ToolBarClass *klass) {

	GObjectClass *object_class = G_OBJECT_CLASS(klass);
 	GtkToolbarClass * toolbar = GTK_TOOLBAR_CLASS(klass);

 	toolbar->popup_context_menu = popup_context_menu;

 	G_OBJECT_CLASS(klass)->finalize = finalize;

	object_class->set_property	= set_property;
	object_class->get_property	= get_property;

	g_object_class_install_property(
		object_class,
		PROP_ACTION_NAMES,
		g_param_spec_string (
			I_("action-names"),
			"Action Names",
			_("The name of the actions in the toolbar"),
			NULL,
			G_PARAM_READABLE|G_PARAM_WRITABLE)
	);

	g_object_class_install_property(
		object_class,
		PROP_ICON_SIZE,
		g_param_spec_int(
			I_("icon-size"),
			"icon size",
			_("The toolbar icon size"),
			INT_MIN,
			INT_MAX,
			0,
			G_PARAM_READABLE|G_PARAM_WRITABLE)
	);

	g_object_class_install_property(
		object_class,
		PROP_STYLE,
		g_param_spec_int(
			I_("style"),
			"style",
			_("The toolbar style"),
			INT_MIN,
			INT_MAX,
			0,
			G_PARAM_READABLE|G_PARAM_WRITABLE)
	);

	g_object_class_install_property(
		object_class,
		PROP_ICON_TYPE,
		g_param_spec_int(
			I_("icon-type"),
			I_("icon-type"),
			_("The toolbar icon type"),
			0,
			1,
			0,
			G_PARAM_READABLE|G_PARAM_WRITABLE)
	);

 }

  void get_property(GObject *object, guint prop_id, GValue *value, GParamSpec G_GNUC_UNUSED(*pspec)) {

	switch (prop_id) {
    case PROP_ACTION_NAMES:
    	g_value_take_string(value,pw3270_toolbar_get_actions(GTK_WIDGET(object)));
		break;

	case PROP_ICON_SIZE:
		g_value_set_int(value,pw3270_toolbar_get_icon_size(GTK_TOOLBAR(object)));
		break;

	case PROP_STYLE:
		g_value_set_int(value,pw3270_toolbar_get_style(GTK_TOOLBAR(object)));
		break;

	case PROP_ICON_TYPE:
		g_value_set_int(value,pw3270_toolbar_get_icon_type(GTK_TOOLBAR(object)));
		break;

	default:
		g_assert_not_reached ();
	}

 }

 void set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec G_GNUC_UNUSED(*pspec)) {

	switch (prop_id)
	{
    case PROP_ACTION_NAMES:
		pw3270_toolbar_set_actions(GTK_WIDGET(object), g_value_get_string(value));
		break;

	case PROP_ICON_SIZE:
		pw3270_toolbar_set_icon_size(GTK_TOOLBAR(object),(GtkIconSize) g_value_get_int(value));
		break;

	case PROP_STYLE:
		pw3270_toolbar_set_style(GTK_TOOLBAR(object),(GtkToolbarStyle) g_value_get_int(value));
		break;

	case PROP_ICON_TYPE:
		pw3270_toolbar_set_icon_type(GTK_TOOLBAR(object),(GtkToolbarStyle) g_value_get_int(value));
		break;

	default:
		g_assert_not_reached ();
	}

 }

 static void detacher(GtkWidget *attach_widget, GtkMenu G_GNUC_UNUSED(*menu)) {

 	pw3270ToolBar * toolbar = PW3270_TOOLBAR(attach_widget);
 	toolbar->menu = NULL;

 }

 static void open_preferences(GtkMenuItem G_GNUC_UNUSED(*menuitem), GtkWidget *toolbar) {

	GtkWidget * window = gtk_widget_get_toplevel(toolbar);
	GtkWidget * dialog = pw3270_settings_dialog_new(NULL,FALSE);

	gtk_container_add(GTK_CONTAINER(dialog),pw3270_toolbar_settings_new());

	gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
	gtk_window_set_attached_to(GTK_WINDOW(dialog), window);
	gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(window));

	gtk_window_set_title(GTK_WINDOW(dialog),_("Setup toolbar"));

	gtk_widget_show_all(dialog);

 }

 static void pw3270ToolBar_init(pw3270ToolBar *widget) {

	size_t ix;

	widget->menu = gtk_menu_new();

	// Create menus.
	{
		for(ix = 0; ix < G_N_ELEMENTS(widget->submenu); ix++) {
			widget->submenu[ix] = pw3270_menu_item_from_model(GTK_WIDGET(widget),toolbar_menus[ix]);
			gtk_menu_shell_append(GTK_MENU_SHELL(widget->menu),widget->submenu[ix]);
		}
	}

	// Toolbar preferences.
	{
		GtkWidget * item = gtk_menu_item_new_with_mnemonic( _("_Preferences") );
		gtk_menu_shell_append(GTK_MENU_SHELL(widget->menu),gtk_separator_menu_item_new());
		gtk_menu_shell_append(GTK_MENU_SHELL(widget->menu),item);
		g_signal_connect(item, "activate", G_CALLBACK(open_preferences), widget);
	}

	gtk_widget_show_all(widget->menu);
	gtk_menu_attach_to_widget(GTK_MENU(widget->menu),GTK_WIDGET(widget),detacher);

 }

 static void finalize(GObject *object) {

	G_OBJECT_CLASS(pw3270ToolBar_parent_class)->finalize(object);

 }

 GtkWidget * pw3270_toolbar_new(void) {
	return g_object_new(PW3270_TYPE_TOOLBAR, NULL);
 }

 gboolean popup_context_menu(GtkToolbar *widget, gint G_GNUC_UNUSED(x), gint G_GNUC_UNUSED(y), gint button_number) {

 	pw3270ToolBar * toolbar = PW3270_TOOLBAR(widget);

	debug("%s button_number=%d",__FUNCTION__,button_number);

	if(toolbar->menu) {
#if GTK_CHECK_VERSION(3,22,0)
		gtk_menu_popup_at_pointer(GTK_MENU(toolbar->menu),NULL);
#else
		gtk_menu_popup(GTK_MENU(toolbar->menu), NULL, NULL, NULL, NULL, 0, 0);
#endif
	}

	return TRUE;

 }

 void pw3270_toolbar_set_style(GtkToolbar *toolbar, GtkToolbarStyle style) {

	debug("%s(%d)",__FUNCTION__,(int) style);

	PW3270_TOOLBAR(toolbar)->style = style;

	if(style == GTK_TOOLBAR_DEFAULT_STYLE)
		gtk_toolbar_unset_style(GTK_TOOLBAR(toolbar));
	else
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbar),style);

	g_object_notify(G_OBJECT(toolbar), "style");

 }

 GtkToolbarStyle pw3270_toolbar_get_style(GtkToolbar *toolbar) {
	return PW3270_TOOLBAR(toolbar)->style;
 }

 gint pw3270_toolbar_get_icon_type(GtkToolbar *toolbar) {
 	return PW3270_TOOLBAR(toolbar)->icon_type;
 }

 void pw3270_toolbar_set_icon_type(GtkToolbar *toolbar, gint icon_type) {

 	if(PW3270_TOOLBAR(toolbar)->icon_type == icon_type)
		return;

 	PW3270_TOOLBAR(toolbar)->icon_type = icon_type;

 	// Redefine icon types
 	GList * children = gtk_container_get_children(GTK_CONTAINER(toolbar));
 	GList * item;

 	for(item = children;item;item = g_list_next(item)) {

		if(GTK_IS_TOOL_BUTTON(item->data)) {

			g_autofree gchar * icon_name = g_strdup(gtk_tool_button_get_icon_name(GTK_TOOL_BUTTON(item->data)));
			if(g_str_has_suffix(icon_name,"-symbolic")) {
				icon_name[strlen(icon_name)-9] = 0;
			}

			if(icon_type) {
				g_autofree gchar * new_name = g_strconcat(icon_name,"-symbolic",NULL);
				gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(item->data),new_name);
			} else {
				gtk_tool_button_set_icon_name(GTK_TOOL_BUTTON(item->data),icon_name);
			}

		}

 	}

 	g_list_free(children);
	g_object_notify(G_OBJECT(toolbar), "icon-type");

 }

 void pw3270_toolbar_set_icon_size(GtkToolbar *toolbar, GtkIconSize icon_size) {

	debug("%s(%d)",__FUNCTION__,(int) icon_size);

	if(icon_size == GTK_ICON_SIZE_INVALID)
		gtk_toolbar_unset_icon_size(GTK_TOOLBAR(toolbar));
	else
		gtk_toolbar_set_icon_size(GTK_TOOLBAR(toolbar),icon_size);

	// Store value
	g_object_notify(G_OBJECT(toolbar), "icon-size");

 }

 GtkIconSize pw3270_toolbar_get_icon_size(GtkToolbar *toolbar) {

	GValue value = G_VALUE_INIT;
	g_value_init(&value, G_TYPE_BOOLEAN);
	g_object_get_property(G_OBJECT(toolbar),"icon-size-set",&value);

	gboolean is_set = g_value_get_boolean(&value);

	g_value_unset(&value);

	if(is_set)
		return gtk_toolbar_get_icon_size(GTK_TOOLBAR(toolbar));

	return GTK_ICON_SIZE_INVALID;
 }

 void pw3270_toolbar_set_actions(GtkWidget *toolbar, const gchar *action_names) {

	size_t ix;
	gint pos = 0;

	gchar ** blocks = g_strsplit(action_names,":",-1);

 	gtk_container_remove_all(GTK_CONTAINER(toolbar));

	// Left block
	{
		gchar ** actions = g_strsplit(blocks[0],",",-1);

		for(ix = 0; actions[ix]; ix++) {
			pw3270_toolbar_insert_action(toolbar,actions[ix],pos++);
		}

		g_strfreev(actions);

	}

	// Right block
	if(blocks[1]) {

		GtkToolItem * item = gtk_separator_tool_item_new();

		gtk_separator_tool_item_set_draw(GTK_SEPARATOR_TOOL_ITEM(item),FALSE);
		gtk_tool_item_set_expand(item,TRUE);
		gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item,pos++);

		gchar ** actions = g_strsplit(blocks[1],",",-1);

		for(ix = 0; actions[ix]; ix++) {
			pw3270_toolbar_insert_action(toolbar,actions[ix],pos++);
		}

		g_strfreev(actions);

	}

	g_strfreev(blocks);


	g_object_notify(G_OBJECT(toolbar), "action-names");

 }

 gchar * pw3270_toolbar_get_actions(GtkWidget *toolbar) {

	GString * str = g_string_new("");

 	GList * children = gtk_container_get_children(GTK_CONTAINER(toolbar));
 	GList * item;

 	for(item = children;item;item = g_list_next(item)) {

		if(*str->str)
			g_string_append(str,",");

		if(GTK_IS_SEPARATOR_TOOL_ITEM(item->data)) {
			g_string_append(str,"separator");
		} else if(GTK_IS_TOOL_BUTTON(item->data)) {
			g_string_append(str,gtk_actionable_get_action_name(GTK_ACTIONABLE(item->data)));
		}

 	}

 	g_list_free(children);

	return g_string_free(str,FALSE);
 }

