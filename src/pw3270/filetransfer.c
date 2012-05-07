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
 * Este programa está nomeado como filetransfer.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 * Agradecimento:
 *
 * Roberto Soares 			(a_r_soares@hotmail.com)
 *
 */

#include "globals.h"
#include "uiparser/parser.h"
#include "filetransfer.h"

/*--[ FT dialog ]------------------------------------------------------------------------------------*/

 struct ftdialog
 {
 	LIB3270_FT_OPTION	  option;
 	const gchar			* name;
 	GtkWidget			* dialog;
 	GtkEntry			* file[2];
 	GtkEntry			* dft;
 };

 struct ftoption
 {
	unsigned int	  flag;
	const gchar		* name;
	const gchar		* label;
 };

/*--[ Implement ]------------------------------------------------------------------------------------*/


static void error_dialog(GtkWidget *widget, const gchar *title, const gchar *msg, const gchar *text)
{
	GtkWidget *popup = gtk_message_dialog_new_with_markup(
								GTK_WINDOW(gtk_widget_get_toplevel(widget)),
								GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
								GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,
								"%s",msg);

	gtk_window_set_title(GTK_WINDOW(popup),title);

	if(text)
		gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(popup),"%s",text);

	gtk_dialog_run(GTK_DIALOG(popup));
	gtk_widget_destroy(popup);
}

static void begin_ft_session(GtkAction *action, GtkWidget *widget, LIB3270_FT_OPTION opt)
{
	// Create file-transfer options dialog

}

static void browse_file(GtkButton *button,struct ftdialog *dlg)
{
	int 		  recv = dlg->option&LIB3270_FT_OPTION_RECEIVE;
	gchar		* ptr;
	GtkWidget 	* dialog = gtk_file_chooser_dialog_new(	recv ? _( "Select file to receive" ) : _( "Select file to send" ),
														GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button))),
														GTK_FILE_CHOOSER_ACTION_OPEN,
														GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
														recv ? GTK_STOCK_SAVE : GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
														NULL );

	ptr = get_string_from_config(dlg->name,"uri","");
	if(*ptr)
		gtk_file_chooser_set_uri(GTK_FILE_CHOOSER(dialog),ptr);
	g_free(ptr);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		gchar *uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));
		gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		set_string_to_config(dlg->name,"uri",uri);
		gtk_entry_set_text(GTK_ENTRY(dlg->file[0]),filename);
		g_free(uri);
		g_free(filename);
	}

	gtk_widget_destroy(dialog);

}

static void add_file_fields(GObject *action, struct ftdialog *dlg)
{
	static const gchar	* label[]	= { N_( "_Local file name:" ), N_( "_Host file name:" ) };
	static const gchar	* attr[]	= { "local", "remote" };
	GtkTable			* table		= GTK_TABLE(gtk_table_new(2,3,FALSE));
	GtkWidget			* widget;
	int					  f;

	for(f=0;f<2;f++)
	{
		const gchar	*val;

		widget = gtk_label_new_with_mnemonic(gettext(label[f]));

		gtk_misc_set_alignment(GTK_MISC(widget),0,.5);
		gtk_table_attach(GTK_TABLE(table),widget,0,1,f,f+1,GTK_FILL,GTK_FILL,2,2);

		dlg->file[f] = GTK_ENTRY(gtk_entry_new());

		gtk_widget_set_name(GTK_WIDGET(dlg->file[f]),attr[f]);

		val = g_object_get_data(action,attr[f]);

		if(val)
		{
			gtk_entry_set_text(dlg->file[f],val);
		}
		else
		{
			gchar *name =  get_string_from_config(dlg->name,attr[f],"");
			gtk_entry_set_text(dlg->file[f],name);
			g_free(name);
		}

		gtk_entry_set_width_chars(dlg->file[f],40);

		gtk_label_set_mnemonic_widget(GTK_LABEL(widget),GTK_WIDGET(dlg->file[f]));

		gtk_table_attach(GTK_TABLE(table),GTK_WIDGET(dlg->file[f]),1,3,f,f+1,GTK_EXPAND|GTK_SHRINK|GTK_FILL,GTK_EXPAND|GTK_SHRINK|GTK_FILL,2,2);

	}

	widget = gtk_button_new_with_mnemonic( _( "_Browse" ) );
	g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(browse_file),dlg);
	gtk_table_attach(GTK_TABLE(table),widget,3,4,0,1,0,0,2,2);

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dlg->dialog))),GTK_WIDGET(table),FALSE,FALSE,2);

}

static void toggle_option(GtkToggleButton *button, const struct ftoption *option)
{
 	gboolean		  active	= gtk_toggle_button_get_active(button);
 	struct ftdialog	* dlg		= (struct ftdialog *) g_object_get_data(G_OBJECT(button),"dlg");

 	if(active)
		dlg->option |= option->flag;
	else
		dlg->option &= ~option->flag;

	set_boolean_to_config(dlg->name,option->name,active);

	trace("option \"%s\" is %s (flags=%04x)",option->label,active ? "Active" : "Inactive" ,dlg->option);
}

static void add_transfer_options(GObject *action, struct ftdialog *dlg)
{
	static const struct ftoption option[]	=
	{	{	LIB3270_FT_OPTION_ASCII,		"text", 	N_( "_Text file" )						},
		{	LIB3270_FT_OPTION_TSO,			"tso",		N_( "Host is T_SO" )					},
		{	LIB3270_FT_OPTION_CRLF,			"cr",		N_( "Add/Remove _CR at end of line" )	},
		{	LIB3270_FT_OPTION_APPEND,		"append",	N_( "_Append" )							},
		{   LIB3270_FT_OPTION_REMAP_ASCII,	"remap",	N_( "_Remap ASCII Characters" )			}
	};

	GtkTable	* table = GTK_TABLE(gtk_table_new(3,2,TRUE));
	GtkWidget	* frame = gtk_frame_new( _( "Transfer options" ) );
	int 		  row, col, f;

	row=0;
	col=0;
	for(f=0;f < G_N_ELEMENTS(option);f++)
	{
		const gchar	* val		= g_object_get_data(action,option[f].name);
		GtkWidget 	* widget 	= gtk_check_button_new_with_mnemonic( gettext(option[f].label) );
		gboolean 	  active	= FALSE;

		gtk_widget_set_name(widget,option[f].name);

		if(val)
			active = g_strcasecmp(val,"yes") == 0 ? TRUE : FALSE;
		else
			active = get_boolean_from_config(dlg->name,option[f].name,FALSE);

		if(active)
			dlg->option |= option[f].flag;

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),active);

		g_object_set_data(G_OBJECT(widget),"dlg",(gpointer) dlg);
		g_signal_connect(G_OBJECT(widget),"toggled", G_CALLBACK(toggle_option),(gpointer) &option[f]);

		gtk_table_attach(table,widget,col,col+1,row,row+1,GTK_EXPAND|GTK_SHRINK|GTK_FILL,GTK_EXPAND|GTK_SHRINK|GTK_FILL,2,2);
		if(col++ > 0)
		{
			row++;
			col=0;
		}
	}

	gtk_container_add(GTK_CONTAINER(frame),GTK_WIDGET(table));
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dlg->dialog))),GTK_WIDGET(frame),FALSE,FALSE,2);

}

static void setup_dft(GObject *action, struct ftdialog *dlg, GtkWidget **label)
{
	gchar *val = g_object_get_data(action,"dft");

	*label = gtk_label_new_with_mnemonic( _( "DFT B_uffer size:" ) );

	gtk_misc_set_alignment(GTK_MISC(*label),0,.5);

	dlg->dft = GTK_ENTRY(gtk_entry_new());
	gtk_entry_set_max_length(dlg->dft,10);
	gtk_entry_set_width_chars(dlg->dft,10);

	gtk_label_set_mnemonic_widget(GTK_LABEL(*label),GTK_WIDGET(dlg->dft));

	if(val)
	{
		gtk_entry_set_text(dlg->dft,val);
	}
	else
	{
		val = get_string_from_config(dlg->name,"dft","");
		gtk_entry_set_text(dlg->dft,val);
		g_free(val);
	}


}

void download_action(GtkAction *action, GtkWidget *widget)
{
	struct ftdialog dlg;


	if(lib3270_get_ft_state(v3270_get_session(widget)) != LIB3270_FT_STATE_NONE)
	{
		error_dialog(widget,_( "Can't start download" ), _( "File transfer is already active" ), NULL);
		return;
	}

	memset(&dlg,0,sizeof(dlg));

	dlg.dialog = gtk_dialog_new_with_buttons(	_( "Receive file from host" ), \
												GTK_WINDOW(gtk_widget_get_toplevel(widget)),
												GTK_DIALOG_DESTROY_WITH_PARENT, \
												GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, \
												GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, \
												NULL );

	dlg.name	= "download";
	dlg.option	= LIB3270_FT_OPTION_RECEIVE;
	add_file_fields(G_OBJECT(action),&dlg);
	add_transfer_options(G_OBJECT(action),&dlg);

	{
		/* Add dft option */
		GtkWidget *hbox 	= gtk_hbox_new(FALSE,2);
		GtkWidget *label	= NULL;

		setup_dft(G_OBJECT(action),&dlg,&label);

		gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
		gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(dlg.dft),FALSE,FALSE,0);
		gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dlg.dialog))),hbox,FALSE,FALSE,2);
	}

	gtk_widget_show_all(dlg.dialog);

	if(gtk_dialog_run(GTK_DIALOG(dlg.dialog)) == GTK_RESPONSE_ACCEPT)
	{
		// Begin file transfer
	}


//	begin_ft_session(action,widget,LIB3270_FT_OPTION_RECEIVE);

	gtk_widget_destroy(dlg.dialog);

}

void upload_action(GtkAction *action, GtkWidget *widget)
{
	if(lib3270_get_ft_state(v3270_get_session(widget)) != LIB3270_FT_STATE_NONE)
	{
		error_dialog(widget,_( "Can't start upload" ), _( "File transfer is already active" ), NULL);
		return;
	}

//	begin_ft_session(action,widget,LIB3270_FT_OPTION_SEND);
}


