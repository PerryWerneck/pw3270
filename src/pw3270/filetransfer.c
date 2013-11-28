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

#include <stdlib.h>
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
 	GtkEntry			* parm[5];
	GtkWidget			* ready;
 };

 struct ftoption
 {
	unsigned int	  flag;
	const gchar		* name;
	const gchar		* label;
 };

 struct ftmask
 {
	unsigned int	  flag;
	unsigned int	  mask;
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

static gchar * get_attribute(GObject *action, struct ftdialog *dlg, const gchar *name)
{
	gchar *val = g_object_get_data(action,name);

	if(val)
		return g_strdup(val);

	return get_string_from_config(dlg->name,name,"");
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

static gboolean is_dialog_ok(GtkEditable *editable, struct ftdialog *dlg)
{
	const gchar *local  = gtk_entry_get_text(GTK_ENTRY(dlg->file[0]));
	const gchar *remote = gtk_entry_get_text(GTK_ENTRY(dlg->file[1]));
	int			 f;

	if(!*remote)
		return FALSE;

	if(!(dlg->option&LIB3270_FT_OPTION_RECEIVE))
	{
		// Sending file, should have local and remote filenames
		if(!( *local && g_file_test(local,G_FILE_TEST_EXISTS)))
			return FALSE;
	}

	for(f=0;f<5;f++)
	{
		if(dlg->parm[f])
		{
			const gchar *val = gtk_entry_get_text(GTK_ENTRY(dlg->parm[f]));

			while(*val)
			{
				if(*val < '0' || *val > '9')
					return FALSE;
				val++;
			}
		}
	}

	return TRUE;
}

static void check_remote_filename(GtkEditable *editable, struct ftdialog *dlg)
{
#if GTK_CHECK_VERSION(3,2,0)
	if(!gtk_entry_get_text_length(dlg->file[0]))
	{
		gchar *basename = g_path_get_basename(gtk_entry_get_text(GTK_ENTRY(editable)));
		gchar *filename = g_build_filename(g_get_user_special_dir(G_USER_DIRECTORY_DOCUMENTS),basename,NULL);
		gtk_entry_set_placeholder_text(dlg->file[0],filename);
		g_free(filename);
		g_free(basename);
	}
#endif // GTK(3,2)
	gtk_widget_set_sensitive(dlg->ready,is_dialog_ok(editable,dlg));
}

static void check_entry(GtkEditable *editable, struct ftdialog *dlg)
{
	gtk_widget_set_sensitive(dlg->ready,is_dialog_ok(editable,dlg));
}

static GtkEntry * add_filename_entry(GObject *action, int ix, int row, struct ftdialog *dlg, GtkTable *table)
{
	static const gchar	* label_text[]	= { N_( "_Local file name:" ), N_( "_Host file name:" ) };
	static const gchar	* attr[]		= { "local", "remote" };

	GtkWidget	* entry		= gtk_entry_new();
	GtkWidget	* label		= gtk_label_new_with_mnemonic(gettext(label_text[ix]));
	gchar		* val;

	gtk_misc_set_alignment(GTK_MISC(label),0,.5);
	gtk_table_attach(GTK_TABLE(table),label,0,1,row,row+1,GTK_FILL,GTK_FILL,2,2);

	gtk_widget_set_name(entry,attr[ix]);

	val = get_attribute(action,dlg,attr[ix]);
	gtk_entry_set_text(dlg->file[ix],val);
	g_free(val);

	gtk_entry_set_width_chars(GTK_ENTRY(entry),40);

	gtk_label_set_mnemonic_widget(GTK_LABEL(label),entry);

	gtk_table_attach(GTK_TABLE(table),entry,1,3,row,row+1,GTK_EXPAND|GTK_SHRINK|GTK_FILL,GTK_EXPAND|GTK_SHRINK|GTK_FILL,2,2);

	return GTK_ENTRY(entry);
}

static void add_file_fields(GObject *action, struct ftdialog *dlg)
{
	GtkTable			* table		= GTK_TABLE(gtk_table_new(2,3,FALSE));
	GtkWidget			* widget;

	gtk_container_set_border_width(GTK_CONTAINER(table),2);

	if(dlg->option&LIB3270_FT_OPTION_RECEIVE)
	{
		// Receiving file, first the remote filename
		dlg->file[1] = add_filename_entry(action,1,0,dlg,table);

		dlg->file[0] = add_filename_entry(action,0,1,dlg,table);
		widget = gtk_button_new_with_mnemonic( _( "_Browse" ) );
		g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(browse_file),dlg);
		gtk_table_attach(GTK_TABLE(table),widget,3,4,1,2,0,0,2,2);
	}
	else
	{
		// Sending file, first the local filename
		dlg->file[0] = add_filename_entry(action,0,0,dlg,table);
		widget = gtk_button_new_with_mnemonic( _( "_Browse" ) );
		g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(browse_file),dlg);
		gtk_table_attach(GTK_TABLE(table),widget,3,4,0,1,0,0,2,2);

		dlg->file[1] = add_filename_entry(action,1,1,dlg,table);
	}

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
		{	LIB3270_FT_OPTION_CRLF,			"cr",		N_( "Add/Remove _CR at end of line" )	},
		{	LIB3270_FT_OPTION_APPEND,		"append",	N_( "_Append" )							},
		{   LIB3270_FT_OPTION_REMAP,		"remap",	N_( "_Remap ASCII Characters" )			}
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
			active = g_ascii_strcasecmp(val,"yes") == 0 ? TRUE : FALSE;
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

	dlg->parm[4] = GTK_ENTRY(gtk_entry_new());
	gtk_widget_set_name(GTK_WIDGET(dlg->parm[4]),"dftsize");
	gtk_entry_set_max_length(dlg->parm[4],10);
	gtk_entry_set_width_chars(dlg->parm[4],10);

	gtk_entry_set_text(GTK_ENTRY(dlg->parm[4]),val ? val : "4096");

	gtk_label_set_mnemonic_widget(GTK_LABEL(*label),GTK_WIDGET(dlg->parm[4]));

}

static void update(H3270FT *ft, unsigned long length, double kbytes_sec)
{
	GtkLabel **info	= (GtkLabel **) g_object_get_data(G_OBJECT(ft->widget),"info");

	if(length && info[0])
	{
		gchar *str = g_strdup_printf("%ld",length);
		gtk_label_set_text(info[0],str);
		g_free(str);
	}

	if(kbytes_sec && info[2])
	{
		gchar *str = g_strdup_printf("%ld KB/s",(unsigned long) kbytes_sec);
		gtk_label_set_text(info[2],str);
		g_free(str);
	}

}

static void ft_complete(H3270FT *ft, unsigned long length,double kbytes_sec,const char *mode)
{
	update(ft,length,kbytes_sec);
}

static void ft_message(struct _h3270ft *ft, const char *text)
{
	GtkLabel **msg	= (GtkLabel **) g_object_get_data(G_OBJECT(ft->widget),"msg");
	gtk_label_set_text(msg[2],gettext(text));
}

static void ft_update(H3270FT *ft, unsigned long current, unsigned long length, double kbytes_sec)
{
	GtkLabel		**info	= (GtkLabel **) g_object_get_data(G_OBJECT(ft->widget),"info");
	GtkProgressBar	* pbar	= g_object_get_data(G_OBJECT(ft->widget),"progress");
	gchar 			* str;

	update(ft, length, kbytes_sec);

	if(current && info[1])
	{
		str = g_strdup_printf("%ld",current);
		gtk_label_set_text(info[1],str);
		g_free(str);
	}

	if(length)
		gtk_progress_bar_set_fraction(pbar,((gdouble) current) / ((gdouble) length));
	else
		gtk_progress_bar_pulse(pbar);

}

static void ft_running(H3270FT *ft, int is_cut)
{

}

static void ft_aborting(H3270FT *ft)
{
	GtkLabel **msg	= (GtkLabel **) g_object_get_data(G_OBJECT(ft->widget),"msg");
	gtk_label_set_text(msg[2],_("Aborting"));
}

static void ft_state_changed(H3270FT *ft, LIB3270_FT_STATE state)
{

}

static void run_ft_dialog(GObject *action, GtkWidget *widget, struct ftdialog *dlg)
{
	H3270FT		* ft			= NULL;
	const char	* msg			= NULL;
	int 		  f;
	int			  parm[G_N_ELEMENTS(dlg->parm)];
	const gchar	* remote_filename;

	g_signal_connect(G_OBJECT(dlg->file[0]),"changed",G_CALLBACK(check_entry),dlg);
	g_signal_connect(G_OBJECT(dlg->file[1]),"changed",G_CALLBACK(check_remote_filename),dlg);

	for(f=0;f<2;f++)
		gtk_widget_set_sensitive(dlg->ready,is_dialog_ok(GTK_EDITABLE(dlg->file[f]),dlg));

	gtk_widget_show_all(dlg->dialog);

	for(f=0;f<G_N_ELEMENTS(dlg->parm);f++)
	{
		if(dlg->parm[f])
		{
			gchar *val = get_attribute(action,dlg,gtk_widget_get_name(GTK_WIDGET(dlg->parm[f])));

			gtk_entry_set_alignment(GTK_ENTRY(dlg->parm[f]),1);

			if(val && *val)
				gtk_entry_set_text(dlg->parm[f],val);

			g_free(val);
			g_signal_connect(G_OBJECT(dlg->parm[f]),"changed",G_CALLBACK(check_entry),dlg);
		}
	}

	if(gtk_dialog_run(GTK_DIALOG(dlg->dialog)) != GTK_RESPONSE_ACCEPT)
	{
		gtk_widget_destroy(dlg->dialog);
		dlg->dialog = NULL;
		return;
	}

	for(f=0;f<G_N_ELEMENTS(dlg->parm);f++)
	{
		if(dlg->parm[f])
		{
			parm[f] = atoi(gtk_entry_get_text(dlg->parm[f]));
			set_string_to_config(dlg->name,gtk_widget_get_name(GTK_WIDGET(dlg->parm[f])),"%d",parm[f]);
		}
		else
		{
			parm[f] = 0;
		}
	}

	remote_filename = gtk_entry_get_text(dlg->file[1]);

	set_string_to_config(dlg->name,"local","%s",gtk_entry_get_text(dlg->file[0]));
	set_string_to_config(dlg->name,"remote","%s",remote_filename);

	if(!gtk_entry_get_text_length(dlg->file[0]))
	{
		// Local filename wasn´t set, create a new one
		gchar *basename = g_path_get_basename(remote_filename);
		gchar *filename = g_build_filename(g_get_user_special_dir(G_USER_DIRECTORY_DOCUMENTS),basename,NULL);
		gtk_entry_set_text(dlg->file[0],filename);
		g_free(filename);
		g_free(basename);
	}

	ft = lib3270_ft_new(	v3270_get_session(widget),
							dlg->option,
							gtk_entry_get_text(dlg->file[0]),
							remote_filename,
							parm[0],	// lrecl
							parm[2],	// blksize
							parm[1],	// primspace
							parm[3],	// secspace
							parm[4],	// dft
							&msg );


#if GTK_CHECK_VERSION(2,18,0)
	gtk_widget_set_visible(dlg->dialog,FALSE);
#else
	gtk_widget_hide(dlg->dialog);
#endif // GTK(2,18,0)

	if(msg)
	{
		GtkWidget *popup = gtk_message_dialog_new_with_markup(
										GTK_WINDOW(gtk_widget_get_toplevel(widget)),
										GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,
										"%s", _( "Can't start file transfer" ));

		trace("msg=%s",msg);
		gtk_window_set_title(GTK_WINDOW(popup),_("File transfer error"));

		gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(popup),"%s",gettext(msg));

		gtk_widget_show_all(popup);
		gtk_dialog_run(GTK_DIALOG(popup));
		gtk_widget_destroy(popup);

	}

	if(ft)
	{
		// Setup FT callbacks, create popup window
		/*
		http://www.suggestsoft.com/images/medieval-software/medieval-bluetooth-obex-file-transfer.gif

		--Informations----------------------------------------
		|
		| From:		xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx
		|
		| To:		xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx
		|
		| Status:	xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx.xxx
		------------------------------------------------------

		--Progress----------------------------------------------
		|
		| Total: 	xxx.xxx.xxx 		Current:	xxx.xxx.xxx
		|
		| Started:	xx:xx:xx			ETA: 		xx:xx:xx
		|
		| xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
		--------------------------------------------------------

														[Cancel]
		*/
		GtkWidget *container;
		GtkWidget *ftdialog;

		ftdialog = gtk_dialog_new_with_buttons(	_( "File transfer" ),
												GTK_WINDOW(gtk_widget_get_toplevel(widget)),
												GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
												GTK_STOCK_CLOSE,GTK_RESPONSE_CLOSE,NULL );


#if GTK_CHECK_VERSION(3,0,0)
		container = gtk_box_new(GTK_ORIENTATION_VERTICAL,2);
#else
		container = gtk_vbox_new(FALSE,2);
#endif // GTK(3,0,0)

		gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(ftdialog))),container,TRUE,TRUE,2);

		// Information frame
		{
			static const gchar *text[] = { N_( "_From" ), N_( "_To" ), N_( "_Status" ) };

			GtkWidget	* frame = gtk_frame_new( _( "Informations" ) );
			GtkWidget	* table = gtk_table_new(G_N_ELEMENTS(text),2,FALSE);
			int			  f;
			GtkWidget	**entry = g_new0(GtkWidget *, G_N_ELEMENTS(text));

			g_object_set_data_full(G_OBJECT(ftdialog),"msg",entry,g_free);
			gtk_container_set_border_width(GTK_CONTAINER(frame),3);

			for(f=0;f<G_N_ELEMENTS(text);f++)
			{
				GtkWidget	* label = gtk_label_new_with_mnemonic("");
				gchar 		* str	= g_strdup_printf("<b>%s:</b>",gettext(text[f]));

				gtk_label_set_markup_with_mnemonic(GTK_LABEL(label),str);
				g_free(str);

				gtk_misc_set_alignment(GTK_MISC(label),0,0);
				gtk_table_attach(GTK_TABLE(table),label,0,1,f,f+1,GTK_FILL,GTK_FILL,2,2);

				entry[f] = gtk_label_new("");
				gtk_label_set_width_chars(GTK_LABEL(entry[f]),70);
				gtk_misc_set_alignment(GTK_MISC(entry[f]),0,0);

//				gtk_entry_set_width_chars(GTK_ENTRY(entry[f]),70);
//				gtk_editable_set_editable(GTK_EDITABLE(entry[f]),FALSE);

				gtk_table_attach(GTK_TABLE(table),entry[f],1,2,f,f+1,GTK_FILL|GTK_EXPAND,GTK_FILL|GTK_EXPAND,2,2);

				gtk_label_set_mnemonic_widget(GTK_LABEL(label),entry[f]);
			}

			for(f=0;f<2;f++)
				gtk_label_set_text(GTK_LABEL(entry[f]),gtk_entry_get_text(dlg->file[f]));

			gtk_container_add(GTK_CONTAINER(frame),table);
			gtk_box_pack_start(GTK_BOX(container),frame,TRUE,TRUE,2);

		}

		// Progress frame
		{
			static const gchar *text[] = { N_( "T_otal" ), N_( "C_urrent" ), N_( "Spee_d" ), N_( "ET_A" ) };

			GtkWidget	* frame 	= gtk_frame_new( _( "Progress" ) );
			GtkWidget	* table 	= gtk_table_new(3,4,TRUE);
			GtkWidget	**entry 	= g_new0(GtkWidget *, G_N_ELEMENTS(text));
			GtkWidget	* progress	= gtk_progress_bar_new();
			int			  pos 		= 0;

			g_object_set_data_full(G_OBJECT(ftdialog),"info",entry,g_free);
			g_object_set_data(G_OBJECT(ftdialog),"progress",progress);

			gtk_container_set_border_width(GTK_CONTAINER(frame),3);
//			gtk_container_set_border_width(GTK_CONTAINER(table),6);

			for(f=0;f<2;f++)
			{
				// Left box
				GtkWidget	* label;
				gchar 		* str;

				str = g_strdup_printf("<b>%s:</b>",gettext(text[pos]));
				label = gtk_label_new("");
				gtk_label_set_markup_with_mnemonic(GTK_LABEL(label),str);
				g_free(str);

				gtk_misc_set_alignment(GTK_MISC(label),0,0);
				gtk_table_attach(GTK_TABLE(table),label,0,1,f,f+1,GTK_FILL,GTK_FILL,2,2);

				entry[pos] = gtk_label_new(_( "N/A" ) );
				gtk_misc_set_alignment(GTK_MISC(entry[f]),0,0);

				gtk_table_attach(GTK_TABLE(table),entry[pos],1,2,f,f+1,GTK_EXPAND,GTK_FILL,2,2);

				gtk_label_set_mnemonic_widget(GTK_LABEL(label),entry[pos++]);

				// Right box
				str = g_strdup_printf("<b>%s:</b>",gettext(text[pos]));
				label = gtk_label_new("");
				gtk_label_set_markup_with_mnemonic(GTK_LABEL(label),str);
				g_free(str);

				gtk_misc_set_alignment(GTK_MISC(label),0,0);
				gtk_table_attach(GTK_TABLE(table),label,2,3,f,f+1,GTK_FILL,GTK_FILL,2,2);

				entry[pos] = gtk_label_new(_("N/A" ));
				gtk_misc_set_alignment(GTK_MISC(entry[f]),0,0);
				gtk_table_attach(GTK_TABLE(table),entry[pos],3,4,f,f+1,GTK_EXPAND,GTK_FILL,2,2);

				gtk_label_set_mnemonic_widget(GTK_LABEL(label),entry[pos++]);

			}

			gtk_table_attach(GTK_TABLE(table),progress,0,4,f,f+1,GTK_EXPAND|GTK_FILL,GTK_EXPAND|GTK_FILL,2,2);

			gtk_container_add(GTK_CONTAINER(frame),table);
			gtk_box_pack_start(GTK_BOX(container),frame,TRUE,TRUE,2);


		}

		ft->widget 			= ftdialog;
		ft->complete 		= ft_complete;
		ft->update			= ft_update;
		ft->running			= ft_running;
		ft->aborting		= ft_aborting;
		ft->state_changed	= ft_state_changed;
		ft->message			= ft_message;

		gtk_widget_show_all(ftdialog);
		lib3270_ft_start(v3270_get_session(widget));

		trace("%s: Running dialog %p",__FUNCTION__,ftdialog);
		gtk_dialog_run(GTK_DIALOG(ftdialog));
		trace("%s: Dialog %p ends",__FUNCTION__,ftdialog);

		lib3270_ft_destroy(v3270_get_session(widget));

		gtk_widget_destroy(ftdialog);
	}

	gtk_widget_destroy(dlg->dialog);
	dlg->dialog = NULL;

}

static void add_buttons(struct ftdialog *dlg)
{
	dlg->ready = gtk_dialog_add_button(GTK_DIALOG(dlg->dialog),
												(dlg->option & LIB3270_FT_OPTION_RECEIVE) != 0 ? GTK_STOCK_SAVE : GTK_STOCK_OPEN,
												GTK_RESPONSE_ACCEPT);

	gtk_widget_set_sensitive(dlg->ready,FALSE);

	gtk_dialog_add_button(GTK_DIALOG(dlg->dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT);
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

	dlg.dialog = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dlg.dialog),_( "Receive file from host" ));
	gtk_window_set_transient_for(GTK_WINDOW(dlg.dialog),GTK_WINDOW(gtk_widget_get_toplevel(widget)));

	dlg.name	= gtk_action_get_name(action);
	dlg.option	= LIB3270_FT_OPTION_RECEIVE;

	add_buttons(&dlg);
	add_file_fields(G_OBJECT(action),&dlg);
	add_transfer_options(G_OBJECT(action),&dlg);

	{
		/* Add dft option */
#if GTK_CHECK_VERSION(3,0,0)
		GtkWidget *hbox		= gtk_box_new(GTK_ORIENTATION_HORIZONTAL,2);
#else
		GtkWidget *hbox		= gtk_hbox_new(FALSE,2);
#endif // GTK(3,0,0)
		GtkWidget *label	= NULL;

		gtk_container_set_border_width(GTK_CONTAINER(hbox),4);

		setup_dft(G_OBJECT(action),&dlg,&label);

		gtk_box_pack_start(GTK_BOX(hbox),label,FALSE,FALSE,0);
		gtk_box_pack_start(GTK_BOX(hbox),GTK_WIDGET(dlg.parm[4]),FALSE,FALSE,0);
		gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dlg.dialog))),hbox,FALSE,FALSE,2);
	}

	run_ft_dialog(G_OBJECT(action),widget,&dlg);

}

static void toggle_format(GtkToggleButton *button, const struct ftmask *option)
{
 	gboolean		  active	= gtk_toggle_button_get_active(button);
 	struct ftdialog	* dlg		= (struct ftdialog *) g_object_get_data(G_OBJECT(button),"dlg");
	const gchar		* name		= (const gchar *) g_object_get_data(G_OBJECT(button),"setupname");

	dlg->option &= ~option->mask;
	dlg->option |= option->flag;

	if(active)
	{
		set_string_to_config(dlg->name,name,"%s",option->name);
		trace("%s=%s (flags=%04x)",name,option->name,dlg->option);
	}
}

void upload_action(GtkAction *action, GtkWidget *widget)
{
	struct ftdialog dlg;

	if(lib3270_get_ft_state(v3270_get_session(widget)) != LIB3270_FT_STATE_NONE)
	{
		error_dialog(widget,_( "Can't start upload" ), _( "File transfer is already active" ), NULL);
		return;
	}

	memset(&dlg,0,sizeof(dlg));

	dlg.dialog = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(dlg.dialog),_( "Send file to host" ));
	gtk_window_set_transient_for(GTK_WINDOW(dlg.dialog),GTK_WINDOW(gtk_widget_get_toplevel(widget)));

	dlg.name	= gtk_action_get_name(action);
	dlg.option	= LIB3270_FT_OPTION_SEND;

	add_buttons(&dlg);
	add_file_fields(G_OBJECT(action),&dlg);
	add_transfer_options(G_OBJECT(action),&dlg);

	{

		static const struct ftmask recfm[]	=
		{
			{	LIB3270_FT_RECORD_FORMAT_DEFAULT, 		LIB3270_FT_RECORD_FORMAT_MASK,		"default", 		N_( "Default" 	)	},
			{	LIB3270_FT_RECORD_FORMAT_FIXED,			LIB3270_FT_RECORD_FORMAT_MASK,		"fixed", 		N_( "Fixed" 	)	},
			{	LIB3270_FT_RECORD_FORMAT_VARIABLE,		LIB3270_FT_RECORD_FORMAT_MASK,		"variable", 	N_( "Variable" 	)	},
			{	LIB3270_FT_RECORD_FORMAT_UNDEFINED,		LIB3270_FT_RECORD_FORMAT_MASK,		"undefined", 	N_( "Undefined" )	},
		};

		static const struct ftmask units[]	=
		{
			{	LIB3270_FT_ALLOCATION_UNITS_DEFAULT,	LIB3270_FT_ALLOCATION_UNITS_MASK,	"default", 		N_( "Default" 	)	},
			{	LIB3270_FT_ALLOCATION_UNITS_TRACKS,		LIB3270_FT_ALLOCATION_UNITS_MASK,	"tracks", 		N_( "Tracks" 	)	},
			{	LIB3270_FT_ALLOCATION_UNITS_CYLINDERS,	LIB3270_FT_ALLOCATION_UNITS_MASK,	"cilinders",	N_( "Cylinders"	)	},
			{	LIB3270_FT_ALLOCATION_UNITS_AVBLOCK,	LIB3270_FT_ALLOCATION_UNITS_MASK,	"avblock", 		N_( "Avblock" 	)	},
		};

		static const struct _fdesc
		{
			const gchar 			* title;
			const gchar 			* name;
			const struct ftmask 	* option;
		} fdesk[] =
		{
			{ N_( "Record format" ), 			"recordformat",		recfm	},
			{ N_( "Space allocation units" ), 	"allocationunits",	units	}
		};


		int f;

#if GTK_CHECK_VERSION(3,0,0)
		GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,2);
#else
		GtkWidget *box = gtk_hbox_new(TRUE,2);
#endif // GTK(3,0,0)

		for(f=0;f<2;f++)
		{
			GtkWidget	* frame 	= gtk_frame_new(gettext(fdesk[f].title));
			GSList		* group		= NULL;
			gchar 		* setup		= get_attribute(G_OBJECT(action),&dlg,fdesk[f].name);
			int 		  p;
#if GTK_CHECK_VERSION(3,0,0)
			GtkWidget	* vbox 		= gtk_box_new(GTK_ORIENTATION_HORIZONTAL,2);
			gtk_box_set_homogeneous(GTK_BOX(vbox),TRUE);
#else
			GtkWidget	* vbox 		= gtk_vbox_new(TRUE,2);
#endif // GTK(3,0,0)

			for(p=0;p<4;p++)
			{
				GtkWidget *widget = gtk_radio_button_new_with_label(group,gettext(fdesk[f].option[p].label));
				g_object_set_data(G_OBJECT(widget),"dlg",(gpointer) &dlg);
				g_object_set_data(G_OBJECT(widget),"setupname",(gpointer) fdesk[f].name);

				g_signal_connect(G_OBJECT(widget),"toggled", G_CALLBACK(toggle_format),(gpointer) &fdesk[f].option[p]);

				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),!g_ascii_strcasecmp(fdesk[f].option[p].name,setup));
				group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(widget));
				gtk_box_pack_start(GTK_BOX(vbox),widget,TRUE,TRUE,0);
			}

			g_free(setup);

			gtk_container_add(GTK_CONTAINER(frame),GTK_WIDGET(vbox));
			gtk_box_pack_start(GTK_BOX(box),frame,TRUE,TRUE,2);
		}

		gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dlg.dialog))),box,TRUE,TRUE,2);
	}

	{
		// Add options
		static const struct _fld
		{
			const gchar *name;
			const gchar *label;
		}
		fld[] = {	{ "lrecl", 			N_( "LRECL:"			)	},
					{ "primary",		N_( "Primary space:"	)	},
					{ "blksize",		N_( "BLKSIZE:"			)	},
					{ "secondary",		N_( "Secondary space:"	)	},
					{ "dftsize",		N_( "DFT B_uffer size:"	)	}
				};

		GtkTable	* table = GTK_TABLE(gtk_table_new(2,2,FALSE));

		int 		  row, col, f;

		gtk_container_set_border_width(GTK_CONTAINER(table),2);

		row=0;
		col=0;
		for(f=0;f < 5;f++)
		{
			GtkWidget *label = gtk_label_new_with_mnemonic(gettext(fld[f].label));

			gtk_misc_set_alignment(GTK_MISC(label),0,.5);
			dlg.parm[f] = GTK_ENTRY(gtk_entry_new());

			gtk_widget_set_name(GTK_WIDGET(dlg.parm[f]),fld[f].name);

			gtk_label_set_mnemonic_widget(GTK_LABEL(label),GTK_WIDGET(dlg.parm[f]));

			gtk_table_attach(table,label,col,col+1,row,row+1,GTK_EXPAND|GTK_SHRINK|GTK_FILL,GTK_EXPAND|GTK_SHRINK|GTK_FILL,2,2);
			gtk_table_attach(table,GTK_WIDGET(dlg.parm[f]),col+1,col+2,row,row+1,GTK_EXPAND|GTK_SHRINK|GTK_FILL,GTK_EXPAND|GTK_SHRINK|GTK_FILL,2,2);

			col += 2;
			if(col++ > 3)
			{
				row++;
				col=0;
			}

		}

		gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dlg.dialog))),GTK_WIDGET(table),TRUE,TRUE,2);


	}

	trace("Running ft fialog %p",&dlg);

	run_ft_dialog(G_OBJECT(action),widget,&dlg);


}


