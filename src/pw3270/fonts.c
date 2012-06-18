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
 * Este programa está nomeado como fonts.c e possui - linhas de código.
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
#include "uiparser/parser.h"

/*--[ Globals ]--------------------------------------------------------------------------------------*/

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void activate_font(GtkCheckMenuItem *item, GtkWidget *widget)
 {
 	if(!gtk_check_menu_item_get_active(item))
		return;
	v3270_set_font_family(widget,gtk_menu_item_get_label(GTK_MENU_ITEM(item)));
 }

 static void load_system_monospaced_fonts(GtkWidget *topmenu, GtkWidget *menu, GtkWidget *obj)
 {
	// Stolen from http://svn.gnome.org/svn/gtk+/trunk/gtk/gtkfontsel.c
	PangoFontFamily **families;
	gint 			n_families, i;
 	GSList 			* group	= NULL;
 	const gchar		* selected = v3270_get_font_family(obj);

	pango_context_list_families(gtk_widget_get_pango_context(topmenu),&families, &n_families);

	for(i=0; i<n_families; i++)
    {
    	if(pango_font_family_is_monospace(families[i]))
    	{
			const gchar 	*name = pango_font_family_get_name (families[i]);
			GtkWidget		*item = gtk_radio_menu_item_new_with_label(group,name);

			group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));

			g_signal_connect(G_OBJECT(item),"toggled",G_CALLBACK(activate_font),obj);

			gtk_widget_show(item);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);

			if(!g_strcasecmp(name,selected))
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),TRUE);

    	}
    }

	g_free(families);

 }

 void setup_font_list(GtkWidget *widget, GtkWidget *obj)
 {
	GtkWidget *menu	= gtk_menu_new();

	load_system_monospaced_fonts(widget,menu,obj);

	gtk_widget_show_all(menu);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(widget),menu);
 }
