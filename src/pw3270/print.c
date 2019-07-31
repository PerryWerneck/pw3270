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
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
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

#ifdef _WIN32

#define save_string(h,k,v) save_settings(k,v,h)
#define save_double(h,k,v) registry_set_double(h,k,v)

static void save_settings(const gchar *key, const gchar *value, HKEY hKey)
{
	RegSetValueEx(hKey,key,0,REG_SZ,(const BYTE *) value,strlen(value)+1);
}

/*
 * From:	http://git.gnome.org/browse/gtk+/tree/gtk/gtkpagesetup.c
 *			something like this should really be in gobject!
 *
 * I Agree!! (Perry Werneck)
 *
 */
static gchar * enum_to_string(GType type, guint enum_value)
{
  GEnumClass *enum_class;
  GEnumValue *value;
  gchar	*retval = NULL;

  enum_class = g_type_class_ref (type);

  value = g_enum_get_value(enum_class, enum_value);
  if (value)
    retval = g_strdup (value->value_nick);

  g_type_class_unref (enum_class);

  return retval;
}

#endif // _WIN32

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

#ifdef _WIN32
 void update_settings(const gchar *key, const gchar *val, gpointer *settings)
 {
 	trace("%s: %s=\"%s\"",__FUNCTION__,key,val);
	gtk_print_settings_set(GTK_PRINT_SETTINGS(settings), key, val);
 }

 // From https://git.gnome.org/browse/gtk+/tree/gtk/gtkprintutils.h
 #define MM_PER_INCH 25.4
 #define POINTS_PER_INCH 72

 // From https://git.gnome.org/browse/gtk+/tree/gtk/gtkprintutils.c
 static gdouble _gtk_print_convert_from_mm (gdouble len, GtkUnit unit)
 {
    switch (unit)
    {
    case GTK_UNIT_MM:
        return len;
    case GTK_UNIT_INCH:
        return len / MM_PER_INCH;

    default:
        g_warning ("Unsupported unit");

    /* Fall through */
    case GTK_UNIT_POINTS:
        return len / (MM_PER_INCH / POINTS_PER_INCH);
        break;
    }
 }

#endif // _WIN32

#if GTK_CHECK_VERSION(3,0,0) && !defined(WIN32)
/*
*/
#endif // !WIN32

/*
 static GtkPrintOperation * begin_print_operation(GObject *obj, GtkWidget *widget, PRINT_INFO **info)
 {
 	GtkPrintOperation	* print 	= gtk_print_operation_new();
	GtkPrintSettings 	* settings	= gtk_print_settings_new();
	GtkPageSetup 		* setup 	= gtk_page_setup_new();
    GtkPaperSize        * papersize = NULL;

 	*info = g_new0(PRINT_INFO,1);
 	(*info)->session 	= v3270_get_session(widget);
 	(*info)->cols		= 80;
 	(*info)->widget		= widget;

	// Basic setup
	gtk_print_operation_set_allow_async(print,get_boolean_from_config("print","allow_async",TRUE));

	if(obj)
	{
		const gchar * attr = (const gchar *) g_object_get_data(obj,"jobname");
		if(attr)
			gtk_print_operation_set_job_name(print,attr);
	}

	gtk_print_operation_set_custom_tab_label(print, _( "Options" ) );
	gtk_print_operation_set_show_progress(print,TRUE);

	// Common signals
    g_signal_connect(print,"done",G_CALLBACK(done),*info);

#if GTK_CHECK_VERSION(3,0,0) && !defined(WIN32)
	g_signal_connect(print,"create-custom-widget",G_CALLBACK(create_custom_widget),	*info);
	g_signal_connect(print,"custom-widget-apply",G_CALLBACK(custom_widget_apply), *info);
#else
	{
		g_autofree gchar *color_scheme = get_string_from_config("print","colors","");

		if(color_scheme && *color_scheme)
			v3270_set_color_table((*info)->color,color_scheme);
		else
			v3270_set_mono_color_table((*info)->color,"black","white");

	 }
#endif // !_WIN32

	// Load page and print settings
	{
#ifdef ENABLE_WINDOWS_REGISTRY

		HKEY registry;

		if(get_registry_handle("print",&registry,KEY_READ))
		{
			HKEY 	  hKey;
			DWORD	  disp;
			gchar   * attr  = g_object_get_data(obj,"papersize");

			registry_foreach(registry,"settings",update_settings,(gpointer) settings);

			if(RegCreateKeyEx(registry,"pagesetup",0,NULL,REG_OPTION_NON_VOLATILE,KEY_READ,NULL,&hKey,&disp) == ERROR_SUCCESS)
			{
				gdouble val;

				#define load_double(h,k,s) if(registry_get_double(h,k,&val)) s(setup, val,GTK_UNIT_MM);

				load_double(hKey, "MarginTop",		gtk_page_setup_set_top_margin		);
				load_double(hKey, "MarginBottom",	gtk_page_setup_set_bottom_margin	);
				load_double(hKey, "MarginLeft",		gtk_page_setup_set_left_margin		);
				load_double(hKey, "MarginRight",	gtk_page_setup_set_right_margin		);

				RegCloseKey(hKey);
			}

            if(attr)
            {
                // Paper is defined in xml, use it
                papersize = gtk_paper_size_new(attr);
            }
			else if(RegCreateKeyEx(registry,"paper",0,NULL,REG_OPTION_NON_VOLATILE,KEY_READ,NULL,&hKey,&disp) == ERROR_SUCCESS)
            {
                // Use saved paper size
                // Reference: https://git.gnome.org/browse/gtk+/tree/gtk/gtkpapersize.c
                struct _papersettings
                {
                    const gchar * name;
                    gchar       * value;
                } papersettings[] =
                {
                    { "PPDName",     NULL   },
                    { "Name",        NULL   },
                    { "DisplayName", NULL   }
                };

                int f;
                gdouble width, height;

                // Read paper settings
                registry_get_double(hKey, "Width",  &width);
                registry_get_double(hKey, "Height", &height);

                for(f=0;f<G_N_ELEMENTS(papersettings);f++)
                {
                    BYTE data[4097];
                    unsigned long datatype;
                    unsigned long datalen 	= 4096;

                    if(RegQueryValueExA(hKey,papersettings[f].name,NULL,&datatype,data,&datalen) == ERROR_SUCCESS)
                    {
                        data[datalen+1] = 0;
                        trace("paper[%s]=\"%s\"",papersettings[f].name,data);
                        papersettings[f].value = g_strdup((gchar *) data);
                    }
                }

                #define ppd_name        papersettings[0].value
                #define name            papersettings[1].value
                #define display_name    papersettings[2].value

                if(!display_name)
                    display_name = g_strdup(name);

                if(ppd_name)
                {
                    papersize = gtk_paper_size_new_from_ppd(    ppd_name,
                                                                display_name,
                                                                _gtk_print_convert_from_mm(width,GTK_UNIT_POINTS),
                                                                _gtk_print_convert_from_mm(height,GTK_UNIT_POINTS));
                }
                else if(name)
                {
                    papersize = gtk_paper_size_new_custom(name, display_name,width, height, GTK_UNIT_MM);
                }
                else
                {
                    g_warning("Invalid paper size settings, using defaults");
                    papersize = gtk_paper_size_new(NULL);
                }

                // Release memory
                #undef ppd_name
                #undef display_name
                #undef name

                for(f=0;f<G_N_ELEMENTS(papersettings);f++)
                {
                    if(papersettings[f].value)
                        g_free(papersettings[f].value);
                }

				RegCloseKey(hKey);

            }
            else
            {
                // Create default
                papersize = gtk_paper_size_new(NULL);
            }


			RegCloseKey(registry);
		}

#else
		GKeyFile	* conf	= get_application_keyfile();
		GError		* err	= NULL;
		gchar       * attr  = g_object_get_data(obj,"papersize");

		if(!gtk_print_settings_load_key_file(settings,conf,"print_settings",&err))
		{
			g_warning("Error getting print settings: %s",err->message);
			g_error_free(err);
			err = NULL;
		}

		if(!gtk_page_setup_load_key_file(setup,conf,"page_setup",&err))
		{
			g_warning("Error getting page setup: %s",err->message);
			g_error_free(err);
			err = NULL;
		}

		if(attr)
        {
            // Paper is defined in xml, use it
            papersize = gtk_paper_size_new(attr);
        }
        else if(g_key_file_has_group(conf,"paper_size"))
        {
            // Use saved paper size
            GError *err = NULL;

            papersize = gtk_paper_size_new_from_key_file(conf,"paper_size",&err);
            if(err)
            {
                g_warning("Error loading paper size: %s",err->message);
                g_error_free(err);
            }

            trace("Papersize: %p",papersize);
        }
        else
        {
            // Create default
            papersize = gtk_paper_size_new(NULL);
        }

#endif
	}

	// Finish settings
	gtk_print_operation_set_print_settings(print,settings);
	gtk_page_setup_set_paper_size_and_default_margins(setup,papersize);
	gtk_print_operation_set_default_page_setup(print,setup);

	trace("%s ends",__FUNCTION__);
 	return print;
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

	setup_print_dialog(operation);

	//
	// Run print dialog
	//
	GError *err = NULL;
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

	return rc;

 }

 	/*
 LIB3270_EXPORT int pw3270_print(GtkWidget *widget, GObject *action, GtkPrintOperationAction oper, LIB3270_CONTENT_OPTION src)
 {
 	PRINT_INFO			* info 		= NULL;
 	GtkPrintOperation 	* print;
 	gchar			    * text;
 	GError				* err		= NULL;

 #ifdef X3270_TRACE
	if(action)
		lib3270_trace_event(v3270_get_session(widget),"Action %s activated on widget %p\n",gtk_action_get_name(GTK_ACTION(action)),widget);
 #endif

 	g_return_val_if_fail(GTK_IS_V3270(widget),EINVAL);

 	print = begin_print_operation(action,widget,&info);
 	if(!print)
		return -1;

 	lib3270_get_screen_size(info->session,&info->rows,&info->cols);

	info->src = src;

    g_signal_connect(print,"begin_print",G_CALLBACK(begin_print),info);

	switch(src)
	{
	case LIB3270_CONTENT_ALL:
	case LIB3270_CONTENT_SELECTED:
		g_signal_connect(print,"draw_page",G_CALLBACK(draw_screen),info);
		break;

	case LIB3270_CONTENT_COPY:

		text = v3270_get_copy(widget);

		if(text)
		{
			int r;

			info->text	= g_strsplit(text,"\n",-1);
			info->rows	= g_strv_length(info->text);

			for(r=0;r < info->rows;r++)
			{
				size_t sz = strlen(info->text[r]);
				if(sz > info->cols)
					info->cols = sz;
			}
			g_free(text);
		}
		g_signal_connect(print,"draw_page",G_CALLBACK(draw_text),info);
		break;


	}

	// Run Print dialog
	gtk_print_operation_run(print,oper,GTK_WINDOW(gtk_widget_get_toplevel(widget)),&err);

	if(err)
	{
		show_print_error(widget,err);
		g_error_free(err);
	}

	g_object_unref(print);

	return 0;
 }
	*/

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
