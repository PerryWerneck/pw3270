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

 static V3270_COPY_MODE get_copy_mode_from_parameter(GVariant *parameter) {

	static const struct {
		const gchar		* name;
		V3270_COPY_MODE	  value;
	} targets[] = {
		{ "auto",			V3270_COPY_DEFAULT		},
		{ "system",			V3270_COPY_DEFAULT		},
		{ "default",		V3270_COPY_DEFAULT		},
		{ "system default",	V3270_COPY_DEFAULT		},
		{ "formatted",		V3270_COPY_FORMATTED	},
		{ "text",			V3270_COPY_TEXT 		},
		{ "table",			V3270_COPY_TABLE		},
		{ "append",			V3270_COPY_APPEND		}

	};

	if(parameter) {

		const gchar * target = g_variant_get_string(parameter,NULL);

		if(target && *target) {

			size_t ix;
			for(ix = 0; ix < G_N_ELEMENTS(targets); ix++) {

				if(!g_ascii_strcasecmp(target,targets[ix].name))
					return targets[ix].value;

			}

		}

	}

	return V3270_COPY_DEFAULT;
 }

 static void activate_copy(GAction G_GNUC_UNUSED(*action), GVariant *parameter, GtkWidget *terminal) {

	debug("%s",__FUNCTION__);
	v3270_clipboard_set(terminal,get_copy_mode_from_parameter(parameter),FALSE);

 }

 static void activate_cut(GAction G_GNUC_UNUSED(*action), GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal) {

	debug("%s",__FUNCTION__);
	v3270_clipboard_set(terminal,get_copy_mode_from_parameter(parameter),TRUE);

 }

  static void activate_paste(GAction G_GNUC_UNUSED(*action), GVariant *parameter, GtkWidget *terminal) {


	if(!parameter) {
		debug("%s %p",__FUNCTION__,"NULL");
		v3270_clipboard_get_from_url(terminal,NULL);
	} else {
		debug("%s \"%s\"",__FUNCTION__,g_variant_get_string(parameter,NULL));
		v3270_clipboard_get_from_url(terminal,g_variant_get_string(parameter,NULL));
	}

 }

 GAction * pw3270_action_copy_new(void) {

	pw3270SimpleAction * action = pw3270_simple_action_new();

	action->parent.activate = activate_copy;
	action->parent.types.parameter = G_VARIANT_TYPE_STRING;

	action->group.id = LIB3270_ACTION_GROUP_SELECTION;
	action->parent.name = "copy";
	action->icon_name = "edit-copy";
	action->label =  N_( "_Copy" );
	action->tooltip = N_( "Copy selected area to clipboard." );

	return G_ACTION(action);

 }

 GAction * pw3270_action_cut_new(void) {

	pw3270SimpleAction * action = pw3270_simple_action_new();

	action->parent.activate = activate_cut;
	action->parent.types.parameter = G_VARIANT_TYPE_STRING;

	action->group.id = LIB3270_ACTION_GROUP_SELECTION;
	action->parent.name = "cut";
	action->icon_name = "edit-cut";
	action->label =  N_( "C_ut" );
	action->tooltip = N_( "Cut selected area." );

	return G_ACTION(action);

 }

 GAction * pw3270_action_paste_new(void) {


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
