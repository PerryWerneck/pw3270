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

/*--[ Structs ]--------------------------------------------------------------------------------------*/

 typedef struct _print_info
 {
	GdkColor	color[V3270_COLOR_COUNT];

 } PRINT_INFO;

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void begin_print(GtkPrintOperation *prt, GtkPrintContext *context, PRINT_INFO *info)
 {
 	trace("%s",__FUNCTION__);
	gtk_print_operation_cancel(prt);
 }

 static void draw_page(GtkPrintOperation *prt, GtkPrintContext *context, gint pg, PRINT_INFO *info)
 {
 	trace("%s",__FUNCTION__);

 }

 static void done(GtkPrintOperation *prt, GtkPrintOperationResult result, PRINT_INFO *info)
 {
 	trace("%s",__FUNCTION__);


	g_free(info);
 }

#if GTK_CHECK_VERSION(3,2,0)
 static gboolean filter_monospaced(const PangoFontFamily *family,const PangoFontFace *face,gpointer data)
 {
	return pango_font_family_is_monospace((PangoFontFamily *) family);
 }
#endif // GTK(3,2,0)

 static GObject * create_custom_widget(GtkPrintOperation *prt, PRINT_INFO *info)
 {
 	static const gchar	* label[]	= { "Font:", "Colors:" };
	GtkWidget			* container = gtk_table_new(2,2,FALSE);
	GtkWidget			* widget;
	int					  f;

	for(f=0;f<G_N_ELEMENTS(label);f++)
	{
		widget = gtk_label_new(label[f]);
		gtk_misc_set_alignment(GTK_MISC(widget),0,0.5);
		gtk_table_attach(GTK_TABLE(container),widget,0,1,f,f+1,GTK_FILL,GTK_FILL,0,0);
	}

	// Font selection button
	widget = gtk_font_button_new();
	gtk_font_button_set_show_size(GTK_FONT_BUTTON(widget),FALSE);
#if GTK_CHECK_VERSION(3,2,0)
	gtk_font_chooser_set_filter_func(widget,filter_monospaced,0);
#endif // GTK(3,2,0)
	gtk_table_attach(GTK_TABLE(container),widget,1,2,0,1,GTK_EXPAND|GTK_FILL,GTK_FILL,5,0);

	// Color scheme dropdown

	// Show and return
	gtk_widget_show_all(container);
 	return G_OBJECT(container);
 }

 static void custom_widget_apply(GtkPrintOperation *prt, GtkWidget *font_dialog, gpointer user_data)
 {
 	trace("%s",__FUNCTION__);
 }

 static GtkPrintOperation * begin_print_operation(GtkAction *action, GtkWidget *widget)
 {
 	PRINT_INFO			* info		= g_new0(PRINT_INFO,1);
 	GtkPrintOperation	* print 	= gtk_print_operation_new();
//	GtkPrintSettings 	* settings	= gtk_print_settings_new();
//	GtkPageSetup 		* setup 	= gtk_page_setup_new();
// 	gchar				* ptr;

	// Basic setup
	gtk_print_operation_set_allow_async(print,TRUE);

/*
	ptr = g_strconcat(PACKAGE_NAME,".",gtk_action_get_name(action),NULL);
	gtk_print_operation_set_job_name(print,ptr);
	g_free(ptr);
*/

	gtk_print_operation_set_custom_tab_label(print,_( "Style" ));

//	gtk_print_operation_set_show_progress(print,TRUE);

	// Common signals
    g_signal_connect(print,"begin_print",G_CALLBACK(begin_print),info);
    g_signal_connect(print,"draw_page",G_CALLBACK(draw_page),info);
    g_signal_connect(print,"done",G_CALLBACK(done),info);
	g_signal_connect(print,"create-custom-widget",G_CALLBACK(create_custom_widget),	info);
	g_signal_connect(print,"custom-widget-apply",G_CALLBACK(custom_widget_apply), info);

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

