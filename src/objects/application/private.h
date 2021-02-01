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

#ifndef PRIVATE_H_INCLUDED

	#define PRIVATE_H_INCLUDED

	#include <config.h>

	#ifndef GETTEXT_PACKAGE
		#define GETTEXT_PACKAGE PACKAGE_NAME
	#endif

	#include <libintl.h>
	#include <glib/gi18n.h>
	#include <gtk/gtk.h>

	#include <pw3270/window.h>
	#include <v3270.h>
	#include <lib3270.h>
	#include <lib3270/log.h>

	G_GNUC_INTERNAL void		  pw3270_application_open(GApplication * application, GFile **files, gint n_files, const gchar *hint);
	G_GNUC_INTERNAL GtkWidget	* pw3270_terminal_new(const gchar *session_file);
	G_GNUC_INTERNAL GSettings	* pw3270_application_settings_new();

	// Actions
    G_GNUC_INTERNAL GAction * pw3270_about_action_new();
	G_GNUC_INTERNAL GAction * pw3270_preferences_action_new();
    G_GNUC_INTERNAL GAction * pw3270_new_tab_action_new();
	G_GNUC_INTERNAL GAction * pw3270_new_window_action_new();
	G_GNUC_INTERNAL GAction * pw3270_quit_action_new();

	G_GNUC_INTERNAL GAction * pw3270_open_session_action_new();
	G_GNUC_INTERNAL GAction * pw3270_open_window_action_new();
	G_GNUC_INTERNAL GAction * pw3270_open_tab_action_new();

#endif // PRIVATE_H_INCLUDED
