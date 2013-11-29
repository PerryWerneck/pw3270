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
 * Este programa está nomeado como ftdialog.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "v3270ft.h"

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 enum _filename
 {
 	FILENAME_LOCAL,
 	FILENAME_HOST,

	FILENAME_COUNT
 };

 struct _v3270FTD
 {
	GtkDialog			  parent;
	GtkWidget			* filename[FILENAME_COUNT];	/**< Filenames for the transfer */
	LIB3270_FT_OPTION	  options;
 };

 struct _v3270FTDClass
 {
	GtkDialogClass parent_class;

	int dummy;
 };

 G_DEFINE_TYPE(v3270FTD, v3270FTD, GTK_TYPE_DIALOG);

/*--[ Globals ]--------------------------------------------------------------------------------------*/

static void v3270FTD_class_init(v3270FTDClass *klass)
{
//	GtkDialogClass	* widget_class	= GTK_DIALOG_CLASS(klass);

#if GTK_CHECK_VERSION(3,0,0)

#else

	#error Implementar

#endif // GTK_CHECK_VERSION

}

static void v3270FTD_init(v3270FTD *widget)
{
}

static void browse_file(GtkButton *button,v3270FTD *parent)
{
	gboolean	  recv		= (parent->options & LIB3270_FT_OPTION_RECEIVE);
	GtkWidget 	* dialog	= gtk_file_chooser_dialog_new
	(
		recv ? _( "Select file to receive" ) : _( "Select file to send" ),
		GTK_WINDOW(parent),
		GTK_FILE_CHOOSER_ACTION_OPEN,
		_("_Cancel" ),	GTK_RESPONSE_CANCEL,
		recv ? _("_Save") : _("_Send"), GTK_RESPONSE_ACCEPT,
		NULL
	);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gtk_entry_set_text(GTK_ENTRY(parent->filename[FILENAME_LOCAL]),filename);
		g_free(filename);
	}

	gtk_widget_destroy(dialog);


}

GtkWidget * v3270_dialog_ft_new(LIB3270_FT_OPTION options)
{
	v3270FTD *dialog = g_object_new(GTK_TYPE_V3270FTD, NULL);

	// Set defaults
	dialog->options = options;

	// Filename entry
	int f;

	GtkWidget * label[FILENAME_COUNT] =
	{
		gtk_label_new_with_mnemonic( _( "_Local file name:" ) ),
		gtk_label_new_with_mnemonic( _( "_Host file name:" ) )
	};

	for(f=0;f<FILENAME_COUNT;f++)
	{
		dialog->filename[f] = gtk_entry_new();
		gtk_label_set_mnemonic_widget(GTK_LABEL(label[f]),dialog->filename[f]);
		gtk_widget_set_hexpand(dialog->filename[f],TRUE);
	}

	GtkGrid *grid = GTK_GRID(gtk_grid_new());
	gtk_grid_set_row_homogeneous(grid,FALSE);
	gtk_grid_set_column_homogeneous(grid,FALSE);
	gtk_grid_set_column_spacing(grid,5);
	gtk_grid_set_row_spacing(grid,5);

//	GtkButton * browse = GTK_BUTTON(gtk_button_new_with_mnemonic(_( "_Browse")));
	GtkButton * browse = GTK_BUTTON(gtk_button_new_from_icon_name("text-x-generic",GTK_ICON_SIZE_BUTTON));
	gtk_button_set_focus_on_click(browse,FALSE);
	gtk_widget_set_tooltip_text(GTK_WIDGET(browse),_("Select file"));
	g_signal_connect(G_OBJECT(browse),"clicked",G_CALLBACK(browse_file),dialog);

	if(options & LIB3270_FT_OPTION_RECEIVE)
	{
		// It's receiving file first host filename, then local filename
		gtk_window_set_title(GTK_WINDOW(dialog),_( "Receive file from host" ));

		gtk_grid_attach(grid,label[FILENAME_HOST],0,0,1,1);
		gtk_grid_attach(grid,label[FILENAME_LOCAL],0,1,1,1);

		gtk_grid_attach(grid,dialog->filename[FILENAME_HOST],1,0,3,1);
		gtk_grid_attach(grid,dialog->filename[FILENAME_LOCAL],1,1,3,1);
		gtk_grid_attach(grid,GTK_WIDGET(browse),5,1,1,1);

		gtk_widget_set_tooltip_text(dialog->filename[FILENAME_HOST],_("Name of the origin file on the host"));
		gtk_widget_set_tooltip_text(dialog->filename[FILENAME_LOCAL],_("Where to save the received file"));

	}
	else
	{
		// It's sending file first local filename, then hostfilename
		gtk_window_set_title(GTK_WINDOW(dialog),_( "Send file to host" ));

		gtk_grid_attach(grid,dialog->filename[FILENAME_LOCAL],0,0,1,1);
		gtk_grid_attach(grid,dialog->filename[FILENAME_HOST],0,1,1,1);

		gtk_grid_attach(grid,dialog->filename[FILENAME_LOCAL],1,0,3,1);
		gtk_grid_attach(grid,dialog->filename[FILENAME_HOST],1,1,3,1);

		gtk_widget_set_tooltip_text(dialog->filename[FILENAME_HOST],_("Name of the target file on the host"));
		gtk_widget_set_tooltip_text(dialog->filename[FILENAME_LOCAL],_("Path of the local file to send"));

	}

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),GTK_WIDGET(grid),FALSE,TRUE,2);

	// File transfer options

	return GTK_WIDGET(dialog);
}

