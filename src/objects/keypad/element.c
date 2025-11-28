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

/*---[ Globals & Object definition ]----------------------------------------------------------------*/

enum {
	PROP_NONE,
	PROP_LABEL,
	PROP_ACTION,
	PROP_ICON_NAME,
	PROP_ROW,
	PROP_COL,
	PROP_WIDTH,
	PROP_HEIGHT,
};

static void get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);

G_DEFINE_TYPE(KeypadElement, KeypadElement, G_TYPE_OBJECT)

/*---[ Implement ]----------------------------------------------------------------------------------*/

static void finalize(GObject *object) {

	KeypadElement * element = PW_KEYPAD_ELEMENT(object);

	if(element->icon_name) {
		g_free(element->icon_name);
		element->icon_name = NULL;
	}

	if(element->label) {
		g_free(element->label);
		element->label = NULL;
	}

	if(element->action) {
		g_free(element->action);
		element->action = NULL;
	}

}

static void KeypadElement_class_init(KeypadElementClass *klass) {

	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->finalize          = finalize;
	object_class->get_property      = get_property;
	object_class->set_property      = set_property;

	// Install properties
	g_object_class_install_property(object_class, PROP_ICON_NAME,
	                                g_param_spec_string (
	                                    I_("icon-name"),
	                                    I_("icon-name"),
	                                    N_("The name of the icon"),
	                                    NULL,
	                                    G_PARAM_STATIC_STRINGS | G_PARAM_READABLE | G_PARAM_WRITABLE
	                                )
	                               );

	g_object_class_install_property(object_class, PROP_ACTION,
	                                g_param_spec_string (
	                                    I_("action"),
	                                    I_("action"),
	                                    N_("The name of associated action"),
	                                    NULL,
	                                    G_PARAM_STATIC_STRINGS | G_PARAM_READABLE | G_PARAM_WRITABLE
	                                )
	                               );

	g_object_class_install_property(object_class, PROP_LABEL,
	                                g_param_spec_string (
	                                    I_("label"),
	                                    I_("label"),
	                                    N_("The Label of the keypad"),
	                                    NULL,
	                                    G_PARAM_STATIC_STRINGS | G_PARAM_READABLE | G_PARAM_WRITABLE
	                                )
	                               );

	g_object_class_install_property(object_class, PROP_WIDTH,
	                                g_param_spec_uint(
	                                    I_("row"),
	                                    I_("width"),
	                                    _("Element row"),
	                                    1,
	                                    10,
	                                    3,
	                                    G_PARAM_STATIC_STRINGS | G_PARAM_READABLE | G_PARAM_WRITABLE
	                                )
	                               );

	g_object_class_install_property(object_class, PROP_WIDTH,
	                                g_param_spec_uint(
	                                    I_("col"),
	                                    I_("width"),
	                                    _("Element col"),
	                                    1,
	                                    10,
	                                    3,
	                                    G_PARAM_STATIC_STRINGS | G_PARAM_READABLE | G_PARAM_WRITABLE
	                                )
	                               );

	g_object_class_install_property(object_class, PROP_WIDTH,
	                                g_param_spec_uint(
	                                    I_("width"),
	                                    I_("width"),
	                                    _("Element width in columns"),
	                                    1,
	                                    10,
	                                    3,
	                                    G_PARAM_STATIC_STRINGS | G_PARAM_READABLE | G_PARAM_WRITABLE
	                                )
	                               );

	g_object_class_install_property(object_class, PROP_HEIGHT,
	                                g_param_spec_uint(
	                                    I_("height"),
	                                    I_("height"),
	                                    _("Element height in rows"),
	                                    0,
	                                    100,
	                                    0,
	                                    G_PARAM_STATIC_STRINGS | G_PARAM_READABLE | G_PARAM_WRITABLE
	                                )
	                               );

}

static void KeypadElement_init(KeypadElement G_GNUC_UNUSED(*object)) {


}

static void get_property(GObject *object, guint prop_id, GValue *value, GParamSpec G_GNUC_UNUSED(*pspec)) {

	KeypadElement * element = PW_KEYPAD_ELEMENT(object);

	switch (prop_id) {
	case PROP_LABEL:
		g_value_set_string(value, element->label);
		break;

	case PROP_ICON_NAME:
		g_value_set_string(value, element->icon_name);
		break;

	case PROP_ACTION:
		g_value_set_string(value, element->action);
		break;

	case PROP_ROW:
		g_value_set_uint(value, element->row);
		break;

	case PROP_COL:
		g_value_set_uint(value, element->col);
		break;

	case PROP_HEIGHT:
		g_value_set_uint(value, element->height);
		break;

	case PROP_WIDTH:
		g_value_set_uint(value, element->width);
		break;

	default:
		g_assert_not_reached ();
	}

}

static void set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec G_GNUC_UNUSED(*pspec)) {

	KeypadElement * element = PW_KEYPAD_ELEMENT(object);

	switch (prop_id) {
	case PROP_LABEL:

		if(element->label) {
			g_free(element->label);
		}
		element->label = g_value_dup_string(value);
		break;

	case PROP_ICON_NAME:

		if(element->icon_name) {
			g_free(element->icon_name);
		}
		element->icon_name = g_value_dup_string(value);
		break;

	case PROP_ACTION:

		if(element->action) {
			g_free(element->action);
		}
		element->action = g_value_dup_string(value);
		break;

	case PROP_ROW:
		element->row = (unsigned short) g_value_get_uint(value);
		break;

	case PROP_COL:
		element->col = (unsigned short) g_value_get_uint(value);
		break;

	case PROP_HEIGHT:
		element->height = (unsigned short) g_value_get_uint(value);
		break;

	case PROP_WIDTH:
		element->width = (unsigned short) g_value_get_uint(value);
		break;

	default:
		g_assert_not_reached();
	}
}

static void element_start(GMarkupParseContext *context, const gchar *element_name, const gchar **names,const gchar **values, GObject *element, GError **error) {

	if(!g_ascii_strcasecmp(element_name,"attribute")) {
		attribute_element_start(context,names,values,element,error);
		return;
	}

	debug("%s(%s)",__FUNCTION__,element_name);

}

static void element_end(GMarkupParseContext *context, const gchar *element_name, GObject *element, GError **error) {

	if(!g_ascii_strcasecmp(element_name,"attribute")) {
		attribute_element_end(context,element,error);
		return;
	}

	debug("%s(%s)",__FUNCTION__,element_name);

}

void keypad_model_element_parse_context(GObject *element, GMarkupParseContext *context) {

	static const GMarkupParser parser = {
		(void (*)(GMarkupParseContext *, const gchar *, const gchar **, const gchar **, gpointer, GError **))
		element_start,

		(void (*)(GMarkupParseContext *, const gchar *, gpointer, GError **))
		element_end,
		NULL,
		NULL,
		NULL
	};

	g_markup_parse_context_push(context, &parser, element);

}
