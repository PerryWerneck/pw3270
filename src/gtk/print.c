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

 #include "globals.h"
 #include "v3270/v3270.h"

/*--[ Structs ]--------------------------------------------------------------------------------------*/

 typedef struct _print_info
 {
	GdkColor				  color[V3270_COLOR_COUNT];
	int 					  show_selection : 1;
	int						  all : 1;

	H3270					* session;
	gchar					* font;
	guint					  fontsize;
	cairo_font_weight_t		  fontweight;
	gchar					* colorname;
	int						  baddr;
	int						  rows;
	int						  cols;
	int						  pages;
	cairo_font_extents_t	  extents;
	double					  left;
	double					  width;
	double					  height;
	cairo_scaled_font_t		* font_scaled;

 } PRINT_INFO;

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void setup_font(GtkPrintContext * context, PRINT_INFO *info)
 {
 	cairo_t *cr = gtk_print_context_get_cairo_context(context);

	cairo_select_font_face(cr, info->font, CAIRO_FONT_SLANT_NORMAL, info->fontweight);

	info->font_scaled = cairo_get_scaled_font(cr);
	cairo_scaled_font_reference(info->font_scaled);
	cairo_scaled_font_extents(info->font_scaled,&info->extents);

	info->width  = ((double) info->cols) * info->extents.max_x_advance;
	info->height = ((double) info->rows) * (info->extents.height + info->extents.descent);

	// Center image
	info->left = (gtk_print_context_get_width(context)-info->width)/2;
	if(info->left < 2)
		info->left = 2;


 }

 static void begin_print(GtkPrintOperation *prt, GtkPrintContext *context, PRINT_INFO *info)
 {
 	setup_font(context,info);
	gtk_print_operation_set_n_pages(prt,1);
 }

 static void draw_page(GtkPrintOperation *prt, GtkPrintContext *context, gint pg, PRINT_INFO *info)
 {
 	int				  row;
 	int				  col;
	cairo_t			* cr 	= gtk_print_context_get_cairo_context(context);
	int		  		  baddr	= info->baddr;
	GdkRectangle	  rect;

	cairo_set_scaled_font(cr,info->font_scaled);

	memset(&rect,0,sizeof(rect));
	rect.y 		= 2;
	rect.height	= (info->extents.height + info->extents.descent);
	rect.width	= info->extents.max_x_advance;

	gdk_cairo_set_source_color(cr,info->color+V3270_COLOR_BACKGROUND);
	cairo_rectangle(cr, info->left-2, 0, (rect.width*info->cols)+4, (rect.height*info->rows)+4);
	cairo_fill(cr);
	cairo_stroke(cr);

	rect.width++;
	rect.height++;

	for(row = 0; row < info->rows; row++)
	{
		rect.x = info->left;
		for(col = 0; col < info->cols; col++)
		{
			unsigned char	c;
			unsigned short	attr;

			if(!lib3270_get_element(info->session,baddr++,&c,&attr) && (info->all || (attr & LIB3270_ATTR_SELECTED)))
			{
				if(!info->show_selection)
					attr &= ~LIB3270_ATTR_SELECTED;
				v3270_draw_element(cr,c,attr,info->session,info->extents.height,&rect,info->color);
			}

			rect.x += (rect.width-1);
		}
		rect.y += (rect.height-1);

	}
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
 	const gchar				* name  = gtk_font_button_get_font_name(widget);
 	PangoFontDescription	* descr = pango_font_description_from_string(name);

	if(!descr)
		return;

 	if(info->font)
		g_free(info->font);

	info->font			= g_strdup(pango_font_description_get_family(descr));
	info->fontsize		= pango_font_description_get_size(descr);
	info->fontweight	= CAIRO_FONT_WEIGHT_NORMAL;

	if(pango_font_description_get_weight(descr) == PANGO_WEIGHT_BOLD)
		info->fontweight = CAIRO_FONT_WEIGHT_BOLD;

	pango_font_description_free(descr);

	set_string_to_config("print","font",name);
	trace("Font set to \"%s\" with size %d",info->font,info->fontsize);
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

//	trace("%s: %s->%s",__FUNCTION__,info->colorname,new_colors);

	if(*info->colorname)
		g_free(info->colorname);

	info->colorname = new_colors;

 }

 static void toggle_show_selection(GtkToggleButton *togglebutton,PRINT_INFO *info)
 {
 	gboolean active = gtk_toggle_button_get_active(togglebutton);
 	info->show_selection = active ? 1 : 0;
 	set_boolean_to_config("print","selection",active);
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
	GtkWidget			* container = gtk_table_new(3,2,FALSE);
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
	font_set((GtkFontButton *) widget,info);
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

	// Selection checkbox
	widget = gtk_check_button_new_with_label(_("Print selection box"));

	if(info->all)
	{
		info->show_selection = get_boolean_from_config("print","selection",FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),info->show_selection);
		g_signal_connect(G_OBJECT(widget),"toggled",G_CALLBACK(toggle_show_selection),info);
	}
	else
	{
		gtk_widget_set_sensitive(widget,FALSE);
	}

	gtk_table_attach(GTK_TABLE(container),widget,1,2,2,3,GTK_EXPAND|GTK_FILL,GTK_FILL,5,0);

	// Show and return
	gtk_widget_show_all(container);
 	return G_OBJECT(container);
 }

 static void custom_widget_apply(GtkPrintOperation *prt, GtkWidget *widget, PRINT_INFO *info)
 {
 	trace("%s",__FUNCTION__);
	set_string_to_config("print","colors",info->colorname);
	v3270_set_color_table(info->color,info->colorname);
 }

 static GtkPrintOperation * begin_print_operation(GtkAction *action, GtkWidget *widget, PRINT_INFO **info)
 {
 	GtkPrintOperation	* print 	= gtk_print_operation_new();
//	GtkPrintSettings 	* settings	= gtk_print_settings_new();
//	GtkPageSetup 		* setup 	= gtk_page_setup_new();
 	const gchar 		* attr;

 	*info = g_new0(PRINT_INFO,1);
 	(*info)->session = v3270_get_session(widget);
 	(*info)->fontweight = CAIRO_FONT_WEIGHT_NORMAL;

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

	gtk_print_operation_set_custom_tab_label(print,_( "Options" ));
	gtk_print_operation_set_show_progress(print,TRUE);

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

 	lib3270_get_screen_size(info->session,&info->rows,&info->cols);

	info->all = 1;
    g_signal_connect(print,"begin_print",G_CALLBACK(begin_print),info);
    g_signal_connect(print,"draw_page",G_CALLBACK(draw_page),info);

	// Run Print dialog
	gtk_print_operation_run(print,GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,GTK_WINDOW(gtk_widget_get_toplevel(widget)),NULL);


	g_object_unref(print);
 }

 void print_selected_action(GtkAction *action, GtkWidget *widget)
 {
 	PRINT_INFO			* info = NULL;
 	int					  start, end, rows;
 	GtkPrintOperation 	* print = begin_print_operation(action,widget,&info);;

	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);

 	if(lib3270_get_selected_addr(info->session,&start,&end))
	{
		g_warning("Can't get selected addresses for action %s",gtk_action_get_name(action));
		g_object_unref(print);
		return;
	}

	info->baddr = start;
 	lib3270_get_screen_size(info->session,&rows,&info->cols);

	info->rows = ((end / info->cols) - (start / info->cols))+1;

	trace("First row: %d  End row: %d  Num rows: %d",(start / info->cols),(end / info->cols),info->rows);

	info->all = 0;
    g_signal_connect(print,"begin_print",G_CALLBACK(begin_print),info);
    g_signal_connect(print,"draw_page",G_CALLBACK(draw_page),info);

	// Run Print dialog
	gtk_print_operation_run(print,GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,GTK_WINDOW(gtk_widget_get_toplevel(widget)),NULL);


	g_object_unref(print);
 }

 void print_copy_action(GtkAction *action, GtkWidget *widget)
 {
	trace("Action %s activated on widget %p",gtk_action_get_name(action),widget);

 }

