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
  * @brief Implement GAction "wrapper" for lib3270's actions.
  *
  */

 #include "../private.h"
 #include <v3270.h>

 #define PW3270_TYPE_LIB3270_ACTION		(Lib3270Action_get_type())
 #define PW3270_LIB3270_ACTION(inst)	(G_TYPE_CHECK_INSTANCE_CAST ((inst), PW3270_TYPE_LIB3270_ACTION, Lib3270Action))
 #define PW3270_IS_LIB3270_ACTION(inst)	(G_TYPE_CHECK_INSTANCE_TYPE ((inst), PW3270_TYPE_LIB3270_ACTION))

 typedef struct _Lib3270ActionClass {
 	pw3270ActionClass parent_class;

 } Lib3270ActionClass;

 typedef struct _Lib3270Action {
 	pw3270Action parent;

	const LIB3270_ACTION 	* definition;
	const void				* listener;

 } Lib3270Action;

 static void Lib3270Action_class_init(Lib3270ActionClass *klass);
 static void Lib3270Action_init(Lib3270Action *action);
 static void change_widget(GAction *object, GtkWidget *from, GtkWidget *to);

 G_DEFINE_TYPE(Lib3270Action, Lib3270Action, PW3270_TYPE_ACTION);

 static gboolean get_enabled(GAction *action, GtkWidget *terminal) {

 	if(terminal) {

		H3270 * hSession = v3270_get_session(terminal);
		if(hSession)
			return PW3270_LIB3270_ACTION(action)->definition->activatable(hSession) > 0 ? TRUE : FALSE;

 	}

	return FALSE;

 }

 static void activate(GAction *action, GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal) {
	PW3270_LIB3270_ACTION(action)->definition->activate(v3270_get_session(terminal));
 }

 static const gchar * get_icon_name(GAction *action) {
	return PW3270_LIB3270_ACTION(action)->definition->icon;
 }

 static const gchar * get_label(GAction *action) {
	return PW3270_LIB3270_ACTION(action)->definition->label;
 }

 static const gchar * get_tooltip(GAction *action) {
	return PW3270_LIB3270_ACTION(action)->definition->summary;
 }

 static void dispose(GObject *object) {

	Lib3270Action *action = PW3270_LIB3270_ACTION(object);

	if(action->listener) {
		lib3270_unregister_action_group_listener(pw3270_action_get_session(G_ACTION(object)),action->definition->group,action->listener);
		action->listener = NULL;
	}

	G_OBJECT_CLASS(Lib3270Action_parent_class)->dispose(object);
 }

 void Lib3270Action_class_init(Lib3270ActionClass *klass) {

	pw3270ActionClass * action = PW3270_ACTION_CLASS(klass);

	action->get_enabled = get_enabled;
	action->change_widget = change_widget;
	action->get_icon_name = get_icon_name;
	action->get_label = get_label;
	action->get_tooltip = get_tooltip;

	G_OBJECT_CLASS(klass)->dispose = dispose;

 }

 void Lib3270Action_init(Lib3270Action *action) {
// 	debug("%s",__FUNCTION__);
	PW3270_ACTION(action)->activate = activate;
 }

 GAction * pw3270_action_new_from_lib3270(const LIB3270_ACTION * definition) {

 	Lib3270Action * action = (Lib3270Action *) g_object_new(PW3270_TYPE_LIB3270_ACTION, NULL);

	// Setup hooks.
	action->definition	= definition;
	action->listener	= NULL;

	// Setup the default name.
	pw3270Action * abstract	= PW3270_ACTION(action);

	if(abstract->name)
		g_free(abstract->name);

	abstract->name = g_strconcat("win.",definition->name,NULL);

 	return G_ACTION(action);
 }

 static gboolean bg_notify_enabled(GAction *action) {
 	debug("Action %s was notified (%s)",g_action_get_name(action),g_action_get_enabled(action) ? "Enabled" : "Disabled");
 	pw3270_action_notify_enabled(action);
	return FALSE;
 }

 static void event_listener(H3270 G_GNUC_UNUSED(*hSession), void *object) {
	g_idle_add((GSourceFunc) bg_notify_enabled, G_ACTION(object));
 }

 void change_widget(GAction *object, GtkWidget *from, GtkWidget *to) {

	// Remove old listener
	Lib3270Action * action = PW3270_LIB3270_ACTION(object);
	if(action->listener) {
		lib3270_unregister_action_group_listener(pw3270_action_get_session(object),action->definition->group,action->listener);
		action->listener = NULL;
	}

	// Change widget
	PW3270_ACTION_CLASS(Lib3270Action_parent_class)->change_widget(object,from,to);

	// Setup new listener
	if(action->definition->group && to) {
		action->listener = lib3270_register_action_group_listener(pw3270_action_get_session(object),action->definition->group,event_listener,object);
	}

	// Does the "enabled" state has changed? If yes notify customers.
	gboolean enabled = get_enabled(object,to);
	if(get_enabled(object,from) != enabled)
		pw3270_action_notify_enabled(object);

 }

