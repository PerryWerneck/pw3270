/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob
 * o nome G3270.
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
 * Este programa está nomeado como testprogram.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <config.h>
 #include <pw3270/actions.h>
 #include <pw3270/window.h>
 #include <lib3270.h>
 #include <lib3270/log.h>

 /*---[ Implement ]----------------------------------------------------------------------------------*/

 GtkWidget * pw3270_window_get_terminal_widget(GtkWidget G_GNUC_UNUSED(*window)) {
 	return NULL;
 }

 H3270 * pw3270_window_get_session_handle(GtkWidget G_GNUC_UNUSED(*window)) {
 	return NULL;
 }

 int main (int argc, char **argv) {

	GAction * action = pw3270_action_get_from_lib3270(lib3270_action_get_by_name("testpattern"));

	g_message("Action name is \"%s\"",g_action_get_name(action));
	g_message("Action is %s",g_action_get_enabled(action) ? "enabled" : "disabled");

	g_object_unref(action);
	return 0;

 }



