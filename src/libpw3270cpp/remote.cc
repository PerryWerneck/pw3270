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

 #include <config.h>
 #include <iostream>

 #if defined(HAVE_DBUS)
	#include <stdio.h>
	#include <dbus/dbus.h>
	#include <string.h>
	#include <malloc.h>
	#include <sys/types.h>
	#include <unistd.h>
	#include <limits.h>

	#ifndef DBUS_TIMEOUT_INFINITE
		#define DBUS_TIMEOUT_INFINITE ((int) 0x7fffffff)
	#endif // !DBUS_TIMEOUT_INFINITE

 #endif // HAVE_DBUS

 #if defined(WIN32)
	#include <windows.h>
	#include <pw3270/ipcpackets.h>
	#include <process.h>
 #else
	#define HLLAPI_PACKET_IS_CONNECTED			"isConnected"
	#define HLLAPI_PACKET_GET_CSTATE			"getConnectionState"
	#define HLLAPI_PACKET_GET_PROGRAM_MESSAGE	"getProgramMessage"
	#define HLLAPI_PACKET_GET_SSL_STATE			"getSecureState"
	#define HLLAPI_PACKET_IS_READY				"isReady"
	#define HLLAPI_PACKET_DISCONNECT			"disconnect"
	#define HLLAPI_PACKET_GET_HOST				"getURL"
	#define HLLAPI_PACKET_SET_HOST				"setURL"
	#define HLLAPI_PACKET_GET_CURSOR			"getCursorAddress"
	#define HLLAPI_PACKET_GET_WIDTH				"getScreenWidth"
	#define HLLAPI_PACKET_GET_HEIGHT			"getScreenHeight"
	#define HLLAPI_PACKET_GET_LENGTH			"getScreenLength"
	#define HLLAPI_PACKET_ENTER					"enter"
	#define HLLAPI_PACKET_QUIT					"quit"
	#define HLLAPI_PACKET_ERASE					"erase"
	#define HLLAPI_PACKET_ERASE_EOF				"eraseEOF"
	#define HLLAPI_PACKET_ERASE_EOL				"eraseEOL"
	#define HLLAPI_PACKET_ERASE_INPUT			"eraseInput"
	#define HLLAPI_PACKET_PRINT					"print"
	#define HLLAPI_PACKET_ASC2EBC				"asc2ebc"
	#define HLLAPI_PACKET_EBC2ASC				"ebc2asc"
	#define HLLAPI_PACKET_SET_UNLOCK_DELAY		"setUnlockDelay"
 #endif // WIN32

 #include <pw3270cpp.h>
 #include <lib3270/log.h>

 #include "private.h"

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

		int query_strval(HLLAPI_PACKET id, unsigned char *buffer, size_t sz)
		{
			DWORD								  cbSize	= sizeof(struct hllapi_packet_text)+sz;
			struct hllapi_packet_text			* query;
			struct hllapi_packet_text			* response;
			int 								  rc		= -1;

			query = (struct hllapi_packet_text *) malloc(cbSize+2);
			memset(query,0,cbSize+2);
			query->packet_id = id;
			memcpy(query->text,buffer,sz);

			response = (struct hllapi_packet_text *) malloc(cbSize+2);
			memset(response,0,cbSize+2);

			if(TransactNamedPipe(hPipe,(LPVOID) query, cbSize, &response, cbSize, &cbSize,NULL))
			{
				if(response->packet_id)
				{
					rc = response->packet_id;
				}
				else
				{
					rc = 0;
					strncpy((char *) buffer,response->text,sz);
				}
			}

			free(response);
			free(query);

			return rc;

		}

		string query_string(void *query, size_t szQuery, size_t len)
		{
			struct hllapi_packet_text	* response;
			DWORD 						  sz		= sizeof(struct hllapi_packet_text)+len;
			DWORD						  cbSize	= (DWORD) sz;
			string						  s;
			char						  buffer[sz+2];

			memset(buffer,0,sz+2);

			response = (struct hllapi_packet_text *) buffer;

			if(TransactNamedPipe(hPipe,(LPVOID) query, szQuery, response, sz, &cbSize,NULL))
			{
				buffer[min(cbSize,sz)] = 0;

				trace("TransactNamedPipe call %d returns \"%s\"",(int) *( (unsigned char *) query), response->text);

				if(!response->packet_id)
					s.assign(response->text);
			}
			else
			{
				trace("TransactNamedPipe error on call %d",(int) *( (unsigned char *) query));
				s.assign("");
			}

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

		void set_intval(HLLAPI_PACKET id, int value)
		{
			struct hllapi_packet_set_int	packet;
			DWORD							cbSize		= (DWORD) sizeof(packet);
			BOOL 							status;

			memset(&packet,0,sizeof(packet));
			packet.packet_id	= id;
			packet.value		= value;

			status = TransactNamedPipe(hPipe,(LPVOID) &packet, cbSize, &packet, sizeof(packet), &cbSize,NULL);

			if(!status)
				throw exception(GetLastError(),"%s","Transaction error");

		}


#elif defined(HAVE_DBUS)

		DBusConnection	* conn;
		char			* dest;
		char			* path;
		char			* intf;
		int 			  sequence;

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

		string get_string(DBusMessage * msg)
		{
			string rc;

			if(msg)
			{
				DBusMessageIter iter;

				if(dbus_message_iter_init(msg, &iter))
				{
					if(dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_STRING)
					{
						const char	* str;
						dbus_message_iter_get_basic(&iter, &str);
						trace("Response: [%s]",str);
						rc.assign(str);
						dbus_message_unref(msg);
						return rc;
					}

					exception e = exception("DBUS Return type was %c, expecting %c",dbus_message_iter_get_arg_type(&iter),DBUS_TYPE_INT32);
					dbus_message_unref(msg);

					throw e;

				}

			}

			return rc;
		}

		string query_string(const char *method)
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

		int query_strval(const char *method, unsigned char *buffer, size_t sz)
		{
			DBusMessage * outMsg = create_message(method);

			if(outMsg)
			{
				dbus_message_append_args(outMsg, DBUS_TYPE_STRING, &buffer, DBUS_TYPE_INVALID);

				DBusMessage * rspMsg = call(outMsg);
				if(rspMsg)
				{
					DBusMessageIter iter;

					if(dbus_message_iter_init(rspMsg, &iter))
					{
						if(dbus_message_iter_get_arg_type(&iter) == DBUS_TYPE_STRING)
						{
							const char	* str;
							dbus_message_iter_get_basic(&iter, &str);
							trace("Response: [%s]",str);
							strncpy((char *) buffer,str,sz);
							dbus_message_unref(rspMsg);
							return 0;
						}

						exception e = exception("DBUS Return type was %c, expecting %c",dbus_message_iter_get_arg_type(&iter),DBUS_TYPE_INT32);
						dbus_message_unref(rspMsg);

						throw e;

					}
				}
			}

			return -1;
		}

		void set_intval(const char *method, int value)
		{
			DBusMessage * outMsg = create_message(method);

			if(outMsg)
			{
				dbus_int32_t v = (dbus_int32_t) value;

				dbus_message_append_args(outMsg, DBUS_TYPE_INT32, &v, DBUS_TYPE_INVALID);

				DBusMessage * rspMsg = call(outMsg);
				dbus_message_unref(rspMsg);

			}
		}

#else


		int query_intval(const char *method)
		{
			throw exception("Call to unimplemented RPC method \"%s\"",method);
			return -1;
		}

		int query_strval(const char *method, unsigned char *buffer, size_t sz)
		{
			throw exception("Call to unimplemented RPC method \"%s\"",method);
			return -1;
		}


#endif

	public:

#if defined(HAVE_DBUS)
		const char * makeBusName(string &name)
		{
			int val;

			// First uses the object ID
			val = this->sequence;
			while(val > 0)
			{
				char str[] = { (char) ('a'+(val % 25)), 0 };
				name.append(str);
				val /= 25;
			}
			name.append(".");

			val = (int) getpid();
			while(val > 0)
			{
				char str[] = { 'a'+(val % 25), 0 };
				name.append(str);
				val /= 25;
			}
			name.append(".");

			// And last, the project info
			name.append(intf);

			/*
			size_t	  bytes = strlen(buffer);
			char 	* ptr	= buffer;
			int 	  val;

			sz -= 2;

			// First uses the object ID
			val = this->sequence;
			while(bytes < sz && val > 0)
			{
				*(ptr++) = 'a'+(val % 25);
				val /= 25;
				bytes++;
			}
			*(ptr++) = '.';

			// Then the PID
			val = (int) getpid();
			while(bytes < sz && val > 0)
			{
				*(ptr++) = 'a'+(val % 25);
				val /= 25;
				bytes++;
			}
			*(ptr++) = '.';

			// And last, the project info
			strncpy(ptr,intf,sz);

			*/
			trace("Busname=\"%s\" sequence=%d this=%p",name.c_str(),sequence,this);

			return name.c_str();

		}
#endif // HAVE_DBUS

#if defined(WIN32)

		static string getRegistryKey(const char *name) throw (std::exception)
		{
			char 					buffer[4096];
			HKEY 					hKey	= 0;
			unsigned long			datalen = sizeof(buffer);

			debug("%s(%s)",__FUNCTION__,name);

			*buffer = 0;
			if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Software\\pw3270",0,KEY_QUERY_VALUE,&hKey) != ERROR_SUCCESS)
			{
				throw exception("Can't open key %s","HKLM\\Software\\pw3270");
			}
			else
			{
				unsigned long datatype;	// #defined in winnt.h (predefined types 0-11)

				if(RegQueryValueExA(hKey,name,NULL,&datatype,(LPBYTE) buffer,&datalen) != ERROR_SUCCESS)
					*buffer = 0;
				RegCloseKey(hKey);
			}

			return string(buffer);

		}

#endif // defined

		remote(const char *session) throw (std::exception)
		{
#if defined(WIN32)
			static DWORD			  dwMode = PIPE_READMODE_MESSAGE;
			char	 				  buffer[4096];
			char					* str;
			char					* ptr;
			time_t					  timer	= time(0)+1;

			hPipe  = INVALID_HANDLE_VALUE;

			trace("%s(%s)",__FUNCTION__,session);

			if(strcasecmp(session,"start") == 0 || strcasecmp(session,"new") == 0)
			{
				// Start a new session
				string					appName = getRegistryKey("appName");
				char 					buffer[80];
				STARTUPINFO				si;
				PROCESS_INFORMATION		pi;

				// Get application path

				if(!appName.size())
				{
					throw exception("key %s\\appName is invalid","HKLM\\Software\\pw3270");
					return;
				}

				trace("%s appname=%s\n",__FUNCTION__,appName.c_str());

				snprintf(buffer,79,"%s --session=\"H%06d\"",appName.c_str(),getpid());

				ZeroMemory( &si, sizeof(si) );
				si.cb = sizeof(si);
				ZeroMemory( &pi, sizeof(pi) );

				// si.dwFlags = STARTF_PREVENTPINNING;
				trace("App: %s",appName.c_str());
				trace("CmdLine: %s",buffer);

				if(CreateProcess(NULL,buffer,NULL,NULL,0,NORMAL_PRIORITY_CLASS,NULL,NULL,&si,&pi))
				{
					CloseHandle( pi.hProcess );
					CloseHandle( pi.hThread );
				}
				else
				{
					throw exception("Can't start %s",appName.c_str());
					return;
				}

				snprintf(buffer,4095,"H%06d_a",getpid());
				str = strdup(buffer);

				// Até 20 segundos para o processo iniciar.
				timer = time(0)+20;

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

				// Wait?
				int delay;

				try
				{
					delay = atoi(getRegistryKey("hllapiWait").c_str());
				}
				catch(std::exception &e)
				{
					delay = 0;
				}

				if(delay) {
					timer = time(0) + delay;
				}


			}

			snprintf(buffer,4095,"\\\\.\\pipe\\%s",str);

			free(str);

			trace("Searching for \"%s\"",buffer);

			hPipe = CreateFile(buffer,GENERIC_WRITE|GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);

			if(hPipe == INVALID_HANDLE_VALUE)
			{
				// Cant get session, wait.
				while(hPipe == INVALID_HANDLE_VALUE && time(0) < timer)
				{
					hPipe = CreateFile(buffer,GENERIC_WRITE|GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);
					Sleep(1);
				}
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

			static int	  sq	= 0;
			DBusError	  err;
			int			  rc;
			char		* str = strdup(session);
			char		* ptr;
			string		  busName;

			this->sequence = (++sq) + time(0);

			trace("%s str=%p sequence=%d",__FUNCTION__,str,sequence);

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


			rc = dbus_bus_request_name(conn, makeBusName(busName), DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
			trace("dbus_bus_request_name(%s) rc=%d",busName.c_str(),rc);

			if (dbus_error_is_set(&err))
			{
				exception e = exception("Name Error (%s)", err.message);
				dbus_error_free(&err);
				throw e;
				return;
			}

			if(rc != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
			{
				trace("%s: DBUS request for name %s failed",__FUNCTION__, busName.c_str());
				throw exception("DBUS request for \"%s\" failed",session);
				return;
			}

			trace("%s: Using DBUS name %s",__FUNCTION__,busName.c_str());

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
				std::cerr << e.what();
			}

			string busName;
			makeBusName(busName);

			free(dest);
			free(path);
			free(intf);

			DBusError	err;

			dbus_error_init(&err);
			dbus_bus_release_name(conn,busName.c_str(),&err);

			if (dbus_error_is_set(&err))
			{
				//exception e = exception("Error when releasing DBUS name (%s)", err.message);
				std::cerr << err.message;
				dbus_error_free(&err);
				//throw e;
			}

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

		LIB3270_MESSAGE get_program_message(void) {
			return (LIB3270_MESSAGE) query_intval(HLLAPI_PACKET_GET_PROGRAM_MESSAGE);
		}

		LIB3270_SSL_STATE get_secure(void) {
			return (LIB3270_SSL_STATE) query_intval(HLLAPI_PACKET_GET_SSL_STATE);
		}

/*
		int connect(void)
		{
			int rc;

#if defined(WIN32)

			size_t							  cbSize	= sizeof(struct hllapi_packet_connect);
			struct hllapi_packet_connect	* pkt		= (struct hllapi_packet_connect *) malloc(cbSize);

			memset(pkt,0,cbSize);

			pkt->packet_id	= HLLAPI_PACKET_CONNECT;
			pkt->wait		= 0;

			rc = query_intval((void *) pkt,cbSize,true);

#elif defined(HAVE_DBUS)

			static const char * str = "";

			rc = query_intval("connect", DBUS_TYPE_STRING, &str, DBUS_TYPE_INVALID);

#else
			rc = -1;

#endif
			return rc;

		}
*/

		virtual int connect()
		{
			return connect("",0);
		}

		virtual int connect(const char *url, time_t wait)
		{
			int rc = EINVAL;

			debug("%s(%s,%u)",__FUNCTION__,url,(unsigned int) wait);

			if(!url) {
				url = "";
			}

#if defined(WIN32)

			size_t						  cbSize	= sizeof(struct hllapi_packet_query) + strlen(url) + 1;
			struct hllapi_packet_query	* pkt		= (struct hllapi_packet_query *) malloc(cbSize);

			memset(pkt,0,cbSize);

			pkt->packet_id = HLLAPI_PACKET_CONNECT_URL;

			strcpy(((char *) (pkt+1)), url);

			rc = query_intval((void *) pkt, cbSize, true);

			if(!rc && wait) {
				time_t end = time(0) + wait;
				while(!is_connected()) {
					if(time(0) > end) {
						debug("%s: Timeout",__FUNCTION__);
						return ETIMEDOUT;
					}
					Sleep(500);
				}
			}

#elif defined(HAVE_DBUS)

			rc = query_intval("connect", DBUS_TYPE_STRING, &url, DBUS_TYPE_INVALID);

			debug("connect(%s) rc=%d (%s)",url,rc,strerror(rc));

			if(!rc && wait) {
				time_t end = time(0) + wait;
				while(!is_connected()) {
					if(time(0) > end) {
						debug("%s: Timeout",__FUNCTION__);
						return ETIMEDOUT;
					}
					usleep(500);
				}
			}

#endif
			debug("connect(%s) rc=%d (%s)",url,rc,strerror(rc));

			return rc;

		}

		int set_url(const char *uri)
		{
			int rc;

#if defined(WIN32)

			size_t						  cbSize	= sizeof(struct hllapi_packet_text)+strlen(uri);
			struct hllapi_packet_text	* pkt		= (struct hllapi_packet_text *) malloc(cbSize);

			pkt->packet_id	= HLLAPI_PACKET_SET_HOST;
			strcpy(pkt->text,uri);

			rc = query_intval((void *) pkt,cbSize,true);

#elif defined(HAVE_DBUS)

			rc = query_intval(HLLAPI_PACKET_SET_HOST, DBUS_TYPE_STRING, &uri, DBUS_TYPE_INVALID);

#else

			rc = -1;

#endif

			return rc;

		}

		string get_url()
		{
#if defined(WIN32)

			struct hllapi_packet_query query	= { HLLAPI_PACKET_GET_HOST };
			return query_string(&query,sizeof(query),1024);

#elif defined(HAVE_DBUS)

			return query_string(HLLAPI_PACKET_GET_HOST);

#else
			return string();
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
				if(!is_connected())
					return ENOTCONN;

				if(is_ready())
					return 0;

				sleep(1);
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
			if(wait)
				Sleep(250);
			return 0;
#elif defined(HAVE_DBUS)
			if(wait)
				sleep(1);
			return 0;
#else
			return -1;
#endif
		}

		string get_text_at(int row, int col, size_t sz)
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

			return string();

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
			debug("%s(%d,%d,\"%s\")",__FUNCTION__,row,col,text);

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

		string get_text(int baddr, size_t len)
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
			return string();
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

		int get_width(void) {
			return query_intval(HLLAPI_PACKET_GET_WIDTH);
		}

		int get_height(void) {
			return query_intval(HLLAPI_PACKET_GET_HEIGHT);
		}

		int	get_length(void) {
			return query_intval(HLLAPI_PACKET_GET_LENGTH);
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

		int action(const char *str)
		{

#if defined(WIN32)

			size_t						  len           = strlen(str);
			struct hllapi_packet_text 	* query;
			size_t					      cbSize		= sizeof(struct hllapi_packet_text)+len;

			query = (struct hllapi_packet_text *) malloc(cbSize);
			query->packet_id 	= HLLAPI_PACKET_ACTION;
			strcpy(query->text,str);

			return query_intval((void *) query, cbSize, true);

#elif defined(HAVE_DBUS)

			return query_intval("action", DBUS_TYPE_STRING, &str, DBUS_TYPE_INVALID);
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

		int get_is_protected(int baddr)
		{
#if defined(WIN32)

			struct hllapi_packet_addr query = { HLLAPI_PACKET_IS_PROTECTED, (unsigned short) baddr };

			return query_intval((void *) &query, sizeof(query));

#elif defined(HAVE_DBUS)

			dbus_int32_t k = (dbus_int32_t) baddr;

			DBusMessage * msg = create_message("getIsProtected");
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

		int get_is_protected_at(int row,int col)
		{
#if defined(WIN32)

			struct hllapi_packet_query_at query	= { HLLAPI_PACKET_IS_PROTECTED_AT, (unsigned short) row, (unsigned short) col, 0 };

			return query_intval((void *) &query, sizeof(query));

#elif defined(HAVE_DBUS)

			dbus_int32_t r = (dbus_int32_t) row;
			dbus_int32_t c = (dbus_int32_t) col;

			DBusMessage * msg = create_message("getIsProtectedAt");
			if(msg)
			{
				dbus_message_append_args(msg, DBUS_TYPE_INT32, &r, DBUS_TYPE_INT32, &c, DBUS_TYPE_INVALID);
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

		string get_host_charset(void)
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
		string get_clipboard(void)
		{
			return query_string("getClipboard");
		}

		int set_clipboard(const char *text)
		{
			return query_intval("setClipboard", DBUS_TYPE_STRING, &text, DBUS_TYPE_INVALID);
		}

		string get_display_charset(void)
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

		int	erase(void)
		{
			return query_intval(HLLAPI_PACKET_ERASE);
		}

		int	erase_eof(void)
		{
			return query_intval(HLLAPI_PACKET_ERASE_EOF);
		}

		int	erase_eol(void)
		{
			return query_intval(HLLAPI_PACKET_ERASE_EOL);
		}

		int	erase_input(void)
		{
			return query_intval(HLLAPI_PACKET_ERASE_INPUT);
		}

		int	print(void)
		{
			return query_intval(HLLAPI_PACKET_PRINT);
		}

		const char * asc2ebc(unsigned char *text, int sz)
		{
			query_strval(HLLAPI_PACKET_ASC2EBC,text,sz);
			return (const char *) text;
		}

		const char * ebc2asc(unsigned char *text, int sz)
		{
			query_strval(HLLAPI_PACKET_EBC2ASC,text,sz);
			return (const char *) text;
		}

		void set_unlock_delay(unsigned short ms)
		{
			set_intval(HLLAPI_PACKET_SET_UNLOCK_DELAY,(int) ms);
		}

 	};

	session	* session::create_remote(const char *session) throw (std::exception)
	{
		debug("create(%s)",session);

		if(strncasecmp(session,"service://",10)) {
			return new remote(session);
		}

#ifdef HAVE_DBUS
		return create_service_client(session+10);
#else
		throw exception("Can't create session \"%s\"",session);
#endif // HAVE_DBUS

	}

 }

