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
 * Este programa está nomeado como gobject.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 * Referencias:
 *
 * https://live.gnome.org/DBusGlibBindings
 *
 */

#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-glib.h>

#include <lib3270/config.h>

#include "service.h"

/*---[ Globals ]---------------------------------------------------------------------------------*/


/*---[ Implement ]-------------------------------------------------------------------------------*/

G_DEFINE_TYPE(PW3270Dbus, pw3270_dbus, G_TYPE_OBJECT)

static void pw3270_dbus_finalize(GObject *object)
{
	G_OBJECT_CLASS(pw3270_dbus_parent_class)->finalize (object);
}


static void pw3270_dbus_class_init(PW3270DbusClass *klass)
{
        GObjectClass *object_class;
        object_class = G_OBJECT_CLASS (klass);
        object_class->finalize = pw3270_dbus_finalize;
}

static void pw3270_dbus_init(PW3270Dbus *object)
{

}

PW3270Dbus * pw3270_dbus_new(void)
{
	return g_object_new(PW3270_TYPE_DBUS, NULL);
}

void pw3270_dbus_get_revision(PW3270Dbus *object, DBusGMethodInvocation *context)
{
	dbus_g_method_return(context,PACKAGE_REVISION);
}
