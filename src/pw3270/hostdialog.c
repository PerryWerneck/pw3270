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
 * Este programa está nomeado como hostdialog.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "globals.h"
 #include <pw3270/v3270.h>

/*--[ Globals ]--------------------------------------------------------------------------------------*/

/*
 static const struct _host_type
 {
	const gchar		* name;
	const gchar		* description;
	LIB3270_OPTION	  option;
 } host_type[] =
 {
	{ "S390",		N_( "IBM S/390"			),	LIB3270_OPTION_S390			},
	{ "AS400",		N_( "IBM AS/400"		),	LIB3270_OPTION_AS400		},
	{ "TSO",		N_( "Other (TSO)"		),	LIB3270_OPTION_TSO			},
	{ "VM/CMS",		N_( "Other (VM/CMS)"	),	0 							}
 };

 static const struct _colortable
 {
	unsigned short 	  colors;
	const gchar		* description;
 } colortable[] =
 {
	{ 16,	N_( "16 colors"  )	},
	{ 8,	N_( "8 colors"	 )	},
	{ 2,	N_( "Monochrome" )	},
 };
*/

/*--[ Implement ]------------------------------------------------------------------------------------*/

/*
 LIB3270_OPTION pw3270_options_by_hosttype(const gchar *systype)
 {
	int f;

	for(f=0;G_N_ELEMENTS(host_type);f++)
	{
		if(!g_ascii_strcasecmp(host_type[f].name,systype))
			return host_type[f].option;
	}

	g_message("Unexpected system type: \"%s\"",systype);
	return 0;
 }

#if GTK_CHECK_VERSION(3,0,0)
 static void set_row(int row, GtkWidget *widget, GtkGrid *container, const gchar *text)
 {
	GtkWidget *label = gtk_label_new_with_mnemonic(text);

	gtk_misc_set_alignment(GTK_MISC(label),0,0.5);

	gtk_grid_attach(container,label,0,row,1,1);
	gtk_grid_attach(container,widget,1,row,1,1);

	gtk_label_set_mnemonic_widget(GTK_LABEL(label),widget);
 }
#else
 static void set_row(int row, GtkWidget *widget, GtkTable *container, const gchar *text)
 {
	GtkWidget *label = gtk_label_new_with_mnemonic(text);

	gtk_misc_set_alignment(GTK_MISC(label),0,0.5);

	gtk_table_attach(container,label,0,1,row,row+1,GTK_FILL,0,5,0);
	gtk_table_attach(container,widget,1,2,row,row+1,GTK_FILL,0,0,0);

	gtk_label_set_mnemonic_widget(GTK_LABEL(label),widget);
 }
#endif // GTK_CHECK_VERSION

 static void systype_changed(GtkComboBox *widget, int *iHostType)
 {
	GValue          value   = { 0, };
	GtkTreeIter iter;

	if(!gtk_combo_box_get_active_iter(widget,&iter))
		return;

	gtk_tree_model_get_value(gtk_combo_box_get_model(widget),&iter,1,&value);

	*iHostType = g_value_get_int(&value);

	trace("Selected host type: %s",host_type[*iHostType].name);

 }

 static void color_changed(GtkComboBox *widget, int *iColorTable)
 {
	GValue          value   = { 0, };
	GtkTreeIter iter;

	if(!gtk_combo_box_get_active_iter(widget,&iter))
		return;

	gtk_tree_model_get_value(gtk_combo_box_get_model(widget),&iter,1,&value);

	*iColorTable = g_value_get_int(&value);

	trace("Selected color type: %d",(int) colortable[*iColorTable].colors);

 }
*/
 void hostname_action(GtkAction *action, GtkWidget *widget)
 {
 	H3270 * hSession = v3270_get_session(widget);
 	gchar * ptr;

 	lib3270_set_color_type(hSession,(unsigned short) get_integer_from_config("host","colortype",16));

	ptr = get_string_from_config("host","systype","s390");
	if(*ptr)
		lib3270_set_host_type(hSession,ptr);
	g_free(ptr);

	v3270_select_host(widget);

/*
 	const gchar 	* title			= g_object_get_data(G_OBJECT(action),"title");
 	gchar			* ptr;
 	gboolean		  again			= TRUE;
 	int				  iHostType 	= 0;
 	int				  iColorTable	= 0;
#if GTK_CHECK_VERSION(3,0,0)
	GtkGrid			* grid			= GTK_GRID(gtk_grid_new());
#else
 	GtkTable		* table			= GTK_TABLE(gtk_table_new(3,4,FALSE));
#endif // GTK_CHECK_VERSION
 	GtkEntry		* host			= GTK_ENTRY(gtk_entry_new());
 	GtkEntry		* port			= GTK_ENTRY(gtk_entry_new());
 	GtkToggleButton	* sslcheck		= GTK_TOGGLE_BUTTON(gtk_check_button_new_with_mnemonic( _( "_Secure connection" ) ));
	GtkWidget 		* dialog 		= gtk_dialog_new_with_buttons(
											gettext(title ? title : N_( "Select hostname" )),
											GTK_WINDOW(gtk_widget_get_toplevel(widget)),
											GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_STOCK_CONNECT,	GTK_RESPONSE_ACCEPT,
											GTK_STOCK_CANCEL,	GTK_RESPONSE_REJECT,
											NULL	);

	gtk_window_set_icon_name(GTK_WINDOW(dialog),GTK_STOCK_HOME);
	gtk_entry_set_max_length(host,0xFF);
	gtk_entry_set_width_chars(host,50);

	gtk_entry_set_max_length(port,6);
	gtk_entry_set_width_chars(port,7);


#if GTK_CHECK_VERSION(3,0,0)

	gtk_grid_set_column_spacing(grid,5);
	gtk_grid_set_row_spacing(grid,3);

	{
		// Host info - GtkGrid version
		struct _line
		{
			const gchar			* label;
			GtkWidget			* widget;
		} line[] =
		{
			{ N_( "_Hostname:" ),	GTK_WIDGET(host) },
			{ N_( "_Port:" ),		GTK_WIDGET(port) }
		};

		int f;
		int c = 0;

		for(f=0;f<G_N_ELEMENTS(line);f++)
		{
			GtkWidget * label = gtk_label_new_with_mnemonic( gettext(line[f].label) );
			gtk_label_set_mnemonic_widget(GTK_LABEL(label),line[f].widget);

			gtk_grid_attach(grid,label,c,0,1,1);
			gtk_grid_attach(grid,line[f].widget,c+1,0,3,1);
			c += 4;
		}

		gtk_grid_attach_next_to(grid,GTK_WIDGET(sslcheck),GTK_WIDGET(host),GTK_POS_BOTTOM,1,1);

	}
#else
	{
		// Host info - GtkTable version
		struct _line
		{
			const gchar			* label;
			GtkWidget			* widget;
			GtkAttachOptions	  xoptions;
		} line[] =
		{
			{ N_( "_Hostname:" ),	GTK_WIDGET(host), 	GTK_EXPAND|GTK_FILL },
			{ N_( "_Port:" ),		GTK_WIDGET(port),	GTK_FILL			}
		};

		int f;

		for(f=0;f<G_N_ELEMENTS(line);f++)
		{
			int col = f*3;

			GtkWidget * label = gtk_label_new_with_mnemonic( gettext(line[f].label) );
			gtk_label_set_mnemonic_widget(GTK_LABEL(label),line[f].widget);
			gtk_table_attach(table,label,col,col+1,0,1,0,0,2,2);
			gtk_table_attach(table,line[f].widget,col+1,col+2,0,1,line[f].xoptions,0,2,2);
		}

		gtk_table_attach(table,GTK_WIDGET(sslcheck),1,2,1,2,GTK_EXPAND|GTK_FILL,0,0,0);
	}
#endif // GTK_CHECK_VERSION

	{
		// Host options
		const gchar	* systype 	= g_object_get_data(G_OBJECT(action),"type");
		const gchar	* colortype	= g_object_get_data(G_OBJECT(action),"colors");

		int			  row		= 0;
		GtkWidget	* expander	= gtk_expander_new_with_mnemonic(_( "_Host options"));

#if GTK_CHECK_VERSION(3,0,0)
		GtkGrid		* container	= GTK_GRID(gtk_grid_new());

		gtk_grid_set_column_spacing(container,5);
		gtk_grid_set_row_spacing(container,3);

#else
		GtkTable	* container	= GTK_TABLE(gtk_table_new(3,2,FALSE));
#endif // GTK_CHECK_VERSION


		if(!systype)
		{
			// No system type defined, ask user
			gchar 			* str	 	= get_string_from_config("host","systype",host_type[0].name);
			GtkTreeModel    * model		= (GtkTreeModel *) gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_INT);
			GtkWidget		* widget 	= gtk_combo_box_new_with_model(model);
			GtkCellRenderer * renderer	= gtk_cell_renderer_text_new();

			int				  f;

			gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget), renderer, TRUE);
			gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), renderer, "text", 0, NULL);

			for(f=0;f<G_N_ELEMENTS(host_type);f++)
			{
				GtkTreeIter		  iter;

				gtk_list_store_append((GtkListStore *) model,&iter);
				gtk_list_store_set((GtkListStore *) model, &iter, 0, gettext(host_type[f].description), 1, f, -1);

				if(!g_ascii_strcasecmp(host_type[f].name,str))
					gtk_combo_box_set_active(GTK_COMBO_BOX(widget),iHostType=f);
			}

			g_free(str);

			set_row(row++,widget,container,_( "System _type:" ));

			g_signal_connect(G_OBJECT(widget),"changed",G_CALLBACK(systype_changed),&iHostType);

		}
		else
		{
            int f;

            iHostType = -1;
			for(f=0;f<G_N_ELEMENTS(host_type);f++)
			{
                if(!g_ascii_strcasecmp(systype,host_type[f].name))
                {
                    g_message("Host set to %s (%s) by action property",host_type[f].name,host_type[f].description);
                    iHostType = f;
                    break;
                }
            }

            if(iHostType == -1)
            {
                iHostType = 0;
                g_message("Unexpected host type \"%s\", using defaults",systype);
            }

		}

		if(!colortype)
		{
			// NO colortype defined, ask user
			unsigned short	  colors 	= (unsigned short) get_integer_from_config("host","colortype",16);
			GtkTreeModel    * model		= (GtkTreeModel *) gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_INT);
			GtkWidget		* widget 	= gtk_combo_box_new_with_model(model);
			GtkCellRenderer * renderer	= gtk_cell_renderer_text_new();

			int				  f;

			gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget), renderer, TRUE);
			gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), renderer, "text", 0, NULL);

			for(f=0;f<G_N_ELEMENTS(colortable);f++)
			{
				GtkTreeIter		  iter;

				gtk_list_store_append((GtkListStore *) model,&iter);
				gtk_list_store_set((GtkListStore *) model, &iter, 0, gettext(colortable[f].description), 1, (int) f, -1);

				if(colortable[f].colors == colors)
					gtk_combo_box_set_active(GTK_COMBO_BOX(widget),iColorTable=f);
			}

			set_row(row++,widget,container,_("_Color table:"));

			g_signal_connect(G_OBJECT(widget),"changed",G_CALLBACK(color_changed),&iColorTable);

		}
		else
		{
			#warning TODO: Configurar tabela de cores de acordo com colortype
		}

		gtk_container_add(GTK_CONTAINER(expander),GTK_WIDGET(container));

#if GTK_CHECK_VERSION(3,0,0)
		gtk_grid_attach_next_to(grid,GTK_WIDGET(expander),GTK_WIDGET(sslcheck),GTK_POS_BOTTOM,1,1);
#else
		gtk_table_attach(table,expander,1,2,2,3,GTK_EXPAND|GTK_FILL,0,0,0);
#endif // GTK_CHECK_VERSION

	}

#if GTK_CHECK_VERSION(3,0,0)
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),GTK_WIDGET(grid),FALSE,FALSE,2);
	gtk_widget_show_all(GTK_WIDGET(grid));
	gtk_container_set_border_width(GTK_CONTAINER(grid),5);
#else
	gtk_container_set_border_width(GTK_CONTAINER(table),5);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),GTK_WIDGET(table),FALSE,FALSE,2);
	gtk_widget_show_all(GTK_WIDGET(table));
#endif

	{
		H3270 *hSession = v3270_get_session(widget);
		gchar *uri		= get_string_from_config("host","uri","");

		if(uri && *uri)
			lib3270_set_host(hSession,uri);

		g_free(uri);

		gtk_entry_set_text(host,lib3270_get_hostname(hSession));
		gtk_entry_set_text(port,lib3270_get_srvcname(hSession));
		gtk_toggle_button_set_active(sslcheck,(lib3270_get_options(hSession) & LIB3270_OPTION_SSL) ? TRUE : FALSE);
	}


 	while(again)
 	{
 		gtk_widget_set_sensitive(dialog,TRUE);
 		switch(gtk_dialog_run(GTK_DIALOG(dialog)))
 		{
		case GTK_RESPONSE_ACCEPT:
			gtk_widget_set_sensitive(dialog,FALSE);

#if GTK_CHECK_VERSION(2,18,0)
			gtk_widget_set_visible(dialog,FALSE);
#else
			gtk_widget_hide(dialog);
#endif
			set_string_to_config("host","systype",host_type[iHostType].name);
			set_integer_to_config("host","colortype",colortable[iColorTable].colors);

			v3270_set_session_color_type(widget,colortable[iColorTable].colors);

//			if(!lib3270_connect(v3270_get_session(widget),hostname,1))
			if(!lib3270_connect_host(	v3270_get_session(widget),
										gtk_entry_get_text(host),
										gtk_entry_get_text(port),
										host_type[iHostType].option | (gtk_toggle_button_get_active(sslcheck) ? LIB3270_OPTION_SSL : LIB3270_OPTION_DEFAULTS)))
			{
				again = FALSE;
			}
			else
			{
#if GTK_CHECK_VERSION(2,18,0)
				gtk_widget_set_visible(dialog,TRUE);
#else
				gtk_widget_show(dialog);
#endif
			}

//			g_free(hostname);
			break;

		case GTK_RESPONSE_REJECT:
			again = FALSE;
			break;
 		}
 	}

	gtk_widget_destroy(dialog);

//	g_free(cfghost);
*/
 }

