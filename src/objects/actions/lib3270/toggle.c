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

 #define PW3270_TYPE_LIB3270_TOGGLE_ACTION		(Lib3270ToggleAction_get_type())
 #define PW3270_LIB3270_TOGGLE_ACTION(inst)	(G_TYPE_CHECK_INSTANCE_CAST ((inst), PW3270_TYPE_LIB3270_TOGGLE_ACTION, Lib3270ToggleAction))
 #define PW3270_IS_LIB3270_TOGGLE_ACTION(inst)	(G_TYPE_CHECK_INSTANCE_TYPE ((inst), PW3270_TYPE_LIB3270_TOGGLE_ACTION))

 typedef struct _Lib3270ToggleActionClass {
 	pw3270ActionClass parent_class;

 } Lib3270ToggleActionClass;

 typedef struct _Lib3270ToggleAction {
 	pw3270Action parent;

	const LIB3270_TOGGLE * definition;
	const void * listener;

 } Lib3270ToggleAction;

 static void Lib3270ToggleAction_class_init(Lib3270ToggleActionClass *klass);
 static void Lib3270ToggleAction_init(Lib3270ToggleAction *action);

 G_DEFINE_TYPE(Lib3270ToggleAction, Lib3270ToggleAction, PW3270_TYPE_ACTION);

 static void change_state(H3270 G_GNUC_UNUSED(*hSession), LIB3270_TOGGLE_ID id, char state, void * action) {
 	debug("%s: %s",__FUNCTION__,state ? "ON" : "OFF");
	pw3270_action_change_state_boolean((GAction *) action, state == 0 ? FALSE : TRUE);
 }

 static void change_widget(GAction *object, GtkWidget *from, GtkWidget *to) {

	Lib3270ToggleAction * action = PW3270_LIB3270_TOGGLE_ACTION(object);

	if(action->listener) {
		lib3270_unregister_toggle_listener(v3270_get_session(from),action->definition->id,object);
	}

	if(to) {
		action->listener = lib3270_register_toggle_listener(v3270_get_session(to),action->definition->id,change_state,object);
		pw3270_action_change_state_boolean(object,lib3270_get_toggle(v3270_get_session(to),action->definition->id));
	}

	PW3270_ACTION_CLASS(Lib3270ToggleAction_parent_class)->change_widget(object,from,to);

 }

 static void activate(GAction *action, GVariant *parameter, GtkWidget *terminal) {

 	debug("Activating \"%s\"",pw3270_action_get_name(action));

 	if(parameter && g_variant_is_of_type(parameter,G_VARIANT_TYPE_BOOLEAN)) {

		lib3270_set_toggle(v3270_get_session(terminal),PW3270_LIB3270_TOGGLE_ACTION(action)->definition->id,g_variant_get_boolean(parameter));
		debug("Toggle set to %s",lib3270_get_toggle(v3270_get_session(terminal),PW3270_LIB3270_TOGGLE_ACTION(action)->definition->id) ? "ON" : "OFF");

 	} else {

		lib3270_toggle(v3270_get_session(terminal),PW3270_LIB3270_TOGGLE_ACTION(action)->definition->id);
		debug("Toggle is %s",lib3270_get_toggle(v3270_get_session(terminal),PW3270_LIB3270_TOGGLE_ACTION(action)->definition->id) ? "ON" : "OFF");

 	}

 }

 void Lib3270ToggleAction_class_init(Lib3270ToggleActionClass *klass) {

	pw3270ActionClass * action = PW3270_ACTION_CLASS(klass);
	action->change_widget = change_widget;

 }

 static const gchar * get_name(GAction *action) {
 	return PW3270_LIB3270_TOGGLE_ACTION(action)->definition->name;
 }

 void Lib3270ToggleAction_init(Lib3270ToggleAction *toggle) {

 	toggle->definition	= NULL;
 	toggle->listener	= NULL;

 	pw3270Action * action = PW3270_ACTION(toggle);
	action->activate = activate;
	action->get_name = get_name;

 }

 GAction * pw3270_toggle_action_new_from_lib3270(const LIB3270_TOGGLE * definition) {

 	Lib3270ToggleAction	* action = (Lib3270ToggleAction *) g_object_new(PW3270_TYPE_LIB3270_TOGGLE_ACTION, NULL);
	action->definition = definition;

	// Setup the default name.
	pw3270Action * abstract	= PW3270_ACTION(action);

	/*
	if(abstract->name)
		g_free(abstract->name);

	abstract->name = g_strconcat("win.",definition->name,NULL);
	*/

 	return G_ACTION(action);

 }


