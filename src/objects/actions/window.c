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
  * @brief Integrate pw3270 actions with the application window.
  *
  */

 #include "private.h"
 #include <lib3270/actions.h>
 #include <pw3270/window.h>

 void pw3270_window_add_actions(GtkWidget * appwindow) {

	GActionMap * map = G_ACTION_MAP(appwindow);
	size_t ix;

	// Map lib3270 actions
	{
		const LIB3270_ACTION * actions = lib3270_get_actions();
		for(ix = 0; actions[ix].name; ix++) {

			GAction *action = pw3270_action_new_from_lib3270(&actions[ix]);
			g_action_map_add_action(map,action);

		}
	}

	// Map toggles
	{
		const LIB3270_TOGGLE * toggles = lib3270_get_toggles();
		for(ix = 0; toggles[ix].name; ix++) {

			GAction *action = pw3270_toggle_action_new_from_lib3270(&toggles[ix]);
			g_action_map_add_action(map,action);

		}
	}

	// Map special actions
	{
		GAction * actions[] = {
			pw3270_action_new_pfkey(),
			pw3270_action_new_pakey(),
			pw3270_connect_action_new(),
			pw3270_copy_action_new(),
			pw3270_cut_action_new(),
			pw3270_paste_action_new()
		};

		for(ix = 0; ix < G_N_ELEMENTS(actions); ix++) {
			g_action_map_add_action(map,actions[ix]);
		}
	}

	debug("%s ends",__FUNCTION__);

 }
