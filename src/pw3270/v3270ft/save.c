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

static const gchar * getNameByFlag(LIB3270_FT_OPTION opt, LIB3270_FT_OPTION mask) {

	const gchar * rc = "Default";
	const gchar * ptr;
	int			  f;

	opt &= mask;

	for(f=0;f<NUM_OPTIONS_WIDGETS;f++) {

		if(opt == ft_option[f].opt) {
			ptr = strchr(ft_option[f].name,'.');
			rc = ptr ? ptr+1 : ft_option[f].name;
		}
	}

	return rc;
}

 void get_item(struct v3270ft_entry *entry, GString *str) {

	int f;

	g_string_append_printf(str,"\t<entry action=\"%s\" type=\"%s\">\n",ft_type[entry->type].name,ft_type[entry->type].type);
	g_string_append_printf(str,"\t\t<file type=\"%s\" path=\"%s\"/>\n","local",entry->local);
	g_string_append_printf(str,"\t\t<file type=\"%s\" path=\"%s\"/>\n","remote",entry->remote);

	g_string_append_printf(str,"\t\t<option name=\"%s\" value=\"%s\"/>\n",
			"ascii", entry->options & LIB3270_FT_OPTION_ASCII ? "yes" : "no");

	g_string_append_printf(str,"\t\t<option name=\"%s\" value=\"%s\"/>\n",
			"crlf", entry->options & LIB3270_FT_OPTION_CRLF ? "yes" : "no");

	g_string_append_printf(str,"\t\t<option name=\"%s\" value=\"%s\"/>\n",
			"append", entry->options & LIB3270_FT_OPTION_APPEND ? "yes" : "no");

	g_string_append_printf(str,"\t\t<option name=\"%s\" value=\"%s\"/>\n",
			"remap", entry->options & LIB3270_FT_OPTION_REMAP ? "yes" : "no");

	g_string_append_printf(str,"\t\t<option name=\"%s\" value=\"%s\"/>\n",
			"recfm", getNameByFlag(entry->options,
								LIB3270_FT_RECORD_FORMAT_DEFAULT
								|LIB3270_FT_RECORD_FORMAT_FIXED
								|LIB3270_FT_RECORD_FORMAT_VARIABLE
								|LIB3270_FT_RECORD_FORMAT_UNDEFINED));

	g_string_append_printf(str,"\t\t<option name=\"%s\" value=\"%s\"/>\n",
			"units", getNameByFlag(entry->options,
								LIB3270_FT_ALLOCATION_UNITS_DEFAULT
								|LIB3270_FT_ALLOCATION_UNITS_TRACKS
								|LIB3270_FT_ALLOCATION_UNITS_CYLINDERS
								|LIB3270_FT_ALLOCATION_UNITS_AVBLOCK));

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

