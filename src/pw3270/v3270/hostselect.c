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

 #include "hostselect.h"
 #include <lib3270/log.h>
 #include <v3270.h>

/*--[ Widget definition ]----------------------------------------------------------------------------*/

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

 enum _entry
 {
 	ENTRY_HOSTNAME,
 	ENTRY_SRVCNAME,

 	ENTRY_COUNT
 };

 static const gchar *comboLabel[] = { N_("System _type:"), N_("_Color table:")  };

 struct _V3270HostSelectWidget
 {
#if GTK_CHECK_VERSION(3,0,0)
 	GtkBin			  parent;
#else
	GtkVBox			  parent;
#endif // GTK_CHECK_VERSION

	LIB3270_OPTION	  options;								/**< Connect option */

	GtkEntry		* entry[ENTRY_COUNT];					/**< Entry fields for host & service name */
	GtkToggleButton	* ssl;									/**< SSL Connection? */
	GtkComboBox		* combo[G_N_ELEMENTS(comboLabel)];		/**< Model & Color combobox */
	unsigned short	  colors;								/**< Number of colors */
	H3270			* hSession;								/**< lib3270's session handle */

 };

 struct _V3270HostSelectWidgetClass
 {
#if GTK_CHECK_VERSION(3,0,0)
	GtkBinClass parent_class;
#else
	GtkVBoxClass parent_class;
#endif // GTK_CHECK_VERSION
 };


#if GTK_CHECK_VERSION(3,0,0)
 G_DEFINE_TYPE(V3270HostSelectWidget, V3270HostSelectWidget, GTK_TYPE_BIN);
#else
 G_DEFINE_TYPE(V3270HostSelectWidget, V3270HostSelectWidget, GTK_TYPE_VBOX);
#endif // GTK_CHECK_VERSION

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void V3270HostSelectWidget_class_init(V3270HostSelectWidgetClass *klass)
{
#if GTK_CHECK_VERSION(3,0,0)
#else
#endif // GTK_CHECK_VERSION
}

static void toggle_ssl(GtkToggleButton *button, V3270HostSelectWidget *dialog)
{
	if(gtk_toggle_button_get_active(button))
		dialog->options |= LIB3270_OPTION_SSL;
	else
		dialog->options &= ~LIB3270_OPTION_SSL;
}

static void systype_changed(GtkComboBox *widget, V3270HostSelectWidget *dialog)
{
	GValue		value   = { 0, };
	GtkTreeIter iter;

	if(!gtk_combo_box_get_active_iter(widget,&iter))
		return;

	gtk_tree_model_get_value(gtk_combo_box_get_model(widget),&iter,1,&value);

	dialog->options &= ~(LIB3270_OPTION_HOST_TYPE);
	dialog->options |= g_value_get_int(&value);

}

static void colortable_changed(GtkComboBox *widget, V3270HostSelectWidget *dialog)
{
	GValue		value   = { 0, };
	GtkTreeIter iter;

	if(!gtk_combo_box_get_active_iter(widget,&iter))
		return;

	gtk_tree_model_get_value(gtk_combo_box_get_model(widget),&iter,1,&value);

	dialog->colors = g_value_get_int(&value);

}

static void V3270HostSelectWidget_init(V3270HostSelectWidget *widget)
{
#if GTK_CHECK_VERSION(3,0,0)
	GtkGrid 	* grid	= GTK_GRID(gtk_grid_new());
#else
	GtkTable	* grid	= GTK_TABLE(gtk_table_new(3,4,FALSE));
#endif // GTK_CHECK_VERSION

	GtkWidget * label[ENTRY_COUNT] =
	{
		gtk_label_new_with_mnemonic( _( "_Host:" ) ),
		gtk_label_new_with_mnemonic( _( "_Service:" ) )
	};

	int f;

	gtk_container_set_border_width(GTK_CONTAINER(grid),3);

	for(f=0;f<ENTRY_COUNT;f++)
	{
		widget->entry[f] = GTK_ENTRY(gtk_entry_new());
		gtk_misc_set_alignment(GTK_MISC(label[f]),0,0.5);
		gtk_label_set_mnemonic_widget(GTK_LABEL(label[f]),GTK_WIDGET(widget->entry[f]));
	}

	gtk_widget_set_tooltip_text(GTK_WIDGET(widget->entry[ENTRY_HOSTNAME]),_("Address or name of the host to connect.") );
	gtk_widget_set_tooltip_text(GTK_WIDGET(widget->entry[ENTRY_SRVCNAME]),_("Port or service name (empty for \"telnet\").") );

	// SSL checkbox
	widget->ssl = GTK_TOGGLE_BUTTON(gtk_check_button_new_with_mnemonic(_( "_Secure connection." )));
	gtk_widget_set_tooltip_text(GTK_WIDGET(widget->ssl),_( "Check for SSL secure connection." ));
	g_signal_connect(G_OBJECT(widget->ssl),"toggled",G_CALLBACK(toggle_ssl),widget);

	// Extended options
	GtkWidget * expander = gtk_expander_new_with_mnemonic(_( "_Host options"));

	// Host type
	{
		GtkTreeModel    * model		= (GtkTreeModel *) gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_INT);
		GtkCellRenderer * renderer	= gtk_cell_renderer_text_new();

		widget->combo[0] = GTK_COMBO_BOX(gtk_combo_box_new_with_model(model));

		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget->combo[0]), renderer, TRUE);
		gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget->combo[0]), renderer, "text", 0, NULL);

		const LIB3270_OPTION_ENTRY *entry = lib3270_get_option_list();
		for(f=0;entry[f].name != NULL;f++)
		{
			GtkTreeIter iter;
			gtk_list_store_append((GtkListStore *) model,&iter);
			gtk_list_store_set((GtkListStore *) model, &iter, 0, gettext(entry[f].description), 1, entry[f].option, -1);
		}

		g_signal_connect(G_OBJECT(widget->combo[0]),"changed",G_CALLBACK(systype_changed),widget);

	}

	// Color table
	{
		GtkTreeModel    * model		= (GtkTreeModel *) gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_INT);
		GtkCellRenderer * renderer	= gtk_cell_renderer_text_new();

		widget->combo[1] = GTK_COMBO_BOX(gtk_combo_box_new_with_model(model));

		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget->combo[1]), renderer, TRUE);
		gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget->combo[1]), renderer, "text", 0, NULL);

		for(f=0;f<G_N_ELEMENTS(colortable);f++)
		{
			GtkTreeIter iter;
			gtk_list_store_append((GtkListStore *) model,&iter);
			gtk_list_store_set((GtkListStore *) model, &iter, 0, gettext(colortable[f].description), 1, colortable[f].colors, -1);
		}

		g_signal_connect(G_OBJECT(widget->combo[1]),"changed",G_CALLBACK(colortable_changed),widget);

	}

	gtk_entry_set_max_length(widget->entry[ENTRY_HOSTNAME],0xFF);
	gtk_entry_set_width_chars(widget->entry[ENTRY_HOSTNAME],50);

	gtk_entry_set_max_length(widget->entry[ENTRY_SRVCNAME],6);
	gtk_entry_set_width_chars(widget->entry[ENTRY_SRVCNAME],7);

#if GTK_CHECK_VERSION(3,0,0)

	gtk_entry_set_placeholder_text(widget->entry[ENTRY_SRVCNAME],"telnet");

	gtk_widget_set_hexpand(GTK_WIDGET(widget->entry[ENTRY_HOSTNAME]),TRUE);
	gtk_widget_set_hexpand(GTK_WIDGET(widget->ssl),TRUE);
	gtk_widget_set_hexpand(GTK_WIDGET(expander),TRUE);

	gtk_grid_set_row_homogeneous(grid,FALSE);
	gtk_grid_set_column_homogeneous(grid,FALSE);
	gtk_grid_set_column_spacing(grid,5);
	gtk_grid_set_row_spacing(grid,5);

	gtk_grid_attach(grid,label[ENTRY_HOSTNAME],0,0,1,1);
	gtk_grid_attach(grid,GTK_WIDGET(widget->entry[ENTRY_HOSTNAME]),1,0,3,1);

	gtk_grid_attach(grid,label[ENTRY_SRVCNAME],4,0,1,1);
	gtk_grid_attach(grid,GTK_WIDGET(widget->entry[ENTRY_SRVCNAME]),5,0,1,1);

	gtk_grid_attach(grid,GTK_WIDGET(widget->ssl),1,1,3,1);


	// Host options
	{
		GtkGrid *opt = GTK_GRID(gtk_grid_new());
		gtk_grid_set_column_spacing(opt,5);
		gtk_grid_set_row_spacing(opt,5);

		for(f=0;f<G_N_ELEMENTS(comboLabel);f++)
		{
			GtkWidget *label = gtk_label_new_with_mnemonic(gettext(comboLabel[f]));
			gtk_misc_set_alignment(GTK_MISC(label),0,0.5);
			gtk_grid_attach(opt,label,0,f+1,1,1);
			gtk_grid_attach(opt,GTK_WIDGET(widget->combo[f]),1,f+1,2,1);
		}

		gtk_container_add(GTK_CONTAINER(expander),GTK_WIDGET(opt));
	}
	gtk_grid_attach(grid,GTK_WIDGET(expander),1,2,5,2);


#else

	gtk_table_set_row_spacings(grid,5);
	gtk_table_set_col_spacings(grid,5);

	gtk_table_attach(grid,label[ENTRY_HOSTNAME],0,1,0,1,GTK_FILL,GTK_FILL,0,0);
	gtk_table_attach(grid,GTK_WIDGET(widget->entry[ENTRY_HOSTNAME]),1,2,0,1,GTK_EXPAND|GTK_FILL,GTK_EXPAND|GTK_FILL,0,0);

	gtk_table_attach(grid,label[ENTRY_SRVCNAME],2,3,0,1,GTK_FILL,GTK_FILL,0,0);
	gtk_table_attach(grid,GTK_WIDGET(widget->entry[ENTRY_SRVCNAME]),3,4,0,1,GTK_FILL,GTK_FILL,0,0);

	gtk_table_attach(grid,GTK_WIDGET(widget->ssl),1,2,1,2,GTK_FILL,GTK_FILL,0,0);

	{
		GtkTable * opt	= GTK_TABLE(gtk_table_new(G_N_ELEMENTS(comboLabel),2,FALSE));
		gtk_table_set_row_spacings(opt,5);
		gtk_table_set_col_spacings(opt,5);

		for(f=0;f<G_N_ELEMENTS(comboLabel);f++)
		{
			GtkWidget *label = gtk_label_new_with_mnemonic(gettext(comboLabel[f]));
			gtk_misc_set_alignment(GTK_MISC(label),0,0.5);

			gtk_table_attach(opt,label,0,1,f,f+1,GTK_FILL,GTK_FILL,0,0);
			gtk_table_attach(opt,GTK_WIDGET(widget->combo[f]),1,2,f,f+1,GTK_FILL,GTK_FILL,0,0);
		}

		gtk_container_add(GTK_CONTAINER(expander),GTK_WIDGET(opt));
	}
	gtk_table_attach(grid,GTK_WIDGET(expander),1,2,2,3,GTK_FILL,GTK_FILL,0,0);


#endif // GTK_CHECK_VERSION


	gtk_widget_show_all(GTK_WIDGET(grid));
	gtk_container_add(GTK_CONTAINER(widget),GTK_WIDGET(grid));
}

GtkWidget * v3270_host_select_new(GtkWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

	V3270HostSelectWidget * selector = GTK_V3270HostSelectWidget(g_object_new(GTK_TYPE_V3270HostSelectWidget, NULL));

	v3270_host_select_set_session(selector,widget);

	return GTK_WIDGET(selector);
}

void v3270_host_select_set_session(V3270HostSelectWidget *widget, GtkWidget *session)
{
	g_return_if_fail(GTK_IS_V3270(session));
	g_return_if_fail(GTK_IS_V3270HostSelectWidget(widget));

	widget->hSession = v3270_get_session(session);

	gtk_entry_set_text(widget->entry[ENTRY_HOSTNAME],lib3270_get_hostname(widget->hSession));
	gtk_entry_set_text(widget->entry[ENTRY_SRVCNAME],lib3270_get_srvcname(widget->hSession));

	LIB3270_OPTION opt = lib3270_get_options(widget->hSession);

	gtk_toggle_button_set_active(widget->ssl,(opt & LIB3270_OPTION_SSL) != 0);

	// Set host type
	{
		GtkTreeModel	* model = gtk_combo_box_get_model(widget->combo[0]);
		GtkTreeIter		  iter;

		if(gtk_tree_model_get_iter_first(model,&iter))
		{
			do
			{
				GValue		value   = { 0, };

				gtk_tree_model_get_value(model,&iter,1,&value);

				if(g_value_get_int(&value) == (opt&LIB3270_OPTION_HOST_TYPE))
				{
					gtk_combo_box_set_active_iter(widget->combo[0],&iter);
					break;
				}

			} while(gtk_tree_model_iter_next(model,&iter));
		}
	}

	// Set color type
	{
		GtkTreeModel	* model = gtk_combo_box_get_model(widget->combo[1]);
		GtkTreeIter		  iter;
		int				  colors = (int) lib3270_get_color_type(widget->hSession);

		if(gtk_tree_model_get_iter_first(model,&iter))
		{
			do
			{
				GValue		value   = { 0, };

				gtk_tree_model_get_value(model,&iter,1,&value);

				g_message("%d - %d",g_value_get_int(&value),colors);

				if(g_value_get_int(&value) == colors)
				{
					gtk_combo_box_set_active_iter(widget->combo[1],&iter);
					break;
				}

			} while(gtk_tree_model_iter_next(model,&iter));
		}
	}

	// Just in case
	widget->options = opt;
}

LIB3270_EXPORT void v3270_select_host(GtkWidget *widget)
{
	g_return_if_fail(GTK_IS_V3270(widget));

	GtkWidget * dialog	= v3270_host_select_new(widget);
	GtkWidget * win		= gtk_dialog_new_with_buttons(
								_( "Configure host" ),
								GTK_WINDOW(gtk_widget_get_toplevel(widget)),
								GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
								_( "_Cancel" ),		GTK_RESPONSE_REJECT,
								_( "C_onnect" ),	GTK_RESPONSE_ACCEPT,
								NULL	);


	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(win))),dialog,FALSE,FALSE,2);
	gtk_widget_show_all(dialog);

	gboolean again = TRUE;
 	while(again)
 	{
 		gtk_widget_set_sensitive(win,TRUE);

#if GTK_CHECK_VERSION(2,18,0)
		gtk_widget_set_visible(win,TRUE);
#else
		gtk_widget_show(win);
#endif

 		switch(gtk_dialog_run(GTK_DIALOG(win)))
 		{
		case GTK_RESPONSE_ACCEPT:
#if GTK_CHECK_VERSION(2,18,0)
			gtk_widget_set_visible(win,FALSE);
#else
			gtk_widget_hide(win);
#endif
			gtk_widget_set_sensitive(win,FALSE);
			again = v3270_host_select_apply(GTK_V3270HostSelectWidget(dialog)) != 0;
			break;

		case GTK_RESPONSE_REJECT:
			again = FALSE;
			break;
 		}
 	}

	gtk_widget_destroy(win);

}

LIB3270_EXPORT int v3270_host_select_apply(V3270HostSelectWidget *widget)
{
	g_return_val_if_fail(GTK_IS_V3270HostSelectWidget(widget),0);

	lib3270_set_hostname(widget->hSession,gtk_entry_get_text(widget->entry[ENTRY_HOSTNAME]));
	lib3270_set_srvcname(widget->hSession,gtk_entry_get_text(widget->entry[ENTRY_SRVCNAME]));

	lib3270_set_options(widget->hSession,widget->options);

	return lib3270_connect(widget->hSession,0);
}

