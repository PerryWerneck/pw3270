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
	GdkColor				  color[V3270_COLOR_COUNT];
	H3270					* session;
	gchar					* font;
	gchar					* colorname;
	int						  rows;
	int						  cols;
	int						  pages;
	cairo_font_extents_t	  extents;
	cairo_scaled_font_t		* font_scaled;

 } PRINT_INFO;

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void setup_font(cairo_t *cr, PRINT_INFO *info)
 {
	cairo_select_font_face(cr, info->font, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

	info->font_scaled = cairo_get_scaled_font(cr);
	cairo_scaled_font_reference(info->font_scaled);
	cairo_scaled_font_extents(info->font_scaled,&info->extents);

 }

 static void begin_print_all(GtkPrintOperation *prt, GtkPrintContext *context, PRINT_INFO *info)
 {
 	cairo_t *cr = gtk_print_context_get_cairo_context(context);

 	lib3270_get_screen_size(info->session,&info->rows,&info->cols);
 	setup_font(cr,info);

	gtk_print_operation_set_n_pages(prt,1);
 }

 static void draw_page(GtkPrintOperation *prt, GtkPrintContext *context, gint pg, PRINT_INFO *info)
 {
	cairo_t *cr = gtk_print_context_get_cairo_context(context);

	cairo_set_scaled_font(cr,info->font_scaled);

	cairo_move_to(cr,0,0);
	cairo_show_text(cr, "Teste");

	cairo_stroke(cr);
 }

 static void done(GtkPrintOperation *prt, GtkPrintOperationResult result, PRINT_INFO *info)
 {
 	trace("%s",__FUNCTION__);

	if(info->font_scaled)
		cairo_scaled_font_destroy(info->font_scaled);

	if(info->font)
		g_free(info->font);

	if(info->colorname)
		g_free(info->colorname);

	g_free(info);
 }

#if GTK_CHECK_VERSION(3,2,0)
 static gboolean filter_monospaced(const PangoFontFamily *family,const PangoFontFace *face,gpointer data)
 {
	return pango_font_family_is_monospace((PangoFontFamily *) family);
 }
#endif // GTK(3,2,0)

 static void font_set(GtkFontButton *widget, PRINT_INFO *info)
 {
 	if(info->font)
		g_free(info->font);

	info->font = g_strdup(gtk_font_button_get_font_name(widget));
	set_string_to_config("print","font","%s",info->font);
	trace("Font set to \"%s\"",info->font);
 }

 static void color_scheme_changed(GtkComboBox *widget,PRINT_INFO *info)
 {
 	gchar *new_colors = NULL;

#if GTK_CHECK_VERSION(3,0,0)

	new_colors = g_strdup(gtk_combo_box_get_active_id(GTK_COMBO_BOX(widget)));

#else

	GValue		value	= { 0, };
	GtkTreeIter iter;

	if(!gtk_combo_box_get_active_iter(widget,&iter))
		return;

	gtk_tree_model_get_value(gtk_combo_box_get_model(widget),&iter,1,&value);
	new_colors = g_strdup(g_value_get_string(&value));

#endif

	if(!info->colorname)
		return;

	trace("%s: %s->%s",__FUNCTION__,info->colorname,new_colors);

	if(*info->colorname)
		g_free(info->colorname);

	info->colorname = new_colors;
 }

 static GObject * create_custom_widget(GtkPrintOperation *prt, PRINT_INFO *info)
 {
	static const gchar *def_colors =	"white," // V3270_COLOR_BACKGROUND
										"black," // V3270_COLOR_BLUE
										"black," // V3270_COLOR_RED
										"black," // V3270_COLOR_PINK
										"black," // V3270_COLOR_GREEN
										"black," // V3270_COLOR_TURQUOISE
										"black," // V3270_COLOR_YELLOW
										"black," // V3270_COLOR_WHITE
										"black," // V3270_COLOR_BLACK
										"black," // V3270_COLOR_DARK_BLUE
										"black," // V3270_COLOR_ORANGE
										"black," // V3270_COLOR_PURPLE
										"black," // V3270_COLOR_DARK_GREEN
										"black," // V3270_COLOR_DARK_TURQUOISE
										"black," // V3270_COLOR_MUSTARD
										"black," // V3270_COLOR_GRAY
										"black," // V3270_COLOR_FIELD_DEFAULT
										"black," // V3270_COLOR_FIELD_INTENSIFIED
										"black," // V3270_COLOR_FIELD_PROTECTED
										"black," // V3270_COLOR_FIELD_PROTECTED_INTENSIFIED
										"black," // V3270_COLOR_SELECTED_BG
										"white," // V3270_COLOR_SELECTED_FG
										"black," // V3270_COLOR_SELECTED_BORDER
										"black," // V3270_COLOR_CURSOR
										"black," // V3270_COLOR_CROSS_HAIR
										"white," // V3270_COLOR_OIA_BACKGROUND
										"black," // V3270_COLOR_OIA
										"black," // V3270_COLOR_OIA_SEPARATOR
										"black," // V3270_COLOR_OIA_STATUS_OK
										"black"; // V3270_COLOR_OIA_STATUS_INVALID

 	static const gchar	* label[]	= { N_( "Font:" ), N_( "Color scheme:" ) };
	GtkWidget			* container = gtk_table_new(2,2,FALSE);
	GtkWidget			* widget;
	int					  f;

	for(f=0;f<G_N_ELEMENTS(label);f++)
	{
		widget = gtk_label_new(gettext(label[f]));
		gtk_misc_set_alignment(GTK_MISC(widget),0,0.5);
		gtk_table_attach(GTK_TABLE(container),widget,0,1,f,f+1,GTK_FILL,GTK_FILL,0,0);
	}

	// Font selection button
	widget = gtk_font_button_new();
#if GTK_CHECK_VERSION(3,2,0)
	gtk_font_chooser_set_filter_func((GtkFontChooser *) widget,filter_monospaced,NULL,NULL);
#endif // GTK(3,2,0)
	gtk_table_attach(GTK_TABLE(container),widget,1,2,0,1,GTK_EXPAND|GTK_FILL,GTK_FILL,5,0);

	info->font = get_string_from_config("print","font","Courier 10");
	gtk_font_button_set_font_name((GtkFontButton *) widget,info->font);
    g_signal_connect(G_OBJECT(widget),"font-set",G_CALLBACK(font_set),info);

	// Color scheme dropdown
#if GTK_CHECK_VERSION(3,0,0)
	widget = gtk_combo_box_text_new();
#else
	widget = gtk_combo_box_new();
#endif // GTK(3,0,0)

	info->colorname = get_string_from_config("print","colors",def_colors);
	load_color_schemes(widget,info->colorname);

	g_signal_connect(G_OBJECT(widget),"changed",G_CALLBACK(color_scheme_changed),info);

	gtk_table_attach(GTK_TABLE(container),widget,1,2,1,2,GTK_EXPAND|GTK_FILL,GTK_FILL,5,0);

	// Show and return
	gtk_widget_show_all(container);
 	return G_OBJECT(container);
 }

 static void custom_widget_apply(GtkPrintOperation *prt, GtkWidget *font_dialog, gpointer user_data)
 {
 	trace("%s",__FUNCTION__);
 }

 static GtkPrintOperation * begin_print_operation(GtkAction *action, GtkWidget *widget, PRINT_INFO **info)
 {
 	GtkPrintOperation	* print 	= gtk_print_operation_new();
//	GtkPrintSettings 	* settings	= gtk_print_settings_new();
//	GtkPageSetup 		* setup 	= gtk_page_setup_new();
 	const gchar 		* attr;

 	*info = g_new0(PRINT_INFO,1);
 	(*info)->session = v3270_get_session(widget);

	// Basic setup
	gtk_print_operation_set_allow_async(print,TRUE);

	attr = (const gchar *) g_object_get_data(G_OBJECT(action),"jobname");
	if(attr)
	{
		gtk_print_operation_set_job_name(print,attr);
	}
	else
	{
		gchar *ptr = g_strconcat(PACKAGE_NAME,".",gtk_action_get_name(action),NULL);
		gtk_print_operation_set_job_name(print,ptr);
		g_free(ptr);
	}

	gtk_print_operation_set_custom_tab_label(print,_( "Style" ));

//	gtk_print_operation_set_show_progress(print,TRUE);

	// Common signals
    g_signal_connect(print,"done",G_CALLBACK(done),*info);
	g_signal_connect(print,"create-custom-widget",G_CALLBACK(create_custom_widget),	*info);
	g_signal_connect(print,"custom-widget-apply",G_CALLBACK(custom_widget_apply), *info);

	// Finish settings
	// gtk_print_operation_set_print_settings(print,settings);
	// gtk_print_operation_set_default_page_setup(print,setup);

 	return print;
 }

 void print_all_action(GtkAction *action, GtkWidget *widget)
 {
 	PRINT_INFO			* info = NULL;
 	GtkPrintOperation 	* print = begin_print_operation(action,widget,&info);

	trace("Action %s activated on widget %p print=%p",gtk_action_get_name(action),widget,print);

    g_signal_connect(print,"begin_print",G_CALLBACK(begin_print_all),info);
    g_signal_connect(print,"draw_page",G_CALLBACK(draw_page),info);

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

