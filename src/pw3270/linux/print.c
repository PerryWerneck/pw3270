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
 * Este programa está nomeado como - e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "../private.h"
 #include <v3270.h>
 #include <v3270/print.h>
 #include <lib3270/selection.h>

 #define FONT_CONFIG 	"font-family"
 #define DEFAULT_FONT	"Courier New"

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static GtkWidget * create_custom_widget(GtkPrintOperation *prt, gpointer G_GNUC_UNUSED(dunno))
 {
 	GtkWidget * widget		= gtk_frame_new("");
 	GtkWidget * settings	= V3270_print_settings_new(v3270_print_operation_get_terminal(prt));

 	// Load values from configuration
	g_autofree gchar * font_family	= get_string_from_config("print",FONT_CONFIG,DEFAULT_FONT);
	if(font_family && *font_family)
		v3270_print_settings_set_font_family(settings,font_family);

	g_autofree gchar * color_scheme	= get_string_from_config("print","colors","");
	if(color_scheme && *color_scheme)
		v3270_print_settings_set_color_scheme(settings,color_scheme);

 	// Create frame
	GtkWidget *label = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(label),_("<b>Text options</b>"));
	gtk_frame_set_label_widget(GTK_FRAME(widget),label);

 	gtk_container_set_border_width(GTK_CONTAINER(widget),12);

	// The print dialog doesn't follow the guidelines from https://developer.gnome.org/hig/stable/visual-layout.html.en )-:
	gtk_frame_set_shadow_type(GTK_FRAME(widget),GTK_SHADOW_NONE);

 	gtk_container_set_border_width(GTK_CONTAINER(settings),6);
 	g_object_set(G_OBJECT(settings),"margin-start",8,NULL);

	gtk_container_add(GTK_CONTAINER(widget),settings);

	gtk_widget_show_all(widget);

    return widget;
 }

 static void custom_widget_apply(GtkPrintOperation *prt, GtkWidget *widget, gpointer G_GNUC_UNUSED(dunno))
 {
 	GtkWidget * settings = gtk_bin_get_child(GTK_BIN(widget));

 	v3270_print_operation_apply_settings(prt,settings);

	// Store font family
	g_autofree gchar * font_family = v3270_print_settings_get_font_family(settings);
	set_string_to_config("print",FONT_CONFIG,font_family);

	// Store save color settings
	g_autofree gchar * colors = v3270_print_settings_get_color_scheme(settings);
	set_string_to_config("print","colors","%s",colors);

 }

 void setup_print_dialog(GtkPrintOperation * operation)
 {
	GtkPrintSettings 	* settings	= gtk_print_settings_new();
	GtkPageSetup 		* setup 	= gtk_page_setup_new();
    GtkPaperSize        * papersize = NULL;

	g_signal_connect(operation,"create-custom-widget",G_CALLBACK(create_custom_widget),NULL);
	g_signal_connect(operation,"custom-widget-apply",G_CALLBACK(custom_widget_apply), NULL);

	// Load page and print settings
	GKeyFile	* conf	= get_application_keyfile();
	GError		* err	= NULL;

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

	if(g_key_file_has_group(conf,"paper_size"))
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

	gtk_print_operation_set_print_settings(operation,settings);
	gtk_page_setup_set_paper_size_and_default_margins(setup,papersize);
	gtk_print_operation_set_default_page_setup(operation,setup);

 }

