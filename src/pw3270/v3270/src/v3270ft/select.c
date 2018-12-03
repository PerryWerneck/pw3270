/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob
 * o nome G3270.
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
 * Este programa está nomeado como select.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#include "private.h"
#include <stdarg.h>

#ifdef WIN32
	#include <gdk/gdkwin32.h>
#endif // WIN32

/*--[ Implement ]------------------------------------------------------------------------------------*/

#if defined(_WIN32)

struct file {
	OPENFILENAME		 	ofn;
	gboolean				enabled;
	char				  	szName[260];	// buffer for file name
	GtkFileChooserAction	action;
	BOOL					ok;
};

static gpointer select_file(struct file *fl) {


	switch(fl->action) {
	case GTK_FILE_CHOOSER_ACTION_SAVE:	// Receber arquivo
										// https://msdn.microsoft.com/en-us/library/windows/desktop/ms646839(v=vs.85).aspx
										// https://msdn.microsoft.com/en-us/library/windows/desktop/ms646829(v=vs.85).aspx#open_file
		fl->ofn.Flags = OFN_OVERWRITEPROMPT | OFN_CREATEPROMPT | OFN_HIDEREADONLY;
		fl->ok = GetSaveFileName(&fl->ofn);
		break;

	case GTK_FILE_CHOOSER_ACTION_OPEN:	// Enviar arquivo
										// https://msdn.microsoft.com/en-us/library/windows/desktop/ms646928(v=vs.85).aspx
		fl->ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		fl->ok = GetOpenFileName(&fl->ofn);
		break;
	}

	fl->enabled = FALSE;

	return 0;
}

#endif // _WIN32

gchar * v3270ft_select_file(v3270ft *dialog, const gchar *title, const gchar *button, GtkFileChooserAction action, const gchar *filename, const gchar *filter, ...) {

	gchar *rc = NULL;

#if GTK_CHECK_VERSION(3,20,0)

	GtkFileChooserNative *native =	gtk_file_chooser_native_new
									(
										title,
										GTK_WINDOW(dialog),
										action,
										button,
										_( "_Cancel" )
									);


	if(gtk_native_dialog_run(GTK_NATIVE_DIALOG (native)) == GTK_RESPONSE_ACCEPT) {
		rc = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(native)));
	}

	g_object_unref(native);

#elif defined(_WIN32)

	GThread 	* thd;
	struct file	  fl;
	GdkWindow	* win 	= gtk_widget_get_window(GTK_WIDGET(dialog));

	gtk_widget_set_sensitive(GTK_WIDGET(dialog),FALSE);

	memset(&fl,0,sizeof(fl));
	fl.ofn.lStructSize		= sizeof(fl.ofn);
	fl.ofn.hwndOwner		= GDK_WINDOW_HWND(win);
	fl.ofn.lpstrFile		= fl.szName;

	fl.ofn.lpstrTitle		= title;

	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not
	// use the contents of szFile to initialize itself.
	fl.ofn.lpstrFile[0] 	= '\0';

	fl.ofn.nMaxFile 		= sizeof(fl.szName);

	// Monta lista de arquivos.
	va_list		  args;
	size_t		  ix		= 0;

	fl.ofn.lpstrFilter = (char *) g_malloc0(4096);

	va_start (args, filter);
	while(filter) {

		filter = gettext(filter);
		size_t sz = strlen(filter)+1;

		if(ix+sz > 4095)
			break;

		debug("%s",filter);

		memcpy(((char *) fl.ofn.lpstrFilter)+ix,filter,sz);
		ix += sz;
		filter = va_arg(args, const char *);
	}
	va_end (args);

	debug("%s",fl.ofn.lpstrFilter);

	fl.ofn.nFilterIndex		= 1;
	fl.ofn.lpstrInitialDir	= NULL;
	fl.ofn.nMaxFileTitle	= 0;

	// Guarda o valor atual
	if(filename)
		strncpy(fl.szName,filename,fl.ofn.nMaxFile);

	fl.action = action;

	thd = g_thread_new("GetFileName",(GThreadFunc) select_file, &fl);

	fl.enabled = TRUE;
	while(fl.enabled) {
		g_main_context_iteration(NULL,TRUE);
	}

	g_thread_unref(thd);

	if(fl.ok) {
		rc = g_strdup(fl.szName);
	}

	g_free( ((char *) fl.ofn.lpstrFilter) );
	gtk_widget_set_sensitive(GTK_WIDGET(dialog),TRUE);


#else

	GtkWidget * chooser = gtk_file_chooser_dialog_new
	(
		title,
		GTK_WINDOW(dialog),
		action,
		_("_Cancel" ),	GTK_RESPONSE_CANCEL,
		button, GTK_RESPONSE_ACCEPT,
		NULL
	);

	if(filename && *filename)
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(chooser),filename);

	if(gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT) {
		rc = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
	}

	gtk_widget_destroy(chooser);


#endif // defined

	return rc;

}

