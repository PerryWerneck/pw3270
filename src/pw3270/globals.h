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
 * Este programa está nomeado como globals.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <lib3270/config.h>
 #define ENABLE_NLS
 #define GETTEXT_PACKAGE PACKAGE_NAME

 #include <libintl.h>
 #include <glib/gi18n.h>
 #include <gtk/gtk.h>

 #include <pw3270.h>
 #include <pw3270/v3270.h>

 // Special actions
 enum
 {
 	ACTION_PASTENEXT,
 	ACTION_RESELECT,
 	ACTION_FULLSCREEN,
 	ACTION_UNFULLSCREEN,

 	ACTION_COUNT
 };


/*--[ Statics ]--------------------------------------------------------------------------------------*/


/*--[ Global prototipes ]----------------------------------------------------------------------------*/

 #include "common/common.h"

 G_GNUC_INTERNAL GtkWidget		* create_main_window(const gchar *uri);
 G_GNUC_INTERNAL void			  setup_font_list(GtkWidget *widget, GtkWidget *obj);
 G_GNUC_INTERNAL void			  load_color_schemes(GtkWidget *widget, gchar *active);
 G_GNUC_INTERNAL GtkWidget		* color_scheme_new(const GdkRGBA *current);
 G_GNUC_INTERNAL LIB3270_OPTION	  pw3270_options_by_hosttype(const gchar *systype);
 G_GNUC_INTERNAL void			  run_security_dialog(GtkWidget *widget);

 // actions
 G_GNUC_INTERNAL void paste_file_action(GtkAction *action, GtkWidget *widget);
 G_GNUC_INTERNAL void hostname_action(GtkAction *action, GtkWidget *widget);
 G_GNUC_INTERNAL void save_all_action(GtkAction *action, GtkWidget *widget);
 G_GNUC_INTERNAL void save_selected_action(GtkAction *action, GtkWidget *widget);
 G_GNUC_INTERNAL void save_copy_action(GtkAction *action, GtkWidget *widget);
 G_GNUC_INTERNAL void print_all_action(GtkAction *action, GtkWidget *widget);
 G_GNUC_INTERNAL void print_selected_action(GtkAction *action, GtkWidget *widget);
 G_GNUC_INTERNAL void print_copy_action(GtkAction *action, GtkWidget *widget);
 G_GNUC_INTERNAL void editcolors_action(GtkAction *action, GtkWidget *widget);
 G_GNUC_INTERNAL void about_dialog_action(GtkAction *action, GtkWidget *widget);
 G_GNUC_INTERNAL void download_action(GtkAction *action, GtkWidget *widget);
 G_GNUC_INTERNAL void upload_action(GtkAction *action, GtkWidget *widget);
 G_GNUC_INTERNAL void print_settings_action(GtkAction *action, GtkWidget *widget);




