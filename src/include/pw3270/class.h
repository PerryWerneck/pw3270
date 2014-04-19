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
 * Este programa está nomeado como class.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#ifndef PW3270_CLASS_H_INCLUDED

 #define PW3270_CLASS_H_INCLUDED 1

 #ifdef WIN32
	#define SYSTEM_CHARSET "CP1252"
 #else
	#define SYSTEM_CHARSET "UTF-8"
 #endif // WIN32

 #include <exception>
 #include <errno.h>
 #include <lib3270/config.h>
 #include <lib3270.h>
 #include <lib3270/popup.h>
 #include <lib3270/filetransfer.h>

 #ifdef HAVE_ICONV
	#include <iconv.h>
 #endif // HAVE_ICONV

 #include <string>
 #include <stdarg.h>
 #include <lib3270.h>
 #include <errno.h>

 #ifdef DEBUG
	#include <stdio.h>
	#define trace( fmt, ... )	fprintf(stderr, "%s(%d) " fmt "\n", __FILE__, __LINE__, __VA_ARGS__ ); fflush(stderr);
 #else
	#define trace(x, ...) 		// __VA_ARGS__
 #endif

 #ifndef ETIMEDOUT
	#define ETIMEDOUT 1238
 #endif // !ETIMEDOUT

 #define PW3270_NAMESPACE h3270

 namespace PW3270_NAMESPACE
 {
 	using namespace std;

	class exception : public std::exception
	{
	public:
			exception(int syserror = errno);
			exception(const char *fmt, ...);

#ifdef WIN32
			exception(DWORD error, const char *fmt, ...);
#else
			exception(int error, const char *fmt, ...);
#endif // WIN32

			virtual const char * what() const throw();

	private:
			char	msg[4096];

	};


#if defined (HAVE_GNUC_VISIBILITY)
	class __attribute__((visibility("default"))) session
#elif defined(WIN32)
	class __declspec (dllexport) session
#else
	class session
#endif
	{
	public:

		virtual ~session();

		// Factory methods and settings
		static session	* start(const char *name = 0);
		static session	* create(const char *name = 0);
		static session	* get_default(void);
		static void		  set_plugin(session * (*factory)(const char *name));

		// Log management
		void log(const char *fmt, ...);
		void logva(const char *fmt, va_list args);

		// Information
		virtual const string	  get_version(void);
		virtual const string	  get_revision(void);

		virtual bool			  is_connected(void)								= 0;
		virtual bool			  is_ready(void)									= 0;

		virtual LIB3270_CSTATE	  get_cstate(void)									= 0;

		// charset
#ifdef WIN32
		void			  		  set_display_charset(const char *remote = 0, const char *local = "CP1252");
		string					  win32_strerror(int e);
#else
		void			  		  set_display_charset(const char *remote = 0, const char *local = "UTF-8");
#endif // WIN32

		virtual int				  set_host_charset(const char *charset)				= 0;
		virtual string			  get_host_charset(void)							= 0;
		virtual string			  get_display_charset(void);

		// Connection & Network
		int						  	  connect(const char *host, time_t wait = 0);
		int						 	  set_host(const char *host);
		virtual int				  connect(void)										= 0;
		virtual int				  set_url(const char *hostname)						= 0;
		virtual int				  disconnect(void)									= 0;
		virtual int				  wait_for_ready(int seconds)						= 0;
		virtual int				  wait(int seconds)									= 0;
		virtual int				  iterate(bool wait = true)							= 0;

		// Get/Set/Test without charset translation
		virtual string			  get_text(int baddr, size_t len)					= 0;
		virtual string 			  get_text_at(int row, int col, size_t sz) 			= 0;
		virtual int 			  set_text_at(int row, int col, const char *str)	= 0;
		virtual int				  cmp_text_at(int row, int col, const char *text)	= 0;
		virtual int				  wait_for_text_at(int row, int col, const char *key, int timeout);
		virtual int               emulate_input(const char *str)                    = 0;

		// Ascii<->EBCDIC translation
		virtual const char  	* asc2ebc(unsigned char *str, int sz = -1)			= 0;
		virtual const char	 	* ebc2asc(unsigned char *str, int sz = -1)			= 0;
		string					  asc2ebc(string &str);
		string					  ebc2asc(string &str);

		// Get/Set/Test with charset translation
		string					  get_string(int baddr, size_t len);
		string					  get_string_at(int row, int col, size_t sz);
		int			 			  set_string_at(int row, int col, const char *str);
		int				  		  cmp_string_at(int row, int col, const char *text);
		int				  		  wait_for_string_at(int row, int col, const char *key, int timeout);
		int						  input_string(const char *str);

		// Cursor management
		virtual int				  set_cursor_position(int row, int col)				= 0;
		virtual int               set_cursor_addr(int addr)                         = 0;
		virtual int               get_cursor_addr(void)                             = 0;

		// Toggle management
		virtual int 			  set_toggle(LIB3270_TOGGLE ix, bool value)			= 0;

		// Keyboard actions
		virtual int				  enter(void)										= 0;
		virtual int				  pfkey(int key)									= 0;
		virtual int				  pakey(int key)									= 0;
		virtual int				  quit(void)										= 0;

		// Actions
		virtual int				  erase_eof(void)									= 0;
		virtual int				  print(void)										= 0;

		// Field management
		virtual int               get_field_start(int baddr = -1)                   = 0;
		virtual int               get_field_len(int baddr = -1)                     = 0;
		virtual int               get_next_unprotected(int baddr = -1)              = 0;

		// Clipboard management
		virtual int               set_copy(const char *text);
		virtual string            get_copy(void);

		virtual string            get_clipboard(void);
		virtual int               set_clipboard(const char *text);

		// Dialogs
		virtual int               popup_dialog(LIB3270_NOTIFY id , const char *title, const char *message, const char *fmt, ...);
		virtual string            file_chooser_dialog(int action, const char *title, const char *extension, const char *filename);

		// File transfer
		virtual int				  file_transfer(LIB3270_FT_OPTION options, const char *local, const char *remote, int lrecl = 0, int blksize = 0, int primspace = 0, int secspace = 0, int dft = 4096);

		// Charset translation
		const char 				* get_encoding(void);

		string 					  get_3270_text(const char *str);
		string 					  get_local_text(const char *str);

	protected:
		session();

	private:

		session			* prev;
		session			* next;

		static session	* first;
		static session	* last;

		static session	* (*factory)(const char *name);

		static session	* create_remote(const char *name);
		static session	* create_local(void);

#ifdef HAVE_ICONV
		iconv_t			  conv2Local;
		iconv_t			  conv2Host;
#endif

	};


 }


#endif // PW3270_CLASS_H_INCLUDED
