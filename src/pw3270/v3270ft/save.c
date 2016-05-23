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
 * Este programa está nomeado como save.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "private.h"


/*--[ Statics ]--------------------------------------------------------------------------------------*/


/*--[ Implement ]------------------------------------------------------------------------------------*/

 void get_item(struct v3270ft_entry *entry, GString *str) {

	int f;

	g_string_append_printf(str,"\t<entry action=\"%s\" type=\"%s\">\n",ft_type[entry->type].name,ft_type[entry->type].type);
	g_string_append_printf(str,"\t\t<file type=\"%s\" path=\"%s\"/>\n","local",entry->local);
	g_string_append_printf(str,"\t\t<file type=\"%s\" path=\"%s\"/>\n","remote",entry->remote);

	for(f=0;f<NUM_OPTIONS_WIDGETS;f++) {

		if( (entry->options & ft_option[f].opt) == ft_option[f].opt) {
			char *name	= g_strdup(ft_option[f].name);
			char *value	= strchr(name,'.');

			if(value)
				*(value++) = 0;
			else
				value = "yes";

			g_string_append_printf(str,"\t\t<option name=\"%s\" value=\"%s\"/>\n",name,value);

			g_free(name);
		}
	}

	for(f=0;f<LIB3270_FT_VALUE_COUNT;f++) {
		g_string_append_printf(str,"\t\t<option name=\"%s\" value=\"%u\"/>\n",ft_value[f].name,entry->value[f]);
	}

	g_string_append(str,"\t</entry>\n");
 }

 void v3270ft_save(GtkWidget *widget,const gchar *filename) {

	GString * str	= g_string_new("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<filelist>\n");
	gchar	* text;
	GError	* error	= NULL;

	g_list_foreach(GTK_V3270FT(widget)->files,(GFunc) get_item, str);

	g_string_append(str,"</filelist>");

	text = g_string_free(str,FALSE);

	if(!g_file_set_contents(filename,text,-1,&error)) {

		GtkWidget *popup = gtk_message_dialog_new_with_markup(
				GTK_WINDOW(widget),
				GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,
				_("Can't save %s"),filename);

		gtk_window_set_title(GTK_WINDOW(popup),_("Operation has failed"));

		gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(popup),"%s",error->message);
		g_error_free(error);

		gtk_dialog_run(GTK_DIALOG(popup));
		gtk_widget_destroy(popup);

	}

	g_free(text);

 }

