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
 * Este programa está nomeado como daemon.c e possui - linhas de código.
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

#include "daemon.h"
#include "dbus-glue.h"

#define PW3270_DBUS_SERVICE_PATH	"/br/com/bb/pw3270"
#define PW3270_DBUS_SERVICE			"br.com.bb.pw3270"


/*---[ Globals ]---------------------------------------------------------------------------------*/

 static DBusGConnection	* connection	= NULL;
 static DBusGProxy		* proxy			= NULL;
 static H3270			* hSession		= NULL;

 GMainLoop				* main_loop		= NULL;


/*---[ Implement ]-------------------------------------------------------------------------------*/

static int initialize(void)
{
	GError	* error = NULL;
	guint     result;

	connection = dbus_g_bus_get_private(DBUS_BUS_SESSION, g_main_context_default(), &error);
	if(error)
	{
		g_message("Error \"%s\" getting session dbus",error->message);
		g_error_free(error);
		return -1;
	}

	proxy = dbus_g_proxy_new_for_name(connection,DBUS_SERVICE_DBUS,DBUS_PATH_DBUS,DBUS_INTERFACE_DBUS);

	org_freedesktop_DBus_request_name(proxy, PW3270_DBUS_SERVICE, DBUS_NAME_FLAG_DO_NOT_QUEUE, &result, &error);

	pw3270_dbus_register_object(connection,proxy,PW3270_TYPE_DBUS,&dbus_glib_pw3270_dbus_object_info,PW3270_DBUS_SERVICE_PATH);

	return 0;
}

static void loghandler(H3270 *session, const char *module, int rc, const char *fmt, va_list args)
{
	g_logv(module,rc ? G_LOG_LEVEL_WARNING : G_LOG_LEVEL_MESSAGE, fmt, args);
}

int main(int numpar, char *param[])
{
	g_type_init ();

	if (!g_thread_supported ())
		g_thread_init (NULL);

	dbus_g_thread_init ();

	lib3270_set_log_handler(loghandler);
	pw3270_dbus_register_io_handlers();

	hSession = lib3270_session_new("");

	main_loop = g_main_loop_new (NULL, FALSE);

	if(initialize())
		return -1;

	g_main_loop_run(main_loop);

	lib3270_session_free(hSession);

	return 0;
}

void pw3270_dbus_quit(PW3270Dbus *object, DBusGMethodInvocation *context)
{
	g_main_loop_quit(main_loop);
	dbus_g_method_return(context,0);
}

H3270 * pw3270_dbus_get_session_handle(PW3270Dbus *object)
{
	return hSession;
}

