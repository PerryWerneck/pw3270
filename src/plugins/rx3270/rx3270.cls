/*-- REXX -----------------------------------------------------------------------*/
/*                                                                               */
/* "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270 */
/* (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a */
/* aplicativos mainframe. Registro no INPI sob o nome G3270.                     */
/*                                                                               */
/* Copyright (C) <2008> <Banco do Brasil S.A.>                                   */
/*                                                                               */
/* Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob */
/* os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela */
/* Free Software Foundation.                                                     */
/*                                                                               */
/* Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER */
/* GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO */
/* A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para */
/* obter mais detalhes.                                                          */
/*                                                                               */
/* Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este  */
/* programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple  */
/* Place, Suite 330, Boston, MA, 02111-1307, USA                                 */
/*                                                                               */
/* Contatos:                                                                     */
/*                                                                               */
/* perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)                   */
/* erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)                         */
/* licinio@bb.com.br		(Licínio Luis Branco)                                */
/*                                                                               */
/*-------------------------------------------------------------------------------*/

::requires "rx3270" LIBRARY

/*-------------------------------------------------------------------------------*/
/* 3270 class                                                                    */
/*-------------------------------------------------------------------------------*/

::class rx3270 public subclass object

::METHOD INIT EXTERNAL "LIBRARY rx3270 rx3270_method_init"
::METHOD UNINIT EXTERNAL "LIBRARY rx3270 rx3270_method_uninit"

::METHOD VERSION EXTERNAL "LIBRARY rx3270 rx3270_method_version"
::METHOD REVISION EXTERNAL "LIBRARY rx3270 rx3270_method_revision"

::METHOD CONNECT EXTERNAL "LIBRARY rx3270 rx3270_method_connect"
::METHOD DISCONNECT EXTERNAL "LIBRARY rx3270 rx3270_method_disconnect"

::METHOD SLEEP EXTERNAL "LIBRARY rx3270 rx3270_method_sleep"

::METHOD CONNECTED EXTERNAL "LIBRARY rx3270 rx3270_method_is_connected"
::METHOD READY EXTERNAL "LIBRARY rx3270 rx3270_method_is_ready"

::METHOD WAITFORREADY EXTERNAL "LIBRARY rx3270 rx3270_method_wait_for_ready"

::METHOD SETCURSOR EXTERNAL "LIBRARY rx3270 rx3270_method_set_cursor"
::METHOD GETCURSORADDR EXTERNAL "LIBRARY rx3270 rx3270_method_get_cursor_addr"
::METHOD SETCURSORADDR EXTERNAL "LIBRARY rx3270 rx3270_method_set_cursor_addr"

::METHOD ENTER EXTERNAL "LIBRARY rx3270 rx3270_method_enter"
::METHOD PFKEY EXTERNAL "LIBRARY rx3270 rx3270_method_pfkey"
::METHOD PAKEY EXTERNAL "LIBRARY rx3270 rx3270_method_pakey"

::METHOD OPTION EXTERNAL "LIBRARY rx3270 rx3270_method_set_option"
::METHOD EVENTTRACE EXTERNAL "LIBRARY rx3270 rx3270_method_event_trace"
::METHOD SCREENTRACE EXTERNAL "LIBRARY rx3270 rx3270_method_screen_trace"
::METHOD DSTRACE EXTERNAL "LIBRARY rx3270 rx3270_method_ds_trace"

::METHOD GETTEXTAT EXTERNAL "LIBRARY rx3270 rx3270_method_get_text_at"
::METHOD SETTEXTAT EXTERNAL "LIBRARY rx3270 rx3270_method_set_text_at"
::METHOD CMPTEXTAT EXTERNAL "LIBRARY rx3270 rx3270_method_cmp_text_at"

::METHOD GET EXTERNAL "LIBRARY rx3270 rx3270_method_get_text"
::METHOD INPUT EXTERNAL "LIBRARY rx3270 rx3270_method_input_text"

::METHOD WAITFORTEXTAT EXTERNAL "LIBRARY rx3270 rx3270_method_wait_for_text_at"
::METHOD TEST EXTERNAL "LIBRARY rx3270 rx3270_method_test"

::METHOD GETFIELDSTART EXTERNAL "LIBRARY rx3270 rx3270_method_get_field_start"
::METHOD GETFIELDLEN EXTERNAL "LIBRARY rx3270 rx3270_method_get_field_len"
::METHOD GETNEXTUNPROTECTED EXTERNAL "LIBRARY rx3270 rx3270_method_get_next_unprotected"

::METHOD GETSELECTION EXTERNAL "LIBRARY rx3270 rx3270_method_get_selection"
::METHOD SETSELECTION EXTERNAL "LIBRARY rx3270 rx3270_method_set_selection"

::METHOD GETCLIPBOARD EXTERNAL "LIBRARY rx3270 rx3270_method_get_clipboard"
::METHOD SETCLIPBOARD EXTERNAL "LIBRARY rx3270 rx3270_method_set_clipboard"

::METHOD POPUP EXTERNAL "LIBRARY rx3270 rx3270_method_popup"
::METHOD GETFILENAME EXTERNAL "LIBRARY rx3270 rx3270_method_get_filename"

::METHOD GETDISPLAYCHARSET EXTERNAL "LIBRARY rx3270 rx3270_method_get_display_charset"
::METHOD GETHOSTCHARSET EXTERNAL "LIBRARY rx3270 rx3270_method_get_host_charset"
::METHOD SETHOSTCHARSET EXTERNAL "LIBRARY rx3270 rx3270_method_set_host_charset"

::method waitForStringAt
        use arg row, col, key, timeout
        if datatype(timeout) <> "NUM"
                then timeout = 60
return self~WaitForTextAt(row,col,key,timeout)

::method getScreenContent
return self~get()

::method queryStringAt
        use arg row, col, key
return self~GetTextAt(row,col,length(key)) == key

::method waitForTerminalReady
        use arg timeout
return self~WaitForReady(timeout)

::method isTerminalReady
return self~ready()

::method setStringAt
        use arg row, col, str
return self~SetTextAt(row,col,str)

::method getStringAt
        use arg row, col, size
return self~GetTextAt(row,col,size)

::method sendEnterKey
return self~enter()

::method sendPFKey
        use arg key
return self~pfKey(key)

::method setCursorPosition
        use arg row, col
return self~SetCursor(row,col)

::method getScreenContentAt
        use arg row, col, size
return self~GetTextAt(row,col,size)


