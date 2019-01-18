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
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

#ifndef PW3270_DBUS_GLOBALS_H_INCLUDED

	#define PW3270_DBUS_GLOBALS_H_INCLUDED 1

	#include <libintl.h>
	#include <glib/gi18n.h>

	#include <errno.h>
	#include <lib3270.h>
	#include <lib3270/log.h>
	#include <glib.h>

	//
	// Disabling warning on unused-function defined in dbus-glib-bindings.h
	// warning: 'org_freedesktop_DBus_reload_config' defined but not used [-Wunused-function] ...
	//
	#pragma GCC diagnostic ignored "-Wunused-function"

	#include <dbus/dbus-glib.h>
	#include <dbus/dbus-glib-bindings.h>
	#include <dbus/dbus-glib-lowlevel.h>

	#define ERROR_DOMAIN g_quark_from_static_string("pw3270DBUS")

	G_GNUC_INTERNAL gpointer pw3270_dbus_register_object (DBusGConnection *connection,DBusGProxy *proxy,GType object_type,const DBusGObjectInfo *info,const gchar *path);


#endif // PW3270_DBUS_GLOBALS_H_INCLUDED
