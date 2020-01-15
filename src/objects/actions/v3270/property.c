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
 #include <stdlib.h>
 #include <pw3270/window.h>
 #include <v3270.h>
 #include <lib3270/properties.h>

 //
 // V3270 Property Action
 //
 #define V3270_TYPE_PROPERTY_ACTION				(v3270PropertyAction_get_type())
 #define V3270_PROPERTY_ACTION(inst)			(G_TYPE_CHECK_INSTANCE_CAST ((inst), V3270_TYPE_PROPERTY_ACTION, v3270PropertyAction))
 #define V3270_PROPERTY_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), V3270_TYPE_PROPERTY_ACTION, v3270PropertyActionClass))
 #define V3270_IS_PROPERTY_ACTION(inst)			(G_TYPE_CHECK_INSTANCE_TYPE ((inst), V3270_TYPE_PROPERTY_ACTION))
 #define V3270_IS_PROPERTY_ACTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), V3270_TYPE_PROPERTY_ACTION))
 #define V3270_PROPERTY_ACTION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), V3270_TYPE_PROPERTY_ACTION, v3270PropertyActionClass))

 typedef struct _v3270PropertyAction {

	V3270SimpleAction parent;

	GParamSpec *pspec;

 } v3270PropertyAction;

 typedef struct _v3270PropertyActionClass {

	V3270SimpleActionClass parent_class;

 } v3270PropertyActionClass;

// static void v3270PropertyAction_class_init(v3270PropertyActionClass *klass);
// static void v3270PropertyAction_init(v3270PropertyAction *action);

 static GVariant				* get_state(GAction *action, GtkWidget *terminal);
 static const GVariantType      * get_state_type(GAction *object);
 static const GVariantType      * get_parameter_type(GAction *object);

 static void					  change_widget(GAction *object, GtkWidget *from, GtkWidget *to);

 G_DEFINE_TYPE(v3270PropertyAction, v3270PropertyAction, V3270_TYPE_SIMPLE_ACTION);

 void v3270PropertyAction_class_init(v3270PropertyActionClass *klass) {
	klass->parent_class.parent_class.change_widget = change_widget;
	klass->parent_class.parent_class.get_parameter_type = get_parameter_type;
	klass->parent_class.parent_class.get_state = get_state;
	klass->parent_class.parent_class.get_state_type = get_state_type;
 }

 static void v3270PropertyAction_init(v3270PropertyAction G_GNUC_UNUSED(*action)) {
 }

 GVariant * get_state(GAction *object, GtkWidget *terminal) {

	v3270PropertyAction * action = V3270_PROPERTY_ACTION(object);

	GVariant * result = NULL;
	GValue value = G_VALUE_INIT;

	g_value_init(&value, action->pspec->value_type);
	g_object_get_property(G_OBJECT(terminal), action->pspec->name, &value);

	switch(action->pspec->value_type) {
	case G_TYPE_UINT:
		result = g_variant_new_take_string(g_strdup_printf("%d",g_value_get_uint(&value)));
		break;

	case G_TYPE_STRING:
		result = g_variant_new_string(g_value_get_string(&value));
		debug("Action %s is on state \"%s\"",g_action_get_name(object),g_value_get_string(&value));
		break;

	case G_TYPE_BOOLEAN:
		result = g_variant_new_boolean(g_value_get_boolean(&value));
		break;
	/*

	case G_TYPE_INT:
		result = g_variant_new_int32(g_value_get_int(&value));
		break;

	case G_TYPE_DOUBLE:
		result = g_variant_new_double(g_value_get_double(&value));
		break;

	case G_TYPE_FLOAT:
		result = g_variant_new_double(g_value_get_float(&value));
		break;

	*/
	default:
		g_warning("Unexpected value type getting state for action \"%s\"",g_action_get_name(object));
	}

	g_value_unset (&value);

	return result;

 }

 static void activate(GAction *object, GVariant *parameter, GtkWidget *terminal) {

	v3270PropertyAction * action = V3270_PROPERTY_ACTION(object);

	GValue value = G_VALUE_INIT;
	g_value_init(&value, action->pspec->value_type);

	switch(action->pspec->value_type)
	{
	case G_TYPE_UINT:
		debug("%s(%s,%s,%p)",__FUNCTION__,g_action_get_name(object),g_variant_get_string(parameter,NULL),terminal);
		g_value_set_uint(&value,atoi(g_variant_get_string(parameter,NULL)));
		break;

	case G_TYPE_BOOLEAN:

		if(parameter) {

			debug("%s(%s,%s,%p)",__FUNCTION__,g_action_get_name(object),g_variant_get_string(parameter,NULL),terminal);

			if(g_variant_is_of_type(parameter,G_VARIANT_TYPE_BOOLEAN))
				g_value_set_boolean(&value,g_variant_get_boolean(parameter));
			else
				g_value_set_boolean(&value,atoi(g_variant_get_string(parameter,NULL)) != 0);

		} else {

			g_object_get_property(G_OBJECT(terminal), action->pspec->name, &value);
			g_value_set_boolean(&value,!g_value_get_boolean(&value));

		}

		break;

	case G_TYPE_STRING:
		g_value_set_string(&value,g_variant_get_string(parameter,NULL));
		break;

	/*
	case G_TYPE_INT:
		break;

	case G_TYPE_DOUBLE:
		break;

	case G_TYPE_FLOAT:
		break;

	*/

	default:
		g_warning("Can't activate action \"%s\"",g_action_get_name(object));
		g_value_unset(&value);
		return;
	}

	g_object_set_property(G_OBJECT(terminal),action->pspec->name,&value);

	g_value_unset(&value);

 }

 static void on_notify(GtkWidget G_GNUC_UNUSED(*terminal), GParamSpec G_GNUC_UNUSED(*pspec), GAction *action) {

 	debug("%s: State of action %s has changed",__FUNCTION__, g_action_get_name(G_ACTION(action)));
 	pw3270_action_notify_state(action);

 }

 GAction * v3270_property_action_new(GtkWidget *widget, const gchar *property_name) {

	GParamSpec *pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(widget), property_name);

	if(!pspec) {

		g_warning(
			"Can't find property '%s::%s'",
			G_OBJECT_TYPE_NAME(G_OBJECT(widget)),
			property_name
		);

		return NULL;

	}

	debug("%s: pspec(%s)=%p",__FUNCTION__,property_name,pspec);

 	if(~pspec->flags & G_PARAM_READABLE || ~pspec->flags & G_PARAM_WRITABLE || pspec->flags & G_PARAM_CONSTRUCT_ONLY) {

		g_warning(
			"Property '%s::%s' must be readable, writable, and not construct-only",
			G_OBJECT_TYPE_NAME(G_OBJECT(widget)),
			property_name
		);

		return NULL;
    }

 	v3270PropertyAction * action = (v3270PropertyAction *) g_object_new(V3270_TYPE_PROPERTY_ACTION, NULL);

 	action->parent.name = g_param_spec_get_name(pspec);

	const LIB3270_PROPERTY * lProperty = lib3270_property_get_by_name(pspec->name);
	if(lProperty) {
		action->parent.label	= lib3270_property_get_label(lProperty);
		action->parent.tooltip 	= lib3270_property_get_summary(lProperty);
//		action->group.id		= lProperty->group;
	}

 	if(!action->parent.tooltip)
		action->parent.tooltip = g_param_spec_get_blurb(pspec);

	action->parent.parent.activate			= activate;
 	action->pspec							= pspec;

 	v3270_action_set_terminal_widget(G_ACTION(action), widget);

	return G_ACTION(action);
 }

 const GVariantType * get_state_type(GAction *object) {

	v3270PropertyAction * action = V3270_PROPERTY_ACTION(object);

	if(action->pspec->value_type == G_TYPE_BOOLEAN)
		return G_VARIANT_TYPE_BOOLEAN;

	return G_VARIANT_TYPE_STRING;

 }

 const GVariantType * get_parameter_type(GAction *object) {

	v3270PropertyAction * action = V3270_PROPERTY_ACTION(object);

	if(action->pspec->value_type == G_TYPE_BOOLEAN)
		return G_VARIANT_TYPE_BOOLEAN;

	return G_VARIANT_TYPE_STRING;

 }

 void change_widget(GAction *object, GtkWidget *from, GtkWidget *to) {

	v3270PropertyAction * action = V3270_PROPERTY_ACTION(object);
	g_autofree gchar * signal_name = g_strconcat("notify::", action->pspec->name,NULL);

	if(from) {
		gulong handler = g_signal_handler_find(
												from,
												G_SIGNAL_MATCH_FUNC|G_SIGNAL_MATCH_DATA,
												0,
												0,
												NULL,
												G_CALLBACK(on_notify),
												action
										);

		if(handler)
			g_signal_handler_disconnect(from, handler);

	}

	V3270_ACTION_CLASS(v3270PropertyAction_parent_class)->change_widget(object,from,to);

	if(to) {
		g_signal_connect(G_OBJECT(to),signal_name,G_CALLBACK(on_notify),action);
	}

 }

