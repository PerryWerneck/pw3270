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

/*--[ Implement ]------------------------------------------------------------------------------------*/

gchar * v3270ft_select_file(v3270ft *dialog, const gchar *title, const gchar *button, GtkFileChooserAction action, const gchar *filename) {

	gchar *rc = NULL;

#if GTK_CHECK_VERSION(3,20,0)

	GtkFileChooserNative *native =	gtk_file_chooser_native_new
									(
										title,
										GTK_WIDGET(dialog),
										action,
										button,
										_( "_Cancel" )
									);


	if(gtk_native_dialog_run(GTK_NATIVE_DIALOG (native)) == GTK_RESPONSE_ACCEPT) {
		rc = g_strdup(gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(native)));
	}

	g_object_unref(native);

#elif defined(_WIN32)

	#error Usar diálogo nativo windows

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

