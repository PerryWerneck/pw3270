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

 #include "private.h"
 #include <stdlib.h>

/*---[ Globals & Object definition ]----------------------------------------------------------------*/

 struct Attribute {
 	GObject * object;
	gboolean translatable;
	GParamSpec *spec;
 };

/*---[ Implement ]----------------------------------------------------------------------------------*/


 static void parse_error (GMarkupParseContext G_GNUC_UNUSED(*context), GError G_GNUC_UNUSED(*error), gpointer data) {
	g_free(data);
 }

 static void parse_text(GMarkupParseContext *context, const gchar *text, gsize text_len, gpointer user_data, GError **error) {

 	if(text && ((struct Attribute *) user_data)->translatable ) {
		text = gettext(text);
 	}

 	GParamSpec *spec = ((struct Attribute *) user_data)->spec;

//	debug("%s=\"%s\"",spec->name,text);

	GValue value = G_VALUE_INIT;
	g_value_init(&value,spec->value_type);

	switch(spec->value_type) {
	case G_TYPE_STRING:
		g_value_set_string(&value,text);
		break;

	case G_TYPE_UINT:
		g_value_set_uint(&value,(unsigned int) atoi(text));
		break;

	case G_TYPE_INT:
		g_value_set_int(&value, atoi(text));
		break;

	default:
		g_set_error_literal(
			error,
			g_quark_from_static_string("keypad"),
			ENOENT,
			_( "Invalid or unknown property type" )
		);

		g_value_unset(&value);
		return;

	}

	g_object_set_property(((struct Attribute *) user_data)->object,spec->name,&value);
	g_value_unset(&value);
 }

 void attribute_element_start(GMarkupParseContext *context,const gchar **names,const gchar **values, GObject *parent, GError **error) {

	struct Attribute * data = g_new0(struct Attribute,1);
	const gchar *name;

	data->object = parent;

	if(!g_markup_collect_attributes(
				"attribute",names,values,error,
				G_MARKUP_COLLECT_BOOLEAN|G_MARKUP_COLLECT_OPTIONAL, "translatable", &data->translatable,
				G_MARKUP_COLLECT_STRING, "name", &name,
				G_MARKUP_COLLECT_INVALID
			)) {

		g_free(data);
		return;

	}

	data->spec = g_object_class_find_property(G_OBJECT_GET_CLASS(parent),name);
	if(!data->spec) {
		g_set_error(
			error,
			g_quark_from_static_string("keypad"),
			ENOENT,
			_( "Property \"%s\" is invalid for this object" ),
			name
		);
		g_free(data);
		return;
	}

	static const GMarkupParser parser = {
		NULL,
		NULL,
		parse_text,
		NULL,
		parse_error
	};

	g_markup_parse_context_push(context, &parser, data);

 }

 void attribute_element_end(GMarkupParseContext *context, GObject *parent, GError **error) {

	struct Attribute * data = g_markup_parse_context_pop(context);
	g_free(data);

 }
