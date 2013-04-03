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

/*-------------------------------------------------------------------------------*/
/* 3270 class                                                                    */
/*-------------------------------------------------------------------------------*/

::class rx3270 public subclass object


::METHOD INIT EXTERNAL "LIBRARY rx3270 rx3270_method_init"
::METHOD UNINIT EXTERNAL "LIBRARY rx3270 rx3270_method_uninit"

::METHOD CONNECT EXTERNAL "LIBRARY rx3270 rx3270_method_connect"
::METHOD DISCONNECT EXTERNAL "LIBRARY rx3270 rx3270_method_disconnect"

::METHOD SLEEP EXTERNAL "LIBRARY rx3270 rx3270_method_sleep"

::METHOD CONNECTED EXTERNAL "LIBRARY rx3270 rx3270_method_is_connected"
::METHOD READY EXTERNAL "LIBRARY rx3270 rx3270_method_is_ready"

::METHOD WAITFORREADY EXTERNAL "LIBRARY rx3270 rx3270_method_wait_for_ready"

::METHOD SETCURSOR EXTERNAL "LIBRARY rx3270 rx3270_method_set_cursor"

::METHOD ENTER EXTERNAL "LIBRARY rx3270 rx3270_method_enter"
::METHOD PFKEY EXTERNAL "LIBRARY rx3270 rx3270_method_pfkey"
::METHOD PFKEY EXTERNAL "LIBRARY rx3270 rx3270_method_pakey"

::METHOD OPTION EXTERNAL "LIBRARY rx3270 rx3270_method_set_toggle"
::METHOD EVENTTRACE EXTERNAL "LIBRARY rx3270 rx3270_method_event_trace"
::METHOD SCREENTRACE EXTERNAL "LIBRARY rx3270 rx3270_method_screen_trace"
::METHOD DSTRACE EXTERNAL "LIBRARY rx3270 rx3270_method_ds_trace"

::METHOD GETTEXTAT EXTERNAL "LIBRARY rx3270 rx3270_method_get_text_at"
::METHOD SETTEXTAT EXTERNAL "LIBRARY rx3270 rx3270_method_set_text_at"
::METHOD CMPTEXTAT EXTERNAL "LIBRARY rx3270 rx3270_method_cmp_text_at"

