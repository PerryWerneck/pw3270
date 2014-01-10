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

 #include "globals.h"
 #include <pw3270/v3270.h>
 #include <lib3270/selection.h>
 #include <lib3270/trace.h>

 #define AUTO_FONT_SIZE 1

#ifdef AUTO_FONT_SIZE
	#define FONT_CONFIG 	"font-family"
	#define DEFAULT_FONT	"Courier New"
#else
	#define FONT_CONFIG 	"font"
	#define DEFAULT_FONT	"Courier New 10"
#endif // AUTO_FONT_SIZE

/*--[ Structs ]--------------------------------------------------------------------------------------*/

 typedef struct _print_info
 {
	GdkRGBA				  color[V3270_COLOR_COUNT];
	int 					  show_selection : 1;
	PW3270_SRC				  src;

	GtkWidget				* widget;
	H3270					* session;

	int						  baddr;
	int						  rows;
	int						  cols;				/**< Max line width */
	int						  pages;
	int						  lpp;				/**< Lines per page */
	cairo_font_extents_t	  extents;
	double					  left;
	double					  width;			/**< Report width */
	double					  height;			/**< Report height (all pages) */
	cairo_scaled_font_t		* font_scaled;

	gchar					* font;				/**< Font name */
	gchar					**text;

 } PRINT_INFO;


/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void begin_print(GtkPrintOperation *prt, GtkPrintContext *context, PRINT_INFO *info)
 {
 	cairo_t * cr	= gtk_print_context_get_cairo_context(context);
 	gchar	* font	= get_string_from_config("print",FONT_CONFIG,DEFAULT_FONT);

	trace("%s: operation=%p context=%p font=\"%s\"",__FUNCTION__,prt,context,font);

	// Setup font

	if(*font)
	{
		PangoFontDescription * descr = pango_font_description_from_string(font);
		if(descr)
		{
			cairo_select_font_face(cr,	pango_font_description_get_family(descr),
										CAIRO_FONT_SLANT_NORMAL,
										pango_font_description_get_weight(descr) == PANGO_WEIGHT_BOLD ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);

#ifdef AUTO_FONT_SIZE
            {
                double                  width    = gtk_print_context_get_width(context);
#if GTK_CHECK_VERSION(3,0,0)
                double                  cols     = (double) info->cols;
#else
                double                  cols     = (double) (info->cols+5);
#endif // GTK(3,0,0)
                double                  current  = width / cols;
                double                  valid    = current;
               	cairo_font_extents_t    extents;

                do
                {
                    valid   = current;
                    current = valid +1.0;
                    cairo_set_font_size(cr,current);
                    cairo_font_extents(cr,&extents);
                }
                while(  (cols * extents.max_x_advance) < width );

				trace("Font size: %d",(int) valid);
				cairo_set_font_size(cr,valid);

			}
#endif // AUTO_FONT_SIZE

			pango_font_description_free(descr);
		}
	}
	g_free(font);

	info->font_scaled = cairo_get_scaled_font(cr);
	cairo_scaled_font_reference(info->font_scaled);
	cairo_scaled_font_extents(info->font_scaled,&info->extents);

	info->width  = ((double) info->cols) * info->extents.max_x_advance;
	info->height = ((double) info->rows) * (info->extents.height + info->extents.descent);

	// Center image
	info->left = (gtk_print_context_get_width(context)-info->width)/2;
	if(info->left < 2)
		info->left = 2;

	// Setup page size
	info->lpp	= (gtk_print_context_get_height(context) / (info->extents.height + info->extents.descent));
	info->pages = (info->rows / info->lpp)+1;

	trace("%d lines per page, %d pages to print",info->lpp,info->pages);

	gtk_print_operation_set_n_pages(prt,info->pages);
 }

 static void draw_screen(GtkPrintOperation *prt, GtkPrintContext *context, gint pg, PRINT_INFO *info)
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

	// Clear page
	gdk_cairo_set_source_rgba(cr,info->color+V3270_COLOR_BACKGROUND);
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

			if(!lib3270_get_element(info->session,baddr++,&c,&attr) && (info->src == PW3270_SRC_ALL || (attr & LIB3270_ATTR_SELECTED)))
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

#ifdef WIN32

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

#endif // WIN32

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

#ifdef HAVE_WIN_REGISTRY
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

	if(info->font_scaled)
		cairo_scaled_font_destroy(info->font_scaled);

	if(info->text)
		g_strfreev(info->text);

	if(info->font)
		g_free(info->font);

	g_free(info);
 }

#ifndef AUTO_FONT_SIZE

#if GTK_CHECK_VERSION(3,2,0)
 static gboolean filter_monospaced(const PangoFontFamily *family,const PangoFontFace *face,gpointer data)
 {
	return pango_font_family_is_monospace((PangoFontFamily *) family);
 }
#endif // GTK(3,2,0)

 static void font_set(GtkFontButton *widget, PRINT_INFO *info)
 {
	trace("%s font=%p",__FUNCTION__,info->font);
 	if(info->font)
		g_free(info->font);
	info->font = g_strdup(gtk_font_button_get_font_name(widget));
 }

#else

 static void font_name_changed(GtkComboBox *combo, PRINT_INFO *info)
 {
	GValue		  value	= { 0, };
	GtkTreeIter	  iter;

	if(!gtk_combo_box_get_active_iter(combo,&iter))
		return;

	gtk_tree_model_get_value(gtk_combo_box_get_model(combo),&iter,0,&value);

	if(info->font)
		g_free(info->font);

	info->font = g_value_dup_string(&value);

 }

#endif // !AUTO_FONT_SIZE

 static void toggle_show_selection(GtkToggleButton *togglebutton,PRINT_INFO *info)
 {
 	gboolean active = gtk_toggle_button_get_active(togglebutton);
 	info->show_selection = active ? 1 : 0;
 	set_boolean_to_config("print","selection",active);
 }

 static void load_settings(PRINT_INFO *info)
 {
	gchar *ptr = get_string_from_config("print","colors","");

	trace("info->color=%p",info->color);
	trace("colorlist=%p",ptr);

	if(*ptr)
		v3270_set_color_table(info->color,ptr);
	else
		v3270_set_mono_color_table(info->color,"black","white");

	g_free(ptr);
 }

 static GObject * create_custom_widget(GtkPrintOperation *prt, PRINT_INFO *info)
 {
	GtkWidget			* container = gtk_table_new(3,2,FALSE);
 	static const gchar	* text[]	= { N_( "_Font:" ), N_( "C_olor scheme:" ) };
	GtkWidget			* label[G_N_ELEMENTS(text)];
	GtkWidget			* widget;
	int					  f;

	trace("%s starts",__FUNCTION__);

	for(f=0;f<G_N_ELEMENTS(label);f++)
	{
		label[f] = gtk_label_new_with_mnemonic(gettext(text[f]));
		gtk_misc_set_alignment(GTK_MISC(label[f]),0,0.5);
		gtk_table_attach(GTK_TABLE(container),label[f],0,1,f,f+1,GTK_FILL,GTK_FILL,0,0);
	}

	if(info->font)
		g_free(info->font);

	info->font = get_string_from_config("print",FONT_CONFIG,DEFAULT_FONT);
	if(!*info->font)
	{
		g_free(info->font);
		info->font = g_strdup(DEFAULT_FONT);
	}

	// Font selection button
#ifdef AUTO_FONT_SIZE
	{
		GtkTreeModel	* model		= (GtkTreeModel *) gtk_list_store_new(1,G_TYPE_STRING);
		GtkCellRenderer * renderer	= gtk_cell_renderer_text_new();
		PangoFontFamily **families;
		gint 			  n_families, i;
		GtkTreeIter		  iter;

		widget	= gtk_combo_box_new_with_model(model);

		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget), renderer, TRUE);
		gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), renderer, "text", 0, NULL);

		g_signal_connect(G_OBJECT(widget),"changed",G_CALLBACK(font_name_changed),info);

		pango_context_list_families(gtk_widget_get_pango_context(container),&families, &n_families);

		for(i=0; i<n_families; i++)
		{
			if(pango_font_family_is_monospace(families[i]))
			{
				const gchar *name = pango_font_family_get_name (families[i]);
				gtk_list_store_append((GtkListStore *) model,&iter);
				gtk_list_store_set((GtkListStore *) model, &iter,0, name, -1);

				if(!g_ascii_strcasecmp(name,info->font))
					gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&iter);
			}
		}

		g_free(families);
	}
#else
	{
		trace("Font=%s",info->font);
		widget = gtk_font_button_new_with_font(info->font);
		gtk_font_button_set_show_size((GtkFontButton *) widget,TRUE);
		gtk_font_button_set_use_font((GtkFontButton *) widget,TRUE);
		gtk_label_set_mnemonic_widget(GTK_LABEL(label[0]),widget);
		g_free(info->font);

#if GTK_CHECK_VERSION(3,2,0)
		gtk_font_chooser_set_filter_func((GtkFontChooser *) widget,filter_monospaced,NULL,NULL);
#endif // GTK(3,2,0)

		g_signal_connect(G_OBJECT(widget),"font-set",G_CALLBACK(font_set),info);

	}
#endif

	gtk_table_attach(GTK_TABLE(container),widget,1,2,0,1,GTK_EXPAND|GTK_FILL,GTK_FILL,5,0);

	load_settings(info);

	widget = color_scheme_new(info->color);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label[1]),widget);

	g_object_set_data(G_OBJECT(container),"combo",widget);
	gtk_table_attach(GTK_TABLE(container),widget,1,2,1,2,GTK_EXPAND|GTK_FILL,GTK_FILL,5,0);

	// Selection checkbox
	widget = gtk_check_button_new_with_label( _("Print selection box") );

	if(info->src == PW3270_SRC_ALL)
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

	trace("%s ends container=%p",__FUNCTION__,container);
 	return G_OBJECT(container);
 }

 static void custom_widget_apply(GtkPrintOperation *prt, GtkWidget *widget, PRINT_INFO *info)
 {
 	GtkWidget	* combo = g_object_get_data(G_OBJECT(widget),"combo");
 	GdkRGBA 	* clr	= g_object_get_data(G_OBJECT(combo),"selected");

	trace("%s starts combo=%p clr=%p widget=%p",__FUNCTION__,combo,clr,widget);

	if(info->font)
		set_string_to_config("print",FONT_CONFIG,info->font);

	if(clr)
	{
		int f;
		GString *str = g_string_new("");
		for(f=0;f<V3270_COLOR_COUNT;f++)
		{
			info->color[f] = clr[f];
			if(f)
				g_string_append_c(str,';');
			g_string_append_printf(str,"%s",gdk_rgba_to_string(clr+f));
		}
		set_string_to_config("print","colors","%s",str->str);
		g_string_free(str,TRUE);
	}

	trace("%s ends",__FUNCTION__);

 }

#ifdef WIN32
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

#endif // WIN32

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
	load_settings(*info);
#endif // WIN32

	// Load page and print settings
	{
#ifdef HAVE_WIN_REGISTRY

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

 void print_all_action(GtkAction *action, GtkWidget *widget)
 {
	pw3270_print(widget,G_OBJECT(action),GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, PW3270_SRC_ALL);
 }

 void print_selected_action(GtkAction *action, GtkWidget *widget)
 {
	pw3270_print(widget,G_OBJECT(action),GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, PW3270_SRC_SELECTED);
 }

 static void draw_text(GtkPrintOperation *prt, GtkPrintContext *context, gint pg, PRINT_INFO *info)
 {
	cairo_t			* cr 	= gtk_print_context_get_cairo_context(context);
	GdkRectangle	  rect;
	int			  	  row	= pg*info->lpp;
	int			  	  l;

	cairo_set_scaled_font(cr,info->font_scaled);

	memset(&rect,0,sizeof(rect));
	rect.y          = 2;
	rect.height     = (info->extents.height + info->extents.descent)+1;
	rect.width      = info->extents.max_x_advance+1;

	for(l=0;l<info->lpp && row < info->rows;l++)
	{
		cairo_move_to(cr,2,rect.y+rect.height);
		cairo_show_text(cr, info->text[row]);
		cairo_stroke(cr);
		row++;
		rect.y += (rect.height-1);
	}

 }

 void print_copy_action(GtkAction *action, GtkWidget *widget)
 {
	pw3270_print(widget,G_OBJECT(action),GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, PW3270_SRC_COPY);
 }

 LIB3270_EXPORT int pw3270_print(GtkWidget *widget, GObject *action, GtkPrintOperationAction oper, PW3270_SRC src)
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
	case PW3270_SRC_ALL:
	case PW3270_SRC_SELECTED:
	case PW3270_SRC_USER:
		g_signal_connect(print,"draw_page",G_CALLBACK(draw_screen),info);
		break;

	case PW3270_SRC_COPY:

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

void print_settings_action(GtkAction *action, GtkWidget *terminal)
{
 	const gchar * title  = g_object_get_data(G_OBJECT(action),"title");
 	PRINT_INFO	  info;
 	GtkWidget	* widget;
	GtkWidget	* dialog = gtk_dialog_new_with_buttons (	gettext(title ? title : N_( "Print settings") ),
															GTK_WINDOW(gtk_widget_get_toplevel(terminal)),
															GTK_DIALOG_DESTROY_WITH_PARENT,
															GTK_STOCK_OK,		GTK_RESPONSE_ACCEPT,
															GTK_STOCK_CANCEL,	GTK_RESPONSE_REJECT,
															NULL );

	memset(&info,0,sizeof(info));

	widget = GTK_WIDGET(create_custom_widget(NULL,&info));

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),GTK_WIDGET(widget),TRUE,TRUE,2);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		// Accepted, save settings
		custom_widget_apply(NULL,widget,&info);
	}

	gtk_widget_destroy(dialog);

}
