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
 * Este programa está nomeado como colors.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <gtk/gtk.h>
 #include "private.h"
 #include <lib3270/trace.h>
 #include <v3270/settings.h>
 #include <v3270/colorscheme.h>

 //#define V3270_COLOR_BASE V3270_COLOR_GRAY+1

 //#if defined(DEBUG) && GTK_CHECK_VERSION(3,4,0)
 //   #define USE_GTK_COLOR_CHOOSER 1
 //#endif // GTK_CHECK_VERSION

/*--[ Implement ]------------------------------------------------------------------------------------*/


 void editcolors_action(GtkAction *action, GtkWidget *terminal)
 {
	int f;
	GString *str;

	g_return_if_fail(GTK_IS_V3270(terminal));

	GtkWidget * dialog = v3270_settings_dialog_new();
	GtkWidget * settings = v3270_color_selection_new();

	gtk_window_set_title(GTK_WINDOW(dialog), v3270_settings_get_title(settings));
	gtk_container_add(GTK_CONTAINER(dialog), settings);

	gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(gtk_widget_get_toplevel(terminal)));
	gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);

	v3270_settings_dialog_set_terminal_widget(dialog, terminal);

	gtk_widget_show_all(dialog);

	switch(gtk_dialog_run(GTK_DIALOG(dialog)))
	{
	case GTK_RESPONSE_APPLY:

		v3270_settings_dialog_apply(dialog);

		str = g_string_new("");
		for(f=0;f<V3270_COLOR_COUNT;f++)
		{
			if(f)
				g_string_append_c(str,';');
			g_string_append_printf(str,"%s",gdk_rgba_to_string(v3270_get_color(terminal,f)));
		}
		set_string_to_config("terminal","colors","%s",str->str);
		g_string_free(str,TRUE);

		break;

	case GTK_RESPONSE_CANCEL:
		v3270_settings_dialog_revert(dialog);
		break;
	}

	gtk_widget_destroy(dialog);

 }
