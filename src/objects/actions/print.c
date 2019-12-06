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
  * @brief Implement PW3270 print actions.
  *
  */

 #include "private.h"
 #include <v3270.h>
 #include <pw3270/application.h>

 static void activate_print_screen(GAction G_GNUC_UNUSED(*action), GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal) {
	debug("%s",__FUNCTION__);
	v3270_print_all(terminal,NULL);
 }

 static void activate_print_selected(GAction G_GNUC_UNUSED(*action), GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal) {
	debug("%s",__FUNCTION__);
	v3270_print_selected(terminal,NULL);
 }

 void pw3270_application_print_copy_activated(GAction G_GNUC_UNUSED(*action), GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal) {

	debug("%s",__FUNCTION__);
	v3270_print_copy(terminal,NULL);
 }

 static void activate_print(GAction G_GNUC_UNUSED(*action), GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal) {

	debug("%s",__FUNCTION__);

 }

 GAction * pw3270_action_print_new(void) {

	pw3270SimpleAction * action = pw3270_simple_action_new();

	action->parent.activate = activate_print;
	action->parent.types.parameter = G_VARIANT_TYPE_STRING;

	action->group.id = LIB3270_ACTION_GROUP_ONLINE;
	action->parent.name = "print";
	action->label =  N_( "Print" );
	action->tooltip = N_( "Print terminal contents." );

	return G_ACTION(action);

 }

 GAction * pw3270_action_print_screen_new(void) {

	pw3270SimpleAction * action = pw3270_simple_action_new();

	action->parent.activate = activate_print_screen;

	action->group.id = LIB3270_ACTION_GROUP_ONLINE;
	action->parent.name = "print_screen";
	action->label =  N_( "Print screen" );

	return G_ACTION(action);

 }

 GAction * pw3270_action_print_selected_new(void) {

	pw3270SimpleAction * action = pw3270_simple_action_new();

	action->parent.activate = activate_print_selected;

	action->group.id = LIB3270_ACTION_GROUP_SELECTION;
	action->parent.name = "print_selected";
	action->label =  N_( "Print selected" );

	return G_ACTION(action);

 }

