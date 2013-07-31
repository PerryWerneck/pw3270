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

 //#if defined(DEBUG) && GTK_CHECK_VERSION(3,4,0)
 //   #define USE_GTK_COLOR_CHOOSER 1
 //#endif // GTK_CHECK_VERSION

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void load_color_scheme(GKeyFile *conf, const gchar *group, GdkRGBA *clr)
{
	#define V3270_COLOR_BASE V3270_COLOR_GRAY+1

	const gchar	* val;
	int		  	  f;

	// Load base colors
	val = g_key_file_get_string(conf,group,"base",NULL);
	if(val)
	{
		// Process base colors
		gchar **str = g_strsplit(val,",",V3270_COLOR_BASE);

		switch(g_strv_length(str))
		{
		case 2:	// Only 2 colors, create monocromatic table
			v3270_set_mono_color_table(clr,str[1],str[0]);
			break;

		case V3270_COLOR_BASE:	// All colors, update it
			for(f=0;f<V3270_COLOR_BASE;f++)
				gdk_rgba_parse(clr+f,str[f]);
			break;

		default:

			// Unexpected size, load new colors over the defaults
			g_warning("base color list in %s has %d elements, should have %d",group,g_strv_length(str),V3270_COLOR_GRAY);

			gdk_rgba_parse(clr,str[0]);
			gdk_rgba_parse(clr+1,str[1]);

			for(f=2;f<V3270_COLOR_BASE;f++)
				clr[f] = clr[1];

			clr[V3270_COLOR_BLACK] = *clr;

			for(f=2;f<MIN(g_strv_length(str),V3270_COLOR_BASE-1);f++)
				gdk_rgba_parse(clr+f,str[f]);

		}
		g_strfreev(str);

	}
	else
	{
		g_warning("Color scheme [%s] has no \"base\" entry, using green on black",group);

		gdk_rgba_parse(clr,"black");
		gdk_rgba_parse(clr+1,"green");

		for(f=2;f<V3270_COLOR_BASE;f++)
			clr[f] = clr[1];
		clr[V3270_COLOR_BLACK] = *clr;
	}

	// Load field colors
	clr[V3270_COLOR_FIELD]							= clr[V3270_COLOR_GREEN];
	clr[V3270_COLOR_FIELD_INTENSIFIED]				= clr[V3270_COLOR_RED];
	clr[V3270_COLOR_FIELD_PROTECTED]				= clr[V3270_COLOR_BLUE];
	clr[V3270_COLOR_FIELD_PROTECTED_INTENSIFIED]	= clr[V3270_COLOR_WHITE];

	val = g_key_file_get_string(conf,group,"field",NULL);
	if(val)
	{
		gchar **str = g_strsplit(val,",",5);

		for(f=0;f< MIN(g_strv_length(str),4); f++)
			gdk_rgba_parse(clr+V3270_COLOR_FIELD+f,str[f]);

		g_strfreev(str);
	}

	// Load selection colors
	clr[V3270_COLOR_SELECTED_BG]	= clr[V3270_COLOR_WHITE];
	clr[V3270_COLOR_SELECTED_FG]	= clr[V3270_COLOR_BLACK];
	val = g_key_file_get_string(conf,group,"selection",NULL);
	if(val)
	{
		gchar **str = g_strsplit(val,",",3);

		for(f=0;f< MIN(g_strv_length(str),2); f++)
			gdk_rgba_parse(clr+V3270_COLOR_SELECTED_BG+f,str[f]);

		g_strfreev(str);
	}

	// Load OIA colors
	clr[V3270_COLOR_OIA_BACKGROUND]		= clr[V3270_COLOR_BACKGROUND];
	clr[V3270_COLOR_OIA_FOREGROUND]		= clr[V3270_COLOR_GREEN];
	clr[V3270_COLOR_OIA_SEPARATOR]		= clr[V3270_COLOR_GREEN];
	clr[V3270_COLOR_OIA_STATUS_OK]		= clr[V3270_COLOR_GREEN];
	clr[V3270_COLOR_OIA_STATUS_INVALID]	= clr[V3270_COLOR_RED];
	clr[V3270_COLOR_OIA_STATUS_WARNING]	= clr[V3270_COLOR_YELLOW];

	val = g_key_file_get_string(conf,group,"OIA",NULL);
	if(val)
	{
		gchar **str = g_strsplit(val,",",6);

		// 0 = V3270_COLOR_OIA_BACKGROUND,
		// 1 = V3270_COLOR_OIA_FOREGROUND,
		// 2 = V3270_COLOR_OIA_SEPARATOR,
		// 3 = V3270_COLOR_OIA_STATUS_OK,
		// 4 = V3270_COLOR_OIA_STATUS_WARNING,
		// 5 = V3270_COLOR_OIA_STATUS_INVALID,

		if(g_strv_length(str) == 5)
		{
			for(f=0;f < 5; f++)
				gdk_rgba_parse(clr+V3270_COLOR_OIA_BACKGROUND+f,str[f]);
			clr[V3270_COLOR_OIA_STATUS_INVALID] = clr[V3270_COLOR_OIA_STATUS_WARNING];
		}
		else
		{
			for(f=0;f< MIN(g_strv_length(str),6); f++)
				gdk_rgba_parse(clr+V3270_COLOR_OIA_BACKGROUND+f,str[f]);
		}

		g_strfreev(str);
	}

	// Setup extended elements
	clr[V3270_COLOR_CROSS_HAIR] = clr[V3270_COLOR_GREEN];

	val = g_key_file_get_string(conf,group,"cross-hair",NULL);
	if(val)
		gdk_rgba_parse(clr+V3270_COLOR_CROSS_HAIR,val);

}

 static void color_scheme_changed(GtkComboBox *combo)
 {
	GtkWidget	* terminal	= (GtkWidget *) g_object_get_data(G_OBJECT(combo),"terminal_widget");
	GtkWidget	* colorsel	= (GtkWidget *) g_object_get_data(G_OBJECT(combo),"color_selection_widget");
	GdkRGBA	* clr		= NULL;
	GValue		  value	= { 0, };
	GtkTreeIter	  iter;

	if(!gtk_combo_box_get_active_iter(combo,&iter))
		return;

	gtk_tree_model_get_value(gtk_combo_box_get_model(combo),&iter,1,&value);

	clr = g_value_get_pointer(&value);
	g_object_set_data(G_OBJECT(combo),"selected",clr);

	if(terminal)
	{
		// Update terminal colors
		int f;
		for(f=0;f<V3270_COLOR_COUNT;f++)
			v3270_set_color(terminal,f,clr+f);

		v3270_reload(terminal);
		gtk_widget_queue_draw(terminal);
	}

	if(colorsel)
	{
		// Update color selection widget
		int	id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(colorsel),"colorid"));
		if(id >= 0 && id < V3270_COLOR_COUNT)
        {
#if USE_GTK_COLOR_CHOOSER
			gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(colorsel),clr+id);
#else
			gtk_color_selection_set_current_rgba(GTK_COLOR_SELECTION(colorsel),clr+id);
#endif // GTK_CHECK_VERSION
        }
	}

 }

 static gboolean compare_colors(const GdkRGBA *colora, const GdkRGBA *colorb)
 {
 	int f;

 	for(f=0;f<V3270_COLOR_COUNT;f++)
	{
		if(!gdk_rgba_equal(colora+f,colorb+f))
			return FALSE;
	}

 	return TRUE;
 }

/**
 * Create a color scheme dropdown button
 *
 * @param clr	Pointer to current color table
 *
 * @return Combobox widget with colors.conf loaded and set
 *
 */
 GtkWidget * color_scheme_new(const GdkRGBA *current)
 {
	gchar			* filename	= build_data_filename("colors.conf",NULL);
	GtkTreeModel	* model		= (GtkTreeModel *) gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_POINTER);
	GtkCellRenderer * renderer	= gtk_cell_renderer_text_new();
	GtkWidget 		* widget	= gtk_combo_box_new_with_model(model);
	GtkTreeIter		  iter;

	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), renderer, "text", 0, NULL);

	gtk_widget_set_sensitive(widget,FALSE);

 	if(!g_file_test(filename,G_FILE_TEST_IS_REGULAR))
	{
		g_warning("Unable to load color schemes in \"%s\"",filename);
	}
	else
	{
		GKeyFile	* conf 		= g_key_file_new();
		GError		* err		= NULL;

		g_key_file_load_from_file(conf,filename,G_KEY_FILE_NONE,&err);
		if(err)
		{
			g_warning("Error \"%s\" loading %s",err->message,filename);
			g_error_free(err);
		}
		else
		{
			gsize   	len		= 0;
			gchar		**group = g_key_file_get_groups(conf,&len);
			GdkRGBA	* table	= g_new0(GdkRGBA,(len*V3270_COLOR_COUNT));
			int			  pos	= 0;
			int			  g;
			gboolean 	  found	= FALSE;

			g_signal_connect(G_OBJECT(widget),"changed",G_CALLBACK(color_scheme_changed),0);

			g_object_set_data_full(G_OBJECT(widget),"colortable",table,g_free);

			for(g=0;g<len;g++)
			{
				// Setup colors for current entry
				GdkRGBA	* clr	= table+pos;
				const gchar	* label	= g_key_file_get_locale_string(conf,group[g],"label",NULL,NULL);

				load_color_scheme(conf,group[g],clr);

				// Set it in the combobox
				gtk_list_store_append((GtkListStore *) model,&iter);
				gtk_list_store_set((GtkListStore *) model, &iter,
													0, label ? label : group[g],
													1, clr,
													-1);

				if(compare_colors(clr,current) && current)
				{
					// It's the same color, select iter
					trace("Current color scheme is \"%s\"",group[g]);
					gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&iter);
					found = TRUE;
				}

				// move to next color list
				pos += V3270_COLOR_COUNT;
			}

			g_strfreev(group);

			if(!found)
			{
				// Custom color table, save it as a new dropdown entry.

				GdkRGBA 	* clr = g_new0(GdkRGBA,V3270_COLOR_COUNT);
				int			  f;

				for(f=0;f<V3270_COLOR_COUNT;f++)
					clr[f] = current[f];

				trace("Current color scheme is \"%s\"","custom");

				g_object_set_data_full(G_OBJECT(widget),"customcolortable",clr,g_free);

				gtk_list_store_append((GtkListStore *) model,&iter);
				gtk_list_store_set((GtkListStore *) model, &iter,
													0, _( "Custom colors" ),
													1, clr,
													-1);

				gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&iter);

			}


			gtk_widget_set_sensitive(widget,TRUE);
		}

		g_key_file_free(conf);

	}
	g_free(filename);

	return widget;
 }

#if USE_GTK_COLOR_CHOOSER
 static void color_activated(GtkColorChooser *chooser, GdkRGBA *clr, GtkWidget *widget)
 {
	int id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(chooser),"colorid"));

	if(id < 0 || id >= V3270_COLOR_COUNT)
		return;

    trace("Updating color %d",id);

	v3270_set_color(widget,id,clr);
	v3270_reload(widget);
	gtk_widget_queue_draw(widget);

 }
#else
 static void color_changed(GtkWidget *colorselection, GtkWidget *widget)
 {
 	GdkRGBA	clr;
	int		id		= GPOINTER_TO_INT(g_object_get_data(G_OBJECT(colorselection),"colorid"));

	if(id < 0 || id >= V3270_COLOR_COUNT)
		return;

	gtk_color_selection_get_current_rgba(GTK_COLOR_SELECTION(colorselection),&clr);

	v3270_set_color(widget,id,&clr);
	v3270_reload(widget);
	gtk_widget_queue_draw(widget);
 }
#endif // GTK_CHECK_VERSION

 static void color_selected(GtkTreeSelection *selection, GtkWidget *color)
 {
	GtkWidget		* widget	= g_object_get_data(G_OBJECT(selection),"v3270");
#if ! USE_GTK_COLOR_CHOOSER
	GdkRGBA		* saved		= g_object_get_data(G_OBJECT(selection),"lastcolors");
#endif // !GTK(3,4,0)
	GValue			  value		= { 0, };
	GtkTreeModel	* model;
	GtkTreeIter		  iter;
	GdkRGBA		* clr;
	int				  id;

	gtk_widget_set_sensitive(color,FALSE);

	if(!gtk_tree_selection_get_selected(selection,&model,&iter))
		return;

	gtk_tree_model_get_value(model,&iter,1,&value);

	id = g_value_get_int(&value);

	if(id < 0 || id >= V3270_COLOR_COUNT)
		return;

	g_object_set_data(G_OBJECT(color),"colorid",GINT_TO_POINTER(id));
	clr = v3270_get_color(widget,id);

#if USE_GTK_COLOR_CHOOSER
    {
        GValue value;

        gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(color),clr);

        g_value_init(&value, G_TYPE_BOOLEAN);
        g_value_set_boolean(&value,FALSE);
        g_object_set_property(G_OBJECT(color),"show-editor",&value);
    }
#else
	gtk_color_selection_set_previous_rgba(GTK_COLOR_SELECTION(color),saved+id);
	gtk_color_selection_set_current_rgba(GTK_COLOR_SELECTION(color),clr);
#endif // GTK_CHECK_VERSION

	gtk_widget_set_sensitive(color,TRUE);
 }

 void editcolors_action(GtkAction *action, GtkWidget *widget)
 {
// 	static const gchar *custom = N_( "Custom colors" );

 	static const struct _node
 	{
 		int			id;
 		const char	*text;
 	} node[] =
 	{
 		{ V3270_COLOR_BACKGROUND,		N_( "Terminal colors" )		},
 		{ V3270_COLOR_FIELD,			N_( "Field colors" )		},
 		{ V3270_COLOR_SELECTED_BG,		N_( "Misc colors" )			},
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

		N_( "Cross hair cursor" ),			// TERMINAL_COLOR_CROSS_HAIR

		// Oia Colors
		N_( "OIA background" ),				// TERMINAL_COLOR_OIA_BACKGROUND
		N_( "OIA foreground" ),				// TERMINAL_COLOR_OIA_FOREGROUND
		N_( "OIA separator" ),				// TERMINAL_COLOR_OIA_SEPARATOR
		N_( "OIA status ok" ),				// TERMINAL_COLOR_OIA_STATUS_OK
		N_( "OIA Warning"	),				// V3270_COLOR_OIA_STATUS_WARNING
		N_( "OIA status invalid" ),			// TERMINAL_COLOR_OIA_STATUS_INVALID

	};

 	const gchar * title  = g_object_get_data(G_OBJECT(action),"title");
	GtkWidget	* dialog = gtk_dialog_new_with_buttons (	gettext(title ? title : N_( "Color setup") ),
															GTK_WINDOW(gtk_widget_get_toplevel(widget)),
															GTK_DIALOG_DESTROY_WITH_PARENT,
															GTK_STOCK_OK,		GTK_RESPONSE_ACCEPT,
															GTK_STOCK_CANCEL,	GTK_RESPONSE_REJECT,
															NULL );
#if GTK_CHECK_VERSION(3,0,0)
	GtkWidget	* panned = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,2);
#else
	GtkWidget	* panned = gtk_hbox_new(FALSE,2);
#endif // GTK(3,0,0)

	GtkWidget	* tree;
	GtkWidget	* color;
	GdkRGBA	  saved[V3270_COLOR_COUNT];

	// Color dialog setup
#if USE_GTK_COLOR_CHOOSER
    color = gtk_color_chooser_widget_new();
    gtk_widget_set_sensitive(color,0);
    g_signal_connect(G_OBJECT(color),"color-activated",G_CALLBACK(color_activated),widget);
#else
    color = gtk_color_selection_new();
    gtk_widget_set_sensitive(color,0);
    gtk_color_selection_set_has_opacity_control(GTK_COLOR_SELECTION(color),FALSE);
    gtk_color_selection_set_has_palette(GTK_COLOR_SELECTION(color),TRUE);
    g_signal_connect(G_OBJECT(color),"color-changed",G_CALLBACK(color_changed),widget);
#endif // GTK_CHECK_VERSION

    gtk_box_pack_end(GTK_BOX(panned),color,TRUE,TRUE,0);
    g_object_set_data(G_OBJECT(color),"colorid",(gpointer) -1);

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

	gtk_widget_show_all(panned);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),GTK_WIDGET(panned),TRUE,TRUE,2);

	// Color scheme combo
	{
#if GTK_CHECK_VERSION(3,0,0)
		GtkWidget * box		= gtk_box_new(GTK_ORIENTATION_HORIZONTAL,2);
#else
		GtkWidget * box		= gtk_hbox_new(FALSE,2);
#endif // GTK(3,0,0)
		GtkWidget * button	= color_scheme_new(v3270_get_color_table(widget));

		g_object_set_data(G_OBJECT(button),"terminal_widget",widget);
		g_object_set_data(G_OBJECT(button),"color_selection_widget",color);

		gtk_box_pack_start(GTK_BOX(box),gtk_label_new(_("Color scheme:")),FALSE,FALSE,2);
		gtk_box_pack_start(GTK_BOX(box),button,TRUE,TRUE,2);
		gtk_widget_show_all(box);
		gtk_box_pack_end(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),box,FALSE,FALSE,2);
	}

	// Run dialog

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		// Accepted, save in configuration file
		int f;
		GString *str = g_string_new("");
		for(f=0;f<V3270_COLOR_COUNT;f++)
		{
			if(f)
				g_string_append_c(str,',');
			g_string_append_printf(str,"%s",gdk_rgba_to_string(v3270_get_color(widget,f)));
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


