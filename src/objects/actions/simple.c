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

 static void pw3270SimpleAction_class_init(pw3270SimpleActionClass *klass);
 static void pw3270SimpleAction_init(pw3270SimpleAction *action);

 G_DEFINE_TYPE(pw3270SimpleAction, pw3270SimpleAction, PW3270_TYPE_ACTION);

 static void activate(GAction G_GNUC_UNUSED(*action), GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal) {

	debug("%s",__FUNCTION__);

 }

 static gboolean get_enabled(GAction *action, GtkWidget *terminal) {
 	return TRUE;
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

 static void pw3270SimpleAction_class_init(pw3270SimpleActionClass *klass) {

 	klass->parent_class.get_icon_name	= get_icon_name;
 	klass->parent_class.get_label 		= get_label;
 	klass->parent_class.get_tooltip		= get_tooltip;
 	klass->parent_class.get_enabled		= get_enabled;

 	debug("%s:%p",__FUNCTION__,klass->parent_class.get_icon_name);

 }

 static void pw3270SimpleAction_init(pw3270SimpleAction *action) {

	action->icon_name = NULL;
	action->label = N_( "No label" );
	action->tooltip = NULL;

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

