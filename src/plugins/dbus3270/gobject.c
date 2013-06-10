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
#include <lib3270.h>
#include <lib3270/actions.h>

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
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,PACKAGE_REVISION);
}

void pw3270_dbus_connect(PW3270Dbus *object, const gchar *uri, DBusGMethodInvocation *context)
{
	trace("%s object=%p context=%p",__FUNCTION__,object,context);

	g_message("Connecting to \"%s\" by remote request",uri);
	dbus_g_method_return(context,lib3270_connect(pw3270_dbus_get_session_handle(PW3270_DBUS(object)),uri,0));
}

void pw3270_dbus_disconnect(PW3270Dbus *object, DBusGMethodInvocation *context)
{
	trace("%s object=%p context=%p",__FUNCTION__,object,context);

	g_message("Disconnecting by remote request");
	lib3270_disconnect(pw3270_dbus_get_session_handle(object));
	dbus_g_method_return(context,0);
}

void pw3270_dbus_get_message_id(PW3270Dbus *object, DBusGMethodInvocation *context)
{
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_get_program_message(pw3270_dbus_get_session_handle(object)));
}

void pw3270_dbus_get_connection_state(PW3270Dbus *object, DBusGMethodInvocation *context)
{
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_get_connection_state(pw3270_dbus_get_session_handle(object)));
}


GError * pw3270_dbus_get_error_from_errno(int code)
{
	return g_error_new(ERROR_DOMAIN,code,"%s",g_strerror(code));
}

int pw3270_dbus_check_valid_state(PW3270Dbus *object, DBusGMethodInvocation *context)
{
	H3270	* hSession = pw3270_dbus_get_session_handle(object);
	GError	* error = NULL;

	trace("%s object=%p context=%p",__FUNCTION__,object,context);

	if(!lib3270_is_connected(hSession))
	{
		error = pw3270_dbus_get_error_from_errno(ENOTCONN);
	}
	else
	{
		LIB3270_MESSAGE state = lib3270_get_program_message(hSession);

		switch(state)
		{
		case LIB3270_MESSAGE_NONE:
			return 0;

		case LIB3270_MESSAGE_DISCONNECTED:
			error = pw3270_dbus_get_error_from_errno(ENOTCONN);
			break;

		case LIB3270_MESSAGE_MINUS:
		case LIB3270_MESSAGE_PROTECTED:
		case LIB3270_MESSAGE_NUMERIC:
		case LIB3270_MESSAGE_OVERFLOW:
		case LIB3270_MESSAGE_INHIBIT:
		case LIB3270_MESSAGE_KYBDLOCK:
		case LIB3270_MESSAGE_X:
			error = g_error_new(ERROR_DOMAIN,-1,_( "State %04d can't accept requests" ),state);
			break;

		case LIB3270_MESSAGE_SYSWAIT:
		case LIB3270_MESSAGE_TWAIT:
		case LIB3270_MESSAGE_CONNECTED:
		case LIB3270_MESSAGE_AWAITING_FIRST:
			error = pw3270_dbus_get_error_from_errno(EBUSY);
			break;

		case LIB3270_MESSAGE_RESOLVING:
		case LIB3270_MESSAGE_CONNECTING:
			error = g_error_new(ERROR_DOMAIN,EINPROGRESS,_( "Connecting to host" ));

		case LIB3270_MESSAGE_USER:
			error = g_error_new(ERROR_DOMAIN,-1,_( "Unexpected state %04d" ),state);
		}
	}

	if(error)
	{
		dbus_g_method_return_error(context,error);
		g_error_free(error);
		return -1;
	}

	return 0;
}

void pw3270_dbus_get_screen_contents(PW3270Dbus *object, DBusGMethodInvocation *context)
{
	char	* text;
	gchar	* utftext;
	H3270	* hSession = pw3270_dbus_get_session_handle(object);

	trace("%s object=%p context=%p",__FUNCTION__,object,context);

	if(pw3270_dbus_check_valid_state(object,context))
		return;

	text = lib3270_get_text(hSession,0,-1);

	utftext = g_convert_with_fallback(text,-1,"UTF-8",lib3270_get_charset(hSession),"?",NULL,NULL,NULL);

	lib3270_free(text);

	dbus_g_method_return(context,utftext);

	g_free(utftext);

}

void pw3270_dbus_enter(PW3270Dbus *object, DBusGMethodInvocation *context)
{
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	if(pw3270_dbus_check_valid_state(object,context))
		return;
	dbus_g_method_return(context,lib3270_enter(pw3270_dbus_get_session_handle(object)));
}

void pw3270_dbus_set_text_at(PW3270Dbus *object, int row, int col, const gchar *utftext, DBusGMethodInvocation *context)
{
	gchar	* text;
	H3270	* hSession = pw3270_dbus_get_session_handle(object);

	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	if(pw3270_dbus_check_valid_state(object,context))
		return;

	text = g_convert_with_fallback(utftext,-1,lib3270_get_charset(hSession),"UTF-8","?",NULL,NULL,NULL);

	dbus_g_method_return(context,lib3270_set_string_at(hSession,row,col,(const unsigned char *) text));

	g_free(text);
}

void pw3270_dbus_get_text_at(PW3270Dbus *object, int row, int col, int len, DBusGMethodInvocation *context)
{
	gchar	* text;
	H3270	* hSession = pw3270_dbus_get_session_handle(object);

	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	if(pw3270_dbus_check_valid_state(object,context))
		return;

	text = lib3270_get_text_at(hSession, row, col, len);
	if(!text)
	{
		GError *error = pw3270_dbus_get_error_from_errno(errno);
		dbus_g_method_return_error(context,error);
		g_error_free(error);
	}
	else
	{
		gchar * utftext = g_convert_with_fallback(text,-1,"UTF-8",lib3270_get_charset(hSession),"?",NULL,NULL,NULL);

		lib3270_free(text);

		dbus_g_method_return(context,utftext);

		g_free(utftext);
	}


}

 void pw3270_dbus_is_connected(PW3270Dbus *object, DBusGMethodInvocation *context)
 {
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_is_connected(pw3270_dbus_get_session_handle(object)));
 }

 void pw3270_dbus_is_ready(PW3270Dbus *object, DBusGMethodInvocation *context)
 {
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_is_ready(pw3270_dbus_get_session_handle(object)));
 }

 void pw3270_dbus_in_tn3270_e(PW3270Dbus *object, DBusGMethodInvocation *context)
 {
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_in_tn3270e(pw3270_dbus_get_session_handle(object)));
 }

 void pw3270_dbus_wait_for_ready(PW3270Dbus *object, int timeout, DBusGMethodInvocation *context)
 {
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_wait_for_ready(pw3270_dbus_get_session_handle(object),timeout));
 }

 void pw3270_dbus_set_cursor_at(PW3270Dbus *object, int row, int col, DBusGMethodInvocation *context)
 {
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_set_cursor_position(pw3270_dbus_get_session_handle(object),row,col));
 }

 void pw3270_dbus_set_toggle(PW3270Dbus *object, int id, int value, DBusGMethodInvocation *context)
 {
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_set_toggle(pw3270_dbus_get_session_handle(object),id,value));
 }

void pw3270_dbus_cmp_text_at(PW3270Dbus *object, int row, int col, const gchar *utftext, DBusGMethodInvocation *context)
{
	gchar	* text;
	H3270	* hSession = pw3270_dbus_get_session_handle(object);

	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	if(pw3270_dbus_check_valid_state(object,context))
		return;

	text = g_convert_with_fallback(utftext,-1,lib3270_get_charset(hSession),"UTF-8","?",NULL,NULL,NULL);

	dbus_g_method_return(context,lib3270_cmp_text_at(hSession,row,col,text));

	g_free(text);
}

void pw3270_dbus_pf_key(PW3270Dbus *object, int key, DBusGMethodInvocation *context)
{
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	if(pw3270_dbus_check_valid_state(object,context))
		return;
	dbus_g_method_return(context,lib3270_pfkey(pw3270_dbus_get_session_handle(object),key));
}

void pw3270_dbus_pa_key(PW3270Dbus *object, int key, DBusGMethodInvocation *context)
{
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	if(pw3270_dbus_check_valid_state(object,context))
		return;
	dbus_g_method_return(context,lib3270_pakey(pw3270_dbus_get_session_handle(object),key));
}

 void pw3270_dbus_get_field_start(PW3270Dbus *object, int baddr, DBusGMethodInvocation *context)
 {
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_get_field_start(pw3270_dbus_get_session_handle(object),baddr));
 }

void pw3270_dbus_get_field_len(PW3270Dbus *object, int baddr, DBusGMethodInvocation *context)
 {
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_get_field_len(pw3270_dbus_get_session_handle(object),baddr));
 }
