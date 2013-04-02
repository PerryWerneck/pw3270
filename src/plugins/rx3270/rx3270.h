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

 #include <errno.h>
 #include <stdio.h>
 #include <lib3270/config.h>
 #include <lib3270.h>
 #include <lib3270/log.h>

#ifdef HAVE_OOREXXAPI_H
	#include <oorexxapi.h>
#else
	#error Only Rexx 4
#endif

#ifndef ETIMEDOUT
	#define ETIMEDOUT -1
#endif // !ETIMEOUT

#ifdef HAVE_ICONV
	#include <iconv.h>
#endif // HAVE_ICONV

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

/*---[ Globals ]---------------------------------------------------------------------------------------------*/

/*--[ 3270 Session ]-----------------------------------------------------------------------------------------*/

#if defined (HAVE_GNUC_VISIBILITY)
 class __attribute__((visibility("default"))) rx3270
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

	rx3270();
	virtual ~rx3270();

	static rx3270	* get_default(void);

	char 			* get_3270_string(const char *str);
	char 			* get_local_string(const char *str);

	virtual const char		* get_version(void) = 0;
	virtual LIB3270_CSTATE	  get_cstate(void)	= 0;

	virtual int				  connect(const char *uri, bool wait = true)	= 0;
	virtual int				  disconnect(void)						= 0;
	virtual int				  is_connected(void)					= 0;
	virtual int				  is_ready(void)						= 0;
	virtual int				  iterate(void)							= 0;
	virtual int				  wait(int seconds)						= 0;
	virtual int				  wait_for_ready(int seconds)			= 0;
	virtual int				  set_cursor_position(int row, int col)	= 0;

	virtual int				  enter(void)							= 0;
	virtual int				  pfkey(int key)						= 0;
	virtual int				  pakey(int key)						= 0;

	virtual char 			* get_text_at(int row, int col, size_t sz) 			= 0;
	virtual int				  cmp_text_at(int row, int col, const char *text)	= 0;
	virtual int 			  set_text_at(int row, int col, const char *str)	= 0;



 };

#endif // RX3270_H_INCLUDED
