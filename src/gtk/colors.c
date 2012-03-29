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
 * Este programa está nomeado como colors.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <gtk/gtk.h>
 #include "globals.h"

/*--[ Implement ]------------------------------------------------------------------------------------*/

 void load_color_schemes(GtkWidget *widget, gchar *active)
 {
	gchar *filename = build_data_filename("colors.conf",NULL);

 	if(!g_file_test(filename,G_FILE_TEST_IS_REGULAR))
	{
		gtk_widget_set_sensitive(widget,FALSE);
		g_warning("Unable to load color schemes in \"%s\"",filename);
	}
	else
	{
		gchar 		** group;
		GKeyFile	*  conf 	= g_key_file_new();
		int			   f		= 0;
		gboolean	   found 	= FALSE;

#if !GTK_CHECK_VERSION(3,0,0)
		GtkTreeModel	* model		= (GtkTreeModel *) gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_STRING);
		GtkCellRenderer * renderer	= gtk_cell_renderer_text_new();
		GtkTreeIter		  iter;

		gtk_combo_box_set_model(GTK_COMBO_BOX(widget),model);

		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget), renderer, TRUE);
		gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), renderer, "text", 0, NULL);

#endif // !GTK(3,0,0)

		g_key_file_load_from_file(conf,filename,G_KEY_FILE_NONE,NULL);

		group = g_key_file_get_groups(conf,NULL);

		for(f=0;group[f];f++)
		{
			gchar *str = g_strjoin( ",",	g_key_file_get_string(conf,group[f],"Terminal",NULL),
											g_key_file_get_string(conf,group[f],"BaseAttributes",NULL),
											g_key_file_get_string(conf,group[f],"SelectedText",NULL),
											g_key_file_get_string(conf,group[f],"Cursor",NULL),
											g_key_file_get_string(conf,group[f],"OIA",NULL),
											NULL
								);
#if GTK_CHECK_VERSION(3,0,0)

			gtk_combo_box_text_insert(		GTK_COMBO_BOX_TEXT(widget),
											f,
											str,
											g_key_file_get_locale_string(conf,group[f],"Label",NULL,NULL));


			if(active && !g_strcasecmp(active,str))
			{
				found = TRUE;
				gtk_combo_box_set_active(GTK_COMBO_BOX(widget),f);
			}
#else

			gtk_list_store_append((GtkListStore *) model,&iter);
			gtk_list_store_set((GtkListStore *) model, &iter,
												0, g_key_file_get_locale_string(conf,group[f],"Label",NULL,NULL),
												1, str,
												-1);

			if(active && !g_strcasecmp(active,str))
			{
				found = TRUE;
				gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&iter);
			}

#endif // GTK(3,0,0)

			g_free(str);
		}

		g_strfreev(group);
		g_key_file_free(conf);

		if(active && !found)
		{
#if GTK_CHECK_VERSION(3,0,0)

			gtk_combo_box_text_insert(		GTK_COMBO_BOX_TEXT(widget),
											0,
											active,
											_( "Custom colors") );
			gtk_combo_box_set_active(GTK_COMBO_BOX(widget),0);

#else

			gtk_list_store_append((GtkListStore *) model,&iter);
			gtk_list_store_set((GtkListStore *) model, &iter,
												0, _( "Custom colors" ),
												1, active,
												-1);

			gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&iter);
#endif
		}

		gtk_widget_set_sensitive(widget,TRUE);

	}

	g_free(filename);
 }

 static void color_changed(GtkColorSelection *colorselection, GtkWidget *widget)
 {
 	GdkColor	clr;
	int			id		= (int) g_object_get_data(G_OBJECT(colorselection),"colorid");

	if(id < 0 || id >= V3270_COLOR_COUNT)
		return;

	gtk_color_selection_get_current_color(colorselection,&clr);
	v3270_set_color(widget,id,&clr);
	v3270_reload(widget);
	gtk_widget_queue_draw(widget);
 }

 static void color_selected(GtkTreeSelection *selection, GtkWidget *color)
 {
	GtkWidget		* widget	= g_object_get_data(G_OBJECT(selection),"v3270");
	GdkColor		* saved		= g_object_get_data(G_OBJECT(selection),"lastcolors");
	GValue			  value		= { 0, };
	GtkTreeModel	* model;
	GtkTreeIter		  iter;
	GdkColor		* clr;
	int				  id;

	gtk_widget_set_sensitive(color,FALSE);

	if(!gtk_tree_selection_get_selected(selection,&model,&iter))
		return;

	gtk_tree_model_get_value(model,&iter,1,&value);

	id = g_value_get_int(&value);

	if(id < 0 || id >= V3270_COLOR_COUNT)
		return;

	g_object_set_data(G_OBJECT(color),"colorid",(gpointer) id);
	clr = v3270_get_color(widget,id);

	gtk_color_selection_set_previous_color(GTK_COLOR_SELECTION(color),saved+id);
	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(color),clr);

	gtk_widget_set_sensitive(color,TRUE);
 }

 void editcolors_action(GtkAction *action, GtkWidget *widget)
 {
 	static const gchar *custom = N_( "Custom colors" );

 	static const struct _node
 	{
 		int			id;
 		const char	*text;
 	} node[] =
 	{
 		{ V3270_COLOR_BACKGROUND,		N_( "Terminal" )			},
 		{ V3270_COLOR_FIELD,			N_( "Fields" )				},
 		{ V3270_COLOR_SELECTED_BG,		N_( "Other" )				},

 	};

	static const gchar *color_name[V3270_COLOR_COUNT] =
	{
		N_( "Background" ),					// V3270_COLOR_BACKGROUND
		N_( "Blue" ),						// V3270_COLOR_BLUE
		N_( "Red" ),						// V3270_COLOR_RED
		N_( "Pink" ),						// V3270_COLOR_PINK
		N_( "Green" ),						// V3270_COLOR_GREEN
		N_( "Turquoise" ),					// V3270_COLOR_TURQUOISE
		N_( "Yellow" ),						// V3270_COLOR_YELLOW
		N_( "White" ),						// V3270_COLOR_WHITE
		N_( "Black" ),						// V3270_COLOR_BLACK
		N_( "Dark Blue" ),					// V3270_COLOR_DARK_BLUE
		N_( "Orange" ),						// V3270_COLOR_ORANGE
		N_( "Purple" ),						// V3270_COLOR_PURPLE
		N_( "Dark Green" ),					// V3270_COLOR_DARK_GREEN
		N_( "Turquoise" ),					// V3270_COLOR_DARK_TURQUOISE
		N_( "Mustard" ),					// V3270_COLOR_MUSTARD
		N_( "Gray" ),						// V3270_COLOR_GRAY

		N_( "Normal/Unprotected" ),			// V3270_COLOR_FIELD
		N_( "Intensified/Unprotected" ),	// V3270_COLOR_FIELD_INTENSIFIED
		N_( "Normal/Protected" ),			// V3270_COLOR_FIELD_PROTECTED
		N_( "Intensified/Protected" ),		// V3270_COLOR_FIELD_PROTECTED_INTENSIFIED

		N_( "Selection background" ),		// TERMINAL_COLOR_SELECTED_BG
		N_( "Selection foreground" ),		// TERMINAL_COLOR_SELECTED_FG

		N_( "Cross-hair cursor" ),			// TERMINAL_COLOR_CROSS_HAIR

		// Oia Colors
		N_( "OIA background" ),				// TERMINAL_COLOR_OIA_BACKGROUND
		N_( "OIA foreground" ),				// TERMINAL_COLOR_OIA_FOREGROUND
		N_( "OIA separator" ),				// TERMINAL_COLOR_OIA_SEPARATOR
		N_( "OIA status ok" ),				// TERMINAL_COLOR_OIA_STATUS_OK
		N_( "OIA status invalid" ),			// TERMINAL_COLOR_OIA_STATUS_INVALID

	};

 	const gchar * title  = g_object_get_data(G_OBJECT(action),"title");
	GtkWidget	* dialog = gtk_dialog_new_with_buttons (	gettext(title ? title : N_( "Color setup") ),
															GTK_WINDOW(gtk_widget_get_toplevel(widget)),
															GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
															GTK_STOCK_OK,		GTK_RESPONSE_ACCEPT,
															GTK_STOCK_CANCEL,	GTK_RESPONSE_REJECT,
															NULL );
	GtkWidget	* panned = gtk_hbox_new(FALSE,2);
	GtkWidget	* tree;
	GtkWidget	* color;
	GdkColor	  saved[V3270_COLOR_COUNT];

	{
		// Color dialog setup
		color = gtk_color_selection_new();
		gtk_widget_set_sensitive(color,0);
		gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(color),FALSE);
		gtk_color_selection_set_has_palette(GTK_COLOR_SELECTION(color),TRUE);
		gtk_box_pack_end(GTK_BOX(panned),color,TRUE,TRUE,0);
		g_object_set_data(G_OBJECT(color),"colorid",(gpointer) -1);
		g_signal_connect(G_OBJECT(color),"color-changed",G_CALLBACK(color_changed),widget);
	}

	// Tree view with all available colors
	{
		GtkTreeModel		* model		= (GtkTreeModel *) gtk_tree_store_new(2,G_TYPE_STRING,G_TYPE_INT);
		GtkWidget			* box;
		GtkTreeIter			  iter;
		GtkTreeIter			  parent;
		GtkTreeSelection	* select;
		int					  f;
		int					  title = 0;

		tree = gtk_tree_view_new_with_model(model);

		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree),FALSE);
		gtk_tree_view_insert_column_with_attributes(	GTK_TREE_VIEW(tree),
														-1,
														"color",gtk_cell_renderer_text_new(),"text",
														0, NULL );

		gtk_tree_store_append((GtkTreeStore *) model,&parent,NULL);
		gtk_tree_store_set((GtkTreeStore *) model, &parent, 0, gettext(node[title++].text), 1, V3270_COLOR_COUNT, -1);

		select = gtk_tree_view_get_selection(GTK_TREE_VIEW (tree));

		g_object_set_data(G_OBJECT(select),"v3270",widget);
		g_object_set_data(G_OBJECT(select),"lastcolors",saved);

		gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
		g_signal_connect(G_OBJECT(select),"changed",G_CALLBACK(color_selected),color);

		for(f=0;f<V3270_COLOR_COUNT;f++)
		{
			saved[f] = *(v3270_get_color(widget,f));

			if(f == node[title].id)
			{
				gtk_tree_store_append((GtkTreeStore *) model,&parent,NULL);
				gtk_tree_store_set((GtkTreeStore *) model, &parent, 0, gettext(node[title++].text), 1, V3270_COLOR_COUNT, -1);
			}
			gtk_tree_store_append((GtkTreeStore *) model,&iter,&parent);
			gtk_tree_store_set((GtkTreeStore *) model, &iter, 0, gettext(color_name[f]), 1, f, -1);
		}

		gtk_tree_view_expand_all(GTK_TREE_VIEW(tree));

		box = gtk_scrolled_window_new(NULL,NULL);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(box),GTK_POLICY_NEVER,GTK_POLICY_ALWAYS);
		gtk_container_add(GTK_CONTAINER(box),tree);
		gtk_box_pack_start(GTK_BOX(panned),box,TRUE,TRUE,0);


	}

	// Run dialog
	gtk_widget_show_all(panned);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),GTK_WIDGET(panned),TRUE,TRUE,2);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		// Accepted, save in configuration file
		int f;
		GString *str = g_string_new("");
		for(f=0;f<V3270_COLOR_COUNT;f++)
		{
			if(f)
				g_string_append_c(str,',');
			g_string_append_printf(str,"%s",gdk_color_to_string(v3270_get_color(widget,f)));
		}
		set_string_to_config("terminal","colors","%s",str->str);
		g_string_free(str,TRUE);
	}
	else
	{
		// Rejected, restore original colors
		int f;

		for(f=0;f<V3270_COLOR_COUNT;f++)
			v3270_set_color(widget,f,saved+f);
	}

	gtk_widget_destroy(dialog);

	// Redraw widget
	v3270_reload(widget);
	gtk_widget_queue_draw(widget);

 }


