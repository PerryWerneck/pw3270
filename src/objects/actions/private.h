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

	#include <pw3270/actions.h>

	#include <lib3270/actions.h>
	#include <lib3270/toggle.h>

	#include <lib3270/log.h>

	/* not really I18N-related, but also a string marker macro */
	#define I_(string) g_intern_static_string (string)

	G_GNUC_INTERNAL void pw3270_action_set_name(GAction *object, const gchar *name);

	G_GNUC_INTERNAL void pw3270_action_notify_enabled(GAction *action);
	G_GNUC_INTERNAL void pw3270_action_notify_state(GAction *object);


#endif // PRIVATE_H_INCLUDED
