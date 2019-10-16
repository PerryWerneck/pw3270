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

 } Lib3270Action;

 static void Lib3270Action_class_init(Lib3270ActionClass *klass);
 static void Lib3270Action_init(Lib3270Action *action);
 static void change_widget(GAction *object, GtkWidget *from, GtkWidget *to);

 G_DEFINE_TYPE(Lib3270Action, Lib3270Action, PW3270_TYPE_ACTION);

 static gboolean get_enabled(GAction *action, GtkWidget *terminal) {

 	if(terminal)
		return PW3270_LIB3270_ACTION(action)->definition->activatable(v3270_get_session(terminal)) > 0 ? TRUE : FALSE;

	return FALSE;

 }

 static void activate(GAction *action, GVariant *parameter, GtkWidget *terminal) {
	PW3270_LIB3270_ACTION(action)->definition->activate(v3270_get_session(terminal));
 }

 void Lib3270Action_class_init(Lib3270ActionClass *klass) {

	pw3270ActionClass * action = PW3270_ACTION_CLASS(klass);

	action->activate = activate;
	action->get_enabled = get_enabled;
	action->change_widget = change_widget;

 }

 void Lib3270Action_init(Lib3270Action *action) {
 }

 GAction * pw3270_action_new_from_lib3270(const LIB3270_ACTION * definition) {

 	Lib3270Action	* action		= (Lib3270Action *) g_object_new(PW3270_TYPE_LIB3270_ACTION, NULL);
	action->definition	= definition;

	// Setup the default name.
	pw3270Action * abstract	= PW3270_ACTION(action);

	if(abstract->name)
		g_free(abstract->name);

	abstract->name = g_strconcat("win.",definition->name,NULL);

 	return G_ACTION(action);
 }

 void change_widget(GAction *object, GtkWidget *from, GtkWidget *to) {

	PW3270_ACTION_CLASS(Lib3270Action_parent_class)->change_widget(object,from,to);

	// Does the "enabled" state has changed? If yes notify customers.
	gboolean enabled = get_enabled(object,to);
	if(get_enabled(object,from) != enabled)
		pw3270_action_set_enabled(object,enabled);

 }
