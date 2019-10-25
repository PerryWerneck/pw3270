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
  * @brief Implement PW3270 "simple" action.
  *
  */

 #include "private.h"
 #include <v3270.h>

 #define PW3270_TYPE_SIMPLE_ACTION		(SimpleAction_get_type())
 #define PW3270_SIMPLE_ACTION(inst)		(G_TYPE_CHECK_INSTANCE_CAST ((inst), PW3270_TYPE_SIMPLE_ACTION, SimpleAction))
 #define PW3270_IS_SIMPLE_ACTION(inst)	(G_TYPE_CHECK_INSTANCE_TYPE ((inst), PW3270_TYPE_SIMPLE_ACTION))

 static void SimpleAction_class_init(SimpleActionClass *klass);
 static void SimpleAction_init(SimpleAction *action);
 static void change_widget(GAction *object, GtkWidget *from, GtkWidget *to);
 static void change_widget(GAction *object, GtkWidget *from, GtkWidget *to);

 G_DEFINE_TYPE(SimpleAction, SimpleAction, PW3270_TYPE_ACTION);

 static void activate(GAction *action, GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal) {
 	PW3270_SIMPLE_ACTION(action)->activate(terminal);
 }

 static const gchar * get_icon_name(GAction *action) {
 	return PW3270_SIMPLE_ACTION(action)->icon_name;
 }

 static const gchar * get_label(GAction *action) {
	const gchar * text = PW3270_SIMPLE_ACTION(action)->label;
 	if(text)
		return gettext(text);
 	return NULL;
 }

 static const gchar * get_tooltip(GAction *action) {
	const gchar * text = PW3270_SIMPLE_ACTION(action)->tooltip;
 	if(text)
		return gettext(text);
 	return NULL;
 }

 static void dispose(GObject *object) {

	SimpleAction *action = PW3270_SIMPLE_ACTION(object);

	if(action->listener) {
		lib3270_unregister_action_group_listener(pw3270_action_get_session(G_ACTION(object)),action->group,action->listener);
		action->listener = NULL;
	}

	G_OBJECT_CLASS(SimpleAction_parent_class)->dispose(object);
 }

 void SimpleAction_class_init(SimpleActionClass *klass) {

	pw3270ActionClass * action = PW3270_ACTION_CLASS(klass);

	action->change_widget = change_widget;
	action->get_icon_name = get_icon_name;
	action->get_label = get_label;
	action->get_tooltip = get_tooltip;
	action->activate = activate;

	G_OBJECT_CLASS(klass)->dispose = dispose;

 }

 static void _activate(GtkWidget G_GNUC_UNUSED(*terminal)) {
 }

 void SimpleAction_init(SimpleAction *action) {
 	action->group = LIB3270_ACTION_GROUP_NONE;
 	action->activate = _activate;
 }

 static void event_listener(H3270 G_GNUC_UNUSED(*hSession), void *object) {
 	pw3270_action_notify_enabled(G_ACTION(object));
 }

 void change_widget(GAction *object, GtkWidget *from, GtkWidget *to) {

	// Remove old listener
	SimpleAction * action = PW3270_SIMPLE_ACTION(object);

	if(action->listener) {
		lib3270_unregister_action_group_listener(pw3270_action_get_session(object),action->group,action->listener);
		action->listener = NULL;
	}

	// Change widget
	PW3270_ACTION_CLASS(SimpleAction_parent_class)->change_widget(object,from,to);

	// Setup new listener
	if(action->group && to) {
		action->listener = lib3270_register_action_group_listener(pw3270_action_get_session(object),action->group,event_listener,object);
	}

	// Does the "enabled" state has changed? If yes notify customers.
	gboolean enabled = PW3270_ACTION_CLASS(SimpleAction_parent_class)->get_enabled(object,to);
	if(PW3270_ACTION_CLASS(SimpleAction_parent_class)->get_enabled(object,from) != enabled)
		pw3270_action_notify_enabled(object);

 }

 SimpleAction * pw3270_simple_action_new() {
	return PW3270_SIMPLE_ACTION(g_object_new(PW3270_TYPE_SIMPLE_ACTION, NULL));
 }

 SimpleAction * pw3270_simple_action_from_name(const gchar *name) {

	SimpleAction * action = pw3270_simple_action_new();

	const LIB3270_ACTION * description = lib3270_action_get_by_name(name);

	if(description) {
		action->group		= description->group;
		action->icon_name 	= description->icon;
		action->label		= description->label;
		action->tooltip		= description->summary;
	} else {
		action->group		= LIB3270_ACTION_GROUP_NONE;
		action->label		= N_("Invalid action");
		action->tooltip		= N_("This action is not valid");
	}


	return action;
 }
