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
 * Este programa está nomeado como remote.cc e possui - linhas de código.
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

#if defined(WIN32)
	#include <pw3270/ipcpackets.h>
#endif // WIN32

 #include <time.h>
 #include <string.h>
 #include <ctype.h>

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

	char			* get_text(int baddr, size_t len);
	char 			* get_text_at(int row, int col, size_t sz);
	int				  cmp_text_at(int row, int col, const char *text);
	int 			  set_text_at(int row, int col, const char *str);
    int               emulate_input(const char *str);

	int				  wait_for_text_at(int row, int col, const char *key, int timeout);

	int				  set_cursor_position(int row, int col);
	int				  set_cursor_addr(int addr);
	int				  get_cursor_addr(void);

	int 			  set_toggle(LIB3270_TOGGLE ix, bool value);

	int				  enter(void);
	int				  pfkey(int key);
	int				  pakey(int key);

	int               get_field_start(int baddr = -1);
	int               get_field_len(int baddr = -1);
	int               get_next_unprotected(int baddr = -1);

    char            * get_clipboard(void);
    int               set_clipboard(const char *text);

 private:
#if defined(WIN32)

	HANDLE			  hPipe;

#elif defined(HAVE_DBUS)

	DBusConnection	* conn;
	char			* dest;
	char			* path;
	char			* intf;
	DBusMessage		* create_message(const char *method);
	DBusMessage		* call(DBusMessage *msg);
	char 			* query_string(const char *method);
	int 			  query_intval(const char *method);

#endif


 };

/*--[ Globals ]--------------------------------------------------------------------------------------*/

#if defined(HAVE_DBUS)
 static const char * prefix_dest	= "br.com.bb.";
 static const char * prefix_path	= "/br/com/bb/";
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
		log("Error creating message for method %s",method);

	return msg;
}
#endif // HAVE_DBUS

rx3270 * rx3270::create_remote(const char *name)
{
	return new remote(name);
}


#if defined(HAVE_DBUS)
remote::remote(const char *name) : rx3270(REXX_DEFAULT_CHARSET,"UTF-8")
#else
remote::remote(const char *name)
#endif // HAVE_DBUS
{
#if defined(WIN32)
	static DWORD	  dwMode = PIPE_READMODE_MESSAGE;
 	char	 		  buffer[4096];
 	char			* str = strdup(name);
 	char			* ptr;

	hPipe  = INVALID_HANDLE_VALUE;

 	for(ptr=str;*ptr;ptr++)
	{
		if(*ptr == ':')
			*ptr = '_';
		else
			*ptr = tolower(*ptr);
	}

	snprintf(buffer,4095,"\\\\.\\pipe\\%s",str);

	free(str);

	if(!WaitNamedPipe(buffer,NMPWAIT_USE_DEFAULT_WAIT))
	{
		log("%s","Invalid service instance");
		return;
	}

	hPipe = CreateFile(buffer,GENERIC_WRITE|GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);

	if(hPipe == INVALID_HANDLE_VALUE)
	{
		log("%s","Can´t create service pipe");
		return;
	}

	if(!SetNamedPipeHandleState(hPipe,&dwMode,NULL,NULL))
	{
		log("%s","Can´t set pipe state");
		CloseHandle(hPipe);
		hPipe = INVALID_HANDLE_VALUE;
		return;
	}

	// Connected

#elif defined(HAVE_DBUS)
	DBusError	  err;
	int			  rc;
	char		* str = strdup(name);
	char		* ptr;

	trace("%s str=%p",__FUNCTION__,str);

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
	trace("dbus_bus_get conn=%p",conn);

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

	rc = dbus_bus_request_name(conn, "br.com.bb." PACKAGE_NAME ".rexx", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
	trace("dbus_bus_request_name rc=%d",rc);

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

#endif
}

remote::~remote()
{
#if defined(WIN32)

	if(hPipe != INVALID_HANDLE_VALUE)
		CloseHandle(hPipe);

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

	if(!reply)
	{
		log("%s",error.message);
		dbus_error_free(&error);
	}

	return reply;

}

char * get_string(DBusMessage * msg)
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

char * remote::query_string(const char *method)
{
	if(conn)
		return get_string(call(create_message(method)));
	return NULL;
}

int get_intval(DBusMessage * msg)
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

int remote::query_intval(const char *method)
{
	if(conn)
		return get_intval(call(create_message(method)));
	return -1;
}


#endif // HAVE_DBUS

char * remote::get_revision(void)
{
#if defined(WIN32)

	return strdup(PACKAGE_REVISION);

#elif defined(HAVE_DBUS)

	return query_string("getRevision");

#else

	return NULL;

#endif

}


LIB3270_CSTATE remote::get_cstate(void)
{
#if defined(WIN32)

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		static const struct hllapi_packet_query query		= { HLLAPI_PACKET_GET_CSTATE };
		struct hllapi_packet_result		  		response;
		DWORD							  		cbSize		= sizeof(query);
		TransactNamedPipe(hPipe,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
		return (LIB3270_CSTATE) response.rc;
	}

	return (LIB3270_CSTATE) -1;

#elif defined(HAVE_DBUS)

	return (LIB3270_CSTATE) query_intval("getConnectionState");

#else

	return (LIB3270_CSTATE) -1;

#endif

}

int remote::disconnect(void)
{
#if defined(WIN32)

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		static const struct hllapi_packet_query query		= { HLLAPI_PACKET_DISCONNECT };
		struct hllapi_packet_result		  		response;
		DWORD							  		cbSize		= sizeof(query);
		TransactNamedPipe(hPipe,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
		return 0;
	}
	return -1;

#elif defined(HAVE_DBUS)

	return query_intval("disconnect");

#else

	return -1;

#endif

}

int remote::connect(const char *uri, bool wait)
{
#if defined(WIN32)
	if(hPipe != INVALID_HANDLE_VALUE)
	{
		struct hllapi_packet_connect	* pkt;
		struct hllapi_packet_result		  response;
		DWORD							  cbSize;

		cbSize	= sizeof(struct hllapi_packet_connect)+strlen(uri);
		pkt 	= (struct hllapi_packet_connect *) malloc(cbSize);

		pkt->packet_id	= HLLAPI_PACKET_CONNECT;
		pkt->wait		= (unsigned char) wait;
		strcpy(pkt->hostname,uri);

		trace("Sending %s",pkt->hostname);

		if(!TransactNamedPipe(hPipe,(LPVOID) pkt, cbSize, &response, sizeof(response), &cbSize,NULL))
		{
			errno 		= GetLastError();
			response.rc = -1;
		}

		free(pkt);

		return response.rc;

	}

#elif defined(HAVE_DBUS)

	int rc;
	DBusMessage * msg = create_message("connect");
	if(!msg)
		return -1;

	dbus_message_append_args(msg, DBUS_TYPE_STRING, &uri, DBUS_TYPE_INVALID);

	rc = get_intval(call(msg));

	if(!rc && wait)
		return wait_for_ready(120);

	return rc;

#endif

	return -1;
}

bool remote::is_connected(void)
{
#if defined(WIN32)

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		static const struct hllapi_packet_query query		= { HLLAPI_PACKET_IS_CONNECTED };
		struct hllapi_packet_result		  		response;
		DWORD							  		cbSize		= sizeof(query);
		TransactNamedPipe(hPipe,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
		return response.rc != 0;
	}

#elif defined(HAVE_DBUS)

	return query_intval("isConnected") != 0;

#endif

	return false;
}

bool remote::is_ready(void)
{
#if defined(WIN32)

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		static const struct hllapi_packet_query query		= { HLLAPI_PACKET_IS_READY };
		struct hllapi_packet_result		  		response;
		DWORD							  		cbSize		= sizeof(query);
		TransactNamedPipe(hPipe,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
		return response.rc != 0;
	}

#elif defined(HAVE_DBUS)

	return query_intval("isReady") != 0;

#endif

	return false;
}

int remote::iterate(bool wait)
{
#if defined(WIN32)

	return 0;

#elif defined(HAVE_DBUS)

	return 0;

#endif

	return -1;
}

int remote::wait(int seconds)
{
#if defined(WIN32)

 	time_t end = time(0)+seconds;

	while(time(0) < end)
	{
		if(!is_connected())
			return ENOTCONN;
		Sleep(500);
	}

	return 0;

#elif defined(HAVE_DBUS)

 	time_t end = time(0)+seconds;

	while(time(0) < end)
	{
		if(!is_connected())
			return ENOTCONN;
		usleep(500);
	}

	return 0;

#endif

	return -1;
}

int remote::wait_for_ready(int seconds)
{
#if defined(WIN32)

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		time_t end = time(0)+seconds;

		while(time(0) < end)
		{
			if(!is_connected())
				return ENOTCONN;

			if(is_ready())
				return 0;

			Sleep(250);
		}

		return ETIMEDOUT;

	}

	return -1;

#elif defined(HAVE_DBUS)

	time_t end = time(0)+seconds;

	while(time(0) < end)
	{
		static const dbus_int32_t delay = 2;

		DBusMessage		* msg = create_message("waitForReady");
		int				  rc;

		if(!msg)
			return -1;

		dbus_message_append_args(msg, DBUS_TYPE_INT32, &delay, DBUS_TYPE_INVALID);

		rc = get_intval(call(msg));
		trace("waitForReady exits with rc=%d",rc);
		if(rc != ETIMEDOUT)
			return rc;
	}

	return ETIMEDOUT;

#else

	return -1;

#endif

}

char * remote::get_text_at(int row, int col, size_t sz)
{
#if defined(WIN32)

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		struct hllapi_packet_query_at	  query		= { HLLAPI_PACKET_GET_TEXT_AT, (unsigned short) row, (unsigned short) col, (unsigned short) sz };
		struct hllapi_packet_text		* response;
		DWORD							  cbSize	= sizeof(struct hllapi_packet_text)+sz;
		char 							* text		= NULL;

		response = (struct hllapi_packet_text *) malloc(cbSize+2);
		memset(response,0,cbSize+2);

		if(!TransactNamedPipe(hPipe,(LPVOID) &query, sizeof(struct hllapi_packet_query_at), &response, cbSize, &cbSize,NULL))
			return NULL;

		if(response->packet_id)
			errno = response->packet_id;
		else
			text  = strdup(response->text);

		free(response);
		return text;

	}

#elif defined(HAVE_DBUS)

	dbus_int32_t r = (dbus_int32_t) row;
	dbus_int32_t c = (dbus_int32_t) col;
	dbus_int32_t l = (dbus_int32_t) sz;

	DBusMessage * msg = create_message("getTextAt");
	if(!msg)
		return NULL;

	trace("%s(%d,%d,%d)",__FUNCTION__,r,c,l);
	dbus_message_append_args(msg, DBUS_TYPE_INT32, &r, DBUS_TYPE_INT32, &c, DBUS_TYPE_INT32, &l, DBUS_TYPE_INVALID);

	return get_string(call(msg));

#endif

	return NULL;
}

int remote::cmp_text_at(int row, int col, const char *text)
{
#if defined(WIN32)

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		struct hllapi_packet_text_at 	* query;
		struct hllapi_packet_result	  	  response;
		DWORD							  cbSize		= sizeof(struct hllapi_packet_text_at)+strlen(text);

		query = (struct hllapi_packet_text_at *) malloc(cbSize);
		query->packet_id 	= HLLAPI_PACKET_CMP_TEXT_AT;
		query->row			= row;
		query->col			= col;
		strcpy(query->text,text);

		TransactNamedPipe(hPipe,(LPVOID) query, cbSize, &response, sizeof(response), &cbSize,NULL);

		free(query);

		return response.rc;
	}


#elif defined(HAVE_DBUS)

	dbus_int32_t r = (dbus_int32_t) row;
	dbus_int32_t c = (dbus_int32_t) col;

	DBusMessage * msg = create_message("cmpTextAt");
	if(msg)
	{
		dbus_message_append_args(msg, DBUS_TYPE_INT32, &r, DBUS_TYPE_INT32, &c, DBUS_TYPE_STRING, &text, DBUS_TYPE_INVALID);
		return get_intval(call(msg));
	}

#endif

	return 0;
}

int remote::set_text_at(int row, int col, const char *str)
{
#if defined(WIN32)

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		struct hllapi_packet_text_at 	* query;
		struct hllapi_packet_result	  	  response;
		DWORD							  cbSize		= sizeof(struct hllapi_packet_text_at)+strlen((const char *) str);

		query = (struct hllapi_packet_text_at *) malloc(cbSize);
		query->packet_id 	= HLLAPI_PACKET_SET_TEXT_AT;
		query->row			= row;
		query->col			= col;
		strcpy(query->text,(const char *) str);

		TransactNamedPipe(hPipe,(LPVOID) query, cbSize, &response, sizeof(response), &cbSize,NULL);

		free(query);

		return response.rc;
	}

#elif defined(HAVE_DBUS)

	dbus_int32_t r = (dbus_int32_t) row;
	dbus_int32_t c = (dbus_int32_t) col;

	DBusMessage * msg = create_message("setTextAt");
	if(msg)
	{
		dbus_message_append_args(msg, DBUS_TYPE_INT32, &r, DBUS_TYPE_INT32, &c, DBUS_TYPE_STRING, &str, DBUS_TYPE_INVALID);
		return get_intval(call(msg));
	}

#endif

	return -1;
}

int remote::emulate_input(const char *str)
{
#if defined(WIN32)

	if(hPipe != INVALID_HANDLE_VALUE)
	{
        size_t                                len           = strlen(str);
		struct hllapi_packet_emulate_input 	* query;
		struct hllapi_packet_result           response;
		DWORD							      cbSize		= sizeof(struct hllapi_packet_emulate_input)+len;

		query = (struct hllapi_packet_emulate_input *) malloc(cbSize);
        query->packet_id 	= HLLAPI_PACKET_EMULATE_INPUT;
        query->len			= len;
        query->pasting		= 1;
        strcpy(query->text,str);

		TransactNamedPipe(hPipe,(LPVOID) query, cbSize, &response, sizeof(response), &cbSize,NULL);

		free(query);

		return response.rc;
	}

#elif defined(HAVE_DBUS)

	DBusMessage * msg = create_message("input");
	if(msg)
	{
		dbus_message_append_args(msg, DBUS_TYPE_STRING, &str, DBUS_TYPE_INVALID);
		return get_intval(call(msg));
	}

#endif

	return -1;
}

int remote::set_cursor_position(int row, int col)
{
#if defined(WIN32)

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		struct hllapi_packet_cursor 	query		= { HLLAPI_PACKET_SET_CURSOR_POSITION, (unsigned short) row, (unsigned short) col };
		struct hllapi_packet_result		response;
		DWORD							cbSize		= sizeof(query);
		TransactNamedPipe(hPipe,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
		return response.rc != 0;
	}

#elif defined(HAVE_DBUS)

	dbus_int32_t r = (dbus_int32_t) row;
	dbus_int32_t c = (dbus_int32_t) col;

	DBusMessage * msg = create_message("setCursorAt");
	if(msg)
	{
		dbus_message_append_args(msg, DBUS_TYPE_INT32, &r, DBUS_TYPE_INT32, &c, DBUS_TYPE_INVALID);
		return get_intval(call(msg));
	}

#endif

	return -1;
}

int remote::enter(void)
{
#if defined(WIN32)

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		static const struct hllapi_packet_query query		= { HLLAPI_PACKET_ENTER };
		struct hllapi_packet_result		  		response;
		DWORD							  		cbSize		= sizeof(query);
		TransactNamedPipe(hPipe,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
		return response.rc;
	}

	return -1;

#elif defined(HAVE_DBUS)

	return query_intval("enter");

#else

	return -1;

#endif

}

int remote::pfkey(int key)
{
#if defined(WIN32)

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		struct hllapi_packet_keycode	query		= { HLLAPI_PACKET_PFKEY, (unsigned short) key };
		struct hllapi_packet_result		response;
		DWORD							cbSize		= sizeof(query);
		TransactNamedPipe(hPipe,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
		return response.rc;
	}

#elif defined(HAVE_DBUS)

	dbus_int32_t k = (dbus_int32_t) key;

	DBusMessage * msg = create_message("pfKey");
	if(msg)
	{
		dbus_message_append_args(msg, DBUS_TYPE_INT32, &k, DBUS_TYPE_INVALID);
		return get_intval(call(msg));
	}

#endif

	return -1;
}

int remote::pakey(int key)
{
#if defined(WIN32)

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		struct hllapi_packet_keycode	query		= { HLLAPI_PACKET_PAKEY, (unsigned short) key };
		struct hllapi_packet_result		response;
		DWORD							cbSize		= sizeof(query);
		TransactNamedPipe(hPipe,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
		return response.rc;
	}

#elif defined(HAVE_DBUS)

	dbus_int32_t k = (dbus_int32_t) key;

	DBusMessage * msg = create_message("paKey");
	if(msg)
	{
		dbus_message_append_args(msg, DBUS_TYPE_INT32, &k, DBUS_TYPE_INVALID);
		return get_intval(call(msg));
	}

#endif

	return -1;
}

int remote::get_field_start(int baddr)
{
#if defined(WIN32)

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		struct hllapi_packet_addr       query		= { HLLAPI_PACKET_FIELD_START, (unsigned short) baddr };
		struct hllapi_packet_result		response;
		DWORD							cbSize		= sizeof(query);
		TransactNamedPipe(hPipe,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
		return response.rc;
	}

#elif defined(HAVE_DBUS)

	dbus_int32_t k = (dbus_int32_t) baddr;

	DBusMessage * msg = create_message("getFieldStart");
	if(msg)
	{
		dbus_message_append_args(msg, DBUS_TYPE_INT32, &k, DBUS_TYPE_INVALID);
		return get_intval(call(msg));
	}

#endif

	return -1;
}

int remote::get_field_len(int baddr)
{
#if defined(WIN32)

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		struct hllapi_packet_addr       query		= { HLLAPI_PACKET_FIELD_LEN, (unsigned short) baddr };
		struct hllapi_packet_result		response;
		DWORD							cbSize		= sizeof(query);
		TransactNamedPipe(hPipe,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
		return response.rc;
	}

#elif defined(HAVE_DBUS)

	dbus_int32_t k = (dbus_int32_t) baddr;

	DBusMessage * msg = create_message("getFieldLength");
	if(msg)
	{
		dbus_message_append_args(msg, DBUS_TYPE_INT32, &k, DBUS_TYPE_INVALID);
		return get_intval(call(msg));
	}

#endif

	return -1;
}



int remote::set_toggle(LIB3270_TOGGLE ix, bool value)
{
#if defined(WIN32)

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		struct hllapi_packet_set		query		= { HLLAPI_PACKET_SET_TOGGLE, (unsigned short) ix, (unsigned short) value };
		struct hllapi_packet_result		response;
		DWORD							cbSize		= sizeof(query);
		TransactNamedPipe(hPipe,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
		return response.rc;
	}

#elif defined(HAVE_DBUS)

	dbus_int32_t i = (dbus_int32_t) ix;
	dbus_int32_t v = (dbus_int32_t) value;

	DBusMessage * msg = create_message("setToggle");
	if(msg)
	{
		dbus_message_append_args(msg, DBUS_TYPE_INT32, &i, DBUS_TYPE_INT32, &v, DBUS_TYPE_INVALID);
		return get_intval(call(msg));
	}

#endif

	return -1;
}

int remote::wait_for_text_at(int row, int col, const char *key, int timeout)
{
	time_t end = time(0)+timeout;

	while(time(0) < end)
	{
		if(!is_connected())
			return ENOTCONN;

		if(!cmp_text_at(row,col,key))
			return 0;

#ifdef WIN32
		Sleep(500);
#else
		usleep(500);
#endif
	}

	return ETIMEDOUT;
}

char * remote::get_text(int baddr, size_t len)
{
	#warning IMPLEMENTAR
	return NULL;
}

int remote::set_cursor_addr(int addr)
{
#if defined(WIN32)

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		struct hllapi_packet_addr       query		= { HLLAPI_PACKET_FIELD_LEN, (unsigned short) addr };
		struct hllapi_packet_result		response;
		DWORD							cbSize		= sizeof(query);
		TransactNamedPipe(hPipe,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
		return response.rc;
	}

#elif defined(HAVE_DBUS)

	dbus_int32_t k = (dbus_int32_t) addr;

	DBusMessage * msg = create_message("setCursorAddress");
	if(msg)
	{
		dbus_message_append_args(msg, DBUS_TYPE_INT32, &k, DBUS_TYPE_INVALID);
		return get_intval(call(msg));
	}

#endif

    return -1;
}

int remote::get_cursor_addr(void)
{
#if defined(WIN32)

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		struct hllapi_packet_query      query		= { HLLAPI_PACKET_GET_CURSOR };
		struct hllapi_packet_result		response;
		DWORD							cbSize		= sizeof(query);
		TransactNamedPipe(hPipe,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
		return response.rc;
	}

#elif defined(HAVE_DBUS)

	return query_intval("getCursorAddress");

#else

    return -1;

#endif
}

int remote::get_next_unprotected(int baddr)
{
#if defined(WIN32)

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		struct hllapi_packet_addr       query		= { HLLAPI_PACKET_NEXT_UNPROTECTED, (unsigned short) baddr };
		struct hllapi_packet_result		response;
		DWORD							cbSize		= sizeof(query);
		TransactNamedPipe(hPipe,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
		return response.rc;
	}

#elif defined(HAVE_DBUS)

	dbus_int32_t k = (dbus_int32_t) baddr;

	DBusMessage * msg = create_message("getNextUnprotected");
	if(msg)
	{
		dbus_message_append_args(msg, DBUS_TYPE_INT32, &k, DBUS_TYPE_INVALID);
		return get_intval(call(msg));
	}

#endif

    return -1;
}

int remote::set_clipboard(const char *text)
{
	return -1;
}

char * remote::get_clipboard(void)
{
#if defined(WIN32)

	return NULL;

#elif defined(HAVE_DBUS)

	DBusMessage * msg = create_message("getClipboard");
	if(!msg)
		return NULL;

	return get_string(call(msg));

#endif

	return NULL;
}
