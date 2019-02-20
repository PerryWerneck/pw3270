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
 * Este programa está nomeado como main.c e possui - linhas de código.
 *
 * Referências:
 *
 * https://github.com/fbuihuu/samples-dbus/blob/master/dbus-server.c
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-glib.h>

#include <pw3270.h>
#include <pw3270/plugin.h>

#include "service.h"
#include "dbus-glue.h"

#include <gtk/gtk.h>

/*---[ Globals ]---------------------------------------------------------------------------------*/

 static DBusGConnection	* connection	= NULL;
 static DBusGProxy		* proxy			= NULL;
 static gchar 			* service_name	= NULL;


/*---[ Implement ]-------------------------------------------------------------------------------*/

 LIB3270_EXPORT int pw3270_plugin_start(GtkWidget *window, GtkWidget *terminal)
 {

	GError	* error 		= NULL;
	guint     result;
	char	  session_id	= 0;
	char	  id			= 'a';

	connection = dbus_g_bus_get_private(DBUS_BUS_SESSION, g_main_context_default(), &error);
	if(error)
	{
		GtkWidget *dialog =  gtk_message_dialog_new(
									GTK_WINDOW(window),
									GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_ERROR,
									GTK_BUTTONS_OK,
									_( "Can't connect to DBUS server" ));

		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s",error->message);

		g_message("Error \"%s\" getting session dbus",error->message);

		g_error_free(error);

		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		return -1;
	}

	proxy = dbus_g_proxy_new_for_name(connection,DBUS_SERVICE_DBUS,DBUS_PATH_DBUS,DBUS_INTERFACE_DBUS);

	for(id='a'; id < 'z' && !error && !service_name; id++)
	{
		gboolean has_owner = FALSE;

		service_name = g_strdup_printf("br.com.bb.%s.%c",pw3270_get_session_name(window),(int) id);

		org_freedesktop_DBus_name_has_owner(proxy, service_name, &has_owner, NULL);

		if(has_owner)
		{
			trace("Service \"%s\" has owner",service_name)
			g_free(service_name);
			service_name = NULL;
		}
		else
		{
			session_id = id;
			g_message("DBUS service name is %s",service_name);
			org_freedesktop_DBus_request_name(proxy, service_name, DBUS_NAME_FLAG_DO_NOT_QUEUE, &result, &error);
		}
	}

	if(error)
	{
		GtkWidget *dialog =  gtk_message_dialog_new(
									GTK_WINDOW(window),
									GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
									GTK_MESSAGE_ERROR,
									GTK_BUTTONS_OK,
									_( "Can't get DBUS object name" ));

		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s",error->message);

		g_message("Error \"%s\" requesting DBUS name",error->message);

		g_error_free(error);

		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);

		return -1;
	}

	if(session_id)
	{
		gchar * path	= g_strdup_printf("/br/com/bb/%s",pw3270_get_session_name(window));
		gchar * session	= g_strdup_printf("%s:%c",pw3270_get_session_name(window),g_ascii_toupper(session_id));
		pw3270_set_session_name(window,session);
		g_free(session);

		g_message("DBUS service path is %s",path);

		pw3270_dbus_register_object(connection,proxy,PW3270_TYPE_DBUS,&dbus_glib_pw3270_dbus_object_info,path);

		g_free(path);
	}

	return 0;
 }

 LIB3270_EXPORT int pw3270_plugin_stop(GtkWidget *window, GtkWidget *terminal)
 {
	if(service_name)
	{
		// org_freedesktop_DBus_release_name

		g_free(service_name);
		service_name = NULL;
	}
	return 0;
 }

 void pw3270_dbus_quit(PW3270Dbus *object, DBusGMethodInvocation *context)
 {
	gtk_main_quit();
	dbus_g_method_return(context,0);
 }

 H3270 * pw3270_dbus_get_session_handle(PW3270Dbus *object)
 {
	return lib3270_get_default_session_handle();
 }



