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
	#error No rexx 3 support (yet)
#endif // HAVE_OOREXXAPI_H

#ifdef HAVE_ICONV
	#include <iconv.h>
#endif // HAVE_ICONV


/*---[ Exports ]---------------------------------------------------------------------------------------------*/

 LIB3270_EXPORT RexxRoutineEntry rx3270_functions[];
 LIB3270_EXPORT RexxPackageEntry rx3270_package_entry;

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
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270ReadScreen);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270queryStringAt);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270SetStringAt);

/*---[ Globals ]---------------------------------------------------------------------------------------------*/

 enum rx3270mode
 {
	RX3270_MODE_STANDALONE,	/**< Running outside pw3270's main process */
	RX3270_MODE_PLUGIN,		/**< Running inside pw3270's main process */

	RX3270_MODE_UNDEFINED
 };

 #define RX3270SESSION lib3270_get_default_session_handle()

#ifdef HAVE_ICONV
 extern iconv_t outputConv;
 extern iconv_t inputConv;
#endif

/*--[ Prototipes ]-------------------------------------------------------------------------------------------*/

 char	* get_contents(H3270 *hSession, int start, int sz);
 char	* set_contents(H3270 *hSession, const char *text);

 LIB3270_EXPORT void rx3270_set_mode(enum rx3270mode mode);

#endif // RX3270_H_INCLUDED
