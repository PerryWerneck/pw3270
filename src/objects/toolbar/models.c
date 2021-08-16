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
#include <pw3270/window.h>

#define GTK_TOOLBAR_DEFAULT_STYLE ((GtkToolbarStyle) -1)

struct _contents {
	const gchar * label;
	int			  value;
};

static const struct _models {
	const gchar *name;
	const gchar *property;
	const gchar *label;
	const struct _contents *contents;
} models[] = {

	{
		"toolbar-icon-size",
		"icon-size",
		N_("Icon _size"),
		(const struct _contents[]) {
			{
				.label = N_( "System default" ),
				.value = GTK_ICON_SIZE_INVALID

			},

			{
				.label = N_( "Small" ),
				.value = GTK_ICON_SIZE_SMALL_TOOLBAR
			},

			{
				.label = N_( "Large" ),
				.value = GTK_ICON_SIZE_LARGE_TOOLBAR
			},

			{
				.label = NULL
			}
		}
	},

	{
		"toolbar-style",
		"style",
		N_("Toolbar s_tyle"),
		(const struct _contents[]) {
			{
				.label = N_( "System default" ),
				.value = GTK_TOOLBAR_DEFAULT_STYLE
			},

			{
				.label = N_( "Icons only" ),
				.value = GTK_TOOLBAR_ICONS
			},

			{
				.label = N_( "Text only" ),
				.value = GTK_TOOLBAR_TEXT
			},

			{
				.label = N_( "Icons & text" ),
				.value = GTK_TOOLBAR_BOTH
			}, {
				.label = NULL
			}
		}
	},

	{
		"toolbar-icon-type",
		"icon-type",
		N_("Icon type"),
		(const struct _contents[]) {
			{
				.label = N_( "System default" ),
				.value = 0
			}, {
				.label = N_( "Symbolic" ),
				.value = 1
			}, {
				.label = NULL
			}
		}
	},

	{
		"toolbar-position",
		"position",
		N_("Position"),
		(const struct _contents[]) {
			{
				.label = N_( "Top (system default)" ),
				.value = 0
			}, {
				.label = N_( "Left" ),
				.value = 3
			}, {
				.label = N_( "Bottom" ),
				.value = 1
			}, {
				.label = N_( "Right" ),
				.value = 2
			}, {
				.label = NULL
			}
		}
	}
};

GtkTreeModel * pw3270_model_from_name(const gchar *name) {

	size_t model;

	for(model = 0; model < G_N_ELEMENTS(models); model++) {

		if(g_ascii_strcasecmp(models[model].name,name))
			continue;

		// Create model
		size_t row;
		GtkTreeIter	  iter;
		GtkListStore * store = gtk_list_store_new(2, G_TYPE_STRING,G_TYPE_UINT);

		for(row = 0; models[model].contents[row].label; row++) {
			gtk_list_store_append(store,&iter);
			gtk_list_store_set(	store,
			                    &iter,
			                    0, gettext(models[model].contents[row].label),
			                    1, models[model].contents[row].value,
			                    -1);
		}

		return GTK_TREE_MODEL(store);

	}

	g_warning("Can't create combobox '%s'",name);
	return NULL;
}

void pw3270_model_get_iter_from_value(GtkTreeModel * model, GtkTreeIter *iter, guint value) {

	if(gtk_tree_model_get_iter_first(model,iter)) {

		do {

			GValue gVal = { 0, };
			gtk_tree_model_get_value(model,iter,1,&gVal);
			guint iVal = g_value_get_uint(&gVal);
			g_value_unset(&gVal);

			if(iVal == value) {
				return;
			}

		} while(gtk_tree_model_iter_next(model,iter));

	}

}

guint pw3270_model_get_value_from_iter(GtkTreeModel * model, GtkTreeIter *iter) {
	GValue gVal = { 0, };
	gtk_tree_model_get_value(model,iter,1,&gVal);
	guint iVal = g_value_get_uint(&gVal);
	g_value_unset(&gVal);
	return iVal;
}

static void set_property(GObject *menuitem, GObject *widget) {

	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem))) {

		const struct _contents *model = (const struct _contents *) g_object_get_data(menuitem, I_("pw3270_model_data"));
		const char *name = (const char *) g_object_get_data(menuitem, I_("pw3270_property_name"));

//		debug("%s(%s,%d)",__FUNCTION__,name,model->value);
		g_object_set(widget,name,model->value,NULL);

	}

}

static void set_toggle_menu_item(GtkCheckMenuItem *item, gint *value) {
	const struct _contents *model = (const struct _contents *) g_object_get_data(G_OBJECT(item), I_("pw3270_model_data"));
	if(model) {
		gtk_check_menu_item_set_active(item,model->value == *value);
	}
}

static void property_changed(GObject *widget, GParamSpec G_GNUC_UNUSED(*pspec), GtkContainer *menu) {

	gint value = -1;
	const gchar * name = g_object_get_data(G_OBJECT(menu), I_("pw3270_property_name"));
	g_object_get(widget,name,&value,NULL);

//	debug("%s(%p,%s)=%d",__FUNCTION__,widget,name,value);

	gtk_container_foreach(menu,(GtkCallback) set_toggle_menu_item,&value);


}

GtkWidget * pw3270_menu_item_from_model(GtkWidget *widget, const gchar *name) {

	size_t model;


	for(model = 0; model < G_N_ELEMENTS(models); model++) {

		if(g_ascii_strcasecmp(models[model].name,name))
			continue;

		// Create submenu
		size_t row;
		GtkWidget * item;
		GtkWidget * menu = gtk_menu_item_new_with_mnemonic(gettext(models[model].label));

		GtkWidget * submenu = gtk_menu_new();
		g_object_set_data(G_OBJECT(submenu),I_("pw3270_property_name"),(gpointer) models[model].property);
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu),submenu);

		gint selected = -1;
		if(widget) {
			g_object_get(G_OBJECT(widget),models[model].property,&selected,NULL);
			g_autofree gchar * signame = g_strconcat("notify::",models[model].property,NULL);
			g_signal_connect(G_OBJECT(widget),signame,G_CALLBACK(property_changed),submenu);
		}

		for(row = 0; models[model].contents[row].label; row++) {

			item = gtk_check_menu_item_new_with_mnemonic(gettext(models[model].contents[row].label));
			gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(item),TRUE);

			g_object_set_data(G_OBJECT(item),I_("pw3270_property_name"),(gpointer) models[model].property);
			g_object_set_data(G_OBJECT(item),I_("pw3270_model_data"),(gpointer) &models[model].contents[row]);

			gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),selected == models[model].contents[row].value);

			if(widget) {
				g_signal_connect(item, "toggled", G_CALLBACK(set_property), widget);
			}

			gtk_menu_shell_append(GTK_MENU_SHELL(submenu),item);

		}

		gtk_widget_show_all(menu);

		return menu;
	}

	return NULL;
}

void pw3270_menu_item_set_value(GtkWidget *menu, guint value) {

	debug("%s(%p,%d)",__FUNCTION__,menu,value);

}

