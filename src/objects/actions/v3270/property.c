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

 /**
  * @brief Implement GAction "wrapper" for v3270 properties.
  *
  * Reference:
  *
  * <https://github.com/GNOME/glib/blob/master/gio/gpropertyaction.c>
  *
  */

 #include "../private.h"
 #include <pw3270/window.h>
 #include <v3270.h>

 static void v3270PropertyAction_class_init(v3270PropertyActionClass *klass);
 static void v3270PropertyAction_init(v3270PropertyAction *action);
 static GVariant * get_state(GAction *action, GtkWidget *terminal);


 G_DEFINE_TYPE(v3270PropertyAction, v3270PropertyAction, PW3270_TYPE_ACTION);

 void v3270PropertyAction_class_init(v3270PropertyActionClass *klass) {

 }

 static void v3270PropertyAction_init(v3270PropertyAction *action) {

 	action->parent.get_state_property = get_state;

 }

 GVariant * get_state(GAction *action, GtkWidget *terminal) {

	if(!terminal)
		return NULL;

	debug("%s",__FUNCTION__);

	v3270PropertyAction * paction = V3270_PROPERTY_ACTION(action);

	GValue value = G_VALUE_INIT;
	GVariant *result = NULL;

	g_value_init(&value, paction->pspec->value_type);
	g_object_get_property(G_OBJECT(terminal), paction->pspec->name, &value);

	switch(paction->pspec->value_type) {
	case G_TYPE_BOOLEAN:
		result = g_variant_new_boolean(g_value_get_boolean(&value));
		break;

	case G_TYPE_INT:
		result = g_variant_new_int32(g_value_get_int(&value));
		break;

	case G_TYPE_UINT:
		result = g_variant_new_uint32(g_value_get_uint(&value));
		debug("state of %s is %u",g_action_get_name(action),g_value_get_uint(&value));
		break;

	case G_TYPE_DOUBLE:
		result = g_variant_new_double(g_value_get_double(&value));
		break;

	case G_TYPE_FLOAT:
		result = g_variant_new_double(g_value_get_float(&value));
		break;

	case G_TYPE_STRING:
		result = g_variant_new_string(g_value_get_string(&value));
		break;

	}

	g_value_unset (&value);

	return result;

 }

 static void activate(GAction *action, GVariant *parameter, GtkWidget *terminal) {

	debug("%s(%s,%p,%p)",__FUNCTION__,g_action_get_name(action),parameter,terminal);

 }

 v3270PropertyAction * v3270_property_action_new(GtkWidget *widget, const gchar *property_name) {

	GParamSpec *pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(widget), property_name);

 	if(~pspec->flags & G_PARAM_READABLE || ~pspec->flags & G_PARAM_WRITABLE || pspec->flags & G_PARAM_CONSTRUCT_ONLY) {

		g_warning(
			"Property '%s::%s' must be readable, writable, and not construct-only",
			G_OBJECT_TYPE_NAME(G_OBJECT(widget)),
			property_name
		);

		return NULL;
    }

    // Get state type
    const GVariantType	* type;

	switch(pspec->value_type) {
	case G_TYPE_BOOLEAN:
		type = G_VARIANT_TYPE_BOOLEAN;
		debug("%s: Type of \"%s\" is %s",__FUNCTION__,property_name,"boolean");
		break;

	case G_TYPE_INT:
		type = G_VARIANT_TYPE_INT32;
		debug("%s: Type of \"%s\" is %s",__FUNCTION__,property_name,"int32");
		break;

	case G_TYPE_UINT:
		type = G_VARIANT_TYPE_UINT32;
		debug("%s: Type of \"%s\" is %s",__FUNCTION__,property_name,"uint32");
		break;

	case G_TYPE_DOUBLE:
	case G_TYPE_FLOAT:
		type = G_VARIANT_TYPE_DOUBLE;
		break;

	case G_TYPE_STRING:
		type = G_VARIANT_TYPE_STRING;
		break;

	default:
		g_warning(
			"Unable to create action for property '%s::%s' of type '%s'",
					g_type_name(pspec->owner_type),
					pspec->name,
					g_type_name(pspec->value_type)
			);
		return NULL;
    }

 	v3270PropertyAction * action = (v3270PropertyAction *) g_object_new(V3270_TYPE_PROPERTY_ACTION, NULL);

 	action->parent.name				= pspec->name;
	action->parent.types.state		= type;
	action->parent.types.parameter	= type;
	action->parent.activate			= activate;
 	action->pspec					= pspec;

 	pw3270_action_set_terminal_widget(G_ACTION(action), widget);
 	return action;
 }
