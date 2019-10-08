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

 #include <config.h>
 #include <pw3270/toolbar.h>
 #include <lib3270/log.h>

 static gboolean popup_context_menu(GtkToolbar *toolbar, gint x, gint y, gint button_number);
 static void finalize(GObject *object);


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

 static void detacher(GtkWidget *attach_widget, GtkMenu *menu) {

 	pw3270ToolBar * toolbar = PW3270_TOOLBAR(attach_widget);
 	toolbar->popup_menu = NULL;

 }

 static void set_icon_size(GtkMenuItem *menuitem, GtkWidget *toolbar) {

	GtkIconSize * icon_size = g_object_get_data(G_OBJECT(menuitem),"icon_size");

	if(icon_size)
		gtk_toolbar_set_icon_size(GTK_TOOLBAR(toolbar),*icon_size);
	else
		gtk_toolbar_unset_icon_size(GTK_TOOLBAR(toolbar));

 }

 static void set_style(GtkMenuItem *menuitem, GtkWidget *toolbar) {

	GtkToolbarStyle * style = g_object_get_data(G_OBJECT(menuitem),"toolbar_style");

	if(style)
		gtk_toolbar_set_style(GTK_TOOLBAR(toolbar),*style);
	else
		gtk_toolbar_unset_style(GTK_TOOLBAR(toolbar));

 }

 static void pw3270ToolBar_init(pw3270ToolBar *widget) {

	widget->popup_menu = gtk_menu_new();

	// Size options
	{
		static const struct {
			const gchar			* label;
			GtkIconSize			  icon_size;
		} itens[] = {

			{
				.label = "Default"

			},

			{
				.label = "Small",
				.icon_size = GTK_ICON_SIZE_SMALL_TOOLBAR
			},

			{
				.label = "Large",
				.icon_size = GTK_ICON_SIZE_LARGE_TOOLBAR
			},
		};

		size_t ix;

		GtkWidget * item = gtk_menu_item_new_with_mnemonic("Icon _size");
		gtk_menu_shell_append(GTK_MENU_SHELL(widget->popup_menu),item);

		GtkWidget * submenu = gtk_menu_new();
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(item),submenu);

		for(ix = 0; ix < G_N_ELEMENTS(itens); ix++) {

			item = gtk_menu_item_new_with_mnemonic(itens[ix].label);

			if(ix > 0)
				g_object_set_data(G_OBJECT(item),"icon_size", (gpointer) &itens[ix].icon_size);

			g_signal_connect(item, "activate", G_CALLBACK(set_icon_size), widget);

			gtk_menu_shell_append(GTK_MENU_SHELL(submenu),item);

		}

	}

	// Style option
	{
		static const struct {
			const gchar * label;
			GtkToolbarStyle style;
		} itens[] = {

			{
				.label = "Default"
			},

			{
				.label = "Icons only",
				.style = GTK_TOOLBAR_ICONS
			},

			{
				.label = "Text only",
				.style = GTK_TOOLBAR_TEXT
			},

			{
				.label = "Icons & text",
				.style = GTK_TOOLBAR_BOTH
			},
		};

		size_t ix;

		GtkWidget * item = gtk_menu_item_new_with_mnemonic("S_tyle");
		gtk_menu_shell_append(GTK_MENU_SHELL(widget->popup_menu),item);

		GtkWidget * submenu = gtk_menu_new();
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(item),submenu);

		for(ix = 0; ix < G_N_ELEMENTS(itens); ix++) {

			item = gtk_menu_item_new_with_mnemonic(itens[ix].label);

			if(ix > 0)
				g_object_set_data(G_OBJECT(item),"toolbar_style", (gpointer) &itens[ix].style);

			g_signal_connect(item, "activate", G_CALLBACK(set_style), widget);

			gtk_menu_shell_append(GTK_MENU_SHELL(submenu),item);

		}

	}

	// gtk_container_set_border_width(GTK_CONTAINER(widget->popup_menu),6);
	gtk_widget_show_all(widget->popup_menu);
	gtk_menu_attach_to_widget(GTK_MENU(widget->popup_menu),GTK_WIDGET(widget),detacher);

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
		g_message("Invalid action identifier");
		return NULL;
	}

	if(!action->icon) {
		g_message("Action \"%s\" doesn't have an icon", action->name);
		return NULL;
	}

	if(!action->label) {
		g_message("Action \"%s\" doesn't have a label", action->name);
		return NULL;
	}

	debug("Action: %s icon: %s", action->name, action->icon);

	GtkToolItem * item = gtk_tool_button_new(gtk_image_new_from_icon_name(action->icon,GTK_ICON_SIZE_LARGE_TOOLBAR),action->label);
	gtk_tool_button_set_use_underline(GTK_TOOL_BUTTON(item),TRUE);

	gtk_widget_set_name(GTK_WIDGET(item), action->name);

	if(action->summary)
		gtk_tool_item_set_tooltip_text(item,action->summary);

	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, pos);

	return GTK_WIDGET(item);
 }

 gboolean popup_context_menu(GtkToolbar *widget, gint x, gint y, gint button_number) {

 	pw3270ToolBar * toolbar = PW3270_TOOLBAR(widget);

	debug("%s button_number=%d",__FUNCTION__,button_number);

	if(toolbar->popup_menu) {
		gtk_menu_popup_at_pointer(GTK_MENU(toolbar->popup_menu),NULL);
	}


	return TRUE;

 }
