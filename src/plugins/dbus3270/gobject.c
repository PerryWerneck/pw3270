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
#include <gtk/gtk.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <dbus/dbus-glib.h>

#include <config.h>
#include <lib3270.h>
#include <pw3270.h>
#include <v3270.h>
#include <lib3270/actions.h>
#include <lib3270/charset.h>

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
#ifdef PACKAGE_REVISION
	dbus_g_method_return(context,PACKAGE_REVISION);
#else
	dbus_g_method_return(context,BUILD_DATE);
#endif
}

void pw3270_dbus_connect(PW3270Dbus *object, const gchar *uri, DBusGMethodInvocation *context)
{
	H3270 *hSession = pw3270_dbus_get_session_handle(PW3270_DBUS(object));

	trace("%s object=%p context=%p",__FUNCTION__,object,context);

	if(uri && *uri)
	{
		lib3270_set_url(hSession,uri);
		g_message("Connecting to \"%s\" by remote request",lib3270_get_url(hSession));
	}
	else
	{
		g_message("%s","Connecting by remote request");
	}

	dbus_g_method_return(context,lib3270_reconnect(hSession,0));
}

void pw3270_dbus_set_ur_l(PW3270Dbus *object, const gchar *uri, DBusGMethodInvocation *context)
{
	trace("%s object=%p context=%p",__FUNCTION__,object,context);

	g_message("Changing host to \"%s\" by remote request",uri);

	dbus_g_method_return(context,lib3270_set_url(pw3270_dbus_get_session_handle(PW3270_DBUS(object)),uri) != 0);
}

void pw3270_dbus_get_ur_l(PW3270Dbus *object, DBusGMethodInvocation *context)
{
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_get_url(pw3270_dbus_get_session_handle(PW3270_DBUS(object))));
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

void pw3270_dbus_get_secure_state(PW3270Dbus *object, DBusGMethodInvocation *context)
{
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_get_secure(pw3270_dbus_get_session_handle(object)));
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

	text = lib3270_get_string_at_address(hSession,0,-1,'\n');

	utftext = g_convert_with_fallback(text,-1,"UTF-8",lib3270_get_display_charset(hSession),"?",NULL,NULL,NULL);

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

	text = g_convert_with_fallback(utftext,-1,lib3270_get_display_charset(hSession),"UTF-8","?",NULL,NULL,NULL);

	int sz = lib3270_set_string_at(hSession,row,col,(const unsigned char *) text);

	trace("%s returns %d",__FUNCTION__,sz);
	dbus_g_method_return(context,sz);

	g_free(text);
}

void pw3270_dbus_input(PW3270Dbus *object, const gchar *utftext, DBusGMethodInvocation *context)
{
	gchar	* text;
	H3270	* hSession = pw3270_dbus_get_session_handle(object);

	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	if(pw3270_dbus_check_valid_state(object,context))
		return;

	text = g_convert_with_fallback(utftext,-1,lib3270_get_display_charset(hSession),"UTF-8","?",NULL,NULL,NULL);

	dbus_g_method_return(context,lib3270_emulate_input(hSession,(const char *) text,-1,1));

	g_free(text);
}


void pw3270_dbus_get_text_at(PW3270Dbus *object, int row, int col, int len, char lf, DBusGMethodInvocation *context)
{
	gchar	* text;
	H3270	* hSession = pw3270_dbus_get_session_handle(object);

	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	if(pw3270_dbus_check_valid_state(object,context))
		return;

	text = lib3270_get_string_at(hSession, row, col, len, lf);
	if(!text)
	{
		GError *error = pw3270_dbus_get_error_from_errno(errno);
		dbus_g_method_return_error(context,error);
		g_error_free(error);
	}
	else
	{
		gchar * utftext = g_convert_with_fallback(text,-1,"UTF-8",lib3270_get_display_charset(hSession),"?",NULL,NULL,NULL);

		lib3270_free(text);

		dbus_g_method_return(context,utftext);

		g_free(utftext);
	}
 }

 void pw3270_dbus_get_text(PW3270Dbus *object, int offset, int len, char lf, DBusGMethodInvocation *context)
 {
	gchar	* text;
	H3270	* hSession = pw3270_dbus_get_session_handle(object);

	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	if(pw3270_dbus_check_valid_state(object,context))
		return;

	if(len < 0) {
		len = lib3270_get_length(hSession);
	}

	text = lib3270_get_string_at_address(hSession,offset,len,lf);
	if(!text)
	{
		GError *error = pw3270_dbus_get_error_from_errno(errno);
		dbus_g_method_return_error(context,error);
		g_error_free(error);
	}
	else
	{
		gchar * utftext = g_convert_with_fallback(text,-1,"UTF-8",lib3270_get_display_charset(hSession),"?",NULL,NULL,NULL);

		lib3270_free(text);

		debug("\n%s\n",utftext);

		dbus_g_method_return(context,utftext);

		g_free(utftext);
	}

 }

 void pw3270_dbus_is_connected(PW3270Dbus *object, DBusGMethodInvocation *context)
 {
	trace("%s object=%p context=%p connected=%s",__FUNCTION__,object,context, lib3270_is_connected(pw3270_dbus_get_session_handle(object)) ? "Yes" : "No");
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

 void pw3270_dbus_set_cursor_address(PW3270Dbus *object, int addr, DBusGMethodInvocation *context)
 {
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_set_cursor_address(pw3270_dbus_get_session_handle(object),addr));
 }

 void pw3270_dbus_get_cursor_address(PW3270Dbus *object, DBusGMethodInvocation *context)
 {
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_get_cursor_address(pw3270_dbus_get_session_handle(object)));
 }

 void pw3270_dbus_get_screen_width(PW3270Dbus *object, DBusGMethodInvocation *context)
 {
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_get_width(pw3270_dbus_get_session_handle(object)));
 }

 void pw3270_dbus_get_screen_height(PW3270Dbus *object, DBusGMethodInvocation *context)
 {
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_get_height(pw3270_dbus_get_session_handle(object)));
 }

 void pw3270_dbus_get_screen_length(PW3270Dbus *object, DBusGMethodInvocation *context)
 {
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_get_width(pw3270_dbus_get_session_handle(object)));
 }

 void pw3270_dbus_set_toggle(PW3270Dbus *object, int id, int value, DBusGMethodInvocation *context)
 {
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_set_toggle(pw3270_dbus_get_session_handle(object),id,value));
 }

void pw3270_dbus_cmp_text_at(PW3270Dbus *object, int row, int col, const gchar *utftext, char lf, DBusGMethodInvocation *context)
{
	gchar	* text;
	H3270	* hSession = pw3270_dbus_get_session_handle(object);

	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	if(pw3270_dbus_check_valid_state(object,context))
		return;

	text = g_convert_with_fallback(utftext,-1,lib3270_get_display_charset(hSession),"UTF-8","?",NULL,NULL,NULL);

	dbus_g_method_return(context,lib3270_cmp_text_at(hSession,row,col,text,lf));

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

 void pw3270_dbus_get_field_length(PW3270Dbus *object, int baddr, DBusGMethodInvocation *context)
 {
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_get_field_len(pw3270_dbus_get_session_handle(object),baddr));
 }

 void pw3270_dbus_get_next_unprotected(PW3270Dbus *object, int baddr, DBusGMethodInvocation *context)
 {
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_get_next_unprotected(pw3270_dbus_get_session_handle(object),baddr));
 }

 void pw3270_dbus_get_is_protected(PW3270Dbus *object, int baddr, DBusGMethodInvocation *context)
 {
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_get_is_protected(pw3270_dbus_get_session_handle(object),baddr));
 }

 void pw3270_dbus_get_is_protected_at(PW3270Dbus *object, int row, int col, DBusGMethodInvocation *context)
 {
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_get_is_protected_at(pw3270_dbus_get_session_handle(object),row,col));
 }

 void pw3270_dbus_action(PW3270Dbus *object, const gchar *text, DBusGMethodInvocation *context)
 {
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_action(pw3270_dbus_get_session_handle(object),text));
 }

 void pw3270_dbus_get_clipboard(PW3270Dbus *object, DBusGMethodInvocation *context)
 {
	gchar *text;

	trace("%s object=%p context=%p",__FUNCTION__,object,context);

	if(pw3270_dbus_check_valid_state(object,context))
		return;

	text = gtk_clipboard_wait_for_text(gtk_widget_get_clipboard(pw3270_get_toplevel(),GDK_SELECTION_CLIPBOARD));

	trace("Clipboard:\n%s\n",text);

	if(!text)
	{
		GError *error = pw3270_dbus_get_error_from_errno(ENOENT);
		dbus_g_method_return_error(context,error);
		g_error_free(error);
	}
	else
	{
		dbus_g_method_return(context,text);
		g_free(text);
	}
}

void pw3270_dbus_set_clipboard(PW3270Dbus *object, const gchar *text, DBusGMethodInvocation *context)
{
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
    gtk_clipboard_set_text(gtk_widget_get_clipboard(pw3270_get_toplevel(),GDK_SELECTION_CLIPBOARD),(gchar *) text, -1);
	dbus_g_method_return(context,0);
}

void pw3270_dbus_set_script(PW3270Dbus *object, const gchar *text, int mode, DBusGMethodInvocation *context)
{
	GtkWidget *widget = pw3270_get_terminal_widget(NULL);

	trace("%s object=%p context=%p",__FUNCTION__,object,context);

	if(!widget)
	{
		GError *error = pw3270_dbus_get_error_from_errno(EINVAL);
		dbus_g_method_return_error(context,error);
		g_error_free(error);
		return;
	}

	dbus_g_method_return(context,v3270_set_script(widget,mode == 0 ? 0 : 'S'));
}

void pw3270_dbus_show_popup(PW3270Dbus *object, int id, const gchar *title, const gchar *msg, const gchar *text, DBusGMethodInvocation *context)
{
	lib3270_popup_dialog(pw3270_dbus_get_session_handle(object), (LIB3270_NOTIFY) id , title, msg, "%s", text);
	dbus_g_method_return(context,0);
}

void pw3270_dbus_get_host_charset(PW3270Dbus *object, DBusGMethodInvocation *context)
{
	dbus_g_method_return(context,lib3270_get_host_charset(pw3270_dbus_get_session_handle(object)));
}

void pw3270_dbus_get_display_charset(PW3270Dbus *object, DBusGMethodInvocation *context)
{
	// Allways return UTF-8 to avoid double conversion
	dbus_g_method_return(context,"UTF-8");
}

void pw3270_dbus_set_host_charset(PW3270Dbus *object, const gchar *charset, DBusGMethodInvocation *context)
{
	dbus_g_method_return(context,lib3270_set_host_charset(pw3270_dbus_get_session_handle(object),charset));
}

void pw3270_dbus_erase_eof(PW3270Dbus *object, DBusGMethodInvocation *context)
{
	trace("%s object=%p context=%p",__FUNCTION__,object,context);
	dbus_g_method_return(context,lib3270_eraseeof(pw3270_dbus_get_session_handle(object)));
}

void pw3270_dbus_print(PW3270Dbus *object, DBusGMethodInvocation *context)
{
	dbus_g_method_return(context,lib3270_print_all(pw3270_dbus_get_session_handle(object)));
}

void pw3270_dbus_set_unlock_delay(PW3270Dbus *object, int value, DBusGMethodInvocation *context)
{
	lib3270_set_unlock_delay(pw3270_dbus_get_session_handle(object),(unsigned short) value);
	dbus_g_method_return(context,0);
}


void pw3270_dbus_ebc2asc(PW3270Dbus *object, const gchar *from, DBusGMethodInvocation *context)
{
	int sz = strlen(from);

	if(sz > 0)
	{
		unsigned char buffer[sz+1];
		memcpy(buffer,from,sz);
		dbus_g_method_return(context,lib3270_ebc2asc(pw3270_dbus_get_session_handle(object),buffer,sz));
		return;
	}

	dbus_g_method_return(context,"");

}

void pw3270_dbus_asc2ebc(PW3270Dbus *object, const gchar *from, DBusGMethodInvocation *context)
{
	int sz = strlen(from);

	if(sz > 0)
	{
		unsigned char buffer[sz+1];
		memcpy(buffer,from,sz);
		dbus_g_method_return(context,lib3270_asc2ebc(pw3270_dbus_get_session_handle(object),buffer,sz));
		return;
	}

	dbus_g_method_return(context,"");

}

void pw3270_dbus_filetransfer(PW3270Dbus *object, const gchar *local, const gchar *remote, int flags, int lrecl, int blksize, int primspace, int secspace, int dft, DBusGMethodInvocation *context)
{
	/*
	dbus_g_method_return(context,
			v3270_transfer_file(
							v3270_get_default_widget(),
							(LIB3270_FT_OPTION) flags,
							local,
							remote,
							lrecl,
							blksize,
							primspace,
							secspace,
							dft
			));
	*/
	return;
}

