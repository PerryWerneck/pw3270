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
 * Este programa está nomeado como local.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "rx3270.h"

#if defined(HAVE_DBUS)
	#include <stdio.h>
	#include <dbus/dbus.h>
#endif // HAVE_DBUS

 #include <string.h>

/*--[ Class definition ]-----------------------------------------------------------------------------*/

 class remote : public rx3270
 {
 public:
	remote(const char *session);
	~remote();

	char			* get_revision(void);
	LIB3270_CSTATE	  get_cstate(void);
	int				  disconnect(void);
	int				  connect(const char *uri, bool wait = true);
	bool			  is_connected(void);
	bool			  is_ready(void);

	int				  iterate(bool wait);
	int				  wait(int seconds);
	int				  wait_for_ready(int seconds);

	char 			* get_text_at(int row, int col, size_t sz);
	int				  cmp_text_at(int row, int col, const char *text);
	int 			  set_text_at(int row, int col, const char *str);

	int				  set_cursor_position(int row, int col);

	void 			  set_toggle(LIB3270_TOGGLE ix, bool value);

	int				  enter(void);
	int				  pfkey(int key);
	int				  pakey(int key);

 private:
#if defined(WIN32)

#elif defined(HAVE_DBUS)
	DBusConnection	* conn;
	char			* dest;
	char			* path;
	char			* intf;
	DBusMessage		* create_message(const char *method);
	DBusMessage		* call(DBusMessage *msg);
	char 			* query_string(const char *method);
#endif


 };

/*--[ Globals ]--------------------------------------------------------------------------------------*/

#if defined(HAVE_DBUS)
 static const char * prefix_dest	= "br.com.bb.";
 static const char * prefix_path	= "/br/com/bb/";
#else
 #error AQUI
#endif // HAVE_DBUS

/*--[ Implement ]------------------------------------------------------------------------------------*/

#if defined(HAVE_DBUS)
DBusMessage * remote::create_message(const char *method)
{
	DBusMessage * msg = dbus_message_new_method_call(	this->dest,		// Destination
														this->path,		// Path
														this->intf,		// Interface
														method);		// method

	if (!msg)
		fprintf(stderr, "Error creating message for method %s\n",method);

	return msg;
}
#endif // HAVE_DBUS


rx3270 * rx3270::create_remote(const char *name)
{
	return new remote(name);
}


remote::remote(const char *name)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)
	DBusError	  err;
	int			  rc;
	char		* str = strdup(name);
	char		* ptr = strchr(str,':');

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
		exit(-1);

	}

	trace("DBUS:\nDestination:\t[%s]\nPath:\t\t[%s]\nInterface:\t[%s]",dest,path,intf);

	free(str);

	dbus_error_init(&err);

	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);

	if (dbus_error_is_set(&err))
	{
		fprintf(stderr, "DBUS Connection Error (%s)\n", err.message);
		dbus_error_free(&err);
	}

	if(!conn)
	{
		fprintf(stderr, "%s\n", "DBUS Connection failed");
		dbus_connection_close(conn);
		conn = NULL;
		return;
	}

	rc = dbus_bus_request_name(conn, "br.com.bb." PACKAGE_NAME ".rexx", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);

	if (dbus_error_is_set(&err))
	{
		fprintf(stderr, "Name Error (%s)\n", err.message);
		dbus_error_free(&err);
	}

	if(rc != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
	{
		fprintf(stderr, "%s\n", "DBUS request name failed");
		dbus_connection_close(conn);
		conn = NULL;
		return;
	}


#else

#endif
}

remote::~remote()
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

	free(dest);
	free(path);
	free(intf);

#else

#endif
}

#if defined(HAVE_DBUS)
DBusMessage	* remote::call(DBusMessage *msg)
{
	DBusMessage		* reply;
	DBusError		  error;

	dbus_error_init(&error);
	reply = dbus_connection_send_with_reply_and_block(conn,msg,10000,&error);
	dbus_message_unref(msg);

	if(reply)
		return reply;

	fprintf(stderr,"%s\n",error.message);
	dbus_error_free(&error);

	return NULL;

}

char * remote::query_string(const char *method)
{
	char *rc = NULL;

	if(conn)
	{
		DBusMessage	* msg = call(create_message(method));
		if(msg)
		{
			DBusMessageIter iter;

			if(dbus_message_iter_init(msg, &iter))
			{
				if(dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_STRING)
				{
					const char * str;
					dbus_message_iter_get_basic(&iter, &str);
					rc = strdup(str);
				}
			}

			dbus_message_unref(msg);
		}
	}

	return rc;
}

#endif // HAVE_DBUS

char * remote::get_revision(void)
{
#if defined(WIN32)

	return NULL;


#elif defined(HAVE_DBUS)

	return query_string("getRevision");

#else

	return NULL;

#endif

}


LIB3270_CSTATE remote::get_cstate(void)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return (LIB3270_CSTATE) -1;

}

int remote::disconnect(void)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return -1;
}

int remote::connect(const char *uri, bool wait)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return -1;
}

bool remote::is_connected(void)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return false;
}

bool remote::is_ready(void)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return false;
}

int remote::iterate(bool wait)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

	if(wait)
		this->wait(1);

#endif

	return -1;
}

int remote::wait(int seconds)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

	sleep(seconds);

#endif

	return -1;
}

int remote::wait_for_ready(int seconds)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return -1;
}

char * remote::get_text_at(int row, int col, size_t sz)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return NULL;
}

int remote::cmp_text_at(int row, int col, const char *text)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return 0;
}

int remote::set_text_at(int row, int col, const char *str)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return -1;
}

int remote::set_cursor_position(int row, int col)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return -1;
}

int remote::enter(void)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return -1;
}

int remote::pfkey(int key)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return -1;
}

int remote::pakey(int key)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return -1;
}

void remote::set_toggle(LIB3270_TOGGLE ix, bool value)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif
}
