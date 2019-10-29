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
  * @brief Implement GAction "wrapper" for lib3270's PFs.
  *
  */

 #include "../private.h"
 #include <v3270.h>

 #define PW3270_TYPE_PFKEY_ACTION				(Lib3270PfAction_get_type())
 #define PW3270_LIB3270_PFKEY_ACTION(inst)		(G_TYPE_CHECK_INSTANCE_CAST ((inst), PW3270_TYPE_PFKEY_ACTION, Lib3270PfAction))
 #define PW3270_IS_LIB3270_PFKEY_ACTION(inst)	(G_TYPE_CHECK_INSTANCE_TYPE ((inst), PW3270_TYPE_PFKEY_ACTION))

 typedef struct _Lib3270PfActionClass {
 	pw3270ActionClass parent_class;

 } Lib3270PfActionClass;

 typedef struct _Lib3270PfAction {
 	pw3270Action parent;

 } Lib3270PfAction;

 static void Lib3270PfAction_class_init(Lib3270PfActionClass *klass);
 static void Lib3270PfAction_init(Lib3270PfAction *action);
 static void change_widget(GAction *object, GtkWidget *from, GtkWidget *to);

 G_DEFINE_TYPE(Lib3270PfAction, Lib3270PfAction, PW3270_TYPE_ACTION);

 static gboolean get_enabled(GAction G_GNUC_UNUSED(*action), GtkWidget *terminal) {

//	debug("%s(%s)",__FUNCTION__,pw3270_action_get_name(action));

 	if(terminal)
		return lib3270_is_connected(v3270_get_session(terminal)) > 0 ? TRUE: FALSE;

	return FALSE;

 }

 static void activate(GAction *action, GVariant *parameter, GtkWidget *terminal) {

	if(action && terminal && parameter) {

		H3270 * hSession = v3270_get_session(terminal);

		if(g_variant_is_of_type(parameter, G_VARIANT_TYPE_INT32)) {

			lib3270_pfkey(hSession,(int) g_variant_get_int32(parameter));

		} else if(g_variant_is_of_type(parameter, G_VARIANT_TYPE_UINT32)) {

			lib3270_pfkey(hSession,(int) g_variant_get_uint32(parameter));

		} else if(g_variant_is_of_type(parameter, G_VARIANT_TYPE_INT16)) {

			lib3270_pfkey(hSession,(int) g_variant_get_int16(parameter));

		} else if(g_variant_is_of_type(parameter, G_VARIANT_TYPE_UINT16)) {

			lib3270_pfkey(hSession,(int) g_variant_get_uint16(parameter));

		}

	}

 }

 static const GVariantType * get_parameter_type(GAction G_GNUC_UNUSED(*action)) {
	return G_VARIANT_TYPE_UINT16;
 }

 void Lib3270PfAction_class_init(Lib3270PfActionClass *klass) {

	pw3270ActionClass * action = PW3270_ACTION_CLASS(klass);

	action->get_enabled = get_enabled;
	action->change_widget = change_widget;
	action->get_parameter_type = get_parameter_type;

 }

 void Lib3270PfAction_init(Lib3270PfAction *action) {
	PW3270_ACTION(action)->activate = activate;
 }

 GAction * pw3270_action_new_pfkey(void) {

 	Lib3270PfAction	* action = (Lib3270PfAction *) g_object_new(PW3270_TYPE_PFKEY_ACTION, NULL);

	// Setup the default name.
	pw3270Action * abstract	= PW3270_ACTION(action);

	if(abstract->name)
		g_free(abstract->name);

	abstract->name = g_strdup("pfkey");

 	return G_ACTION(action);

 }

 void change_widget(GAction *object, GtkWidget *from, GtkWidget *to) {

	PW3270_ACTION_CLASS(Lib3270PfAction_parent_class)->change_widget(object,from,to);

	// Does the "enabled" state has changed? If yes notify customers.
	gboolean enabled = get_enabled(object,to);
	if(get_enabled(object,from) != enabled)
		pw3270_action_notify_enabled(object);

 }

