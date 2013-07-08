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
 * Este programa está nomeado como pw3270class.c e possui - linhas de código.
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
 #include <lib3270/config.h>
 #include <lib3270.h>
 #include <lib3270/popup.h>

 #ifdef HAVE_ICONV
	#include <iconv.h>
 #endif // HAVE_ICONV

 #include <string>
 #include <stdarg.h>
 #include <lib3270.h>
 #include <gtk/gtk.h>

 namespace pw3270
 {
 	using namespace std;

	class exception : public std::exception
	{
	public:
			exception(int code, const char *fmt, ...);
			exception(const char *fmt, ...);

			virtual const char * what() const throw();

	private:
			int		code;
			char	msg[4096];

	};

	class session
	{
	public:

		session();
		virtual ~session();

		// Factory methods and settings
		static session	* create(const char *name = 0);
		static session	* start(const char *name = 0);
		static session	* get_default(void);
		static void		  set_plugin(session * (*factory)(const char *name));

		// Object settings
		void set_charset(const char *charset);

		// Log management
		void log(const char *fmt, ...);
		void logva(const char *fmt, va_list args);

		// 3270 methods
		virtual string			  get_version(void);
		virtual string			  get_revision(void);

		virtual bool			  is_connected(void)								= 0;
		virtual bool			  is_ready(void)									= 0;

		virtual LIB3270_CSTATE	  get_cstate(void)									= 0;

		virtual int				  connect(const char *uri, bool wait = true)		= 0;
		virtual int				  disconnect(void)									= 0;

		virtual int				  wait_for_ready(int seconds)						= 0;
		virtual int				  wait(int seconds)									= 0;
		virtual int				  iterate(bool wait = true)							= 0;

		virtual string 			* get_text_at(int row, int col, size_t sz) 			= 0;
		virtual int 			  set_text_at(int row, int col, const char *str)	= 0;
		virtual string			* get_text(int baddr, size_t len)					= 0;
		virtual int				  cmp_text_at(int row, int col, const char *text)	= 0;
		virtual int				  wait_for_text_at(int row, int col, const char *key, int timeout);

		virtual int				  set_cursor_position(int row, int col)				= 0;
		virtual int               set_cursor_addr(int addr)                         = 0;
		virtual int               get_cursor_addr(void)                             = 0;

		virtual int 			  set_toggle(LIB3270_TOGGLE ix, bool value)			= 0;

		virtual int				  enter(void)										= 0;
		virtual int				  pfkey(int key)									= 0;
		virtual int				  pakey(int key)									= 0;

		virtual int               emulate_input(const char *str)                    = 0;

		virtual int               get_field_start(int baddr = -1)                   = 0;
		virtual int               get_field_len(int baddr = -1)                     = 0;
		virtual int               get_next_unprotected(int baddr = -1)              = 0;

		virtual int               set_copy(const char *text);
		virtual string          * get_copy(void);

		virtual string          * get_clipboard(void);
		virtual int               set_clipboard(const char *text);

		virtual int				  quit(void)										= 0;

		// Dialogs
		virtual int               popup_dialog(LIB3270_NOTIFY id , const char *title, const char *message, const char *fmt, ...);
		virtual string          * file_chooser_dialog(GtkFileChooserAction action, const char *title, const char *extension, const char *filename);

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
