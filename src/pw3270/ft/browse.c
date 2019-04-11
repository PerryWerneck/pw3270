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

/*
#ifdef WIN32

struct file
{
	OPENFILENAME 	ofn;
	char		  	szName[260];	// buffer for file name
	int				mode;
	BOOL			ok;
};

static gpointer select_file(struct file *fl)
{
	if(fl->mode == 1)
	{
		fl->ok = GetSaveFileName(&fl->ofn);
	}
	else
	{
		fl->ok = GetOpenFileName(&fl->ofn);
	}

	fl->mode = 3;

	return 0;
}

void browse_file(GtkButton *button,v3270FTD *parent)
{
	GThread 	* thd;
	struct file	  fl;
	GdkWindow	* win 	= gtk_widget_get_window(GTK_WIDGET(parent));

	gtk_widget_set_sensitive(GTK_WIDGET(parent),FALSE);

	memset(&fl,0,sizeof(fl));
	fl.ofn.lStructSize		= sizeof(fl.ofn);
	fl.ofn.hwndOwner		= GDK_WINDOW_HWND(win);
	fl.ofn.lpstrFile		= fl.szName;

	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
	// use the contents of szFile to initialize itself.
	fl.ofn.lpstrFile[0] 	= '\0';

	fl.ofn.nMaxFile 		= sizeof(fl.szName);
	fl.ofn.lpstrFilter		= "All\0*.*\0Text\0*.TXT\0";
	fl.ofn.nFilterIndex		= 1;
	fl.ofn.nMaxFileTitle	= 0;
	fl.ofn.lpstrInitialDir	= NULL;

	// Guarda o valor atual
	strncpy(fl.szName,gtk_entry_get_text(GTK_ENTRY(parent->filename[FILENAME_LOCAL])),fl.ofn.nMaxFile);

	fl.mode = (parent->options & LIB3270_FT_OPTION_RECEIVE) ? 1 : 0;

	if(fl.mode == 1)
	{
		// Receber arquivo
		// https://msdn.microsoft.com/en-us/library/windows/desktop/ms646839(v=vs.85).aspx
		// https://msdn.microsoft.com/en-us/library/windows/desktop/ms646829(v=vs.85).aspx#open_file

//		fl.ofn.lpstrFileTitle	= _( "Select file to receive" );
		fl.ofn.Flags			= OFN_OVERWRITEPROMPT;

	}
	else
	{
		// Enviar arquivo
		// https://msdn.microsoft.com/en-us/library/windows/desktop/ms646928(v=vs.85).aspx
//		fl.ofn.lpstrFileTitle	= _( "Select file to send" );
		fl.ofn.Flags 			= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	}

	thd = g_thread_new("GetFileName",(GThreadFunc) select_file, &fl);

	while(fl.mode != 3) {
		g_main_context_iteration(NULL,TRUE);
	}

	g_thread_unref(thd);

	if(fl.ok)
	{
		gtk_entry_set_text(GTK_ENTRY(parent->filename[FILENAME_LOCAL]),fl.szName);
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
*/
