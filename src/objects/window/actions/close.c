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

 #include <pw3270.h>
 #include <pw3270/actions.h>
 #include <lib3270.h>
 #include <lib3270/log.h>

 /*
 void pw3270_window_close_activated(GSimpleAction G_GNUC_UNUSED(* action), GVariant G_GNUC_UNUSED(*parameter), gpointer window) {

	debug("%s(%p)",__FUNCTION__,window);
	gtk_window_close(GTK_WINDOW(window));

 }
 */

 static void activate(GAction G_GNUC_UNUSED(*action), GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal) {

 	debug("%s","Window close action was activated");
	gtk_window_close(GTK_WINDOW(gtk_widget_get_toplevel(terminal)));

 }


 GAction * pw3270_action_window_close_new(void) {

	pw3270SimpleAction * action = pw3270_simple_action_new();

	action->parent.activate = activate;
	action->parent.name = "close";
	action->icon_name = "window-close";
	action->label =  N_("Close window");
	action->tooltip = N_("Close the window");

	return G_ACTION(action);


 }

