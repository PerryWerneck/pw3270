/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob
 * o nome G3270.
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
 * Este programa está nomeado como main.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 /**
  * @brief PW3270 Placeholders management.
  *
  */

 #include "private.h"
 #include <pw3270/application.h>
 #include <lib3270.h>
 #include <lib3270/log.h>

/*---[ Implement ]----------------------------------------------------------------------------------*/

 void pw3270_load_placeholders(GtkBuilder * builder) {

	GObject * placeholder = gtk_builder_get_object(builder, "font-select-placeholder");

	if(placeholder && G_IS_MENU(placeholder)) {

		GMenu * font_menu = G_MENU(placeholder);

		gint n_families;
		PangoFontFamily **families;
		pango_context_list_families(gdk_pango_context_get_for_screen(gdk_screen_get_default()),&families, &n_families);

		size_t ix;
		for(ix=0; ix < (size_t) n_families; ix++)
		{
			if(!pango_font_family_is_monospace(families[ix]))
				continue;

			const gchar * family = pango_font_family_get_name(families[ix]);
			g_autofree gchar * detailed_action = g_strconcat("win.font-family::",family,NULL);
			g_menu_append(font_menu,family,detailed_action);

		}

	}


 }
