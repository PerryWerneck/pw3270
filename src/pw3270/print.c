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
 #include <lib3270/selection.h>
 #include <lib3270/trace.h>

 #define FONT_CONFIG 	"font-family"
 #define DEFAULT_FONT	"Courier New"

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void show_print_error(GtkWidget *widget, GError *err)
 {
	GtkWidget *dialog = gtk_message_dialog_new_with_markup(	GTK_WINDOW(gtk_widget_get_toplevel(widget)),
															GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
															GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,
															"%s",_( "Print operation failed" ));

	g_warning("%s",err->message);

	gtk_window_set_title(GTK_WINDOW(dialog),_("Error"));

	gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog),"%s",err->message);

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

 /*
 static void done(GtkPrintOperation *prt, GtkPrintOperationResult result, PRINT_INFO *info)
 {
	if(result == GTK_PRINT_OPERATION_RESULT_ERROR)
	{
		GError		* err		= NULL;

		gtk_print_operation_get_error(prt,&err);
		show_print_error(info->widget,err);
		g_error_free(err);

	}
	else
	{
		// Save settings
		GtkPrintSettings	* settings	= gtk_print_operation_get_print_settings(prt);
		GtkPageSetup		* pgsetup	= gtk_print_operation_get_default_page_setup(prt);
		GtkPaperSize        * papersize = gtk_page_setup_get_paper_size(pgsetup);

		trace("Saving settings PrintSettings=%p page_setup=%p",settings,pgsetup);

#ifdef ENABLE_WINDOWS_REGISTRY
		HKEY registry;

		if(get_registry_handle("print",&registry,KEY_SET_VALUE))
		{
			HKEY	hKey;
			DWORD	disp;

			if(RegCreateKeyEx(registry,"settings",0,NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,&disp) == ERROR_SUCCESS)
			{
				gtk_print_settings_foreach(	settings,(GtkPrintSettingsFunc) save_settings, hKey );
				RegCloseKey(hKey);
			}

			if(RegCreateKeyEx(registry,"page",0,NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,&disp) == ERROR_SUCCESS)
			{
				gchar			* orientation	= enum_to_string(GTK_TYPE_PAGE_ORIENTATION,gtk_page_setup_get_orientation(pgsetup));

				// From http://git.gnome.org/browse/gtk+/tree/gtk/gtkpagesetup.c
				save_double(hKey, "MarginTop",		gtk_page_setup_get_top_margin(pgsetup, GTK_UNIT_MM));
				save_double(hKey, "MarginBottom",	gtk_page_setup_get_bottom_margin(pgsetup, GTK_UNIT_MM));
				save_double(hKey, "MarginLeft",		gtk_page_setup_get_left_margin(pgsetup, GTK_UNIT_MM));
				save_double(hKey, "MarginRight",	gtk_page_setup_get_right_margin(pgsetup, GTK_UNIT_MM));
				save_string(hKey, "Orientation", 	orientation);

				g_free (orientation);

				RegCloseKey(hKey);
			}

            if(papersize && RegCreateKeyEx(registry,"paper",0,NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,&disp) == ERROR_SUCCESS)
            {
                // From http://git.gnome.org/browse/gtk+/tree/gtk/gtkpapersize.c
                static const struct _papersettings
                {
                    const gchar * name;
                    const gchar * (*get)(GtkPaperSize *);
                } papersettings[] =
                {
                    { "PPDName",     gtk_paper_size_get_ppd_name        },
                    { "Name",        gtk_paper_size_get_name            },
                    { "DisplayName", gtk_paper_size_get_display_name    }
                };

                int f;

                for(f=0;f<G_N_ELEMENTS(papersettings);f++)
                {
                    const gchar *ptr = papersettings[f].get(papersize);
                    if(ptr)
                        save_string(hKey,papersettings[f].name,ptr);
                }

                save_double(hKey, "Width", gtk_paper_size_get_width (papersize, GTK_UNIT_MM));
                save_double(hKey, "Height", gtk_paper_size_get_height (papersize, GTK_UNIT_MM));

                RegCloseKey(hKey);
            }


			RegCloseKey(registry);
		}
#else
		GKeyFile * conf = get_application_keyfile();
		gtk_print_settings_to_key_file(settings,conf,"print_settings");
		gtk_page_setup_to_key_file(pgsetup,conf,"page_setup");
        gtk_paper_size_to_key_file(papersize,conf,"paper_size");
#endif

	}

 }
 */

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

	gtk_print_operation_set_allow_async(operation,get_boolean_from_config("print","allow_async",TRUE));

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
		GtkWidget *popup = gtk_message_dialog_new_with_markup(
			GTK_WINDOW(gtk_widget_get_toplevel(widget)),
			GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,
			_("Can't print")
		);

		gtk_window_set_title(GTK_WINDOW(popup),_("Operation has failed"));

		gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(popup),"%s",err->message);

		gtk_dialog_run(GTK_DIALOG(popup));
		gtk_widget_destroy(popup);

		g_error_free(err);

		rc = -1;
	}
	else
	{
		switch(result)
		{
		case GTK_PRINT_OPERATION_RESULT_ERROR:	// An error has occurred.
			g_message("Print operation has failed");
			break;

		case GTK_PRINT_OPERATION_RESULT_APPLY:	// The print settings should be stored.
			save_print_operation_settings(operation);
			break;

		case GTK_PRINT_OPERATION_RESULT_CANCEL:	// The print operation has been canceled, the print settings should not be stored.
			g_message("Print operation was cancelled");
			break;

		case GTK_PRINT_OPERATION_RESULT_IN_PROGRESS:	// The print operation is not complete yet. This value will only be returned when running asynchronously.
			g_message("Print operation is in progress");
			break;

		default:
			g_warning("Unexpected print operation result: %d",(int) result);

		}
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
