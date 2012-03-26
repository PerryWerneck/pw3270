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
 * Este programa está nomeado como dialog.c e possui - linhas de código.
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
 #include "v3270/v3270.h"

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void charset_changed(GtkComboBox *widget,gchar **encoding)
 {
 	gchar *new_encoding = NULL;

#if GTK_CHECK_VERSION(3,0,0)

	new_encoding = g_strdup(gtk_combo_box_get_active_id(GTK_COMBO_BOX(widget)));

#else

	GValue		value	= { 0, };
	GtkTreeIter iter;

	if(!gtk_combo_box_get_active_iter(widget,&iter))
		return;

	gtk_tree_model_get_value(gtk_combo_box_get_model(widget),&iter,1,&value);
	new_encoding = g_strdup(g_value_get_string(&value));

#endif

	if(!new_encoding)
		return;

	trace("%s: %s->%s",__FUNCTION__,*encoding,new_encoding);
	if(*encoding)
		g_free(*encoding);

	*encoding = new_encoding;
 }

 static void add_option_menus(GtkWidget *widget, GtkAction *action, gchar **encoding)
 {
 	GtkWidget	*box = gtk_hbox_new(FALSE,6);
 	gchar		*ptr = g_object_get_data(G_OBJECT(action),"charset");

	if(ptr)
	{
		*encoding = g_strdup(ptr);
	}
	else
	{
		// Add charset options
		static const struct _list
		{
			const gchar *charset;
			const gchar *text;
		} list[] =
		{
			// http://en.wikipedia.org/wiki/Character_encoding
			{ "UTF-8",		N_( "UTF-8"	)								},
			{ "ISO-8859-1", N_( "Western Europe (ISO 8859-1)" ) 		},
			{ "CP1252",		N_( "Windows Western languages (CP1252)" )	},

			{ NULL, NULL }
		};

		GtkWidget		* label 	= gtk_label_new_with_mnemonic (_("C_haracter Coding:"));
		const gchar		* charset	= NULL;
#if GTK_CHECK_VERSION(3,0,0)
		GtkWidget		* menu		= gtk_combo_box_text_new();
#else
		GtkTreeModel	* model		= (GtkTreeModel *) gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_STRING);
		GtkWidget		* menu		= gtk_combo_box_new_with_model(model);
		GtkCellRenderer * renderer	= gtk_cell_renderer_text_new();
		GtkTreeIter		  iter;
#endif // GTK(3,0,0)
		gchar			* text;
		int			  f;
		int			  p = 0;

		g_get_charset(&charset);
		*encoding = g_strdup(charset);

		text = g_strdup_printf(_("Current (%s)"),charset);

#if GTK_CHECK_VERSION(3,0,0)

		gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(menu),p,charset,text);

#else

		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(menu), renderer, TRUE);
		gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(menu), renderer, "text", 0, NULL);

		gtk_list_store_append((GtkListStore *) model,&iter);
		gtk_list_store_set((GtkListStore *) model, &iter, 0, text, 1, charset, -1);

#endif // GTK(3,0,0)

		g_free(text);

		gtk_combo_box_set_active(GTK_COMBO_BOX(menu),p++);

		for(f=0;list[f].charset;f++)
		{
			if(strcasecmp(charset,list[f].charset))
			{
#if GTK_CHECK_VERSION(3,0,0)
				gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(menu),p++,list[f].charset,gettext(list[f].text));
#else
				gtk_list_store_append((GtkListStore *) model,&iter);
				gtk_list_store_set((GtkListStore *) model, &iter, 0, gettext(list[f].text), 1, list[f].charset, -1);
#endif // GTK(3,0,0)
			}
		}


		gtk_label_set_mnemonic_widget(GTK_LABEL(label), menu);

		gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,0);

		gtk_box_pack_start(GTK_BOX(box),menu,TRUE,TRUE,0);

		g_signal_connect(G_OBJECT(menu),"changed",G_CALLBACK(charset_changed),encoding);

	}


	gtk_widget_show_all(box);
	gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(widget),box);

}

 static void save_text(GtkWindow *toplevel,const gchar *filename, const gchar *text, const gchar *encoding, const gchar *errmsg)
 {
	GError * error = NULL;

	if(g_strcasecmp(encoding,"UTF-8"))
	{
		// Convert to target charset and save
		gsize	  bytes_written;
		gchar	* converted = g_convert_with_fallback(text,-1,encoding,"UTF-8",NULL,NULL,&bytes_written,&error);

		if(!error)
			g_file_set_contents(filename,converted,-1,&error);

		g_free(converted);
	}
	else
	{
		// Same charset, save file
		g_file_set_contents(filename,text,-1,&error);
	}

	if(error)
	{
		GtkWidget *popup = gtk_message_dialog_new_with_markup(
											toplevel,
											GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,
											gettext(errmsg),filename);

		gtk_window_set_title(GTK_WINDOW(popup),_("Can´t save file"));

		gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(popup),"%s",error->message);
		g_error_free(error);

		gtk_dialog_run(GTK_DIALOG(popup));
		gtk_widget_destroy(popup);

	}

 }

 static GtkFileChooserConfirmation confirm_overwrite(GtkFileChooser *chooser, GtkAction *action)
 {
	const gchar					* attr		= g_object_get_data(G_OBJECT(action),"overwrite");
	GtkFileChooserConfirmation	  ret 		= GTK_FILE_CHOOSER_CONFIRMATION_ACCEPT_FILENAME;
	GtkWidget					* dialog;

	if(attr && !g_strcasecmp(attr,"yes"))
		return ret;

	dialog = gtk_message_dialog_new_with_markup(	GTK_WINDOW(chooser),
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_QUESTION,GTK_BUTTONS_OK_CANCEL,
													"%s",_("The file already exists. Replace it?"));


	if(gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_OK)
		ret = GTK_FILE_CHOOSER_CONFIRMATION_SELECT_AGAIN;

	gtk_widget_destroy(dialog);

	return ret;

 }

 static int save_dialog(GtkAction *action, GtkWidget *widget, const gchar *title, const gchar *errmsg, const gchar *text)
 {
 	GtkWindow	* toplevel		= GTK_WINDOW(gtk_widget_get_toplevel(widget));
 	const gchar * user_title	= g_object_get_data(G_OBJECT(action),"title");
 	const gchar * filename		= g_object_get_data(G_OBJECT(action),"filename");

	if(!text)
		return 0;

	if(filename)
	{
		save_text(toplevel,filename,text,g_object_get_data(G_OBJECT(action),"encoding"),errmsg);
	}
	else
	{
		GtkWidget	* dialog;
		gchar		* ptr;
		gchar 		* encattr		= NULL;

		dialog = gtk_file_chooser_dialog_new( 	gettext(user_title ? user_title : title),
												toplevel,
												GTK_FILE_CHOOSER_ACTION_SAVE,
												GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
												GTK_STOCK_SAVE,		GTK_RESPONSE_ACCEPT,
												NULL );

		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
		g_signal_connect(GTK_FILE_CHOOSER(dialog), "confirm-overwrite", G_CALLBACK(confirm_overwrite), action);

		add_option_menus(dialog, action, &encattr);

		ptr = get_string_from_config("save",gtk_action_get_name(action),"");
		if(*ptr)
			gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),ptr);
		g_free(ptr);

		if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
		{
			ptr = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
			if(ptr)
			{
				trace("Saving \"%s\"",ptr);
				set_string_to_config("save",gtk_action_get_name(action),"%s",ptr);
				save_text(toplevel,ptr,text,encattr,errmsg);
				g_free(ptr);
			}
		}

		if(encattr)
			g_free(encattr);

		gtk_widget_destroy(dialog);
	}

	return 0;
 }

 void hostname_action(GtkAction *action, GtkWidget *widget)
 {
 	const gchar 	* title 	= g_object_get_data(G_OBJECT(action),"title");
 	gchar			* cfghost	= get_string_from_config("host","uri","");
 	gchar			* hostname;
 	gchar			* ptr;
 	gboolean		  again		= TRUE;
 	GtkTable		* table		= GTK_TABLE(gtk_table_new(2,4,FALSE));
 	GtkEntry		* host		= GTK_ENTRY(gtk_entry_new());
 	GtkEntry		* port		= GTK_ENTRY(gtk_entry_new());
 	GtkToggleButton	* checkbox	= GTK_TOGGLE_BUTTON(gtk_check_button_new_with_label( _( "Secure connection" ) ));
 	GtkWidget 		* dialog 	= gtk_dialog_new_with_buttons(	gettext(title ? title : N_( "Select hostname" )),
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

	gtk_table_attach(table,gtk_label_new( _( "Hostname:" ) ), 0,1,0,1,0,0,5,0);
	gtk_table_attach(table,GTK_WIDGET(host), 1,2,0,1,GTK_EXPAND|GTK_FILL,0,0,0);

	gtk_table_attach(table,gtk_label_new( _( "Port:" ) ), 2,3,0,1,0,0,5,0);
	gtk_table_attach(table,GTK_WIDGET(port), 3,4,0,1,GTK_FILL,0,0,0);

	gtk_table_attach(table,GTK_WIDGET(checkbox), 1,2,1,2,GTK_EXPAND|GTK_FILL,0,0,0);

	gtk_container_set_border_width(GTK_CONTAINER(table),5);

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),GTK_WIDGET(table),FALSE,FALSE,2);

	hostname = cfghost;

#ifdef HAVE_LIBSSL
	if(!strncmp(hostname,"L:",2))
	{
		gtk_toggle_button_set_active(checkbox,TRUE);
		hostname += 2;
	}
#else
	gtk_toggle_button_set_active(checkbox,FALSE);
	gtk_widget_set_sensitive(GTK_WIDGET(checkbox),FALSE);
	if(!strncmp(hostname,"L:",2))
		hostname += 2;
#endif

	ptr = strchr(hostname,':');
	if(ptr)
	{
		*(ptr++) = 0;
		gtk_entry_set_text(port,ptr);
	}
	else
	{
		gtk_entry_set_text(port,"23");
	}

	gtk_entry_set_text(host,hostname);

	gtk_widget_show_all(GTK_WIDGET(table));

 	while(again)
 	{
 		gtk_widget_set_sensitive(dialog,TRUE);
 		switch(gtk_dialog_run(GTK_DIALOG(dialog)))
 		{
		case GTK_RESPONSE_ACCEPT:
			gtk_widget_set_sensitive(dialog,FALSE);

			hostname = g_strconcat(	gtk_toggle_button_get_active(checkbox) ? "L:" : "",
									gtk_entry_get_text(host),
									":",
									gtk_entry_get_text(port),
									NULL
								);

			if(!lib3270_connect(v3270_get_session(widget),hostname,1))
			{
				// Connection OK
				set_string_to_config("host","uri","%s",hostname);
				again = FALSE;
			}

			g_free(hostname);
			break;

		case GTK_RESPONSE_REJECT:
			again = FALSE;
			break;
 		}
 	}

	gtk_widget_destroy(dialog);

	g_free(cfghost);
 }

 void save_all_action(GtkAction *action, GtkWidget *widget)
 {
 	gchar *text = v3270_get_text(widget);

	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);

	if(!text)
		return;

	save_dialog(	action,
					widget,
					N_( "Save screen to file" ),
					N_( "Can't save screen to file\n%s" ),
					text);

	g_free(text);

 }

 void save_selected_action(GtkAction *action, GtkWidget *widget)
 {
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);

	save_dialog(	action,
					widget,
					N_( "Save selection to file" ),
					N_( "Can't save selection to file\n%s" ),
					v3270_get_selected_text(widget));
 }

 void save_copy_action(GtkAction *action, GtkWidget *widget)
 {
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);

	save_dialog(	action,
					widget,
					N_( "Save copy to file" ),
					N_( "Can't save copy to file\n%s" ),
					v3270_get_copy(widget));
 }

 static void paste_filename(GtkWidget *widget, const gchar *filename, const gchar *encoding)
 {
	GError *error = NULL;
	gchar *text = NULL;

	if(!encoding)
		g_get_charset(&encoding);

	trace("Loading \"%s\" encoding=%s",filename,encoding);

	if(!g_file_get_contents(filename,&text,NULL,&error))
	{
		GtkWidget *popup = gtk_message_dialog_new_with_markup(
											GTK_WINDOW(gtk_widget_get_toplevel(widget)),
											GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,
											_( "Error loading %s" ),filename);

		gtk_window_set_title(GTK_WINDOW(popup),_("Can´t load file"));

		gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(popup),"%s",error->message);
		g_error_free(error);

		gtk_dialog_run(GTK_DIALOG(popup));
		gtk_widget_destroy(popup);

	}

	if(text)
	{
		v3270_paste_string(widget,text,encoding);
		g_free(text);
	}

 }

 void paste_file_action(GtkAction *action, GtkWidget *widget)
 {
 	const gchar * user_title	= g_object_get_data(G_OBJECT(action),"title");
 	const gchar * filename		= g_object_get_data(G_OBJECT(action),"filename");
 	gchar 		* encattr		= NULL;
	GtkWidget	* dialog;
	gchar		* ptr;

	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);

	if(filename)
	{
		ptr = g_object_get_data(G_OBJECT(action),"charset");
		paste_filename(widget,filename,ptr);
		return;
	}

	dialog = gtk_file_chooser_dialog_new( 	gettext(user_title ? user_title : N_( "Paste text file contents" )),
											GTK_WINDOW(gtk_widget_get_toplevel(widget)),
											GTK_FILE_CHOOSER_ACTION_OPEN,
											GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
											GTK_STOCK_OPEN,		GTK_RESPONSE_ACCEPT,
											NULL );

	add_option_menus(dialog, action, &encattr);

	ptr = get_string_from_config("load",gtk_action_get_name(action),"");
	if(*ptr)
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),ptr);
	g_free(ptr);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		ptr = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

		if(ptr)
		{
			set_string_to_config("load",gtk_action_get_name(action),"%s",ptr);
			paste_filename(widget,ptr,encattr);
			g_free(ptr);
		}
	}
	gtk_widget_destroy(dialog);

	if(encattr)
		g_free(encattr);
 }

