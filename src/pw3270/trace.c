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
 #include <lib3270/log.h>

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
	trace("%s",__FUNCTION__);
 }

#if GTK_CHECK_VERSION(3,0,0)
static void destroy(GtkWidget *widget)
#else
static void destroy(GtkObject *widget)
#endif
 {
	pw3270_trace * hwnd = PW3270_TRACE(widget);

	hwnd->line = NULL;

#if GTK_CHECK_VERSION(3,0,0)
	GTK_WIDGET_CLASS(pw3270_trace_parent_class)->destroy(widget);
#else
	GTK_OBJECT_CLASS(pw3270_trace_parent_class)->destroy(widget);
#endif // GTK3

 }

 static void pw3270_trace_class_init(pw3270_traceClass *klass)
 {
	GtkWindowClass	* window_class	= GTK_WINDOW_CLASS(klass);

	trace("%s",__FUNCTION__);

	window_class->activate_default	= activate_default;

#if GTK_CHECK_VERSION(3,0,0)
	{
		GtkWidgetClass	* widget_class	= GTK_WIDGET_CLASS(klass);
		widget_class->destroy = destroy;
	}
#else
	{
		GtkObjectClass *object_class = (GtkObjectClass*) klass;
		object_class->destroy = destroy;
	}
#endif // GTK3

 }

 static void pw3270_trace_init(pw3270_trace *window)
 {
 	GtkWidget *widget;
 	GtkWidget *view;
 	GtkWidget *vbox		= gtk_box_new(GTK_ORIENTATION_VERTICAL,0);

	// Trace container
	widget = gtk_scrolled_window_new(NULL,NULL);
	window->scroll = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(widget));
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(widget),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
	view = gtk_text_view_new();
	window->text = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
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

	window->button = gtk_button_new_from_stock(GTK_STOCK_OK);
	gtk_box_pack_end(GTK_BOX(widget),window->button,FALSE,FALSE,4);
	gtk_widget_set_sensitive(window->button,FALSE);
	gtk_button_set_focus_on_click(GTK_BUTTON(window->button),FALSE);

	g_signal_connect(G_OBJECT(window->button),"clicked",G_CALLBACK(gtk_window_activate_default),window);

	gtk_box_pack_start(GTK_BOX(vbox),widget,FALSE,TRUE,0);

	gtk_widget_show_all(vbox);

	gtk_container_add(GTK_CONTAINER(window),vbox);
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
	pw3270_trace	* hwnd = PW3270_TRACE(widget);
	gchar			* line = NULL;

	hwnd->line = &line;

	gtk_window_present(GTK_WINDOW(widget));
	gtk_widget_set_sensitive(hwnd->entry,TRUE);
	gtk_widget_set_sensitive(hwnd->button,TRUE);
	gtk_widget_grab_focus(hwnd->entry);

	while(hwnd->line)
	{
		gtk_main_iteration();
	}

	return line;
 }
