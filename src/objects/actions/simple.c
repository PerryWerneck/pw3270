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
  * @brief Implement PW3270 Simple Action.
  *
  */

 #include "private.h"
 #include <v3270.h>

 static void pw3270SimpleAction_class_init(pw3270SimpleActionClass *klass);
 static void pw3270SimpleAction_init(pw3270SimpleAction *action);
 static void change_widget(GAction *object, GtkWidget *from, GtkWidget *to);

 G_DEFINE_TYPE(pw3270SimpleAction, pw3270SimpleAction, PW3270_TYPE_ACTION);

 static void activate(GAction G_GNUC_UNUSED(*action), GVariant G_GNUC_UNUSED(*parameter), GtkWidget G_GNUC_UNUSED(*terminal)) {
	debug("%s",__FUNCTION__);
 }

 static gboolean get_enabled(GAction *action, GtkWidget *terminal) {

 	if(terminal) {
		return lib3270_action_group_get_activatable(v3270_get_session(terminal),PW3270_SIMPLE_ACTION(action)->group.id);
 	}

 	return FALSE;
 }

 static const gchar * get_icon_name(GAction *action) {
	return PW3270_SIMPLE_ACTION(action)->icon_name;
 }

 static const gchar * get_label(GAction *action) {
	return PW3270_SIMPLE_ACTION(action)->label;
 }

 static const gchar * get_tooltip(GAction *action) {
	return PW3270_SIMPLE_ACTION(action)->tooltip;
 }

 static void dispose(GObject *object) {

	pw3270SimpleAction *action = PW3270_SIMPLE_ACTION(object);

	if(action->group.listener) {
		lib3270_unregister_action_group_listener(pw3270_action_get_session(G_ACTION(object)),action->group.id,action->group.listener);
		action->group.listener = NULL;
	}

	G_OBJECT_CLASS(pw3270SimpleAction_parent_class)->dispose(object);
 }

 static void pw3270SimpleAction_class_init(pw3270SimpleActionClass *klass) {

 	klass->parent_class.get_icon_name	= get_icon_name;
 	klass->parent_class.get_label 		= get_label;
 	klass->parent_class.get_tooltip		= get_tooltip;
 	klass->parent_class.get_enabled		= get_enabled;
	klass->parent_class.change_widget	= change_widget;

	G_OBJECT_CLASS(klass)->dispose = dispose;

 }

 static void pw3270SimpleAction_init(pw3270SimpleAction *action) {

	action->icon_name = NULL;
	action->label = N_( "No label" );
	action->tooltip = NULL;

	action->group.id = LIB3270_ACTION_GROUP_NONE;
	action->group.listener = NULL;

 }

 pw3270SimpleAction * pw3270_simple_action_new_from_lib3270(const LIB3270_ACTION * definition, const gchar *name) {

	if(!definition)
		return NULL;

	debug("%s(%s,%s)",__FUNCTION__,definition->name,name);

 	pw3270SimpleAction * action = (pw3270SimpleAction *) g_object_new(PW3270_TYPE_SIMPLE_ACTION, NULL);

 	action->parent.name = name ? name : definition->name;
	action->icon_name = definition->icon;
	action->label = definition->label;
	action->tooltip = definition->summary;
	action->activate = activate;

	return action;

 }

 pw3270SimpleAction * pw3270_simple_action_new_from_name(const gchar *source_name, const gchar *name) {
	return pw3270_simple_action_new_from_lib3270(lib3270_action_get_by_name(source_name),name);
 }

 pw3270SimpleAction * pw3270_simple_action_new() {
 	return (pw3270SimpleAction *) g_object_new(PW3270_TYPE_SIMPLE_ACTION, NULL);
 }

 static gboolean bg_notify_enabled(GAction *action) {
 	pw3270_action_notify_enabled(action);
	return FALSE;
 }

 static void event_listener(H3270 G_GNUC_UNUSED(*hSession), void *object) {
	g_idle_add((GSourceFunc) bg_notify_enabled, G_ACTION(object));
 }

 void change_widget(GAction *object, GtkWidget *from, GtkWidget *to) {

	// Remove old listener
	pw3270SimpleAction * action = PW3270_SIMPLE_ACTION(object);

	if(action->group.listener) {
		lib3270_unregister_action_group_listener(pw3270_action_get_session(object),action->group.id,action->group.listener);
		action->group.listener = NULL;
	}

	// Get the current "enabled" state
	gboolean enabled = g_action_get_enabled(object);

	// Change widget
	PW3270_ACTION_CLASS(pw3270SimpleAction_parent_class)->change_widget(object,from,to);

	// Setup new listener
	if(action->group.id != LIB3270_ACTION_GROUP_NONE && to) {
		action->group.listener = lib3270_register_action_group_listener(pw3270_action_get_session(object),action->group.id,event_listener,object);
	}

	// Does the "enabled" state has changed? If yes notify customers.
	if(g_action_get_enabled(object) != enabled)
		pw3270_action_notify_enabled(object);

 }

