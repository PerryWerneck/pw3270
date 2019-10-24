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

	struct _pw3270ApplicationWindow {

		GtkApplicationWindow parent;

		GtkNotebook * notebook;
		GtkToolbar	* toolbar;

	};

	struct _pw3270ApplicationWindowClass {

		GtkApplicationWindowClass parent_class;

	};

	// Internal methods
	G_GNUC_INTERNAL GtkWidget * pw3270_setup_image_button(GtkWidget *button, const gchar *image_name);

	// Actions
    G_GNUC_INTERNAL void pw3270_application_generic_activated(GSimpleAction * action, GVariant *parameter, gpointer application);
    G_GNUC_INTERNAL void pw3270_application_quit_activated(GSimpleAction * action, GVariant *parameter, gpointer application);
    G_GNUC_INTERNAL void pw3270_application_new_tab_activated(GSimpleAction * action, GVariant *parameter, gpointer application);
    G_GNUC_INTERNAL void pw3270_application_new_window_activated(GSimpleAction * action, GVariant *parameter, gpointer application);

#endif // PRIVATE_H_INCLUDED
