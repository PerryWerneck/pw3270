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
  * @brief Implement GAction "wrapper" for lib3270's PAs.
  *
  */

 #include "../private.h"
 #include <v3270.h>
 #include <v3270/actions.h>

 #define LIB3270_TYPE_PA_ACTION		(Lib3270PaAction_get_type())
 #define LIB3270_PA_ACTION(inst)	(G_TYPE_CHECK_INSTANCE_CAST ((inst), LIB3270_TYPE_PA_ACTION, Lib3270PaAction))
 #define LIB3270_IS_PA_ACTION(inst)	(G_TYPE_CHECK_INSTANCE_TYPE ((inst), LIB3270_TYPE_PA_ACTION))

 typedef struct _Lib3270PaActionClass {
 	V3270ActionClass parent_class;

 } Lib3270PaActionClass;

 typedef struct _Lib3270PaAction {
 	V3270Action parent;
 } Lib3270PaAction;

 static void Lib3270PaAction_class_init(Lib3270PaActionClass *klass);
 static void Lib3270PaAction_init(Lib3270PaAction *action);

 G_DEFINE_TYPE(Lib3270PaAction, Lib3270PaAction, V3270_TYPE_ACTION);

 static void activate(GAction *action, GVariant *parameter, GtkWidget *terminal) {

	if(action && terminal && parameter) {

		H3270 * hSession = v3270_get_session(terminal);

		if(g_variant_is_of_type(parameter, G_VARIANT_TYPE_INT32)) {

			lib3270_pakey(hSession,(int) g_variant_get_int32(parameter));

		} else if(g_variant_is_of_type(parameter, G_VARIANT_TYPE_UINT32)) {

			lib3270_pakey(hSession,(int) g_variant_get_uint32(parameter));

		} else if(g_variant_is_of_type(parameter, G_VARIANT_TYPE_INT16)) {

			lib3270_pakey(hSession,(int) g_variant_get_int16(parameter));

		} else if(g_variant_is_of_type(parameter, G_VARIANT_TYPE_UINT16)) {

			lib3270_pakey(hSession,(int) g_variant_get_uint16(parameter));

		}

	}

 }

 void Lib3270PaAction_class_init(Lib3270PaActionClass G_GNUC_UNUSED(*klass)) {
 }

 void Lib3270PaAction_init(Lib3270PaAction *action) {

	static const LIB3270_PROPERTY info = {
		.name = "pakey",
		.group = LIB3270_ACTION_GROUP_ONLINE
	};

	action->parent.activate = activate;
	action->parent.info		= &info;

 }

 GAction * v3270_pakey_action_new(void) {
 	return G_ACTION(g_object_new(LIB3270_TYPE_PA_ACTION, NULL));
 }

