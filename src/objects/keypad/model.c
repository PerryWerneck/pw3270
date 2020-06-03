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

 enum {
	PROP_NONE,
	PROP_NAME,
	PROP_LABEL,
	PROP_POSITION,
	PROP_WIDTH,
	PROP_HEIGHT,
 };

 static const char * positions[] = {
	"top",
	"left",
	"bottom",
	"right"
 };

 static void get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
 static void set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);

 G_DEFINE_TYPE(KeypadModel, KeypadModel, G_TYPE_OBJECT)

/*---[ Implement ]----------------------------------------------------------------------------------*/

 static void finalize(GObject *object) {

	KeypadModel * model = PW_KEYPAD_MODEL(object);

	if(model->elements) {
		g_list_free_full(model->elements,g_object_unref);
		model->elements = NULL;
	}

	if(model->name) {
		g_free(model->name);
		model->name = NULL;
	}

	if(model->label) {
		g_free(model->label);
		model->label = NULL;
	}

 }

 static void KeypadModel_class_init(KeypadModelClass *klass) {

	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	klass->domain = g_quark_from_static_string("keypad");

	object_class->finalize          = finalize;
	object_class->get_property      = get_property;
	object_class->set_property      = set_property;

	// Install properties
	g_object_class_install_property(object_class, PROP_NAME,
		g_param_spec_string (
			I_("name"),
			N_("Keypad Name"),
			N_("The name used to identify the keypad"),
			NULL,
			G_PARAM_STATIC_STRINGS | G_PARAM_READABLE | G_PARAM_WRITABLE
		)
	);

	g_object_class_install_property(object_class, PROP_LABEL,
		g_param_spec_string (
			_("label"),
			N_("Keypad Label"),
			N_("The Label of the keypad"),
			NULL,
			G_PARAM_STATIC_STRINGS | G_PARAM_READABLE | G_PARAM_WRITABLE
		)
	);

	g_object_class_install_property(object_class, PROP_POSITION,
		g_param_spec_string (
			I_("position"),
			I_("position"),
			N_("The position of the keypad"),
			NULL,
			G_PARAM_STATIC_STRINGS | G_PARAM_READABLE | G_PARAM_WRITABLE
		)
	);

	g_object_class_install_property(object_class, PROP_WIDTH,
		 g_param_spec_uint(
					I_("width"),
					I_("width"),
					_("Keypad width in columns"),
					1,
					10,
					3,
					G_PARAM_STATIC_STRINGS | G_PARAM_READABLE | G_PARAM_WRITABLE
			)
		);

	g_object_class_install_property(object_class, PROP_WIDTH,
		 g_param_spec_uint(
					I_("height"),
					I_("height"),
					_("Keypad height in rows"),
					0,
					100,
					0,
					G_PARAM_STATIC_STRINGS | G_PARAM_READABLE | G_PARAM_WRITABLE
			)
		);
 }

 static void KeypadModel_init(KeypadModel *object) {

	object->position = (unsigned short) KEYPAD_POSITION_BOTTOM;

 }

 static void get_property(GObject *object, guint prop_id, GValue *value, GParamSpec G_GNUC_UNUSED(*pspec)) {

	KeypadModel * model = PW_KEYPAD_MODEL(object);

	switch (prop_id) {
	case PROP_NAME:
		g_value_set_string(value, model->name);
		break;

	case PROP_LABEL:
		g_value_set_string(value, model->label);
		break;

	case PROP_POSITION:
		g_value_set_static_string(value,keypad_model_get_position(object));
		break;

	case PROP_HEIGHT:
		g_value_set_uint(value, model->height);
		break;

	case PROP_WIDTH:
		g_value_set_uint(value, model->width);
		break;

	default:
		g_assert_not_reached ();
	}

 }

 static void set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec G_GNUC_UNUSED(*pspec)) {

	KeypadModel * model = PW_KEYPAD_MODEL(object);

	switch (prop_id) {
	case PROP_NAME:

		if(model->name) {
			g_free(model->name);
		}
		model->name = g_value_dup_string(value);
		break;

	case PROP_LABEL:

		if(model->label) {
			g_free(model->label);
		}
		model->label = g_value_dup_string(value);
		break;

	case PROP_POSITION:
		keypad_model_set_position(object,g_value_get_string(value));
		break;

	case PROP_HEIGHT:
		model->height = (unsigned short) g_value_get_uint(value);
		break;

	case PROP_WIDTH:
		model->width = (unsigned short) g_value_get_uint(value);
		break;

	default:
		g_assert_not_reached();
	}
 }

 void keypad_model_set_position(GObject *model, const gchar *position) {

	if(position) {
		size_t ix;
		for(ix = 0; ix < G_N_ELEMENTS(positions); ix++) {
			if(!g_ascii_strcasecmp(positions[ix],position)) {
				PW_KEYPAD_MODEL(model)->position = (unsigned short) ix;
				break;
			}
		}
	}

 }

 const gchar * keypad_model_get_position(GObject *model) {

	size_t ix = (size_t) PW_KEYPAD_MODEL(model)->position;

 	if(ix < G_N_ELEMENTS(positions))
		return positions[ix];

 	return "undefined";
 }

 static void element_start(GMarkupParseContext *context, const gchar *element_name, const gchar **names,const gchar **values, KeypadModel *keypad, GError **error) {

	debug("%s(%s)",__FUNCTION__,element_name);

	if(!g_ascii_strcasecmp(element_name,"button")) {

		const gchar *row, *col, *width, *height;
		KeypadElement * element = PW_KEYPAD_ELEMENT(g_object_new(PW_TYPE_KEYPAD_ELEMENT,NULL));

		if(!g_markup_collect_attributes(
                    element_name,names,values,error,
                    G_MARKUP_COLLECT_STRING|G_MARKUP_COLLECT_OPTIONAL, "row", &row,
                    G_MARKUP_COLLECT_STRING|G_MARKUP_COLLECT_OPTIONAL, "column", &col,
                    G_MARKUP_COLLECT_STRING|G_MARKUP_COLLECT_OPTIONAL, "width", &width,
                    G_MARKUP_COLLECT_STRING|G_MARKUP_COLLECT_OPTIONAL, "height", &height,
                    G_MARKUP_COLLECT_INVALID
                )) {

            return;

        }

		if(col) {
			element->col = (unsigned short) atoi(col);

			if(element->col < keypad->current.col) {
				keypad->current.row++;
			}

		} else {
			element->col = keypad->current.col;
		}

		if(row) {
			element->row = (unsigned short) atoi(row);
		} else {
			element->row = keypad->current.row;
		}


		if(width) {
			element->width = (unsigned short) atoi(width);
		} else {
			element->width = 1;
		}

		if(height) {
			element->height = (unsigned short) atoi(height);
		} else {
			element->height = 1;
		}

		keypad->elements = g_list_prepend(keypad->elements,element);
		keypad_model_element_parse_context(G_OBJECT(element),context);

	} else if(!g_ascii_strcasecmp(element_name,"attribute")) {
		attribute_element_start(context,names,values,G_OBJECT(keypad),error);
	}

 }

 static void element_end(GMarkupParseContext *context, const gchar *element_name, KeypadModel *keypad, GError G_GNUC_UNUSED(**error)) {

	debug("%s(%s)",__FUNCTION__,element_name);

	if(!g_ascii_strcasecmp(element_name,"button")) {
		g_markup_parse_context_pop(context);

		KeypadElement * element = PW_KEYPAD_ELEMENT(g_list_first(keypad->elements)->data);

		keypad->current.row = element->row;
		keypad->current.col = element->col + element->width;

		if(keypad->width && keypad->current.col >= keypad->width) {
			keypad->current.col = 0;
			keypad->current.row++;
		}

	} else if(!g_ascii_strcasecmp(element_name,"attribute")) {
		attribute_element_end(context,G_OBJECT(keypad),error);
	}

 }

 void keypad_model_parse_context(GObject *object, GMarkupParseContext *context) {

   static const GMarkupParser parser = {
		(void (*)(GMarkupParseContext *, const gchar *, const gchar **, const gchar **, gpointer, GError **))
		element_start,

	(void (*)(GMarkupParseContext *, const gchar *, gpointer, GError **))
			element_end,
			NULL,
			NULL,
			NULL
	};

	KeypadModel * model = PW_KEYPAD_MODEL(object);

	model->elements = g_list_reverse(model->elements);
	g_markup_parse_context_push(context, &parser, model);
	model->elements = g_list_reverse(model->elements);

 }

 KEYPAD_POSITION pw3270_keypad_get_position(GObject *model) {
	g_return_val_if_fail(PW_IS_KEYPAD_MODEL(model), (KEYPAD_POSITION) -1);
	return (KEYPAD_POSITION) PW_KEYPAD_MODEL(model)->position;
 }

 const gchar * pw3270_keypad_model_get_name(GObject *model) {
	g_return_val_if_fail(PW_IS_KEYPAD_MODEL(model), NULL);
	return PW_KEYPAD_MODEL(model)->name;
 }

 const gchar * pw3270_keypad_model_get_label(GObject *model) {
	g_return_val_if_fail(PW_IS_KEYPAD_MODEL(model), NULL);
	return PW_KEYPAD_MODEL(model)->label;
 }

