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
 * Este programa está nomeado como pluginmain.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#ifndef RX3270_H_INCLUDED

 #define RX3270_H_INCLUDED 1

 #include <lib3270/config.h>

 #ifdef HAVE_OOREXXAPI_H
	#ifdef WIN32
		#define _SSIZE_T_DEFINED
	#endif
	#include <oorexxapi.h>
 #else
	#error Only Rexx 4
 #endif

 #include <errno.h>
 #include <stdio.h>
 #include <lib3270.h>
 #include <lib3270/log.h>
 #include <lib3270/popup.h>
 #include <stdarg.h>
 #include <gtk/gtk.h>

#ifndef ETIMEDOUT
	#define ETIMEDOUT -1
#endif // !ETIMEOUT

#ifdef HAVE_ICONV
	#include <iconv.h>
#endif // HAVE_ICONV

#ifdef WIN32
	#define REXX_DEFAULT_CHARSET "CP1252"
#else
	#define REXX_DEFAULT_CHARSET "UTF-8"
#endif // WIN32

// #include <exception>

/*---[ Rexx entry points ]-----------------------------------------------------------------------------------*/

 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270version);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270QueryCState);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270Disconnect);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270Connect);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270isConnected);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270WaitForEvents);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270Sleep);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270SendENTERKey);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270SendPFKey);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270SendPAKey);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270WaitForTerminalReady);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270WaitForStringAt);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270GetStringAt);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270IsTerminalReady);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270queryStringAt);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270SetStringAt);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270CloseApplication);
 REXX_TYPED_ROUTINE_PROTOTYPE(ebc2asc);
 REXX_TYPED_ROUTINE_PROTOTYPE(asc2ebc);

 REXX_METHOD_PROTOTYPE(rx3270_method_version);
 REXX_METHOD_PROTOTYPE(rx3270_method_revision);
 REXX_METHOD_PROTOTYPE(rx3270_method_init);
 REXX_METHOD_PROTOTYPE(rx3270_method_uninit);
 REXX_METHOD_PROTOTYPE(rx3270_method_connect);
 REXX_METHOD_PROTOTYPE(rx3270_method_disconnect);
 REXX_METHOD_PROTOTYPE(rx3270_method_sleep);
 REXX_METHOD_PROTOTYPE(rx3270_method_is_connected);
 REXX_METHOD_PROTOTYPE(rx3270_method_is_ready);
 REXX_METHOD_PROTOTYPE(rx3270_method_wait_for_ready);
 REXX_METHOD_PROTOTYPE(rx3270_method_set_cursor);
 REXX_METHOD_PROTOTYPE(rx3270_method_get_cursor_addr);
 REXX_METHOD_PROTOTYPE(rx3270_method_set_cursor_addr);
 REXX_METHOD_PROTOTYPE(rx3270_method_enter);
 REXX_METHOD_PROTOTYPE(rx3270_method_pfkey);
 REXX_METHOD_PROTOTYPE(rx3270_method_pakey);
 REXX_METHOD_PROTOTYPE(rx3270_method_get_text);
 REXX_METHOD_PROTOTYPE(rx3270_method_get_text_at);
 REXX_METHOD_PROTOTYPE(rx3270_method_set_text_at);
 REXX_METHOD_PROTOTYPE(rx3270_method_cmp_text_at);
 REXX_METHOD_PROTOTYPE(rx3270_method_event_trace);
 REXX_METHOD_PROTOTYPE(rx3270_method_screen_trace);
 REXX_METHOD_PROTOTYPE(rx3270_method_ds_trace);
 REXX_METHOD_PROTOTYPE(rx3270_method_set_option);
 REXX_METHOD_PROTOTYPE(rx3270_method_test);
 REXX_METHOD_PROTOTYPE(rx3270_method_wait_for_text_at);
 REXX_METHOD_PROTOTYPE(rx3270_method_get_field_len);
 REXX_METHOD_PROTOTYPE(rx3270_method_get_field_start);
 REXX_METHOD_PROTOTYPE(rx3270_method_get_next_unprotected);
 REXX_METHOD_PROTOTYPE(rx3270_method_get_selection);
 REXX_METHOD_PROTOTYPE(rx3270_method_set_selection);
 REXX_METHOD_PROTOTYPE(rx3270_method_get_clipboard);
 REXX_METHOD_PROTOTYPE(rx3270_method_set_clipboard);
 REXX_METHOD_PROTOTYPE(rx3270_method_popup);
 REXX_METHOD_PROTOTYPE(rx3270_method_get_filename);
 REXX_METHOD_PROTOTYPE(rx3270_method_get_cursor_addr);
 REXX_METHOD_PROTOTYPE(rx3270_method_set_cursor_addr);
 REXX_METHOD_PROTOTYPE(rx3270_method_input_text);
 REXX_METHOD_PROTOTYPE(rx3270_method_get_display_charset);
 REXX_METHOD_PROTOTYPE(rx3270_method_get_host_charset);
 REXX_METHOD_PROTOTYPE(rx3270_method_set_host_charset);

/*---[ Globals ]---------------------------------------------------------------------------------------------*/

/*--[ 3270 Session ]-----------------------------------------------------------------------------------------*/

/*
#if defined (HAVE_GNUC_VISIBILITY)
 class __attribute__((visibility("default"))) rx3270
#elif defined(WIN32)
 class __declspec (dllexport) rx3270
#else
	#error NOT_IMPLEMENTED
#endif
 {

 protected:
#ifdef HAVE_ICONV
	iconv_t conv2Local;
	iconv_t conv2Host;
#endif

 public:

	class exception : public std::exception
	{
	public:
			exception(int code, const char *fmt, ...);
			exception(const char *fmt, ...);

			const char	* getMessage(void);
			void 		  logMessage(void);

			void		  RaiseException(RexxMethodContext *context);
			void		  RaiseException(RexxCallContext *context);

			virtual const char * what() const throw();

	private:
			int		code;
			char	msg[4096];

	};

	rx3270(const char *local = REXX_DEFAULT_CHARSET, const char *remote = "ISO-8859-1");

	virtual ~rx3270();

    virtual void free(void *ptr);


	static rx3270			* create(const char *name = NULL);
	static rx3270			* create_remote(const char *name);
	static rx3270			* create_local(void);
	static rx3270			* get_default(void);

	static void				  set_plugin(rx3270 * (*factory)(const char *name));

	char 					* get_3270_string(const char *str);
	char 					* get_local_string(const char *str);

	void			  		  log(const char *fmt, ...);
	virtual void			  logva(const char *fmt, va_list arg);

	virtual char			* get_version(void);
	virtual char			* get_revision(void);
	virtual LIB3270_CSTATE	  get_cstate(void)	= 0;

	virtual int				  connect(const char *uri, bool wait = true)		= 0;
	virtual int				  disconnect(void)									= 0;
	virtual bool			  is_connected(void)								= 0;
	virtual bool			  is_ready(void)									= 0;
	virtual int				  iterate(bool wait = true)							= 0;
	virtual int				  wait(int seconds)									= 0;
	virtual int				  wait_for_ready(int seconds)						= 0;
	virtual int				  wait_for_text_at(int row, int col, const char *key, int timeout);

	virtual int				  set_cursor_position(int row, int col)				= 0;
	virtual int               set_cursor_addr(int addr)                         = 0;
	virtual int               get_cursor_addr(void)                             = 0;

	virtual int 			  set_toggle(LIB3270_TOGGLE ix, bool value)			= 0;

	virtual int				  enter(void)										= 0;
	virtual int				  pfkey(int key)									= 0;
	virtual int				  pakey(int key)									= 0;

	virtual char 			* get_text_at(int row, int col, size_t sz) 			= 0;
	virtual char			* get_text(int baddr, size_t len)					= 0;
	virtual int				  cmp_text_at(int row, int col, const char *text)	= 0;
	virtual int 			  set_text_at(int row, int col, const char *str)	= 0;
	virtual int               emulate_input(const char *str)                    = 0;

	virtual int               get_field_start(int baddr = -1)                   = 0;
	virtual int               get_field_len(int baddr = -1)                     = 0;
	virtual int               get_next_unprotected(int baddr = -1)              = 0;

	virtual int               set_copy(const char *text);
	virtual char            * get_copy(void);

    virtual char            * get_clipboard(void);
    virtual int               set_clipboard(const char *text);

    virtual int				  quit(void)										= 0;

    // Dialogs
	virtual int               popup_dialog(LIB3270_NOTIFY id , const char *title, const char *message, const char *fmt, ...);
	virtual char            * file_chooser_dialog(GtkFileChooserAction action, const char *title, const char *extension, const char *filename);

 };

 rx3270 * create_lib3270_instance(void);
*/

#ifdef __cplusplus
	extern "C" {
#endif

    LIB3270_EXPORT void rx3270_set_package_option(RexxOption *option);

#ifdef __cplusplus
	}
#endif

#endif // RX3270_H_INCLUDED
