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
  * @brief Implement GAction "wrapper" for lib3270's toggles.
  *
  */

 #include "../private.h"
 #include <pw3270/window.h>
 #include <v3270.h>
 #include <v3270/actions.h>

 #define LIB3270_TYPE_TOGGLE_ACTION		(Lib3270ToggleAction_get_type())
 #define LIB3270_TOGGLE_ACTION(inst)	(G_TYPE_CHECK_INSTANCE_CAST ((inst), LIB3270_TYPE_TOGGLE_ACTION, Lib3270ToggleAction))
 #define LIB3270_IS_TOGGLE_ACTION(inst)	(G_TYPE_CHECK_INSTANCE_TYPE ((inst), LIB3270_TYPE_TOGGLE_ACTION))

 #define GET_DESCRIPTOR(obj)  ((const LIB3270_TOGGLE *) ((V3270Action *) obj)->info)

 typedef struct _Lib3270ToggleActionClass {
 	V3270ActionClass parent_class;
 } Lib3270ToggleActionClass;

 typedef struct _Lib3270ToggleAction {
 	V3270Action parent;
	const void * listener;
 } Lib3270ToggleAction;

 static void Lib3270ToggleAction_class_init(Lib3270ToggleActionClass *klass);
 static void Lib3270ToggleAction_init(Lib3270ToggleAction *action);

 G_DEFINE_TYPE(Lib3270ToggleAction, Lib3270ToggleAction, V3270_TYPE_ACTION);

 static void change_state(H3270 G_GNUC_UNUSED(*hSession), LIB3270_TOGGLE_ID G_GNUC_UNUSED(id), char G_GNUC_UNUSED(state), void G_GNUC_UNUSED(*action)) {
 	v3270_action_notify_state(G_ACTION(action));
 }

 static void change_widget(GAction *object, GtkWidget *from, GtkWidget *to) {

	Lib3270ToggleAction * action = LIB3270_TOGGLE_ACTION(object);

	if(action->listener)
		lib3270_unregister_toggle_listener(v3270_get_session(from),GET_DESCRIPTOR(object)->id,object);

	if(to)
		action->listener = lib3270_register_toggle_listener(v3270_get_session(to),GET_DESCRIPTOR(object)->id,change_state,object);

	V3270_ACTION_CLASS(Lib3270ToggleAction_parent_class)->change_widget(object,from,to);

 }

 static void activate(GAction *action, GVariant *parameter, GtkWidget *terminal) {

 	debug("Activating \"%s\"",g_action_get_name(action));

 	if(parameter && g_variant_is_of_type(parameter,G_VARIANT_TYPE_BOOLEAN)) {

		lib3270_set_toggle(v3270_get_session(terminal),GET_DESCRIPTOR(action)->id,g_variant_get_boolean(parameter));
		debug("Toggle set to %s",lib3270_get_toggle(v3270_get_session(terminal),GET_DESCRIPTOR(action)->id) ? "ON" : "OFF");

 	} else {

		lib3270_toggle(v3270_get_session(terminal),GET_DESCRIPTOR(action)->id);
		debug("Toggle is %s",lib3270_get_toggle(v3270_get_session(terminal),GET_DESCRIPTOR(action)->id) ? "ON" : "OFF");

 	}

 }

 static GVariant * get_state(GAction *action, GtkWidget *terminal) {

	debug("%s(%s)",__FUNCTION__,g_action_get_name(action));

 	return g_variant_new_boolean(
				lib3270_get_toggle(
					v3270_get_session(terminal),
					GET_DESCRIPTOR(action)->id
				)
			);

 }

 void Lib3270ToggleAction_class_init(Lib3270ToggleActionClass *klass) {

	klass->parent_class.change_widget	= change_widget;
	klass->parent_class.state.type		= G_VARIANT_TYPE_BOOLEAN;
	klass->parent_class.get_state		= get_state;

 }

 void Lib3270ToggleAction_init(Lib3270ToggleAction *action) {
	action->parent.activate	= activate;
 }

 GAction * g_action_new_from_toggle(const LIB3270_TOGGLE * definition) {

 	Lib3270ToggleAction	* action = (Lib3270ToggleAction *) g_object_new(LIB3270_TYPE_TOGGLE_ACTION, NULL);

 	action->parent.info = (const LIB3270_PROPERTY *) definition;

 	return G_ACTION(action);

 }


