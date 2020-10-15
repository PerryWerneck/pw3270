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

 static const struct _models {
	const gchar *name;
	const gchar *label;
	const struct _contents {
		const gchar * label;
		guint		  value;
	} *contents;
 } models[] = {

	{
		"toolbar-icon-size",
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
			},
			{
				.label = NULL
			}
		}
	},
	{
		"toolbar-icon-type",
		N_("Icon type"),
		(const struct _contents[]) {
			{
				.label = N_( "System default" ),
				.value = 0
			},
			{
				.label = N_( "Symbolic" ),
				.value = 1
			},
			{
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

 GtkWidget * pw3270_menu_item_from_name(const gchar *name) {

	size_t model;

	for(model = 0; model < G_N_ELEMENTS(models); model++) {

		if(g_ascii_strcasecmp(models[model].name,name))
			continue;

		// Create submenu
		size_t row;
		GtkWidget * item;
		GtkWidget * menu = gtk_menu_item_new_with_mnemonic(gettext(models[model].label));

		GtkWidget * submenu = gtk_menu_new();
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu),submenu);

		g_object_set_data(G_OBJECT(submenu),"model-data",(gpointer) &models[model]);

		for(row = 0; models[model].contents[row].label; row++) {

			item = gtk_check_menu_item_new_with_mnemonic(gettext(models[model].contents[row].label));
			gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(item),TRUE);

			//g_signal_connect(item, "toggled", G_CALLBACK(set_icon_size), menu);

			gtk_menu_shell_append(GTK_MENU_SHELL(submenu),item);

		}

		gtk_widget_show_all(menu);

		return menu;
	}

	return NULL;
 }
