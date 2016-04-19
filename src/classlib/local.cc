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
 * Este programa está nomeado como local.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#if defined WIN32

	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms684179(v=vs.85).aspx
	#ifndef LOAD_LIBRARY_SEARCH_DEFAULT_DIRS
		#define LOAD_LIBRARY_SEARCH_DEFAULT_DIRS	0x00001000
	#endif // LOAD_LIBRARY_SEARCH_DEFAULT_DIRS

	#ifndef LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR
		#define LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR 	0x00000100
	#endif // LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR

	#include <windows.h>

#else

	#include <dlfcn.h>

#endif

#include <pw3270/class.h>
#include <lib3270/log.h>
#include <lib3270/popup.h>
#include <string.h>
#include <stdio.h>

#ifdef HAVE_SYSLOG
	#include <syslog.h>
	#include <stdarg.h>
#endif // HAVE_SYSLOG

/*--[ Implement ]--------------------------------------------------------------------------------------------------*/

 extern "C"
 {
	static void loghandler(H3270 *session, const char *module, int rc, const char *fmt, va_list args)
	{
	#ifdef HAVE_SYSLOG
		openlog(PACKAGE_NAME, LOG_NDELAY, LOG_USER);
		vsyslog(LOG_INFO,fmt,args);
		closelog();
	#endif // HAVE_SYSLOG
	}

	static void tracehandler(H3270 *session, const char *fmt, va_list args)
	{
	#ifdef HAVE_SYSLOG

		#define MAX_LOG_LENGTH 200

		static char	  line[MAX_LOG_LENGTH+1];
		char 		  temp[MAX_LOG_LENGTH];
		char		* ptr;
		size_t		  len = strlen(line);

		vsnprintf(temp,MAX_LOG_LENGTH-len,fmt,args);

		ptr = strchr(temp,'\n');
		if(!ptr)
		{
			strncat(line,temp,MAX_LOG_LENGTH);
			if(strlen(line) >= MAX_LOG_LENGTH)
			{
				openlog(PACKAGE_NAME, LOG_NDELAY, LOG_USER);
				syslog(LOG_INFO,"%s",line);
				closelog();
				*line = 0;
			}
			return;
		}

		*ptr = 0;
		strncat(line,temp,MAX_LOG_LENGTH);

		openlog(PACKAGE_NAME, LOG_NDELAY, LOG_USER);
		syslog(LOG_DEBUG,"%s",line);
		closelog();

		strncpy(line,ptr+1,MAX_LOG_LENGTH);

	#endif // HAVE_SYSLOG
	}

 }

 namespace PW3270_NAMESPACE
 {

 	class local : public session, private module
 	{
	private:

		H3270 * hSession;

		// Lib3270 entry points
		const char * 		(*_get_version)(void);
		LIB3270_CSTATE		(*_get_connection_state)(H3270 *h);
		LIB3270_MESSAGE		(*_get_program_message)(H3270 *h);
		LIB3270_SSL_STATE	(*_get_secure)(H3270 *h);


		int 				(*_disconnect)(H3270 *h);
		int 				(*_connect)(H3270 *h,int wait);
		const char 			(*_set_url)(H3270 *h, const char *n);
		int 				(*_is_connected)(H3270 *h);
		void 				(*_main_iterate)(H3270 *h, int wait);
		int 				(*_wait)(H3270 *hSession, int seconds);
		int 				(*_enter)(H3270 *hSession);
		int 				(*_pfkey)(H3270 *hSession, int key);
		int 				(*_pakey)(H3270 *hSession, int key);
		int 				(*_wait_for_ready)(H3270 *hSession, int seconds);
		char * 				(*_get_text)(H3270 *h, int offset, int len);
		char *  			(*_get_text_at)(H3270 *h, int row, int col, int len);
		int 				(*_cmp_text_at)(H3270 *h, int row, int col, const char *text);
		int 				(*_set_text_at)(H3270 *h, int row, int col, const unsigned char *str);
		int 				(*_is_ready)(H3270 *h);
		int 				(*_set_cursor_position)(H3270 *h, int row, int col);
		int 				(*_set_toggle)(H3270 *h, LIB3270_TOGGLE ix, int value);
		int             	(*_get_field_start)(H3270 *h, int baddr);
		int             	(*_get_field_len)(H3270 *h, int baddr);
		int             	(*_set_cursor_addr)(H3270 *h, int addr);
		int             	(*_get_cursor_addr)(H3270 *h);
		int             	(*_emulate_input)(H3270 *session, const char *s, int len, int pasting);
		int             	(*_get_next_unprotected)(H3270 *hSession, int baddr0);
		int 				(*_get_is_protected)(H3270 *hSession, int baddr);
		int 				(*_get_is_protected_at)(H3270 *hSession, int row, int col);
		void            	(*_popup_va)(H3270 *session, LIB3270_NOTIFY id , const char *title, const char *message, const char *fmt, va_list);
		void *				(*_free)(void *);
		const char *		(*_get_display_charset)(H3270 *hSession);
		int					(*_set_host_charset)(H3270 *hSession, const char *name);
		const char * 		(*_get_host_charset)(H3270 *hSession);
		int 				(*_print)(H3270 *hSession);
		int					(*_erase)(H3270 *hSession);
		int					(*_erase_eof)(H3270 *hSession);
		int					(*_erase_eol)(H3270 *hSession);
		int					(*_erase_input)(H3270 *hSession);
		int					(*_action)(H3270 *hSession, const char *name);
		int					(*_set_unlock_delay)(H3270 *hSession, unsigned short ms);

		const char * 		(*_ebc2asc)(H3270 *hSession, unsigned char *buffer, int sz);
		const char * 		(*_asc2ebc)(H3270 *hSession, unsigned char *buffer, int sz);

	public:

		local() throw(std::exception) : module("lib3270",PACKAGE_VERSION)
		{
			H3270 * (*lib3270_new)(const char *);
			void	(*set_log_handler)(void (*loghandler)(H3270 *, const char *, int, const char *, va_list));
			void 	(*set_trace_handler)( void (*handler)(H3270 *session, const char *fmt, va_list args) );

			struct _call
			{
				void 		**entry;
				const char 	* name;
			} call[] =
			{
				{ (void **) & lib3270_new,				"lib3270_session_new"				},
				{ (void **) & set_log_handler,			"lib3270_set_log_handler"			},
				{ (void **) & set_trace_handler,		"lib3270_set_trace_handler"			},

				{ (void **) & _is_connected,			"lib3270_is_connected"				},
				{ (void **) & _get_connection_state,	"lib3270_get_connection_state"		},
				{ (void **) & _get_program_message,		"lib3270_get_program_message"		},
				{ (void **) & _get_secure,				"lib3270_get_secure"				},

				{ (void **) & _get_version,				"lib3270_get_version"				},
				{ (void **) & _disconnect,				"lib3270_disconnect"				},
				{ (void **) & _connect,					"lib3270_connect"					},
				{ (void **) & _set_url,					"lib3270_set_url"					},
				{ (void **) & _main_iterate,			"lib3270_main_iterate"				},
				{ (void **) & _wait,					"lib3270_wait"						},
				{ (void **) & _enter,					"lib3270_enter"						},
				{ (void **) & _pfkey,					"lib3270_pfkey"						},
				{ (void **) & _pakey,					"lib3270_pakey"						},
				{ (void **) & _wait_for_ready,			"lib3270_wait_for_ready"			},
				{ (void **) & _get_text,				"lib3270_get_text"					},
				{ (void **) & _get_text_at,				"lib3270_get_text_at"				},
				{ (void **) & _cmp_text_at,				"lib3270_cmp_text_at"				},
				{ (void **) & _set_text_at,				"lib3270_set_string_at"				},
				{ (void **) & _is_ready,				"lib3270_is_ready"					},
				{ (void **) & _set_cursor_position,		"lib3270_set_cursor_position"		},
				{ (void **) & _set_toggle,				"lib3270_set_toggle"				},
				{ (void **) & _get_field_start,			"lib3270_get_field_start"			},
				{ (void **) & _get_field_len,			"lib3270_get_field_len"				},
				{ (void **) & _set_cursor_addr,			"lib3270_set_cursor_address"		},
				{ (void **) & _get_cursor_addr,			"lib3270_get_cursor_address"		},
				{ (void **) & _emulate_input,			"lib3270_emulate_input"	        	},
				{ (void **) & _get_next_unprotected,	"lib3270_get_next_unprotected"	   	},
				{ (void **) & _get_is_protected,		"lib3270_get_is_protected"			},
				{ (void **) & _get_is_protected_at,		"lib3270_get_is_protected_at"			},
				{ (void **) & _popup_va,	            "lib3270_popup_va"          	   	},
				{ (void **) & _free,					"lib3270_free"						},
				{ (void **) & _get_display_charset,		"lib3270_get_display_charset"		},
				{ (void **) & _set_host_charset,		"lib3270_set_host_charset"			},
				{ (void **) & _get_host_charset,		"lib3270_get_host_charset"			},

				{ (void **) & _erase,					"lib3270_erase"						},
				{ (void **) & _erase_eof,				"lib3270_eraseeof"					},
				{ (void **) & _erase_eol,				"lib3270_eraseeol"					},
				{ (void **) & _erase_input,				"lib3270_eraseinput"				},

				{ (void **) & _print,					"lib3270_print"						},
				{ (void **) & _ebc2asc,					"lib3270_ebc2asc"					},
				{ (void **) & _asc2ebc,					"lib3270_asc2ebc"					},

				{ (void **) & _action,					"lib3270_action"					},
				{ (void **) & _set_unlock_delay,		"lib3270_set_unlock_delay"			},

			};

			for(unsigned int f = 0; f < (sizeof (call) / sizeof ((call)[0]));f++)
			{
				*call[f].entry = (void *) get_symbol(call[f].name);
				if(!*call[f].entry)
					throw exception("Can't find symbol %s",call[f].name);
			}

			session::lock();

			// Get Session handle, setup base callbacks
			set_log_handler(loghandler);
			set_trace_handler(tracehandler);
			this->hSession = lib3270_new("");

			set_display_charset();

			session::unlock();

		}

		virtual ~local()
		{
			session::lock();

			debug("%s(%p,%p)",__FUNCTION__,this,this->hSession);
			if(is_connected()) {
				disconnect();
			}

			debug("%s(%p,%p)",__FUNCTION__,this,this->hSession);
			try
			{
				static void	(*session_free)(H3270 *h) = (void (*)(H3270 *)) get_symbol("lib3270_session_free");

				debug("%s(%p,%p)",__FUNCTION__,this,this->hSession);

				if(session_free && this->hSession)
					session_free(this->hSession);

				this->hSession = 0;

			}
			catch(exception e) { }

			session::unlock();

		}

		bool is_connected(void)
		{
			return _is_connected(hSession);
		}

		LIB3270_CSTATE get_cstate(void)
		{
			return _get_connection_state(hSession);
		}

		LIB3270_MESSAGE get_program_message(void) {
			return _get_program_message(hSession);
		}

		LIB3270_SSL_STATE get_secure(void) {
			return _get_secure(hSession);
		};

		int connect(void)
		{
			session::lock();
			int rc = _connect(hSession,0);
			session::unlock();

			return rc;
		}

		int set_url(const char *uri)
		{
			return (_set_url(hSession,uri) == 0);
		}

		int disconnect(void)
		{
			session::lock();
			int rc = _disconnect(hSession);
			session::unlock();

			return rc;
		}

		bool is_ready(void)
		{
			return _is_ready(hSession) != 0;
		}

		int wait_for_ready(int seconds)
		{
			return _wait_for_ready(hSession,seconds);
		}

		int wait(int seconds)
		{
			return _wait(hSession,seconds);
		}

		int iterate(bool wait)
		{
			session::lock();
			_main_iterate(hSession,wait);
			session::unlock();
			return 0;
		}

		string get_text_at(int row, int col, size_t sz)
		{
			string 	  rc;
			char	* ptr	= _get_text_at(hSession,row,col,sz);

			if(ptr)
			{
				rc.assign(ptr);
				_free(ptr);
			}

			return rc;
		}

		int set_text_at(int row, int col, const char *str)
		{
			return _set_text_at(hSession,row,col,(const unsigned char *) str);
		}

		int cmp_text_at(int row, int col, const char *text)
		{
			return _cmp_text_at(hSession,row,col,text);
		}

		string get_text(int offset, size_t len)
		{
			string	  rc;
			char	* ptr = _get_text(hSession,offset,len);

			if(ptr)
			{
				rc.assign(ptr);
				_free(ptr);
			}

			return rc;
		}

		int set_cursor_position(int row, int col)
		{
			return _set_cursor_position(hSession,row,col);
		}

		int set_cursor_addr(int addr)
		{
			return _set_cursor_addr(hSession,addr);
		}

		int get_cursor_addr(void)
		{
			return _get_cursor_addr(hSession);
		}

		int enter(void)
		{
			return _enter(hSession);
		}

		int pfkey(int key)
		{
			return _pfkey(hSession,key);
		}

		int pakey(int key)
		{
			return _pakey(hSession,key);
		}

		int quit(void)
		{
			return EINVAL;
		}

		int set_toggle(LIB3270_TOGGLE ix, bool value)
		{
			return _set_toggle(hSession, ix, (int) value);
		}

		int emulate_input(const char *str)
		{
			return _emulate_input(hSession,str,-1,1);
		}

		int get_field_start(int baddr)
		{
			return _get_field_start(hSession,baddr);
		}

		int get_field_len(int baddr)
		{
			return _get_field_len(hSession,baddr);
		}

		int get_next_unprotected(int baddr)
		{
			return _get_next_unprotected(hSession,baddr);
		}

		int get_is_protected(int baddr)
		{
			return _get_is_protected(hSession,baddr);
		}

		int get_is_protected_at(int row, int col)
		{
			return _get_is_protected_at(hSession,row,col);
		}

		int popup_dialog(LIB3270_NOTIFY id , const char *title, const char *message, const char *fmt, ...)
		{
			va_list	args;
			va_start(args, fmt);
			_popup_va(hSession, id, title, message, fmt, args);
			va_end(args);
			return 0;
		}

		string get_display_charset(void)
		{
			return string(_get_display_charset(hSession));
		}

		int set_host_charset(const char *charset)
		{
			return _set_host_charset(hSession,charset);
		}

		string get_host_charset(void)
		{
			return string(_get_host_charset(hSession));
		}

		int	erase(void)
		{
			return _erase(hSession);
		}

		int	erase_eof(void)
		{
			return _erase_eof(hSession);
		}

		int	erase_eol(void)
		{
			return _erase_eol(hSession);
		}

		int	erase_input(void)
		{
			return _erase_input(hSession);
		}

		int	print(void)
		{
			return _print(hSession);
		}


		const char * asc2ebc(unsigned char *str, int sz)
		{
			return _asc2ebc(hSession,str,sz);
		}

		const char * ebc2asc(unsigned char *str, int sz)
		{
			return _ebc2asc(hSession,str,sz);
		}

		int action(const char *name)
		{
			return _action(hSession,name);
		}

		void set_unlock_delay(unsigned short ms)
		{
			_set_unlock_delay(hSession,ms);
		}

 	};

	session	* session::create_local(void) throw (std::exception)
	{
		return new local();
	}

 }

