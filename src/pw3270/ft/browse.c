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
 * Este programa está nomeado como browse.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "private.h"

#ifdef WIN32
	#include <gdk/gdkwin32.h>
#endif // WIN32

/*---[ Implement ]----------------------------------------------------------------------------------*/

#ifdef WIN32
void browse_file(GtkButton *button,v3270FTD *parent)
{
	char		  szFile[260];	// buffer for file name
	GdkWindow	* win 	= gtk_widget_get_window(GTK_WIDGET(parent));

	gtk_widget_set_sensitive(GTK_WIDGET(parent),FALSE);

	OPENFILENAME ofn;

	memset(&ofn,0,sizeof(ofn));
	ofn.lStructSize		= sizeof(ofn);
	ofn.hwndOwner		= GDK_WINDOW_HWND(win);
	ofn.lpstrFile		= szFile;

	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] 	= '\0';

	ofn.nMaxFile 		= sizeof(szFile);
	ofn.lpstrFilter		= "All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex	= 1;
	ofn.nMaxFileTitle	= 0;
	ofn.lpstrInitialDir	= NULL;

	if(parent->options & LIB3270_FT_OPTION_RECEIVE)
	{
		// Receber arquivo
		// https://msdn.microsoft.com/en-us/library/windows/desktop/ms646839(v=vs.85).aspx
		// https://msdn.microsoft.com/en-us/library/windows/desktop/ms646829(v=vs.85).aspx#open_file

//		ofn.lpstrFileTitle	= _( "Select file to receive" );
		ofn.Flags			= OFN_OVERWRITEPROMPT;

		if(GetSaveFileName(&ofn)==TRUE)
		{
			gtk_entry_set_text(GTK_ENTRY(parent->filename[FILENAME_LOCAL]),szFile);
		}

	}
	else
	{
		// Enviar arquivo
		// https://msdn.microsoft.com/en-us/library/windows/desktop/ms646928(v=vs.85).aspx
		OPENFILENAME ofn;

//		ofn.lpstrFileTitle	= _( "Select file to send" );
		ofn.Flags 			= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		if(GetOpenFileName(&ofn)==TRUE)
		{
			gtk_entry_set_text(GTK_ENTRY(parent->filename[FILENAME_LOCAL]),szFile);
		}
	}

	gtk_widget_set_sensitive(GTK_WIDGET(parent),TRUE);

}
#else
void browse_file(GtkButton *button,v3270FTD *parent)
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

	const gchar * current = gtk_entry_get_text(GTK_ENTRY(parent->filename[FILENAME_LOCAL]));
	if(current && *current)
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog),current);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gtk_entry_set_text(GTK_ENTRY(parent->filename[FILENAME_LOCAL]),filename);
		g_free(filename);
	}

	gtk_widget_destroy(dialog);


}
#endif // WIN32
