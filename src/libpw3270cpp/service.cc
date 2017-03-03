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
 * Este programa está nomeado como service.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 *
 */

 #include <lib3270/config.h>

 #include <iostream>
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

 #include <pw3270cpp.h>
 #include <lib3270/log.h>

/*--[ Implement ]--------------------------------------------------------------------------------------------------*/

 #ifdef HAVE_DBUS
 namespace PW3270_NAMESPACE
 {
	DBusConnection	* conn;

	class client : public session
	{
	private:

		#define DBUS_DESTINATION	"br.com.bb.pw3270.service"
		#define DBUS_PATH			"/br/com/bb/pw3270/service"
		#define DBUS_INTERFACE		"br.com.bb.pw3270.service"

		DBusConnection	* conn;
		string 			  name;
		const char		* id;

		DBusMessage * createMessage(const char *method)
		{
			DBusMessage * msg = dbus_message_new_method_call(	DBUS_DESTINATION,				// Destination
																DBUS_PATH,						// Path
																DBUS_INTERFACE,					// Interface
																method);						// method

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

			if(dbus_error_is_set(&error))
			{
				exception e = exception("%s",error.message);
				dbus_error_free(&error);
				throw e;
			}

			if(!reply)
			{
				exception e = exception("No reply for %s message","DBUS");
				throw e;
			}

			return reply;
		}

		string getString(DBusMessage *msg)
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

		int getInteger(DBusMessage *msg) {

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

		string getString(const char *method)
		{
			return getString(call(createMessage(method)));
		}

		int getInteger(const char *method)
		{
			return getInteger(call(createMessage(method)));
		}

		int getInteger(const char *method, int first_arg_type, ...)
		{
			va_list 	  var_args;
			DBusMessage * msg = createMessage(method);

			va_start(var_args, first_arg_type);
			dbus_message_append_args_valist(msg,first_arg_type,var_args);
			va_end(var_args);

			return getInteger(call(msg));
		}

		string getString(const char *method, int first_arg_type, ...)
		{
			va_list 	  var_args;
			DBusMessage * msg = createMessage(method);

			va_start(var_args, first_arg_type);
			dbus_message_append_args_valist(msg,first_arg_type,var_args);
			va_end(var_args);

			return getString(call(msg));
		}

	protected:

		virtual string get_text(int baddr = 0, size_t len = 1)
		{

		}

		virtual string get_text_at(int row, int col, size_t sz)
		{
			dbus_int32_t r = (dbus_int32_t) row;
			dbus_int32_t c = (dbus_int32_t) col;
			dbus_int32_t s = (dbus_int32_t) sz;

			return getString(	"getTextAt",
								DBUS_TYPE_STRING, this->id,
								DBUS_TYPE_INT32, &r,
								DBUS_TYPE_INT32, &c,
								DBUS_TYPE_INT32, &s,
								DBUS_TYPE_INVALID);

		}

		virtual int set_text_at(int row, int col, const char *str)
		{
			dbus_int32_t r = (dbus_int32_t) row;
			dbus_int32_t c = (dbus_int32_t) col;

			return getInteger(	"setTextAt",
								DBUS_TYPE_STRING, this->id,
								DBUS_TYPE_INT32, &r,
								DBUS_TYPE_INT32, &c,
								DBUS_TYPE_STRING, str,
								DBUS_TYPE_INVALID);
		}

		virtual int cmp_text_at(int row, int col, const char *str)
		{
			dbus_int32_t r = (dbus_int32_t) row;
			dbus_int32_t c = (dbus_int32_t) col;

			return getInteger(	"cmpTextAt",
								DBUS_TYPE_STRING, this->id,
								DBUS_TYPE_INT32, &r,
								DBUS_TYPE_INT32, &c,
								DBUS_TYPE_STRING, str,
								DBUS_TYPE_INVALID);
		}

		virtual int emulate_input(const char *str)
		{

		}

	public:
		client(const char *session) throw (std::exception)
		{
			DBusError err;

			dbus_error_init(&err);
			conn = dbus_bus_get(DBUS_BUS_SESSION, &err);

			if(dbus_error_is_set(&err))
			{
				exception e = exception("DBUS Connection Error (%s)", err.message);
				dbus_error_free(&err);
				throw e;
			}

			if(!conn)
			{
				throw exception("%s", "DBUS Connection failed");
			}

			if(*session != '?')
			{
				// Já tem sessão definida, usa.
				this->name = session;
			}
			else
			{
				// Obter um ID de sessão no serviço
				this->name = getString("createSession");
			}

			trace("Session=%s",this->name.c_str());

			this->id = this->name.c_str();
		}

		virtual ~client()
		{

		}

		virtual bool is_connected(void)
		{
			return getInteger("isConnected", DBUS_TYPE_STRING, &this->id, DBUS_TYPE_INVALID);
		}

		virtual bool is_ready(void)
		{
			return getInteger("isReady", DBUS_TYPE_STRING, &this->id, DBUS_TYPE_INVALID);
		}

		virtual LIB3270_CSTATE get_cstate(void)
		{
			return (LIB3270_CSTATE) getInteger("getConnectionState", DBUS_TYPE_STRING, &this->id, DBUS_TYPE_INVALID);
		}

		virtual LIB3270_MESSAGE get_program_message(void)
		{
			return (LIB3270_MESSAGE) getInteger("getProgramMessage", DBUS_TYPE_STRING, &this->id, DBUS_TYPE_INVALID);
		}

		virtual LIB3270_SSL_STATE get_secure(void)
		{
			return (LIB3270_SSL_STATE) getInteger("getSecureState", DBUS_TYPE_STRING, &this->id, DBUS_TYPE_INVALID);
		}

		virtual int get_width(void)
		{
			return getInteger("getScreenWidth", DBUS_TYPE_STRING, &this->id, DBUS_TYPE_INVALID);
		}

		virtual int get_height(void)
		{
			return getInteger("getScreenHeight", DBUS_TYPE_STRING, &this->id, DBUS_TYPE_INVALID);
		}

		virtual int get_length(void)
		{
			return getInteger("getScreenLength", DBUS_TYPE_STRING, &this->id, DBUS_TYPE_INVALID);
		}

		virtual void set_unlock_delay(unsigned short ms)
		{
			dbus_int32_t val = (dbus_int32_t) ms;
			getInteger("setUnlockDelay", DBUS_TYPE_STRING, &this->id, DBUS_TYPE_INT32, &val, DBUS_TYPE_INVALID);
		}

		virtual int set_host_charset(const char *charset)
		{

		}

		virtual string get_host_charset(void)
		{

		}

		virtual int connect()
		{
			return connect("",0);
		}

		virtual int connect(const char *url, time_t wait)
		{
			int rc = getInteger("connect", DBUS_TYPE_STRING, &this->id, DBUS_TYPE_STRING, &url, DBUS_TYPE_INVALID);

			if(!rc && wait) {
				rc = wait_for_ready(wait);
			}

			return rc;

		}

		virtual int	set_url(const char *hostname)
		{
			return getInteger("setURL", DBUS_TYPE_STRING, &this->id, DBUS_TYPE_STRING, &hostname, DBUS_TYPE_INVALID);
		}

		virtual string get_url()
		{

		}

		virtual int disconnect(void)
		{
			return getInteger("disconnect", DBUS_TYPE_STRING, &this->id, DBUS_TYPE_INVALID);
		}

		virtual int wait_for_ready(int seconds)
		{
			time_t end = time(0)+seconds;

			while(time(0) < end)
			{
				if(!is_connected())
					return ENOTCONN;

				if(is_ready())
					return 0;

				usleep(500);
			}

			return ETIMEDOUT;
		}

		virtual int wait(int seconds)
		{

			time_t end = time(0)+seconds;

			while(time(0) < end)
			{
				if(!is_connected())
					return ENOTCONN;
				usleep(500);
			}

			return 0;
		}

		virtual int iterate(bool wait)
		{
			if(wait)
				usleep(100);
			return 0;
		}

		virtual const char * asc2ebc(unsigned char *str, int sz = -1)
		{

		}

		virtual const char * ebc2asc(unsigned char *str, int sz = -1)
		{

		}

		virtual int set_cursor_position(int row, int col)
		{

		}

		virtual int set_cursor_addr(int addr)
		{

		}

		virtual int get_cursor_addr(void)
		{

		}

		virtual int set_toggle(LIB3270_TOGGLE ix, bool value)
		{

		}

		virtual int	enter(void)
		{
			return getInteger("enter", DBUS_TYPE_STRING, &this->id, DBUS_TYPE_INVALID);
		}

		virtual int	pfkey(int key)
		{
			dbus_int32_t val = (dbus_int32_t) key;
			getInteger("pfKey", DBUS_TYPE_STRING, &this->id, DBUS_TYPE_INT32, &val, DBUS_TYPE_INVALID);
		}

		virtual int	pakey(int key)
		{
			dbus_int32_t val = (dbus_int32_t) key;
			getInteger("paKey", DBUS_TYPE_STRING, &this->id, DBUS_TYPE_INT32, &val, DBUS_TYPE_INVALID);
		}

		virtual int	quit(void)
		{

		}

		virtual int	action(const char *name)
		{

		}

		virtual int erase(void)
		{

		}

		virtual int erase_eof(void)
		{

		}

		virtual int erase_eol(void)
		{

		}

		virtual int erase_input(void)
		{

		}

		virtual int print(void)
		{

		}

		virtual int get_field_start(int baddr = -1)
		{

		}

		virtual int get_field_len(int baddr = -1)
		{

		}

		virtual int get_next_unprotected(int baddr = -1)
		{

		}

		virtual int get_is_protected(int baddr = -1)
		{

		}

		virtual int get_is_protected_at(int row, int col)
		{

		}


	};

	session	* create_service_client(const char *session) throw (std::exception)
	{
		return new client(session);
	}


 }
 #endif // HAVE_DBUS
