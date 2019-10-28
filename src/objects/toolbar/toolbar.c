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

 #define GTK_TOOLBAR_DEFAULT_STYLE ((GtkToolbarStyle) -1)

 static gboolean popup_context_menu(GtkToolbar *toolbar, gint x, gint y, gint button_number);
 static void finalize(GObject *object);

 static const struct icon_size {
	const gchar			* label;
	GtkIconSize			  icon_size;
 } icon_sizes[] = {

	{
		.label = N_( "System default" ),
		.icon_size = GTK_ICON_SIZE_INVALID

	},

	{
		.label = N_( "Small" ),
		.icon_size = GTK_ICON_SIZE_SMALL_TOOLBAR
	},

	{
		.label = N_( "Large" ),
		.icon_size = GTK_ICON_SIZE_LARGE_TOOLBAR
	},
 };

 static const struct toolbar_style {
	const gchar * label;
	GtkToolbarStyle style;
 } styles[] = {

	{
		.label = N_( "System default" ),
		.style = GTK_TOOLBAR_DEFAULT_STYLE
	},

	{
		.label = N_( "Icons only" ),
		.style = GTK_TOOLBAR_ICONS
	},

	{
		.label = N_( "Text only" ),
		.style = GTK_TOOLBAR_TEXT
	},

	{
		.label = N_( "Icons & text" ),
		.style = GTK_TOOLBAR_BOTH
	},
};



 struct _pw3270ToolBar {
 	GtkToolbar parent;

 	/// @brief Popup Menu
	GtkWidget * popup_menu;

 };

 struct _pw3270ToolBarClass {

	GtkToolbarClass parent_class;


 };

 G_DEFINE_TYPE(pw3270ToolBar, pw3270ToolBar, GTK_TYPE_TOOLBAR);

 static void pw3270ToolBar_class_init(pw3270ToolBarClass *klass) {

 	GtkToolbarClass * toolbar = GTK_TOOLBAR_CLASS(klass);

 	toolbar->popup_context_menu = popup_context_menu;

 	G_OBJECT_CLASS(klass)->finalize = finalize;

 }

 static void detacher(GtkWidget *attach_widget, GtkMenu G_GNUC_UNUSED(*menu)) {

 	pw3270ToolBar * toolbar = PW3270_TOOLBAR(attach_widget);
 	toolbar->popup_menu = NULL;

 }

 static void set_icon_size(GtkMenuItem *menuitem, GtkWidget *toolbar) {

	const struct icon_size * size = g_object_get_data(G_OBJECT(menuitem),"icon_size");

	debug("%s(%d,%s)",__FUNCTION__,(int) size->icon_size, size->label);
	pw3270_toolbar_set_icon_size(GTK_TOOLBAR(toolbar), size->icon_size);

 }

 static void set_style(GtkMenuItem *menuitem, GtkWidget *toolbar) {
	struct toolbar_style * style = g_object_get_data(G_OBJECT(menuitem),"toolbar_style");

	debug("%s(%d,%s)",__FUNCTION__,(int) style->style, style->label);
 	pw3270_toolbar_toolbar_set_style(GTK_TOOLBAR(toolbar), style->style);

 }

 static void pw3270ToolBar_init(pw3270ToolBar *widget) {

	widget->popup_menu = gtk_menu_new();

	// Size options
	{
		size_t ix;

		GtkWidget * item = gtk_menu_item_new_with_mnemonic( _("Icon _size") );
		gtk_menu_shell_append(GTK_MENU_SHELL(widget->popup_menu),item);

		GtkWidget * submenu = gtk_menu_new();
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(item),submenu);

		for(ix = 0; ix < G_N_ELEMENTS(icon_sizes); ix++) {

			item = gtk_menu_item_new_with_mnemonic(gettext(icon_sizes[ix].label));

			g_object_set_data(G_OBJECT(item),"icon_size", (gpointer) &icon_sizes[ix]);
			g_signal_connect(item, "activate", G_CALLBACK(set_icon_size), widget);

			gtk_menu_shell_append(GTK_MENU_SHELL(submenu),item);

		}

	}

	// Style option
	{
		size_t ix;

		GtkWidget * item = gtk_menu_item_new_with_mnemonic( _("S_tyle") );
		gtk_menu_shell_append(GTK_MENU_SHELL(widget->popup_menu),item);

		GtkWidget * submenu = gtk_menu_new();
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(item),submenu);

		for(ix = 0; ix < G_N_ELEMENTS(styles); ix++) {

			item = gtk_menu_item_new_with_mnemonic(gettext(styles[ix].label));

			g_object_set_data(G_OBJECT(item),"toolbar_style", (gpointer) &styles[ix]);
			g_signal_connect(item, "activate", G_CALLBACK(set_style), widget);

			gtk_menu_shell_append(GTK_MENU_SHELL(submenu),item);

		}

	}

	// gtk_container_set_border_width(GTK_CONTAINER(widget->popup_menu),6);
	gtk_widget_show_all(widget->popup_menu);
	gtk_menu_attach_to_widget(GTK_MENU(widget->popup_menu),GTK_WIDGET(widget),detacher);

	// Bind settings
	GSettings *settings = pw3270_application_get_settings(g_application_get_default());

	if(settings) {
		pw3270_toolbar_toolbar_set_style(GTK_TOOLBAR(widget),g_settings_get_int(settings,"toolbar-style"));
		pw3270_toolbar_set_icon_size(GTK_TOOLBAR(widget),g_settings_get_int(settings,"toolbar-icon-size"));
	}

 }

 static void finalize(GObject *object) {

// 	pw3270ToolBar * toolbar = PW3270_TOOLBAR(object);


	G_OBJECT_CLASS(pw3270ToolBar_parent_class)->finalize(object);

 }

 GtkWidget * pw3270_toolbar_new(void) {
	return g_object_new(PW3270_TYPE_TOOLBAR, NULL);
 }

 GtkWidget * pw3270_toolbar_insert_lib3270_action(GtkWidget *toolbar, const LIB3270_ACTION *action, gint pos) {

	g_return_val_if_fail(GTK_IS_TOOLBAR(toolbar),NULL);

	if(!action) {
		g_message(_("Invalid action identifier"));
		return NULL;
	}

	if(!action->icon) {
		g_message(_("Action \"%s\" doesn't have an icon"), action->name);
		return NULL;
	}

	if(!action->label) {
		g_message(_("Action \"%s\" doesn't have a label"), action->name);
		return NULL;
	}

	debug("Action: %s icon: %s", action->name, action->icon);

	GtkToolItem * item = gtk_tool_button_new(gtk_image_new_from_icon_name(action->icon,GTK_ICON_SIZE_LARGE_TOOLBAR),gettext(action->label));
	gtk_tool_button_set_use_underline(GTK_TOOL_BUTTON(item),TRUE);

	gtk_widget_set_name(GTK_WIDGET(item), action->name);

	if(action->summary)
		gtk_tool_item_set_tooltip_text(item,gettext(action->summary));

	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, pos);

	return GTK_WIDGET(item);
 }

 gboolean popup_context_menu(GtkToolbar *widget, gint G_GNUC_UNUSED(x), gint G_GNUC_UNUSED(y), gint button_number) {

 	pw3270ToolBar * toolbar = PW3270_TOOLBAR(widget);

	debug("%s button_number=%d",__FUNCTION__,button_number);

	if(toolbar->popup_menu) {
		gtk_menu_popup_at_pointer(GTK_MENU(toolbar->popup_menu),NULL);
	}

	return TRUE;

 }

 void pw3270_toolbar_toolbar_set_style(GtkToolbar *toolbar, GtkToolbarStyle style) {

	debug("%s(%d)",__FUNCTION__,(int) style);

	if(style == GTK_TOOLBAR_DEFAULT_STYLE)
		gtk_toolbar_unset_style(GTK_TOOLBAR(toolbar));
	else
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbar),style);

	pw3270_settings_set_int("toolbar-style",(int) style);

 }

 void pw3270_toolbar_set_icon_size(GtkToolbar *toolbar, GtkIconSize icon_size) {

	debug("%s(%d)",__FUNCTION__,(int) icon_size);

	if(icon_size == GTK_ICON_SIZE_INVALID)
		gtk_toolbar_unset_icon_size(GTK_TOOLBAR(toolbar));
	else
		gtk_toolbar_set_icon_size(GTK_TOOLBAR(toolbar),icon_size);

	pw3270_settings_set_int("toolbar-icon-size", (gint) icon_size);

 }
