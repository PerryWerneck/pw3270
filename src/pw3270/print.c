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
 * Este programa está nomeado como print.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "private.h"
 #include <v3270.h>
 #include <v3270/print.h>
 #include <v3270/dialogs.h>
 #include <lib3270/selection.h>
 #include <lib3270/trace.h>
 #include <lib3270/log.h>

 #define FONT_CONFIG 	"font-family"
 #define DEFAULT_FONT	"Courier New"

/*--[ Implement ]------------------------------------------------------------------------------------*/

 void print_all_action(GtkAction *action, GtkWidget *widget)
 {
	pw3270_print(widget,G_OBJECT(action),GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, LIB3270_CONTENT_ALL);
 }

 void print_selected_action(GtkAction *action, GtkWidget *widget)
 {
	pw3270_print(widget,G_OBJECT(action),GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, LIB3270_CONTENT_SELECTED);
 }

  void print_copy_action(GtkAction *action, GtkWidget *widget)
 {
	pw3270_print(widget,G_OBJECT(action),GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, LIB3270_CONTENT_COPY);
 }

 static void done(GtkPrintOperation *operation, GtkPrintOperationResult result, GtkWidget *terminal)
 {
	debug("%s(%u)",__FUNCTION__,(unsigned int) result);

	switch(result)
	{
	case GTK_PRINT_OPERATION_RESULT_ERROR:	// An error has occurred.
		{
			// Get error code
			GError * err = NULL;

			gtk_print_operation_get_error(operation,&err);

			v3270_error_popup(
				terminal,
				_("Operation has failed"),
				_( "Unable to complete print job" ),
				err->message
			);

			g_error_free(err);

		}
		break;

	case GTK_PRINT_OPERATION_RESULT_APPLY:	// The print settings should be stored.
		trace("%s","GTK_PRINT_OPERATION_RESULT_APPLY");
		save_print_operation_settings(operation);
		break;

	case GTK_PRINT_OPERATION_RESULT_CANCEL:	// The print operation has been canceled, the print settings should not be stored.
		trace("%s","GTK_PRINT_OPERATION_RESULT_CANCEL");
		g_message("Print operation was cancelled");
		break;

	case GTK_PRINT_OPERATION_RESULT_IN_PROGRESS:	// The print operation is not complete yet. This value will only be returned when running asynchronously.
		trace("%s","GTK_PRINT_OPERATION_RESULT_IN_PROGRESS");
		g_message("Print operation is in progress");
		break;

	default:
		trace("%s","Unexpected");
		g_warning("Unexpected print operation result: %d",(int) result);

	}

 }

 LIB3270_EXPORT int pw3270_print(GtkWidget *widget, GObject *action, GtkPrintOperationAction oper, LIB3270_CONTENT_OPTION src)
 {
 	int rc = 0;

	g_return_val_if_fail(GTK_IS_V3270(widget),EINVAL);

	if(action)
		lib3270_trace_event(v3270_get_session(widget),"Action %s activated on widget %p\n",gtk_action_get_name(GTK_ACTION(action)),widget);

	//
	// Create and setup dialog
	//
 	GtkPrintOperation * operation = v3270_print_operation_new(widget,src);
 	if(!operation)
	{
		g_message("Can't create print operation");
		return -1;
	}

	g_signal_connect(operation,"done",G_CALLBACK(done),widget);

 	{
 		// Setup async mode
 		gboolean async = get_boolean_from_config("terminal","allow_async_print",TRUE);
		gtk_print_operation_set_allow_async(operation,async);
 	}

	load_print_operation_settings(operation);

	//
	// Run print dialog
	//
	GError *err = NULL;
	GtkPrintOperationResult result =
		gtk_print_operation_run(
			operation,
			GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
			GTK_WINDOW(gtk_widget_get_toplevel(widget)),
			&err
		);

	if(err)
	{
		v3270_error_popup(
			widget,
			_("Operation has failed"),
			_( "Unable to complete print job" ),
			err->message
		);

		g_error_free(err);
		rc = -1;
	}

	g_object_unref(operation);

	return rc;

 }

 void print_settings_action(GtkAction *action, GtkWidget *terminal)
 {
 	const gchar * title  = g_object_get_data(G_OBJECT(action),"title");
 	GtkWidget	* widget;
	GtkWidget	* dialog = gtk_dialog_new_with_buttons (	gettext(title ? title : N_( "Print settings") ),
															GTK_WINDOW(gtk_widget_get_toplevel(terminal)),
															GTK_DIALOG_DESTROY_WITH_PARENT,
															GTK_STOCK_OK,		GTK_RESPONSE_ACCEPT,
															GTK_STOCK_CANCEL,	GTK_RESPONSE_REJECT,
															NULL );

	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);

	// https://developer.gnome.org/hig/stable/visual-layout.html.en
	gtk_container_set_border_width(
		GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		18
	);

	gtk_box_set_spacing(
		GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
		18
	);

	// Create settings widget & load values from configuration.
 	GtkWidget * settings = V3270_print_settings_new(terminal);

 	// Load settings.
 	{
		g_autofree gchar * font_family	= get_string_from_config("print",FONT_CONFIG,DEFAULT_FONT);
		if(font_family && *font_family)
			v3270_print_settings_set_font_family(settings,font_family);

		g_autofree gchar * color_scheme	= get_string_from_config("print","colors","");
		if(color_scheme && *color_scheme)
			v3270_print_settings_set_color_scheme(settings,color_scheme);
 	}

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),settings,TRUE,TRUE,2);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		// Accepted, save settings

		// Save font family
		g_autofree gchar * font_family = v3270_print_settings_get_font_family(settings);
		set_string_to_config("print",FONT_CONFIG,font_family);

		// Save colors
		g_autofree gchar * colors = v3270_print_settings_get_color_scheme(settings);
		set_string_to_config("print","colors","%s",colors);
	}

	gtk_widget_destroy(dialog);

 }
