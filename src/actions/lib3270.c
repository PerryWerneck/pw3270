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

 #include "private.h"
 #include <pw3270/window.h>

 #define PW3270_TYPE_LIB3270_ACTION		(Lib3270Action_get_type())
 #define PW3270_LIB3270_ACTION(inst)	(G_TYPE_CHECK_INSTANCE_CAST ((inst), PW3270_TYPE_LIB3270_ACTION, Lib3270Action))
 #define PW3270_IS_LIB3270_ACTION(inst)	(G_TYPE_CHECK_INSTANCE_TYPE ((inst), PW3270_TYPE_LIB3270_ACTION))

 typedef struct _Lib3270ActionClass {
 	pw3270ActionClass parent_class;

 } Lib3270ActionClass;

 typedef struct _Lib3270Action {
 	pw3270Action parent;

	const LIB3270_ACTION * definition;


 } Lib3270Action;

 static void Lib3270Action_class_init(Lib3270ActionClass *klass);
 static void Lib3270Action_init(Lib3270Action *action);


 G_DEFINE_TYPE(Lib3270Action, Lib3270Action, PW3270_TYPE_ACTION);

 static gboolean action_enabled(GAction *action, GtkWidget *window) {

	H3270 * hSession = pw3270_window_get_session_handle(window);

	if(hSession)
		return PW3270_LIB3270_ACTION(action)->definition->activatable(hSession) > 0 ? TRUE : FALSE;

 	return FALSE;
 }

 static void action_activate(GAction *action, GtkWidget *window) {

	H3270 * hSession = pw3270_window_get_session_handle(window);

	if(hSession)
		PW3270_LIB3270_ACTION(action)->definition->activate(hSession);

 }

 void Lib3270Action_class_init(Lib3270ActionClass *klass) {

 	pw3270ActionClass * action = PW3270_ACTION_CLASS(klass);

	action->get_enabled 	= action_enabled;
	action->activate		= action_activate;


 }

 void Lib3270Action_init(Lib3270Action *action) {
 }

 GAction * pw3270_action_get_from_lib3270(const LIB3270_ACTION * definition) {

 	Lib3270Action * action = (Lib3270Action *) g_object_new(PW3270_TYPE_LIB3270_ACTION, NULL);
	action->definition = definition;

	{
		pw3270Action * abstract = PW3270_ACTION(action);

		abstract->name = definition->name;

	}

 	return G_ACTION(action);
 }


