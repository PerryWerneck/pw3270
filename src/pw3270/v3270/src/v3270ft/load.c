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
 * Este programa está nomeado como load.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "private.h"
 #include <ctype.h>
 #include <stdlib.h>


/*--[ Statics ]--------------------------------------------------------------------------------------*/


/*--[ Implement ]------------------------------------------------------------------------------------*/

static LIB3270_FT_OPTION getFlagByName(const gchar *option, const char *optval) {

	LIB3270_FT_OPTION	  rc	= 0;
	int					  f;
	gchar				* key	= g_strconcat(option,".",optval,NULL);

	for(f=0;f<NUM_OPTIONS_WIDGETS;f++) {

//		debug("%u %s",f,ft_option[f].name);

		if(!g_ascii_strcasecmp(key,ft_option[f].name)) {
			rc = ft_option[f].opt;
			debug("%s=%08lx",key,(unsigned int) rc);
			break;
		}

	}

	g_free(key);

	return rc;
}

static void entry_start(GMarkupParseContext *context, const gchar *element_name, const gchar **names,const gchar **values, struct v3270ft_entry *info, GError **error) {

	int f;

	if(!(g_ascii_strcasecmp(element_name,"name") && g_ascii_strcasecmp(element_name,"file"))) {

		const gchar *type;
		const gchar *path;

		if(!g_markup_collect_attributes(
				element_name,names,values,error,
				G_MARKUP_COLLECT_STRING, "type", &type,
				G_MARKUP_COLLECT_STRING, "path", &path,
				G_MARKUP_COLLECT_INVALID
				)) {

			return;

		}

		debug("file[%s]=\"%s\"",type,path);

		if(!g_ascii_strcasecmp(type,"local")) {

			strncpy(info->local,path,sizeof(info->local));

		} else if(!g_ascii_strcasecmp(type,"remote")) {

			strncpy(info->remote,path,sizeof(info->remote));

		}

	} if(!g_ascii_strcasecmp(element_name,"option")) {

		const gchar *option = NULL;
		const gchar	*optval = NULL;

		if(!g_markup_collect_attributes(
				element_name,names,values,error,
				G_MARKUP_COLLECT_STRING, "name", &option,
				G_MARKUP_COLLECT_STRING, "value", &optval,
				G_MARKUP_COLLECT_INVALID
				)) {

			return;

		}

		debug("%s=%s",option,optval);

		if(!g_ascii_strcasecmp(option,"recfm")) {

			// Recfm, limpo todas as flags correspondentes e remonto
			info->options &= ~(LIB3270_FT_RECORD_FORMAT_DEFAULT|LIB3270_FT_RECORD_FORMAT_FIXED|LIB3270_FT_RECORD_FORMAT_VARIABLE|LIB3270_FT_RECORD_FORMAT_UNDEFINED);	// Reseta flags
			info->options |= getFlagByName(option,optval);

		} else if(!g_ascii_strcasecmp(option,"units")) {

			// Units, limpo todas as flags correspondentes e remonto
			info->options &= ~(LIB3270_FT_ALLOCATION_UNITS_DEFAULT|LIB3270_FT_ALLOCATION_UNITS_TRACKS|LIB3270_FT_ALLOCATION_UNITS_CYLINDERS|LIB3270_FT_ALLOCATION_UNITS_AVBLOCK);
			info->options |= getFlagByName(option,optval);

		} else {

			for(f=0;f<NUM_OPTIONS_WIDGETS;f++) {

				if(!g_ascii_strcasecmp(option,ft_option[f].name)) {

					if(!g_ascii_strcasecmp(optval,"yes")) {
						info->options |= ft_option[f].opt;
					} else if(!g_ascii_strcasecmp(optval,"no")) {
						info->options &= ~ft_option[f].opt;
					} else {
						g_warning("Unexpected value for %s: %s",option,optval);
					}
					break;
				}

			}


		}

		// Check for FT values
		for(f=0;f<LIB3270_FT_VALUE_COUNT;f++) {

			if(!g_ascii_strcasecmp(option,ft_value[f].name)) {

				info->value[f] = atoi(optval);
//				debug("%s=%d",option,info->value[f]);
				return;

			}
		}

	}


}

static void element_start(GMarkupParseContext *context, const gchar *element_name, const gchar **names,const gchar **values, v3270ft *widget, GError **error) {

	static const GMarkupParser entry_parser = {
		(void (*)(GMarkupParseContext *, const gchar *, const gchar **, const gchar **, gpointer, GError **)) entry_start,
		(void (*)(GMarkupParseContext *, const gchar *, gpointer, GError **)) NULL,
		(void (*)(GMarkupParseContext *, const gchar *, gsize, gpointer, GError **)) NULL,
		(void (*)(GMarkupParseContext *, const gchar *, gsize,  gpointer, GError **)) NULL,
		(void (*)(GMarkupParseContext *, GError *, gpointer)) NULL
	};

	if(!(g_ascii_strcasecmp(element_name,"entry") && g_ascii_strcasecmp(element_name,"file"))) {

		const gchar *action;
		const gchar *type;
		int f;

		struct v3270ft_entry *info = g_new0(struct v3270ft_entry,1);

		if(!g_markup_collect_attributes(
				element_name,names,values,error,
				G_MARKUP_COLLECT_STRING, "action", &action,
				G_MARKUP_COLLECT_STRING, "type", &type,
				G_MARKUP_COLLECT_INVALID
				)) {
			return;
		}

//		debug("action=%s type=%s",action,type);
		for(f=0;f<NUM_TYPES;f++) {

			if(!(g_ascii_strcasecmp(action,ft_type[f].name) || (g_ascii_strcasecmp(type,ft_type[f].type)))) {
				info->type = f;
//				debug("info->type=%d",f);
				break;
			}

		}

		widget->files = g_list_append(widget->files,info);

		g_markup_parse_context_push(context,&entry_parser,info);

	}

}

static void element_end(GMarkupParseContext *context, const gchar *element_name, void *info, GError **error) {

	if(!(g_ascii_strcasecmp(element_name,"entry") && g_ascii_strcasecmp(element_name,"file"))) {

		g_markup_parse_context_pop(context);

	} else {

		debug("%s(%s)",__FUNCTION__,element_name);

	}



}

static void validate_item(struct v3270ft_entry *entry, GError *error) {
	v3270ft_update_state(entry);
}

void v3270ft_load(GtkWidget *widget,const gchar *filename) {

	static const GMarkupParser parser = {
		(void (*)(GMarkupParseContext *, const gchar *, const gchar **, const gchar **, gpointer, GError **)) element_start,
		(void (*)(GMarkupParseContext *, const gchar *, gpointer, GError **)) element_end,
		(void (*)(GMarkupParseContext *, const gchar *, gsize, gpointer, GError **)) NULL,
		(void (*)(GMarkupParseContext *, const gchar *, gsize,  gpointer, GError **)) NULL,
		(void (*)(GMarkupParseContext *, GError *, gpointer)) NULL
	};

	GError				* error		= NULL;
	gchar				* text		= NULL;
	v3270ft 			* dialog	= GTK_V3270FT(widget);

	if(dialog->files) {
		g_list_free_full(dialog->files,g_free);
		dialog->files	= NULL;
		dialog->active	= NULL;
	}

	if(g_file_get_contents(filename,&text,NULL,&error)) {

		GMarkupParseContext	* context = g_markup_parse_context_new(&parser,G_MARKUP_TREAT_CDATA_AS_TEXT|G_MARKUP_PREFIX_ERROR_POSITION,GTK_V3270FT(widget),NULL);
		g_markup_parse_context_parse(context,text,strlen(text),&error);
		g_markup_parse_context_free(context);
		g_free(text);


	}

	g_list_foreach(GTK_V3270FT(widget)->files,(GFunc) validate_item, error);
	v3270ft_select_last(widget);

	if(error) {

		GtkWidget *popup = gtk_message_dialog_new_with_markup(
				GTK_WINDOW(widget),
				GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,GTK_BUTTONS_CLOSE,
				_("Can't load %s"),filename);

		gtk_window_set_title(GTK_WINDOW(popup),_("Operation has failed"));

		if(error->message && *error->message) {
			gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(popup),"%s",error->message);
		}

		g_error_free(error);

		gtk_dialog_run(GTK_DIALOG(popup));
		gtk_widget_destroy(popup);

	}

}


