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
 * Este programa está nomeado como trace.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <gtk/gtk.h>

 #define ENABLE_NLS
 #define GETTEXT_PACKAGE PACKAGE_NAME

 #include <libintl.h>
 #include <glib/gi18n.h>

 #include <pw3270/trace.h>
 #include "common/common.h"

#if defined( HAVE_SYSLOG )
 #include <syslog.h>
#endif // HAVE_SYSLOG

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 G_BEGIN_DECLS

 struct _pw3270_traceClass
 {
	GtkWindowClass parent_class;
 };

 struct _pw3270_trace
 {
	GtkWindow		  parent;
	GtkAdjustment	* scroll;
	GtkTextBuffer	* text;
	GtkWidget		* entry;
	GtkWidget		* button;
	gchar			**line;
	guint 			  log_handler;
	gboolean		* enabled;
	gboolean		  destroy_on_close;
 };

 const GtkWindowClass	* pw3270_trace_get_parent_class(void);

 G_END_DECLS

 G_DEFINE_TYPE(pw3270_trace, pw3270_trace, GTK_TYPE_WINDOW);

/*--[ Implement ]------------------------------------------------------------------------------------*/

 const GtkWindowClass * pw3270_trace_get_parent_class(void)
 {
	trace("%s",__FUNCTION__);
	return GTK_WINDOW_CLASS(pw3270_trace_parent_class);
 }

 static void activate_default(GtkWindow *window)
 {
	pw3270_trace * hwnd = PW3270_TRACE(window);

	trace("%s",__FUNCTION__);

	if(hwnd->enabled)
	{
		if(*hwnd->line)
			g_free(*hwnd->line);

		*hwnd->line = g_strdup(gtk_entry_get_text(GTK_ENTRY(hwnd->entry)));

		gtk_widget_set_sensitive(hwnd->entry,FALSE);
		gtk_widget_set_sensitive(hwnd->button,FALSE);

		*hwnd->enabled = FALSE;
	}

 }

#if GTK_CHECK_VERSION(3,0,0)
static void destroy(GtkWidget *widget)
#else
static void destroy(GtkObject *widget)
#endif
 {
	pw3270_trace * hwnd = PW3270_TRACE(widget);

	if(hwnd->log_handler)
	{
		g_log_remove_handler(NULL,hwnd->log_handler);
		hwnd->log_handler = 0;
	}

	if(hwnd->line)
		*hwnd->line = NULL;

	if(hwnd->enabled)
		*hwnd->enabled = FALSE;

 }

 static gboolean delete_event(GtkWidget *widget, GdkEventAny  *event)
 {
	pw3270_trace * hwnd = PW3270_TRACE(widget);

 	trace("%s destroy=%s",__FUNCTION__,hwnd->destroy_on_close ? "Yes" : "No");

	if(hwnd->line)
		*hwnd->line = NULL;

	if(hwnd->enabled)
		*hwnd->enabled = FALSE;

	if(hwnd->destroy_on_close)
		return FALSE;

	gtk_widget_hide(widget);
	return TRUE;
 }

 static void pw3270_trace_class_init(pw3270_traceClass *klass)
 {
	GtkWindowClass	* window_class	= GTK_WINDOW_CLASS(klass);
	GtkWidgetClass	* widget_class	= GTK_WIDGET_CLASS(klass);

	trace("%s",__FUNCTION__);

	window_class->activate_default	= activate_default;
	widget_class->delete_event 		= delete_event;

#if GTK_CHECK_VERSION(3,0,0)
	{
		widget_class->destroy = destroy;
	}
#else
	{
		GtkObjectClass *object_class = (GtkObjectClass*) klass;
		object_class->destroy = destroy;
	}
#endif // GTK3

 }

 static void activate(GtkButton *button, GtkWindow *window)
 {
 	trace("%s",__FUNCTION__);
 	activate_default(window);
 }

 static void menu_save(GtkWidget *button, pw3270_trace *window)
 {
 	GtkWindow	* toplevel		= GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(window)));
	GtkWidget	* dialog;

	dialog = gtk_file_chooser_dialog_new( 	_( "Save trace file" ),
											toplevel,
											GTK_FILE_CHOOSER_ACTION_SAVE,
											GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
											GTK_STOCK_SAVE,		GTK_RESPONSE_ACCEPT,
											NULL );

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),g_get_user_special_dir(G_USER_DIRECTORY_DOCUMENTS));

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		if(filename)
		{
			GError		* error = NULL;
			gchar		* text;
			GtkTextIter	  start;
			GtkTextIter	  end;

			gtk_text_buffer_get_start_iter(window->text,&start);
			gtk_text_buffer_get_end_iter(window->text,&end);
			text = gtk_text_buffer_get_text(window->text,&start,&end,FALSE);

			g_file_set_contents(filename,text,-1,&error);

			g_free(text);

			if(error)
			{
				GtkWidget *popup = gtk_message_dialog_new_with_markup(GTK_WINDOW(dialog),GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,_( "Can't save %s" ),filename);

				gtk_window_set_title(GTK_WINDOW(popup),_("Can't save file"));

				gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(popup),"%s",error->message);
				g_error_free(error);

				gtk_dialog_run(GTK_DIALOG(popup));
				gtk_widget_destroy(popup);

			}

			g_free(filename);
		}
	}

	gtk_widget_destroy(dialog);

 }

 static void menu_close(GtkWidget *button, GtkWidget *window)
 {
	gtk_widget_destroy(window);
 }

 struct submenu
 {
	const gchar * stock_id;
	GCallback	  action;
 };

 static void build_menu(GtkWidget *menubar, pw3270_trace *window, const gchar *name, const struct submenu *item, size_t sz)
 {
 	int			  f;
	GtkWidget	* menu		= gtk_menu_new();
	GtkWidget	* topitem	= gtk_image_menu_item_new_from_stock( name, NULL );

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(topitem), menu);

	for(f=0;f<sz;f++)
	{
		GtkWidget *widget = gtk_image_menu_item_new_from_stock( item[f].stock_id, NULL );
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),widget);
		g_signal_connect(G_OBJECT(widget), "activate",item[f].action,window);
	}

	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), topitem);
 }

 static void glog(const gchar *log_domain,GLogLevelFlags log_level,const gchar *message,GtkWidget *window)
 {
	#ifndef LOG_INFO
		#define LOG_INFO 0
	#endif // LOG_INFO

	#ifndef LOG_ERR
		#define LOG_ERR 0
	#endif // LOG_ERR

	#ifndef LOG_DEBUG
		#define LOG_DEBUG 0
	#endif // LOG_DEBUG

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

			pw3270_trace_printf(window,"%s\n",text);

#ifdef HAVE_SYSLOG
			syslog(logtype[f].priority,"%s",text);
#endif // HAVE_SYSLOG
			g_free(text);
			return;
		}
	}

	pw3270_trace_printf(window,"%s %s\n",log_domain ? log_domain : "", message);

#ifdef HAVE_SYSLOG
	syslog(LOG_INFO,"%s %s",log_domain ? log_domain : "", message);
#endif // HAVE_SYSLOG

 }

 static void pw3270_trace_init(pw3270_trace *window)
 {
 	GtkWidget				* widget;
 	GtkWidget				* view;
 	GtkWidget				* vbox		= gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
 	gchar					* fontname	= get_string_from_config("trace", "fontname", "Monospace 8");
	PangoFontDescription	* fontdesc	= pango_font_description_from_string("Monospace 8");

	g_free(fontname);

	// Top menu
	{

		static const struct submenu filemenu[] =
		{
			{ GTK_STOCK_SAVE_AS,	G_CALLBACK(menu_save)	},
			{ GTK_STOCK_QUIT,		G_CALLBACK(menu_close)	},

		};

		widget = gtk_menu_bar_new();

		build_menu(widget, window, GTK_STOCK_FILE, filemenu, G_N_ELEMENTS(filemenu));

		gtk_box_pack_start(GTK_BOX(vbox),widget,FALSE,TRUE,0);
	}

	// Trace container
	widget = gtk_scrolled_window_new(NULL,NULL);
	window->scroll = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(widget));
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	view = gtk_text_view_new();

#if GTK_CHECK_VERSION(3,0,0)
	gtk_widget_override_font(GTK_WIDGET(view), fontdesc);
#else
	gtk_widget_modify_font(GTK_WIDGET(view), fontdesc);
#endif // GTK_CHECK_VERSION

	window->text = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view), TRUE);

#if GTK_CHECK_VERSION(3,8,0)
	gtk_container_add(GTK_CONTAINER(widget),view);
#else
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(widget),view);
#endif // GTK_CHECK_VERSION
	gtk_box_pack_start(GTK_BOX(vbox),widget,TRUE,TRUE,0);

	// Edit box
	widget = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
	gtk_box_pack_start(GTK_BOX(widget),gtk_label_new( _( "Command:" )),FALSE,TRUE,4);
	window->entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(widget),window->entry,TRUE,TRUE,4);
	gtk_widget_set_sensitive(window->entry,FALSE);
	g_signal_connect(G_OBJECT(window->entry),"activate",G_CALLBACK(activate),window);

	window->button = gtk_button_new_from_stock(GTK_STOCK_OK);
	gtk_box_pack_end(GTK_BOX(widget),window->button,FALSE,FALSE,4);
	gtk_widget_set_sensitive(window->button,FALSE);
	gtk_button_set_focus_on_click(GTK_BUTTON(window->button),FALSE);

	g_signal_connect(G_OBJECT(window->button),"clicked",G_CALLBACK(activate),window);

	gtk_box_pack_start(GTK_BOX(vbox),widget,FALSE,TRUE,0);

	gtk_widget_show_all(vbox);

	gtk_container_add(GTK_CONTAINER(window),vbox);

	pango_font_description_free(fontdesc);

	window->log_handler = g_log_set_handler(NULL,G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION,(GLogFunc) glog,window);
	trace("Log handler set to %d",window->log_handler);

 }

 GtkWidget * pw3270_trace_new(void)
 {
	return g_object_new(PW3270_TYPE_TRACE, NULL);
 }

 void pw3270_trace_vprintf(GtkWidget *widget, const char *fmt, va_list args)
 {
	GtkTextIter		  itr;
	gchar			* msg;
	pw3270_trace	* hwnd = PW3270_TRACE(widget);

	gtk_text_buffer_get_end_iter(hwnd->text,&itr);

	msg = g_strdup_vprintf(fmt,args);
	gtk_text_buffer_insert(hwnd->text,&itr,msg,strlen(msg));
	g_free(msg);

	gtk_text_buffer_get_end_iter(hwnd->text,&itr);

#if GTK_CHECK_VERSION(2,14,0)
	gtk_adjustment_set_value(hwnd->scroll,gtk_adjustment_get_upper(hwnd->scroll));
#else
	gtk_adjustment_set_value(hwnd->scroll,(GTK_ADJUSTMENT(hwnd->scroll))->upper);
#endif 	//

 }

 void pw3270_trace_printf(GtkWidget *widget, const char *fmt, ... )
 {
	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	pw3270_trace_vprintf(widget,fmt,arg_ptr);
	va_end(arg_ptr);
 }

 LIB3270_EXPORT gchar * pw3270_trace_get_command(GtkWidget *widget)
 {
	pw3270_trace	* hwnd		= PW3270_TRACE(widget);
	gchar			* line 		= NULL;
	gboolean		  enabled	= TRUE;

	hwnd->line		= &line;
	hwnd->enabled	= &enabled;

	gtk_window_present(GTK_WINDOW(widget));
	gtk_widget_set_sensitive(hwnd->entry,TRUE);
	gtk_widget_set_sensitive(hwnd->button,TRUE);
	gtk_widget_grab_focus(hwnd->entry);

	while(enabled)
	{
		gtk_main_iteration();
	}

	hwnd->line		= NULL;
	hwnd->enabled	= NULL;

	return line;
 }

 LIB3270_EXPORT void pw3270_trace_set_destroy_on_close(GtkWidget *widget,gboolean on)
 {
 	PW3270_TRACE(widget)->destroy_on_close = on;
 }
