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
  * @brief Implement PW3270 clipboard actions.
  *
  */

 #include "private.h"
 #include <v3270.h>

 static void activate_copy(GAction G_GNUC_UNUSED(*action), GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal) {

	debug("%s",__FUNCTION__);
	v3270_copy_selection(terminal,V3270_SELECT_TEXT,FALSE);

 }

 static void activate_cut(GAction G_GNUC_UNUSED(*action), GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal) {

	debug("%s",__FUNCTION__);
	v3270_copy_selection(terminal,V3270_SELECT_TEXT,TRUE);

 }

  static void activate_paste(GAction G_GNUC_UNUSED(*action), GVariant *parameter, GtkWidget *terminal) {


	if(!parameter) {
		debug("%s %p",__FUNCTION__,"NULL");
		v3270_paste_from_url(terminal,NULL);
	} else {
		debug("%s \"%s\"",__FUNCTION__,g_variant_get_string(parameter,NULL));
		v3270_paste_from_url(terminal,g_variant_get_string(parameter,NULL));
	}

 }

 GAction * pw3270_copy_action_new(void) {

	/*
	static const LIB3270_ACTION action_descriptor = {
		.name = "copy",
		.type = LIB3270_ACTION_TYPE_SELECTION,

		.key = "<ctrl>c",
		.icon = "edit-copy",
		.label = N_( "_Copy" ),
		.summary = N_( "Copy selected area to clipboard." ),
		.activate = NULL,

		.group = LIB3270_ACTION_GROUP_SELECTION,
		.activatable = lib3270_has_selection

	};

	GAction * action = pw3270_action_new_from_lib3270(&action_descriptor);

	PW3270_ACTION(action)->activate = activate_copy;


	return action;


	*/

	pw3270SimpleAction * action = pw3270_simple_action_new();

	action->parent.activate = activate_copy;
	action->group.id = LIB3270_ACTION_GROUP_SELECTION;
	action->parent.name = "file.transfer";
	action->icon_name = "edit-copy";
	action->label =  N_( "_Copy" );
	action->tooltip = N_( "Copy selected area to clipboard." );

	return G_ACTION(action);

 }

 GAction * pw3270_cut_action_new(void) {

	/*
	static const LIB3270_ACTION action_descriptor = {
		.name = "cut",
		.type = LIB3270_ACTION_TYPE_SELECTION,

		.key = "<ctrl>x",
		.icon = "edit-cut",
		.label = N_( "_Cut" ),
		.summary = N_( "Cut selected area." ),
		.activate = NULL,

		.group = LIB3270_ACTION_GROUP_SELECTION,
		.activatable = lib3270_has_selection

	};

	GAction * action = pw3270_action_new_from_lib3270(&action_descriptor);

	PW3270_ACTION(action)->activate = activate_cut;

	return action;
	*/

	pw3270SimpleAction * action = pw3270_simple_action_new();

	action->parent.activate = activate_cut;
	action->group.id = LIB3270_ACTION_GROUP_SELECTION;
	action->parent.name = "file.transfer";
	action->icon_name = "edit-cut";
	action->label =  N_( "C_ut" );
	action->tooltip = N_( "Cut selected area." );

	return G_ACTION(action);

 }

 GAction * pw3270_paste_action_new(void) {

	/*
	static const LIB3270_ACTION action_descriptor = {
		.name = "paste",
		.type = LIB3270_ACTION_TYPE_SELECTION,

		.key = "<ctrl>v",
		.icon = "edit-paste",
		.label = N_( "_Paste" ),
		.summary = N_( "Paste data from clipboard." ),
		.activate = NULL,

		.group = LIB3270_ACTION_GROUP_LOCK_STATE,
		.activatable = lib3270_is_unlocked

	};

	pw3270Action * action = PW3270_ACTION(pw3270_action_new_from_lib3270(&action_descriptor));

	action->types.parameter = G_VARIANT_TYPE_STRING;
	action->activate = activate_paste;

	return G_ACTION(action);
	*/

	pw3270SimpleAction * action = pw3270_simple_action_new();

	action->parent.activate = activate_paste;
	action->parent.types.parameter = G_VARIANT_TYPE_STRING;

	action->group.id = LIB3270_ACTION_GROUP_LOCK_STATE;
	action->parent.name = "paste";
	action->icon_name = "edit-paste";
	action->label =  N_( "_Paste" );
	action->tooltip = N_( "Paste text from clipboard." );

	return G_ACTION(action);

 }
