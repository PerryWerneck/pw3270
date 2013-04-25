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
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como remote.cxx e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

 #include "globals.hpp"
 #include <errno.h>
 #include <string.h>

/*---[ Statics ]-------------------------------------------------------------------------------------------*/

#if defined(HAVE_DBUS)
 static const char * prefix_dest	= "br.com.bb.";
 static const char * prefix_path	= "/br/com/bb/";
#endif // HAVE_DBUS

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

#if defined(HAVE_DBUS)
DBusMessage * pw3270::ipc3270_session::create_message(const char *method)
{
	DBusMessage * msg = dbus_message_new_method_call(	this->dest,		// Destination
														this->path,		// Path
														this->intf,		// Interface
														method);		// method

	if (!msg)
		log("Error creating message for method %s",method);

	return msg;
}

DBusMessage	* pw3270::ipc3270_session::call(DBusMessage *msg)
{
	DBusMessage		* reply;
	DBusError		  error;

	dbus_error_init(&error);
	reply = dbus_connection_send_with_reply_and_block(conn,msg,10000,&error);
	dbus_message_unref(msg);

	if(!reply)
	{
		log("%s",error.message);
		dbus_error_free(&error);
	}

	return reply;

}

static char * get_string(DBusMessage * msg)
{
	char *rc = NULL;
	if(msg)
	{
		DBusMessageIter iter;

		if(dbus_message_iter_init(msg, &iter))
		{
			if(dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_STRING)
			{
				const char * str;
				dbus_message_iter_get_basic(&iter, &str);
				trace("Response: [%s]",str);
				rc = strdup(str);
			}
#ifdef DEBUG
			else
			{
				trace("Return type is %c, expecting %c",dbus_message_iter_get_arg_type(&iter),DBUS_TYPE_STRING);
			}
#endif
		}

		dbus_message_unref(msg);
	}
	return rc;
}

char * pw3270::ipc3270_session::query_string(const char *method)
{
	if(conn)
		return get_string(call(create_message(method)));
	return NULL;
}

static int get_intval(DBusMessage * msg)
{
	int rc = -1;

	if(msg)
	{
		DBusMessageIter iter;

		if(dbus_message_iter_init(msg, &iter))
		{
			if(dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_INT32)
			{
				dbus_int32_t iSigned;
				dbus_message_iter_get_basic(&iter, &iSigned);
				rc = (int) iSigned;
			}
#ifdef DEBUG
			else
			{
				trace("Return type is %c, expecting %c",dbus_message_iter_get_arg_type(&iter),DBUS_TYPE_INT32);
			}
#endif
		}

		dbus_message_unref(msg);
	}

	return rc;
}

int pw3270::ipc3270_session::query_intval(const char *method)
{
	if(conn)
		return get_intval(call(create_message(method)));
	return -1;
}

#endif // HAVE_DBUS


pw3270::ipc3270_session::ipc3270_session(const char *name) : pw3270::session()
{
#ifdef HAVE_DBUS

	DBusError	  err;
	int			  rc;
	char		* str = strdup(name);
	char		* ptr;

	for(ptr=str;*ptr;ptr++)
		*ptr = tolower(*ptr);

	ptr = strchr(str,':');

	if(ptr)
	{
		size_t 		  sz;

		*(ptr++) = 0;

		// Build destination
		sz		= strlen(ptr)+strlen(str)+strlen(prefix_dest)+2;
		dest	= (char *) malloc(sz+1);
		strncpy(dest,prefix_dest,sz);
		strncat(dest,str,sz);
		strncat(dest,".",sz);
		strncat(dest,ptr,sz);

		// Build path
		sz		= strlen(str)+strlen(prefix_path);
		path	= (char *) malloc(sz+1);
		strncpy(path,prefix_path,sz);
		strncat(path,str,sz);

		// Build intf
		sz		= strlen(str)+strlen(prefix_dest)+1;
		intf	= (char *) malloc(sz+1);
		strncpy(intf,prefix_dest,sz);
		strncat(intf,str,sz);

	}
	else
	{
		size_t sz;

		// Build destination
		sz		= strlen(str)+strlen(prefix_dest)+2;
		dest	= (char *) malloc(sz+1);
		strncpy(dest,prefix_dest,sz);
		strncat(dest,str,sz);

		// Build path
		sz		= strlen(str)+strlen(prefix_path);
		path	= (char *) malloc(sz+1);
		strncpy(path,prefix_path,sz);
		strncat(path,str,sz);

		// Build intf
		sz		= strlen(str)+strlen(prefix_dest)+1;
		intf	= (char *) malloc(sz+1);
		strncpy(intf,prefix_dest,sz);
		strncat(intf,str,sz);

	}

	trace("DBUS:\nDestination:\t[%s]\nPath:\t\t[%s]\nInterface:\t[%s]",dest,path,intf);

	free(str);

	dbus_error_init(&err);

	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);

	if (dbus_error_is_set(&err))
	{
		log("DBUS Connection Error (%s)", err.message);
		dbus_error_free(&err);
	}

	if(!conn)
	{
		log("%s", "DBUS Connection failed");
		return;
	}

	rc = dbus_bus_request_name(conn, "br.com.bb." PACKAGE_NAME ".oo", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);

	if (dbus_error_is_set(&err))
	{
		log("Name Error (%s)", err.message);
		dbus_error_free(&err);
		conn = NULL;
		return;
	}

	if(rc != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
	{
		log("%s", "DBUS request name failed");
		conn = NULL;
		return;
	}

#else

#endif // HAVE_DBUS
}

pw3270::ipc3270_session::~ipc3270_session()
{
#ifdef HAVE_DBUS

	free(dest);
	free(path);
	free(intf);

#endif // HAVE_DBUS
}

int pw3270::ipc3270_session::get_revision(void)
{
#ifdef HAVE_DBUS

	return -1;

#else

	return -1;

#endif // HAVE_DBUS
}

LIB3270_MESSAGE pw3270::ipc3270_session::get_state(void)
{
#ifdef HAVE_DBUS

	return (LIB3270_MESSAGE) -1;

#else

	return (LIB3270_MESSAGE) -1;

#endif // HAVE_DBUS
}

char * pw3270::ipc3270_session::get_text_at(int row, int col, int len)
{
#ifdef HAVE_DBUS

	return NULL;

#else

	return NULL;

#endif // HAVE_DBUS
}

int pw3270::ipc3270_session::set_text_at(int row, int col, const char *text)
{
#ifdef HAVE_DBUS

	return -1;

#else

	return -1;

#endif // HAVE_DBUS
}

int pw3270::ipc3270_session::cmp_text_at(int row, int col, const char *text)
{
#ifdef HAVE_DBUS

	return -1;

#else

	return -1;

#endif // HAVE_DBUS
}

void pw3270::ipc3270_session::set_toggle(LIB3270_TOGGLE toggle, bool state)
{
}

int pw3270::ipc3270_session::connect(const char *uri)
{
#ifdef HAVE_DBUS

	return -1;

#else

	return -1;

#endif // HAVE_DBUS
}

int pw3270::ipc3270_session::disconnect(void)
{
#ifdef HAVE_DBUS

	return -1;

#else

	return -1;

#endif // HAVE_DBUS
}

bool pw3270::ipc3270_session::connected(void)
{
#ifdef HAVE_DBUS

	return false;

#else

	return false;

#endif // HAVE_DBUS
}

int pw3270::ipc3270_session::enter(void)
{
#ifdef HAVE_DBUS

	return -1;

#else

	return -1;

#endif // HAVE_DBUS
}

int pw3270::ipc3270_session::pfkey(int key)
{
#ifdef HAVE_DBUS

	return -1;

#else

	return -1;

#endif // HAVE_DBUS
}

int	pw3270::ipc3270_session::pakey(int key)
{
#ifdef HAVE_DBUS

	return -1;

#else

	return -1;

#endif // HAVE_DBUS
}

bool pw3270::ipc3270_session::in_tn3270e()
{
#ifdef HAVE_DBUS

	return false;

#else

	return false;

#endif // HAVE_DBUS
}

void pw3270::ipc3270_session::mem_free(void *ptr)
{
#ifdef HAVE_DBUS


#else


#endif // HAVE_DBUS
}


