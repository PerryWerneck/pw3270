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
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como dialog.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

 #include "globals.h"
 #include "v3270/v3270.h"

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void begin_print(GtkPrintOperation *prt, GtkPrintContext *context, gpointer user_data)
 {
 	trace("%s",__FUNCTION__);
	gtk_print_operation_cancel(prt);
 }

 static void draw_page(GtkPrintOperation *prt, GtkPrintContext *context, gint pg, gpointer user_data)
 {
 	trace("%s",__FUNCTION__);

 }

 static void done(GtkPrintOperation *prt, GtkPrintOperationResult result, gpointer user_data)
 {
 	trace("%s",__FUNCTION__);

 }

 static GObject * create_custom_widget(GtkPrintOperation *prt, gpointer user_data)
 {
 	GtkWidget * font_dialog = gtk_font_selection_new();
 	trace("%s",__FUNCTION__);
 	return G_OBJECT(font_dialog);
 }

 static void custom_widget_apply(GtkPrintOperation *prt, GtkWidget *font_dialog, gpointer user_data)
 {
 	trace("%s",__FUNCTION__);
 }

 static GtkPrintOperation * begin_print_operation(GtkAction *action, GtkWidget *widget)
 {
 	GtkPrintOperation	* print 	= gtk_print_operation_new();
//	GtkPrintSettings 	* settings	= gtk_print_settings_new();
//	GtkPageSetup 		* setup 	= gtk_page_setup_new();
// 	gchar				* ptr;

	// Basic setup
	gtk_print_operation_set_allow_async(print,FALSE);

/*
	ptr = g_strconcat(PACKAGE_NAME,".",gtk_action_get_name(action),NULL);
	gtk_print_operation_set_job_name(print,ptr);
	g_free(ptr);
*/

//	gtk_print_operation_set_custom_tab_label(print,_( "Style" ));

//	gtk_print_operation_set_show_progress(print,TRUE);

	// Common signals
    g_signal_connect(print,"begin_print",G_CALLBACK(begin_print),0);
    g_signal_connect(print,"draw_page",G_CALLBACK(draw_page),0);
    g_signal_connect(print,"done",G_CALLBACK(done),0);
//	g_signal_connect(print,"create-custom-widget",G_CALLBACK(create_custom_widget),	0);
//	g_signal_connect(print,"custom-widget-apply",G_CALLBACK(custom_widget_apply),0);

	// Finish settings
	// gtk_print_operation_set_print_settings(print,settings);
	// gtk_print_operation_set_default_page_setup(print,setup);

 	return print;
 }

 void print_all_action(GtkAction *action, GtkWidget *widget)
 {
 	GtkPrintOperation *print = begin_print_operation(action,widget);

	trace("Action %s activated on widget %p print=%p",gtk_action_get_name(action),widget,print);

	// Run Print dialog
	gtk_print_operation_run(print,GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,GTK_WINDOW(gtk_widget_get_toplevel(widget)),NULL);


	g_object_unref(print);
 }

 void print_selected_action(GtkAction *action, GtkWidget *widget)
 {
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);

 }

 void print_copy_action(GtkAction *action, GtkWidget *widget)
 {
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);

 }

