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
 * Este programa está nomeado como main.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

#include <glib.h>
#include <glib/gstdio.h>
#include "globals.h"

#ifdef HAVE_GTKMAC
 #include <gtkosxapplication.h>
#endif // HAVE_GTKMAC

#include <pw3270/v3270.h>
#include <pw3270/plugin.h>
#include "v3270/accessible.h"
#include <lib3270/trace.h>
#include <pw3270/trace.h>
#include <stdlib.h>

#if defined( HAVE_SYSLOG )
 #include <syslog.h>
#endif // HAVE_SYSLOG

#define ERROR_DOMAIN g_quark_from_static_string(PACKAGE_NAME)

/*--[ Statics ]--------------------------------------------------------------------------------------*/

 static GtkWidget		* toplevel		= NULL;
 static GtkWidget 		* trace_window	= NULL;
 static unsigned int	  syscolors		= 16;
 static const gchar		* systype		= NULL;
 static const gchar		* toggleset		= NULL;
 static const gchar		* togglereset	= NULL;
 static const gchar     * logfile       = NULL;
 static const gchar		* tracefile		= NULL;
 static const gchar		* charset		= NULL;

#ifdef HAVE_GTKMAC
 GtkOSXApplication		* osxapp		= NULL;
#endif // HAVE_GTKMAC

#if defined( WIN32 )
 static const gchar		* appname		= PACKAGE_NAME;
#endif // WIN32

#if defined( HAVE_SYSLOG )
 static gboolean	  	  log_to_syslog	= FALSE;
#endif // HAVE_SYSLOG



/*--[ Implement ]------------------------------------------------------------------------------------*/

static int initialize(void)
{
	const gchar * msg = gtk_check_version(GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);

	if(msg)
	{
		// Invalid GTK version, notify user
		int rc;

		GtkWidget *dialog = gtk_message_dialog_new(	NULL,
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_WARNING,
													GTK_BUTTONS_OK_CANCEL,
													_( "%s requires GTK version %d.%d.%d" ),PACKAGE_NAME,GTK_MAJOR_VERSION,GTK_MINOR_VERSION,GTK_MICRO_VERSION );


		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s",msg);
		gtk_window_set_title(GTK_WINDOW(dialog),_( "GTK Version mismatch" ));
		gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);

        rc = gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);

        if(rc != GTK_RESPONSE_OK)
			return EINVAL;
	}

	return 0;
}

static void toplevel_setup(GtkWindow *window)
{
 	gchar * name		= g_strdup_printf("%s.png",g_get_application_name());
	gchar * filename 	= pw3270_build_filename(GTK_WIDGET(window),name,NULL);
 	gchar * role		= g_strdup_printf("%s_top",g_get_application_name());

	gtk_window_set_type_hint(window,GDK_WINDOW_TYPE_HINT_NORMAL);
	gtk_window_set_position(window,GTK_WIN_POS_CENTER);
	gtk_window_set_role(window,role);

	g_free(role);

	// Set default icon
	if(g_file_test(filename,G_FILE_TEST_EXISTS))
	{
		GError * error = NULL;

		trace("Loading default icon from %s",filename);

		if(!gtk_window_set_default_icon_from_file(filename,&error))
		{
			g_warning("Error %s loading default icon from %s",error->message,filename);
			g_error_free(error);
		}
	}

	g_free(filename);
	g_free(name);
}

#if ! defined( WIN32 )

static gboolean appname(const gchar *option_name, const gchar *value, gpointer data, GError **error)
{
	g_set_application_name(value);
	return TRUE;
}

#else

static gboolean datadir(const gchar *option_name, const gchar *value, gpointer data, GError **error)
{
		gchar	* path = g_strconcat("SOFTWARE\\",appname,NULL);
		HKEY	  hKey;
		DWORD	  disp;
		int		  rc;

		rc = RegCreateKeyEx(HKEY_LOCAL_MACHINE,path,0,NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,&disp);
		SetLastError(rc);

		trace("%s=\"%s\" create=%d",path,value,rc);

		if(rc == ERROR_SUCCESS)
		{
			trace("%s: Value set",__FUNCTION__);
			RegSetValueEx(hKey,"datadir",0,REG_SZ,(const BYTE *) value,strlen(value)+1);
			RegCloseKey(hKey);
		}
        else
        {
			gchar *msg = g_win32_error_message(rc);
			trace("%s failed: %s",__FUNCTION__,msg);
			*error = g_error_new(ERROR_DOMAIN,EINVAL, "%s", msg);
			g_free(msg);
        }

		g_free(path);

		return rc == ERROR_SUCCESS;
}

#endif // !win32

static gboolean optcolors(const gchar *option_name, const gchar *value, gpointer data, GError **error)
{
	static const unsigned short	valid[] = { 2,8,16 };
	int 		 				f;
	unsigned short				optval = (unsigned short) atoi(value);

	for(f=0;f<G_N_ELEMENTS(valid);f++)
	{
		if(optval == valid[f])
		{
			syscolors = optval;
			return TRUE;
		}
	}

	*error = g_error_new(ERROR_DOMAIN,EINVAL, _("Unexpected or invalid color value \"%s\""), value );
	return FALSE;
}

#if defined( HAVE_SYSLOG )
static void g_syslog(const gchar *log_domain,GLogLevelFlags log_level,const gchar *message,gpointer user_data)
{
 	static const struct _logtype
 	{
 		GLogLevelFlags	  log_level;
 		int 			  priority;
 		const gchar		* msg;
 	} logtype[] =
 	{
		{ G_LOG_FLAG_RECURSION,	LOG_INFO,		"recursion"			},
		{ G_LOG_FLAG_FATAL,		LOG_ERR,		"fatal error"		},

		/* GLib log levels */
		{ G_LOG_LEVEL_ERROR,	LOG_ERR,		"error"				},
		{ G_LOG_LEVEL_CRITICAL,	LOG_ERR,		"critical error"	},
		{ G_LOG_LEVEL_WARNING,	LOG_ERR,		"warning"			},
		{ G_LOG_LEVEL_MESSAGE,	LOG_ERR,		"message"			},
		{ G_LOG_LEVEL_INFO,		LOG_INFO,		"info"				},
		{ G_LOG_LEVEL_DEBUG,	LOG_DEBUG,		"debug"				},
 	};

	int f;

	for(f=0;f<G_N_ELEMENTS(logtype);f++)
	{
		if(logtype[f].log_level == log_level)
		{
			gchar *ptr;
			gchar *text = g_strdup_printf("%s: %s %s",logtype[f].msg,log_domain ? log_domain : "",message);
			for(ptr = text;*ptr;ptr++)
			{
				if(*ptr < ' ')
					*ptr = ' ';
			}

			syslog(logtype[f].priority,"%s",text);
			g_free(text);
			return;
		}
	}

	syslog(LOG_INFO,"%s %s",log_domain ? log_domain : "", message);
}
#endif // HAVE_SYSLOG

static void g_logfile(const gchar *log_domain,GLogLevelFlags log_level,const gchar *message,gpointer user_data)
{
    FILE *out = fopen(logfile,"a");
    if(out)
    {
        time_t  ltime;
        char    wrk[40];
        time(&ltime);
        strftime(wrk, 39, "%d/%m/%Y %H:%M:%S", localtime(&ltime));
        fprintf(out,"%s\t%s\n",wrk,message);
        fclose(out);
    }
}

static void trace_window_destroy(GtkWidget *widget, H3270 *hSession)
{
	trace("%s",__FUNCTION__);
	lib3270_set_toggle(hSession,LIB3270_TOGGLE_DS_TRACE,0);
	lib3270_set_toggle(hSession,LIB3270_TOGGLE_SCREEN_TRACE,0);
	lib3270_set_toggle(hSession,LIB3270_TOGGLE_EVENT_TRACE,0);
	lib3270_set_toggle(hSession,LIB3270_TOGGLE_NETWORK_TRACE,0);
	trace_window = NULL;
}

static void g_trace(H3270 *hSession, const char *fmt, va_list args)
{
	gchar *ptr = g_strdup_vprintf(fmt,args);

    if(tracefile)
	{
		// Has trace file, use it
		int err;

		FILE *out = fopen(tracefile,"a");
		err = errno;

		if(!out)
		{
			// Error opening trace file, notify user and disable it
			GtkWidget *popup = gtk_message_dialog_new_with_markup(
												GTK_WINDOW(pw3270_get_toplevel()),
												GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
												GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,
												_( "Can't save trace data to file %s" ),tracefile);

			gtk_window_set_title(GTK_WINDOW(popup),_("Can't open file"));

			gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(popup),"%s",strerror(err));

			gtk_dialog_run(GTK_DIALOG(popup));
			gtk_widget_destroy(popup);

			tracefile = NULL;
		}
		else
		{
			fprintf(out,"%s",ptr);
			fclose(out);
		}
	}
	else
	{
		// No trace file, open standard window
		gchar * utftext = g_convert_with_fallback(ptr,-1,"UTF-8",lib3270_get_display_charset(hSession),"?",NULL,NULL,NULL);

		if(!trace_window)
		{
			trace_window = pw3270_trace_new();
			pw3270_trace_set_destroy_on_close(trace_window,TRUE);
			g_signal_connect(trace_window, "destroy", G_CALLBACK(trace_window_destroy), hSession);
			gtk_window_set_default_size(GTK_WINDOW(trace_window),590,430);

#if GTK_CHECK_VERSION(3,4,0)
			gtk_window_set_attached_to(GTK_WINDOW(trace_window),toplevel);
#endif // GTK_CHECK_VERSION(3,4,0)

			gtk_widget_show(trace_window);
		}
		pw3270_trace_printf(trace_window,"%s",utftext);
		g_free(utftext);
	}

	g_free(ptr);
}

int main(int argc, char *argv[])
{
	static const gchar	* session_name	= PACKAGE_NAME;
	static const gchar	* host			= NULL;
	int 				  rc 			= 0;

    trace("%s",__FUNCTION__);

#if ! GLIB_CHECK_VERSION(2,32,0)
	g_thread_init(NULL);
#endif // !GLIB(2,32)

	gtk_init(&argc, &argv);

	// Setup locale
#ifdef LC_ALL
	setlocale( LC_ALL, "" );
#endif

#if defined( WIN32 )
	{
		gchar * appdir = g_win32_get_package_installation_directory_of_module(NULL);
		gchar * locdir = g_build_filename(appdir,"locale",NULL);

		g_chdir(appdir);
		bindtextdomain( PACKAGE_NAME, locdir );

		g_free(locdir);
		g_free(appdir);

	}
#elif defined(HAVE_GTKMAC)
	{
		GtkMacBundle * macbundle = gtk_mac_bundle_get_default();

		g_chdir(gtk_mac_bundle_get_datadir(macbundle));
		bindtextdomain(PACKAGE_NAME,gtk_mac_bundle_get_localedir(macbundle));

		osxapp = GTK_OSX_APPLICATION(g_object_new(GTK_TYPE_OSX_APPLICATION,NULL));

	}
#elif defined( DATAROOTDIR )
	{
		gchar * appdir = g_build_filename(DATAROOTDIR,PACKAGE_NAME,NULL);
		gchar * locdir = g_build_filename(DATAROOTDIR,"locale",NULL);

		g_chdir(appdir);
		bindtextdomain( PACKAGE_NAME, locdir);

		g_free(locdir);
		g_free(appdir);

	}
#endif // DATAROOTDIR

	bind_textdomain_codeset(PACKAGE_NAME, "UTF-8");
	textdomain(PACKAGE_NAME);

	// Process command-line options
	{
		static const GOptionEntry app_options[] =
		{
#if ! defined( WIN32 )
			{ "appname",		'a', 0, G_OPTION_ARG_CALLBACK,	appname,			N_( "Application name" ),					PACKAGE_NAME	},
#else
			{ "appname",		'a', 0, G_OPTION_ARG_STRING,	&appname,			N_( "Application name" ),					PACKAGE_NAME	},
			{ "datadir",		'd', 0, G_OPTION_ARG_CALLBACK,	datadir,			N_( "Path to application data files" ),				NULL         	},
#endif // WIN32
			{ "session",		's', 0, G_OPTION_ARG_STRING,	&session_name,		N_( "Session name" ),						PACKAGE_NAME	},
			{ "host",			'h', 0, G_OPTION_ARG_STRING,	&host,				N_( "Host to connect"),						NULL			},
			{ "colors",			'c', 0, G_OPTION_ARG_CALLBACK,	optcolors,			N_( "Set reported colors (8/16)" ),			"16"			},
			{ "systype",		't', 0, G_OPTION_ARG_STRING,	&system,			N_( "Host system type" ),					"S390"			},
			{ "toggleset",		'S', 0, G_OPTION_ARG_STRING,	&toggleset,			N_( "Set toggles ON" ),						NULL			},
			{ "togglereset",	'R', 0, G_OPTION_ARG_STRING,	&togglereset,		N_( "Set toggles OFF" ),					NULL			},
			{ "charset",	    'C', 0, G_OPTION_ARG_STRING,	&charset,		    N_( "Set host charset" ),					NULL			},

#if defined( HAVE_SYSLOG )
			{ "syslog",			'l', 0, G_OPTION_ARG_NONE,		&log_to_syslog,		N_( "Send messages to syslog" ),			NULL			},
#endif
			{ "tracefile",		'T', 0, G_OPTION_ARG_FILENAME,	&tracefile,			N_( "Set trace filename" ),					NULL			},
			{ "log",		    'L', 0, G_OPTION_ARG_FILENAME,	&logfile,		    N_( "Log to file" ),						NULL        	},

			{ NULL }
		};

		GOptionContext	* context		= g_option_context_new (_("- 3270 Emulator for Gtk"));
		GError			* error			= NULL;
		GOptionGroup 	* group			= g_option_group_new( PACKAGE_NAME, NULL, NULL, NULL, NULL);

		g_option_context_set_main_group(context, group);

		g_option_context_add_main_entries(context, app_options, NULL);

		if(!g_option_context_parse( context, &argc, &argv, &error ))
		{
			int f;
			GString 	* str;
			GtkWidget 	* dialog = gtk_message_dialog_new(	NULL,
														GTK_DIALOG_DESTROY_WITH_PARENT,
														GTK_MESSAGE_ERROR,
														GTK_BUTTONS_CANCEL,
														"%s", error->message);

			gtk_window_set_title(GTK_WINDOW(dialog),_( "Parse error" ));

			str = g_string_new( _( "<b>Valid options:</b>\n\n" ) );

			for(f=0;app_options[f].description;f++)
				g_string_append_printf(str,"--%-20s\t%s\n",app_options[f].long_name,gettext(app_options[f].description));

			gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog), "%s", str->str);

			gtk_dialog_run(GTK_DIALOG (dialog));
			gtk_widget_destroy(dialog);

			g_error_free(error);
			g_string_free(str,TRUE);

			return -1;
		}

#if defined( HAVE_SYSLOG )
		if(log_to_syslog)
		{
			openlog(g_get_prgname(), LOG_NDELAY, LOG_USER);
			g_log_set_default_handler(g_syslog,NULL);
		}
		else if(logfile)
#else
        if(logfile)
#endif // HAVE_SYSLOG
        {
			g_log_set_default_handler(g_logfile,NULL);
        }

		lib3270_set_trace_handler(g_trace);

	}

	{
		const gchar	*msg = gtk_check_version(GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);

		if(msg)
		{
			// Invalid GTK version, notify user and exit
			int 		  rc;
			GtkWidget	* dialog = gtk_message_dialog_new(	NULL,
															GTK_DIALOG_DESTROY_WITH_PARENT,
															GTK_MESSAGE_WARNING,
															GTK_BUTTONS_OK_CANCEL,
															_( "This program requires GTK version %d.%d.%d" ),GTK_MAJOR_VERSION,GTK_MINOR_VERSION,GTK_MICRO_VERSION );


			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s",msg);
			gtk_window_set_title(GTK_WINDOW(dialog),_( "GTK Version mismatch" ));

#if GTK_CHECK_VERSION(2,10,0)
			gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);
#endif

			rc = gtk_dialog_run(GTK_DIALOG (dialog));
			gtk_widget_destroy(dialog);

			if(rc != GTK_RESPONSE_OK)
				return EINVAL;
		}

	}

#if defined(WIN32)
	g_set_application_name(appname);
#endif // WIN32

	// Just in case!
	g_mkdir_with_parents(g_get_tmp_dir(),0777);

	rc = initialize();
	if(!rc)
	{
		GtkSettings *settings = gtk_settings_get_default();

		if(settings)
		{
			// http://developer.gnome.org/gtk/2.24/GtkSettings.html
			gtk_settings_set_string_property(settings,"gtk-menu-bar-accel","Menu","");
		}

		pw3270_init_plugins();
		toplevel = pw3270_new(host,systype,syscolors);
		pw3270_set_session_name(toplevel,session_name);

		if(toggleset)
		{
			gchar **str = g_strsplit(toggleset,",",-1);
			int f;

			for(f=0;str[f];f++)
				pw3270_set_toggle_by_name(toplevel,str[f],TRUE);


			g_strfreev(str);
		}

		if(togglereset)
		{
			gchar **str = g_strsplit(togglereset,",",-1);
			int f;

			for(f=0;str[f];f++)
				pw3270_set_toggle_by_name(toplevel,str[f],FALSE);

			g_strfreev(str);
		}

		if(charset)
		{
			pw3270_set_host_charset(toplevel,charset);
		}

		toplevel_setup(GTK_WINDOW(toplevel));

		if(pw3270_get_toggle(toplevel,LIB3270_TOGGLE_FULL_SCREEN))
			gtk_window_fullscreen(GTK_WINDOW(toplevel));
		else
			pw3270_restore_window(toplevel,"toplevel");

		pw3270_start_plugins(toplevel);
		gtk_window_present(GTK_WINDOW(toplevel));

#ifdef HAVE_GTKMAC
		gtk_osxapplication_ready(osxapp);
#endif // HAVE_GTKMAC

		gtk_main();

		if(trace_window)
			gtk_widget_destroy(trace_window);

		pw3270_stop_plugins(toplevel);
		pw3270_deinit_plugins();

	}

	return rc;
}
