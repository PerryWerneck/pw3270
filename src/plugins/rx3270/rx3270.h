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
 #include <stdint.h>

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
	#define int8_t		REXX_INT8_T
	#define ssize_t		REXX_SSIZE_T
#else
	#define REXX_DEFAULT_CHARSET "UTF-8"
#endif // WIN32

#include <oorexxapi.h>

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

 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270Erase);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270EraseEOF);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270EraseEOL);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270EraseInput);

 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270IsProtected);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270IsProtectedAt);
 REXX_TYPED_ROUTINE_PROTOTYPE(rx3270SetUnlockDelay);

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
 REXX_METHOD_PROTOTYPE(rx3270_method_erase);
 REXX_METHOD_PROTOTYPE(rx3270_method_erase_eof);
 REXX_METHOD_PROTOTYPE(rx3270_method_erase_eol);
 REXX_METHOD_PROTOTYPE(rx3270_method_erase_input);
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
 REXX_METHOD_PROTOTYPE(rx3270_method_get_is_protected);
 REXX_METHOD_PROTOTYPE(rx3270_method_get_is_protected_at);
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
 REXX_METHOD_PROTOTYPE(rx3270_method_set_display_charset);
 REXX_METHOD_PROTOTYPE(rx3270_method_get_host_charset);
 REXX_METHOD_PROTOTYPE(rx3270_method_set_host_charset);
 REXX_METHOD_PROTOTYPE(rx3270_method_set_unlock_delay);

/*---[ Globals ]---------------------------------------------------------------------------------------------*/

/*--[ 3270 Session ]-----------------------------------------------------------------------------------------*/

#ifdef __cplusplus
	extern "C" {
#endif

    LIB3270_EXPORT void rx3270_set_package_option(RexxOption *option);

#ifdef __cplusplus
	}
#endif

#endif // RX3270_H_INCLUDED
