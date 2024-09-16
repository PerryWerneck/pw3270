/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes paul.mattes@case.edu), de emulação de terminal 3270 para acesso a
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

#include "private.h"
#include <stdlib.h>

/*---[ Implement ]----------------------------------------------------------------------------------*/

static void element_start(GMarkupParseContext *context, const gchar *element_name, const gchar **names,const gchar **values, GList **keypads, GError **error) {

	if(!g_ascii_strcasecmp(element_name,"keypad")) {

		const gchar *name, *label, *position, *width, *height;
		GObject * keypad = g_object_new(PW_TYPE_KEYPAD_MODEL,NULL);

		if(!g_markup_collect_attributes(
		            element_name,names,values,error,
		            G_MARKUP_COLLECT_STRING, "name", &name,
		            G_MARKUP_COLLECT_STRING|G_MARKUP_COLLECT_OPTIONAL, "label", &label,
		            G_MARKUP_COLLECT_STRING|G_MARKUP_COLLECT_OPTIONAL, "position", &position,
		            G_MARKUP_COLLECT_STRING|G_MARKUP_COLLECT_OPTIONAL, "width", &width,
		            G_MARKUP_COLLECT_STRING|G_MARKUP_COLLECT_OPTIONAL, "height", &height,
		            G_MARKUP_COLLECT_INVALID
		        )) {

			g_object_unref(keypad);
			return;

		}

		*keypads = g_list_append(*keypads,keypad);

		keypad_model_set_position(keypad,position);

		if(name) {
			PW_KEYPAD_MODEL(keypad)->name = g_strdup(name);
		}

		if(label) {
			PW_KEYPAD_MODEL(keypad)->label = g_strdup(label);
		}

		if(width) {
			PW_KEYPAD_MODEL(keypad)->width = (unsigned short) atoi(width);
		}

		if(height) {
			PW_KEYPAD_MODEL(keypad)->height = (unsigned short) atoi(height);
		}

		keypad_model_parse_context(keypad,context);

	}

	debug("%s(%s)",__FUNCTION__,element_name);

}

static void element_end(GMarkupParseContext *context, const gchar *element_name, GList G_GNUC_UNUSED(**keypads), GError G_GNUC_UNUSED(**error)) {

	debug("%s(%s)",__FUNCTION__,element_name);

	if(!g_ascii_strcasecmp(element_name,"keypad")) {
		g_markup_parse_context_pop(context);
	}

}

GList * pw3270_keypad_model_new_from_xml(GList *keypads, const gchar *filename) {

	GError	* error = NULL;
	g_autofree gchar *text = NULL;

	if(g_file_get_contents(filename,&text,NULL,&error)) {

		g_message("Loading keypad from %s",filename);

		static const GMarkupParser parser = {
			(void (*)(GMarkupParseContext *, const gchar *, const gchar **, const gchar **, gpointer, GError **))
			element_start,

			(void (*)(GMarkupParseContext *, const gchar *, gpointer, GError **))
			element_end,
			NULL,
			NULL,
			NULL
		};

		GMarkupParseContext * context = g_markup_parse_context_new(&parser,G_MARKUP_TREAT_CDATA_AS_TEXT,&keypads,NULL);
		g_markup_parse_context_parse(context,text,strlen(text),&error);
		g_markup_parse_context_free(context);
	}

	if(error) {

		// TODO: Popup error message.
		g_message("%s",error->message);
		g_error_free(error);
		error = NULL;
	}

	return keypads;

}
