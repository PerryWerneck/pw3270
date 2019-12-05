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
  * @brief Implement PW3270 copy actions.
  *
  */

 #include "../private.h"
 #include <v3270.h>

 static void v3270CopyAction_class_init(v3270CopyActionClass *klass);
 static void v3270CopyAction_init(v3270CopyAction *action);
 static GVariant * get_state(GAction *action, GtkWidget *terminal);
 static void change_widget(GAction *object, GtkWidget *from, GtkWidget *to);

 G_DEFINE_TYPE(v3270CopyAction, v3270CopyAction, PW3270_TYPE_SIMPLE_ACTION);

 void v3270CopyAction_class_init(v3270CopyActionClass *klass) {
	klass->parent_class.parent_class.change_widget = change_widget;
 }

 static void v3270CopyAction_init(v3270CopyAction *action) {

 	action->parent.parent.get_state_property = get_state;

 }

 GVariant * get_state(GAction *object, GtkWidget *terminal) {


	return NULL;

 }

 static void activate(GAction *object, GVariant *parameter, GtkWidget *terminal) {


 }

 static void on_notify(GtkWidget G_GNUC_UNUSED(*terminal), GParamSpec G_GNUC_UNUSED(*pspec), GAction *action) {

 	debug("%s: State of action %s has changed",__FUNCTION__, g_action_get_name(G_ACTION(action)));
 	pw3270_action_notify_state(action);

 }

 void change_widget(GAction *object, GtkWidget *from, GtkWidget *to) {

	v3270CopyAction * action = V3270_COPY_ACTION(object);

	if(from) {
		gulong handler = g_signal_handler_find(
												"has-text",
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

	PW3270_ACTION_CLASS(v3270CopyAction_parent_class)->change_widget(object,from,to);

	if(to) {
		g_signal_connect(G_OBJECT(to),"has-text",G_CALLBACK(on_notify),action);
	}

 }


 GAction * pw3270_action_print_copy_new(GtkWidget *widget) {

	pw3270SimpleAction * action = (pw3270SimpleAction *) g_object_new(V3270_TYPE_COPY_ACTION, NULL);;

	action->group.id = LIB3270_ACTION_GROUP_ONLINE;
	action->parent.name = "print_copy";
	action->label =  N_( "Print copy" );

	return G_ACTION(action);

 }

 GAction * pw3270_action_save_copy_new(GtkWidget *widget) {

	pw3270SimpleAction * action = (pw3270SimpleAction *) g_object_new(V3270_TYPE_COPY_ACTION, NULL);;

	action->group.id = LIB3270_ACTION_GROUP_ONLINE;
	action->parent.name = "save_copy";
	action->label =  N_( "Save copy" );

	return G_ACTION(action);

 }
