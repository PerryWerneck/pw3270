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

 static int save_dialog(GtkAction *action, GtkWidget *widget, const gchar *title, const gchar *errmsg, gchar *text)
 {
 	GtkWindow	* toplevel		= GTK_WINDOW(gtk_widget_get_toplevel(widget));
 	const gchar * user_title	= g_object_get_data(G_OBJECT(action),"title");

	GtkWidget *dialog = gtk_file_chooser_dialog_new( gettext(user_title ? user_title : title),
                                                     toplevel,
                                                     GTK_FILE_CHOOSER_ACTION_SAVE,
                                                     GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
                                                     GTK_STOCK_SAVE,	GTK_RESPONSE_ACCEPT,
                                                     NULL );


	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		GError	*error 		= NULL;
		gchar	*filename	= gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

		if(!g_file_set_contents(filename,text,-1,&error))
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

		g_free(filename);
	}

	gtk_widget_destroy(dialog);
 	return 0;
 }


 void paste_file_action(GtkAction *action, GtkWidget *widget)
 {
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);

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
 	GtkWidget 		* dialog 	= gtk_dialog_new_with_buttons(	gettext(title ? title : "Select hostname"),
																GTK_WINDOW(gtk_widget_get_toplevel(widget)),
																GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
																GTK_STOCK_CONNECT,	GTK_RESPONSE_ACCEPT,
																GTK_STOCK_CANCEL,	GTK_RESPONSE_REJECT,
																NULL	);

	gtk_window_set_icon_name(GTK_WINDOW(dialog),GTK_STOCK_HOME);
	gtk_entry_set_max_length(host,0xFF);
	gtk_entry_set_width_chars(host,60);

	gtk_entry_set_max_length(port,6);
	gtk_entry_set_width_chars(port,7);

	gtk_table_attach(table,gtk_label_new( _( "Hostname:" ) ), 0,1,0,1,0,0,5,0);
	gtk_table_attach(table,GTK_WIDGET(host), 1,2,0,1,GTK_EXPAND|GTK_FILL,0,0,0);

	gtk_table_attach(table,gtk_label_new( _( "Port:" ) ), 2,3,0,1,0,0,5,0);
	gtk_table_attach(table,GTK_WIDGET(port), 3,4,0,1,GTK_FILL,0,0,0);

	gtk_table_attach(table,GTK_WIDGET(checkbox), 1,2,1,2,GTK_EXPAND|GTK_FILL,0,0,0);

	gtk_container_set_border_width(GTK_CONTAINER(table),5);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),GTK_WIDGET(table),FALSE,FALSE,2);

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

			#warning Work in progress

/*
			gtk_widget_set_sensitive(dialog,FALSE);

			if(gtk_toggle_button_get_active(checkbox))
				strcpy(buffer,"L:");
			else
				*buffer = 0;

			strncat(buffer,gtk_entry_get_text(host),1023);
			strncat(buffer,":",1023);
			strncat(buffer,gtk_entry_get_text(port),1023);

			if(!lib3270_connect(GTK_V3270(widget)->host,host,1))
			{
				// Connection OK
				again = FALSE;
				set_string_to_config("host","uri","%s",buffer);
				SetString("Network","Hostname",buffer);
			}
*/
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
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);

 }

 void save_selected_action(GtkAction *action, GtkWidget *widget)
 {
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);

 }

 void save_copy_action(GtkAction *action, GtkWidget *widget)
 {
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);

 }
