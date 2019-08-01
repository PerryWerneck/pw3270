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
 #include <lib3270/trace.h>

 #define FONT_CONFIG 	"font-family"
 #define DEFAULT_FONT	"Courier New"

/*--[ Implement ]------------------------------------------------------------------------------------*/

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

 void load_print_operation_settings(GtkPrintOperation * operation)
 {
	GtkPrintSettings 	* settings	= gtk_print_settings_new();
	GtkPageSetup 		* setup 	= gtk_page_setup_new();
    GtkPaperSize        * papersize = NULL;

#ifdef ENABLE_WINDOWS_REGISTRY

	HKEY registry;

	if(get_registry_handle("print",&registry,KEY_READ))
	{
		HKEY 	  hKey;
		DWORD	  disp;

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

		if(RegCreateKeyEx(registry,"paper",0,NULL,REG_OPTION_NON_VOLATILE,KEY_READ,NULL,&hKey,&disp) == ERROR_SUCCESS)
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
#endif // ENABLE_WINDOWS_REGISTRY

	gtk_print_operation_set_print_settings(print,settings);
	gtk_page_setup_set_paper_size_and_default_margins(setup,papersize);
	gtk_print_operation_set_default_page_setup(print,setup);

 }

 void save_print_operation_settings(GtkPrintOperation * operation)
 {


 }
