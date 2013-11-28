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
 * Este programa está nomeado como remote.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <lib3270/config.h>

 #if defined(HAVE_DBUS)
	#include <stdio.h>
	#include <dbus/dbus.h>
	#include <string.h>
	#include <malloc.h>
	#include <sys/types.h>
	#include <unistd.h>

	#ifndef DBUS_TIMEOUT_INFINITE
		#define DBUS_TIMEOUT_INFINITE ((int) 0x7fffffff)
	#endif // !DBUS_TIMEOUT_INFINITE

 #endif // HAVE_DBUS

 #if defined(WIN32)
	#include <windows.h>
	#include <pw3270/ipcpackets.h>
	#include <process.h>
 #else
	#define HLLAPI_PACKET_IS_CONNECTED	"isConnected"
	#define HLLAPI_PACKET_GET_CSTATE	"getConnectionState"
	#define HLLAPI_PACKET_IS_READY		"isReady"
	#define HLLAPI_PACKET_DISCONNECT	"disconnect"
	#define HLLAPI_PACKET_GET_CURSOR	"getCursorAddress"
	#define HLLAPI_PACKET_ENTER			"enter"
	#define HLLAPI_PACKET_QUIT			"quit"
	#define HLLAPI_PACKET_ERASE_EOF		"eraseEOF"
	#define HLLAPI_PACKET_PRINT			"print"
 #endif // WIN32

 #include <pw3270/class.h>
 #include <lib3270/log.h>

#if defined(HAVE_DBUS)
 static const char	* prefix_dest	= "br.com.bb.";
 static const char	* prefix_path	= "/br/com/bb/";
#endif // HAVE_DBUS

/*--[ Implement ]--------------------------------------------------------------------------------------------------*/

 namespace PW3270_NAMESPACE
 {

	class remote : public session
 	{
	private:

#if defined(WIN32)

		HANDLE			  hPipe;

		int query_intval(HLLAPI_PACKET id)
		{
			struct hllapi_packet_query      query		= { id };
			struct hllapi_packet_result		response;
			DWORD							cbSize		= sizeof(query);
			if(TransactNamedPipe(hPipe,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL))
				return response.rc;

			throw exception(GetLastError(),"%s","Transaction error");
		}

		string * query_string(void *query, size_t szQuery, size_t len)
		{
			struct hllapi_packet_text			* response;
			DWORD								  cbSize	= sizeof(struct hllapi_packet_text)+len;
			string								* s;

			response = (struct hllapi_packet_text *) malloc(cbSize+2);
			memset(response,0,cbSize+2);

			if(TransactNamedPipe(hPipe,(LPVOID) query, szQuery, &response, cbSize, &cbSize,NULL))
			{
				if(response->packet_id)
					s = new string("");
				else
					s = new string(response->text);
			}
			else
			{
				s = new string("");
			}

			free(response);

			return s;
		}

		int query_intval(void *pkt, size_t szQuery, bool dynamic = false)
		{
			struct hllapi_packet_result		response;
			DWORD							cbSize		= (DWORD) szQuery;
			BOOL 							status;

			status = TransactNamedPipe(hPipe,(LPVOID) pkt, cbSize, &response, sizeof(response), &cbSize,NULL);

			if(dynamic)
				free(pkt);

			if(status)
				return response.rc;

			throw exception(GetLastError(),"%s","Transaction error");

		}


#elif defined(HAVE_DBUS)

		DBusConnection	* conn;
		char			* dest;
		char			* path;
		char			* intf;

		DBusMessage * create_message(const char *method)
		{
			DBusMessage * msg = dbus_message_new_method_call(	this->dest,		// Destination
																this->path,		// Path
																this->intf,		// Interface
																method);		// method

			if (!msg)
				throw exception("Error creating DBUS message for method %s",method);

			return msg;

		}

		DBusMessage	* call(DBusMessage *msg)
		{
			DBusMessage		* reply;
			DBusError		  error;

			dbus_error_init(&error);
			reply = dbus_connection_send_with_reply_and_block(conn,msg,10000,&error);
			dbus_message_unref(msg);

			if(!reply)
			{
				exception e = exception("%s",error.message);
				dbus_error_free(&error);
				throw e;
			}
			return reply;
		}

		string * get_string(DBusMessage * msg)
		{
			if(msg)
			{
				DBusMessageIter iter;

				if(dbus_message_iter_init(msg, &iter))
				{
					if(dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_STRING)
					{
						string 		* rc;
						const char	* str;
						dbus_message_iter_get_basic(&iter, &str);
						trace("Response: [%s]",str);
						rc = new string(str);
						dbus_message_unref(msg);
						return rc;
					}

					exception e = exception("DBUS Return type was %c, expecting %c",dbus_message_iter_get_arg_type(&iter),DBUS_TYPE_INT32);
					dbus_message_unref(msg);

					throw e;

				}

			}

			return NULL;
		}

		string * query_string(const char *method)
		{
			return get_string(call(create_message(method)));
		}

		int get_intval(DBusMessage * msg)
		{
			if(msg)
			{
				DBusMessageIter iter;

				if(dbus_message_iter_init(msg, &iter))
				{
					if(dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_INT32)
					{
						dbus_int32_t iSigned;
						dbus_message_iter_get_basic(&iter, &iSigned);
						dbus_message_unref(msg);
						return (int) iSigned;
					}

					exception e = exception("DBUS Return type was %c, expecting %c",dbus_message_iter_get_arg_type(&iter),DBUS_TYPE_INT32);

					dbus_message_unref(msg);
					throw e;
					return -1;
				}
				dbus_message_unref(msg);
			}
			return -1;
		}

		int query_intval(const char *method)
		{
			return get_intval(call(create_message(method)));
		}

		int query_intval(const char *method, int first_arg_type, ...)
		{
			va_list 	  var_args;
			DBusMessage * msg = dbus_message_new_method_call(	this->dest,		// Destination
																this->path,		// Path
																this->intf,		// Interface
																method);		// method

			if (!msg)
			{
				throw exception("Error creating DBUS message for method %s",method);
				return -1;
			}

			va_start(var_args, first_arg_type);
			dbus_message_append_args_valist(msg,first_arg_type,var_args);
			va_end(var_args);

			return get_intval(call(msg));
		}

#else


		int query_intval(const char *method)
		{
			throw exception("Call to unimplemented RPC method \"%s\"",method);
			return -1;
		}

#endif

	public:

		remote(const char *session)
		{
#if defined(WIN32)
			static DWORD			  dwMode = PIPE_READMODE_MESSAGE;
			char	 				  buffer[4096];
			char					* str;
			char					* ptr;
			time_t					  timer;
			WIN32_FIND_DATA			  FindFileData;


			hPipe  = INVALID_HANDLE_VALUE;

			if(strcasecmp(session,"start") == 0 || strcasecmp(session,"new") == 0)
			{
				// Start a new session
				char 					buffer[80];
				char 					appName[4096];
				HKEY 					hKey	= 0;
				unsigned long			datalen = 4096;
				STARTUPINFO				si;
				PROCESS_INFORMATION		pi;

				// Get application path
				*appName = 0;
				if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Software\\pw3270",0,KEY_QUERY_VALUE,&hKey) != ERROR_SUCCESS)
				{
					throw exception("Can't open key %s","HKLM\\Software\\pw3270");
					return;
				}
				else
				{
					unsigned long datatype;					// #defined in winnt.h (predefined types 0-11)
					if(RegQueryValueExA(hKey,"appName",NULL,&datatype,(LPBYTE) appName,&datalen) != ERROR_SUCCESS)
						*appName = 0;
					RegCloseKey(hKey);
				}

				if(!*appName)
				{
					throw exception("key %s\\appName is invalid","HKLM\\Software\\pw3270");
					return;
				}

				trace("%s appname=%s\n",__FUNCTION__,appName);

				snprintf(buffer,79,"%s --session=\"H%06d\"",appName,getpid());

				ZeroMemory( &si, sizeof(si) );
				si.cb = sizeof(si);
				ZeroMemory( &pi, sizeof(pi) );

				// si.dwFlags = STARTF_PREVENTPINNING;
				trace("App: %s",appName);
				trace("CmdLine: %s",buffer);

				if(CreateProcess(NULL,buffer,NULL,NULL,0,NORMAL_PRIORITY_CLASS,NULL,NULL,&si,&pi))
				{
					CloseHandle( pi.hProcess );
					CloseHandle( pi.hThread );
				}
				else
				{
					throw exception("Can't start %s",appName);
					return;
				}

				snprintf(buffer,4095,"H%06d_a",getpid());
				str = strdup(buffer);

			}
			else
			{
				// Use an existing session
				str = strdup(session);

				// Convert session name
				for(ptr=str;*ptr;ptr++)
				{
					if(*ptr == ':')
						*ptr = '_';
					else
						*ptr = tolower(*ptr);
				}
			}

			snprintf(buffer,4095,"\\\\.\\pipe\\%s",str);

			free(str);

			timer = time(0)+20;
			while(hPipe == INVALID_HANDLE_VALUE && time(0) < timer)
			{
				hPipe = FindFirstFile(buffer, &FindFileData);
				Sleep(10);
			}

			if(hPipe != INVALID_HANDLE_VALUE)
			{
				CloseHandle(hPipe);
				hPipe = CreateFile(buffer,GENERIC_WRITE|GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);
			}
			else
			{
				throw exception(GetLastError(),"Timeout waiting for %s instance",PACKAGE_NAME);
				return;
			}

			if(hPipe == INVALID_HANDLE_VALUE)
			{
				throw exception("Can´t create service pipe %s",buffer);
			}
			else if(!SetNamedPipeHandleState(hPipe,&dwMode,NULL,NULL))
			{
				exception e = exception(GetLastError(),"%s","Can´t set pipe state");
				CloseHandle(hPipe);
				hPipe = INVALID_HANDLE_VALUE;
				throw e;
			}

#elif defined(HAVE_DBUS)

			DBusError	  err;
			int			  rc;
			char		* str = strdup(session);
			char		* ptr;
			char		  busname[4096];
			char		  pidname[10];
			int			  pid			= (int) getpid();

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
				exception e = exception("DBUS Connection Error (%s)", err.message);
				dbus_error_free(&err);
				throw e;
				return;
			}

			if(!conn)
			{
				throw exception("%s", "DBUS Connection failed");
				return;
			}

			memset(pidname,0,10);
			for(int f = 0; f < 9 && pid > 0;f++)
			{
				pidname[f] = 'a'+(pid % 25);
				pid /= 25;
			}

			snprintf(busname, 4095, "%s.rx3270.br.com.bb",pidname);

			trace("Busname: [%s]",busname);

			rc = dbus_bus_request_name(conn, busname, DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
			trace("dbus_bus_request_name rc=%d",rc);

			if (dbus_error_is_set(&err))
			{
				exception e = exception("Name Error (%s)", err.message);
				dbus_error_free(&err);
				throw e;
				return;
			}

			if(rc != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
			{
				trace("%s: DBUS request for name %s failed",__FUNCTION__, busname);
				throw exception("DBUS request for \"%s\" failed",session);
				return;
			}

			trace("%s: Using DBUS name %s",__FUNCTION__,busname);

			const char * id   = "r";
			static const dbus_int32_t flag = 1;
			query_intval("setScript", DBUS_TYPE_STRING, &id, DBUS_TYPE_INT32, &flag, DBUS_TYPE_INVALID);


#else

			throw exception("%s","RPC support is incomplete.");

#endif
		}

		virtual ~remote()
		{
#if defined(WIN32)

			if(hPipe != INVALID_HANDLE_VALUE)
				CloseHandle(hPipe);

#elif defined(HAVE_DBUS)

			try
			{
				const char * id   = "r";
				static const dbus_int32_t flag = 0;
				query_intval("setScript", DBUS_TYPE_STRING, &id, DBUS_TYPE_INT32, &flag, DBUS_TYPE_INVALID);
			}
			catch(exception e)
			{

			}

			free(dest);
			free(path);
			free(intf);

#else

#endif
		}

		bool is_connected(void)
		{
			return query_intval(HLLAPI_PACKET_IS_CONNECTED) != 0;
		}

		LIB3270_CSTATE get_cstate(void)
		{
			return (LIB3270_CSTATE) query_intval(HLLAPI_PACKET_GET_CSTATE);
		}

		int connect(const char *uri, bool wait)
		{
#if defined(WIN32)

			size_t							  cbSize	= sizeof(struct hllapi_packet_connect)+strlen(uri);
			struct hllapi_packet_connect	* pkt		= (struct hllapi_packet_connect *) malloc(cbSize);

			pkt->packet_id	= HLLAPI_PACKET_CONNECT;
			pkt->wait		= (unsigned char) wait;
			strcpy(pkt->hostname,uri);

			trace("Sending %s",pkt->hostname);

			return query_intval((void *) pkt,cbSize,true);

#elif defined(HAVE_DBUS)

			int rc = query_intval("connect", DBUS_TYPE_STRING, &uri, DBUS_TYPE_INVALID);

			if(!rc && wait)
				return wait_for_ready(120);

			return rc;

#else
			return -1;

#endif
		}

		int wait_for_ready(int seconds)
		{
#if defined(WIN32)

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

#elif defined(HAVE_DBUS)

			time_t end = time(0)+seconds;

			while(time(0) < end)
			{
				static const dbus_int32_t delay = 2;

				int rc = query_intval("waitForReady", DBUS_TYPE_INT32, &delay, DBUS_TYPE_INVALID);

				trace("waitForReady exits with rc=%d",rc);

				if(rc != ETIMEDOUT)
					return rc;
			}

			return ETIMEDOUT;

#else

			return -1;

#endif

		}

		bool is_ready(void)
		{
			return query_intval(HLLAPI_PACKET_IS_READY) != 0;
		}


		int disconnect(void)
		{
			return query_intval(HLLAPI_PACKET_DISCONNECT);
		}


		int wait(int seconds)
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

#else

			return -1;

#endif

		}

		int iterate(bool wait)
		{
#if defined(WIN32)
			return 0;
#elif defined(HAVE_DBUS)
			return 0;
#else
			return -1;
#endif
		}

		string * get_text_at(int row, int col, size_t sz)
		{
#if defined(WIN32)

			struct hllapi_packet_query_at query	= { HLLAPI_PACKET_GET_TEXT_AT, (unsigned short) row, (unsigned short) col, (unsigned short) sz };

			return query_string(&query,sizeof(query),sz);

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

#else

			return NULL;

#endif

		}

		int set_text_at(int row, int col, const char *str)
		{
#if defined(WIN32)

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

#elif defined(HAVE_DBUS)

			dbus_int32_t r = (dbus_int32_t) row;
			dbus_int32_t c = (dbus_int32_t) col;

			return query_intval("setTextAt", DBUS_TYPE_INT32, &r, DBUS_TYPE_INT32, &c, DBUS_TYPE_STRING, &str, DBUS_TYPE_INVALID);

#else

			return -1;

#endif

		}

		int cmp_text_at(int row, int col, const char *text)
		{
#if defined(WIN32)

			struct hllapi_packet_text_at 	* query;
			size_t							  cbSize		= sizeof(struct hllapi_packet_text_at)+strlen(text);

			query = (struct hllapi_packet_text_at *) malloc(cbSize);
			query->packet_id 	= HLLAPI_PACKET_CMP_TEXT_AT;
			query->row			= row;
			query->col			= col;
			strcpy(query->text,text);

			return query_intval((void *) query, cbSize, true);

#elif defined(HAVE_DBUS)

			dbus_int32_t r = (dbus_int32_t) row;
			dbus_int32_t c = (dbus_int32_t) col;

			return query_intval("cmpTextAt", DBUS_TYPE_INT32, &r, DBUS_TYPE_INT32, &c, DBUS_TYPE_STRING, &text, DBUS_TYPE_INVALID);

#endif

			return 0;
		}

		int wait_for_text_at(int row, int col, const char *key, int timeout)
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

		string * get_text(int baddr, size_t len)
		{
#if defined(WIN32)
			struct hllapi_packet_query_offset query = { HLLAPI_PACKET_GET_TEXT_AT_OFFSET, (unsigned short) baddr, (unsigned short) len };
			return query_string(&query,sizeof(query),len);

#elif defined(HAVE_DBUS)

			dbus_int32_t b = (dbus_int32_t) baddr;
			dbus_int32_t l = (dbus_int32_t) len;

			DBusMessage * msg = create_message("getText");
			if(!msg)
				return NULL;

			trace("%s(%d,%d)",__FUNCTION__,b,l);
			dbus_message_append_args(msg, DBUS_TYPE_INT32, &b, DBUS_TYPE_INT32, &l, DBUS_TYPE_INVALID);

			return get_string(call(msg));
#else
			throw exception("%s","IPC support is unavailable");
			return NULL;
#endif
		}


		int set_cursor_position(int row, int col)
		{
#if defined(WIN32)

			struct hllapi_packet_cursor query = { HLLAPI_PACKET_SET_CURSOR_POSITION, (unsigned short) row, (unsigned short) col };

			return query_intval((void *) &query, sizeof(query));

#elif defined(HAVE_DBUS)

			dbus_int32_t r = (dbus_int32_t) row;
			dbus_int32_t c = (dbus_int32_t) col;

			return query_intval("setCursorAt", DBUS_TYPE_INT32, &r, DBUS_TYPE_INT32, &c, DBUS_TYPE_INVALID);

#endif

			return -1;
		}

		int set_cursor_addr(int addr)
		{
#if defined(WIN32)

			struct hllapi_packet_addr query = { HLLAPI_PACKET_SET_CURSOR, (unsigned short) addr };

			return query_intval((void *) &query, sizeof(query));

#elif defined(HAVE_DBUS)

			dbus_int32_t k = (dbus_int32_t) addr;

			return query_intval("setCursorAddress", DBUS_TYPE_INT32, &k, DBUS_TYPE_INVALID);

#endif

			return -1;
		}

		int get_cursor_addr(void)
		{
			return query_intval(HLLAPI_PACKET_GET_CURSOR);
		}


		int enter(void)
		{
			return query_intval(HLLAPI_PACKET_ENTER);
		}

		int pfkey(int key)
		{
#if defined(WIN32)

			struct hllapi_packet_keycode query = { HLLAPI_PACKET_PFKEY, (unsigned short) key };

			return query_intval((void *) &query, sizeof(query));

#elif defined(HAVE_DBUS)

			dbus_int32_t k = (dbus_int32_t) key;

			return query_intval("pfKey", DBUS_TYPE_INT32, &k, DBUS_TYPE_INVALID);

#else

			return -1;

#endif

		}

		int pakey(int key)
		{
#if defined(WIN32)

			struct hllapi_packet_keycode query = { HLLAPI_PACKET_PAKEY, (unsigned short) key };

			return query_intval((void *) &query, sizeof(query));

#elif defined(HAVE_DBUS)

			dbus_int32_t k = (dbus_int32_t) key;

			return query_intval("paKey", DBUS_TYPE_INT32, &k, DBUS_TYPE_INVALID);

#else

			return -1;

#endif

		}

		int quit(void)
		{
			return query_intval(HLLAPI_PACKET_QUIT);
		}

		int set_toggle(LIB3270_TOGGLE ix, bool value)
		{
#if defined(WIN32)

			struct hllapi_packet_set query = { HLLAPI_PACKET_SET_TOGGLE, (unsigned short) ix, (unsigned short) value };

			return query_intval((void *) &query, sizeof(query));

#elif defined(HAVE_DBUS)

			dbus_int32_t i = (dbus_int32_t) ix;
			dbus_int32_t v = (dbus_int32_t) value;

			return query_intval("setToggle", DBUS_TYPE_INT32, &i, DBUS_TYPE_INT32, &v, DBUS_TYPE_INVALID);

#else
			return -1;

#endif

		}

		int emulate_input(const char *str)
		{
#if defined(WIN32)

			size_t                                len           = strlen(str);
			struct hllapi_packet_emulate_input 	* query;
			size_t							      cbSize		= sizeof(struct hllapi_packet_emulate_input)+len;

			query = (struct hllapi_packet_emulate_input *) malloc(cbSize);
			query->packet_id 	= HLLAPI_PACKET_EMULATE_INPUT;
			query->len			= len;
			query->pasting		= 1;
			strcpy(query->text,str);

			return query_intval((void *) query, cbSize, true);

#elif defined(HAVE_DBUS)

			return query_intval("input", DBUS_TYPE_STRING, &str, DBUS_TYPE_INVALID);
#else

			return -1;

#endif

		}

		int get_field_start(int baddr)
		{
#if defined(WIN32)

			struct hllapi_packet_addr query = { HLLAPI_PACKET_FIELD_START, (unsigned short) baddr };

			return query_intval((void *) &query, sizeof(query));

#elif defined(HAVE_DBUS)

			dbus_int32_t k = (dbus_int32_t) baddr;

			return query_intval("getFieldStart", DBUS_TYPE_INT32, &k, DBUS_TYPE_INVALID);

#else

			return -1;

#endif

		}

		int get_field_len(int baddr)
		{
#if defined(WIN32)

			struct hllapi_packet_addr query = { HLLAPI_PACKET_FIELD_LEN, (unsigned short) baddr };

			return query_intval((void *) &query, sizeof(query));

#elif defined(HAVE_DBUS)

			dbus_int32_t k = (dbus_int32_t) baddr;

			return query_intval("getFieldLength", DBUS_TYPE_INT32, &k, DBUS_TYPE_INVALID);

#else

			return -1;

#endif
		}

		int get_next_unprotected(int baddr)
		{
#if defined(WIN32)

			struct hllapi_packet_addr query = { HLLAPI_PACKET_NEXT_UNPROTECTED, (unsigned short) baddr };

			return query_intval((void *) &query, sizeof(query));

#elif defined(HAVE_DBUS)

			dbus_int32_t k = (dbus_int32_t) baddr;

			DBusMessage * msg = create_message("getNextUnprotected");
			if(msg)
			{
				dbus_message_append_args(msg, DBUS_TYPE_INT32, &k, DBUS_TYPE_INVALID);
				return get_intval(call(msg));
			}

			return -1;

#else

			return -1;

#endif

		}

		int set_host_charset(const char *charset)
		{
#if defined(WIN32)

			size_t							  len		= strlen(charset);
			struct hllapi_packet_set_text 	* query;
			size_t							  cbSize	= sizeof(struct hllapi_packet_set_text)+len;

			query = (struct hllapi_packet_set_text *) malloc(cbSize);
			query->packet_id 	= HLLAPI_PACKET_SET_HOST_CHARSET;
			query->len			= len;
			strcpy(query->text,charset);

			return query_intval((void *) query, cbSize, true);

#elif defined(HAVE_DBUS)

			return query_intval("setHostCharset", DBUS_TYPE_STRING, &charset, DBUS_TYPE_INVALID);

#else
			return -1;
#endif
		}

		string * get_host_charset(void)
		{
#if defined(WIN32)

			struct hllapi_packet_query query = { HLLAPI_PACKET_GET_HOST_CHARSET };
			return query_string(&query,sizeof(query),100);

#elif defined(HAVE_DBUS)

			return query_string("getHostCharset");

#else

			return NULL;

#endif
		}


#if defined(HAVE_DBUS)
		string * get_clipboard(void)
		{
			return query_string("getClipboard");
		}

		int set_clipboard(const char *text)
		{
			return query_intval("setClipboard", DBUS_TYPE_STRING, &text, DBUS_TYPE_INVALID);
		}

		string * get_display_charset(void)
		{
			return query_string("getDisplayCharset");
		}

		int popup_dialog(LIB3270_NOTIFY id , const char *title, const char *message, const char *fmt, ...)
		{
			DBusMessage * msg = dbus_message_new_method_call(	this->dest,		// Destination
																this->path,		// Path
																this->intf,		// Interface
																"showPopup");	// method

			if (!msg)
			{
				throw exception("%s","Error creating DBUS message for popup");
				return -1;
			}
			else
			{
				char			  text[4096];
				char			* ptr = text;
				va_list			  arg_ptr;
				dbus_int32_t	  i = (dbus_int32_t) id;

				va_start(arg_ptr, fmt);
				vsnprintf(text,4095,fmt,arg_ptr);
				va_end(arg_ptr);

				if(!dbus_message_append_args(msg, DBUS_TYPE_INT32, &i, DBUS_TYPE_STRING, &title, DBUS_TYPE_STRING, &message, DBUS_TYPE_STRING, &ptr, DBUS_TYPE_INVALID))
				{
					dbus_message_unref(msg);
					throw exception("%s","Cant append args for popup message");
				}
				else
				{
					DBusMessage             * reply;
					DBusError                 error;

					dbus_error_init(&error);
					reply = dbus_connection_send_with_reply_and_block(conn,msg,DBUS_TIMEOUT_INFINITE,&error);
					dbus_message_unref(msg);

					if(!reply)
					{
						exception e = exception("%s",error.message);
						dbus_error_free(&error);
						throw e;
						return -1;
					}

					return get_intval(reply);

				}
			}
			return 0;
		}

#endif // HAVE_DBUS

		int	erase_eof(void)
		{
			return query_intval(HLLAPI_PACKET_ERASE_EOF);
		}

		int	print(void)
		{
			return query_intval(HLLAPI_PACKET_PRINT);
		}

		const char * asc2ebc(unsigned char *str, size_t sz)
		{
			#warning Incomplete
			return (const char *) str;
		}

		const char * ebc2asc(unsigned char *str, size_t sz)
		{
			#warning Incomplete
			return (const char *) str;
		}


 	};

	session	* session::create_remote(const char *session)
	{
		return new remote(session);
	}

 }

