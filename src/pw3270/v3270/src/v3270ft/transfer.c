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
 * Este programa está nomeado como transfer.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <v3270.h>
 #include "private.h"


/*--[ Statics ]--------------------------------------------------------------------------------------*/

/*--[ Implement ]------------------------------------------------------------------------------------*/

static gboolean transfer_mapped(GtkWidget *widget, GdkEvent *event, v3270ft *ft) {
	v3270ftprogress_set_transfer(widget,ft->active->data);
	v3270ftprogress_start_transfer(widget);
	return TRUE;
}

static void transfer_complete(GtkWidget *progress, const gchar *primary, const gchar *secondary, v3270ft *ft) {

	debug("%s",__FUNCTION__);

	if(ft->active) {

		GList *next = g_list_next(ft->active);

		ft->files = g_list_delete_link(ft->files,ft->active);

		v3270ft_set_active(ft,next);
	}

	if(ft->active) {

		v3270ftprogress_set_transfer(progress,ft->active->data);
		v3270ftprogress_start_transfer(progress);

	} else {

		gtk_dialog_response(GTK_DIALOG(progress),GTK_RESPONSE_OK);

	}

}

static void transfer_failed(GtkWidget *progress, const gchar *primary, const gchar *secondary, v3270ft *ft) {

	GtkWidget	* widget;
	GtkWidget 	* message = gtk_message_dialog_new(GTK_WINDOW(progress),
							 GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
							 GTK_MESSAGE_ERROR,
							 GTK_BUTTONS_NONE,
							 "%s",primary);

	widget = gtk_dialog_add_button(GTK_DIALOG(message), _("Ignore"), GTK_RESPONSE_OK);
	gtk_widget_set_tooltip_markup(widget,_("Ignore the fail and remove the file from queue."));

	widget = gtk_dialog_add_button(GTK_DIALOG(message), _("Retry"), GTK_RESPONSE_APPLY);
	gtk_widget_set_tooltip_markup(widget,_("Try again with the same file."));

	widget = gtk_dialog_add_button(GTK_DIALOG(message), _("Skip"), GTK_RESPONSE_ACCEPT);
	gtk_widget_set_tooltip_markup(widget,_("Skip this transfer, keep the file on queue."));

	widget = gtk_dialog_add_button(GTK_DIALOG(message), _("Cancel"), GTK_RESPONSE_CANCEL);
	gtk_widget_set_tooltip_markup(widget,_("Cancel transfer operation."));

	if(secondary && *secondary)
		gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(message),"%s",secondary);

	gtk_window_set_title(GTK_WINDOW(message),v3270ft_get_remote_filename(GTK_WIDGET(ft)));

	switch(gtk_dialog_run(GTK_DIALOG(message))) {

	case GTK_RESPONSE_CANCEL:		// Cancel
		gtk_dialog_response(GTK_DIALOG(progress),GTK_RESPONSE_CANCEL);
		break;

	case GTK_RESPONSE_APPLY:		// Retry
		v3270ftprogress_start_transfer(progress);
		break;

	case GTK_RESPONSE_OK:			// Remove from queue and start next one
		transfer_complete(progress,primary,secondary,ft);
		break;

	case GTK_RESPONSE_ACCEPT:		// skip

		if(ft->active) {
			v3270ft_set_active(ft,g_list_next(ft->active));
		}

		if(ft->active) {

			v3270ftprogress_set_transfer(progress,ft->active->data);
			v3270ftprogress_start_transfer(progress);

		} else {

			gtk_dialog_response(GTK_DIALOG(progress),GTK_RESPONSE_OK);

		}

		break;

	}

	gtk_widget_destroy(message);

}

gint v3270ft_transfer(GtkWidget *dialog, H3270 *session) {

	GtkWidget 	* progress	= v3270ftprogress_new();
	gint		  rc		= GTK_RESPONSE_NONE;

	gtk_window_set_transient_for(GTK_WINDOW(progress),GTK_WINDOW(dialog));
//	gtk_window_set_deletable(progress,FALSE);

	v3270ft_select_first(dialog);
	v3270ftprogress_set_session(progress,session);

	g_signal_connect(progress,"success",G_CALLBACK(transfer_complete),dialog);
	g_signal_connect(progress,"failed",G_CALLBACK(transfer_failed),dialog);
	g_signal_connect(progress,"map-event",G_CALLBACK(transfer_mapped),dialog);

	// Executa diálogo de transferência
	gtk_widget_show_all(progress);

	rc = gtk_dialog_run(GTK_DIALOG(progress));

	gtk_widget_destroy(progress);

	// Garante que vou sair com um arquivo selecionado.
	if(!v3270ft_has_selected(dialog))
		v3270ft_select_last(dialog);

	return rc;
}


