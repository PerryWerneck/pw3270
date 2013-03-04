/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob o nome G3270.
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
 * Este programa está nomeado como hllapi.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

#ifndef HLLAPI_H_INCLUDED

	#define HLLAPI_H_INCLUDED 1
	#include <lib3270.h>

#ifdef __cplusplus
extern "C" {
#endif

 #define HLLAPI_MAXLENGTH				4096

 #define HLLAPI_CMD_CONNECTPS				   1	/**< connect presentation space							*/
 #define HLLAPI_CMD_DISCONNECTPS			   2	/**< disconnect presentation space        				*/
 #define HLLAPI_CMD_INPUTSTRING				   3	/**< send string										*/
 #define HLLAPI_CMD_WAIT					   4	/**< Wait if the session is waiting for a host response	*/
 #define HLLAPI_CMD_COPYPS					   5	/**< Copies the contents of the presentation space into a string buffer. */
 #define HLLAPI_CMD_SEARCHPS				   6	/**< Search the presentation space for a specified string. */
 #define HLLAPI_CMD_QUERYCURSOR				   7	/**< Determines the location of the cursor in the presentation space. */
 #define HLLAPI_CMD_COPYPSTOSTR				   8	/**< Copy presentation space to string					*/
 #define HLLAPI_CMD_COPYSTRTOPS				  15	/**< Copies an ASCII string directly to a specified position in the presentation space. */
 #define HLLAPI_CMD_SETCURSOR				  40	/**< Places the cursor at a specified position in presentation space.*/
 #define HLLAPI_CMD_SENDFILE				  90	/**< Send file to the host */
 #define HLLAPI_CMD_RECEIVEFILE				  91	/**< Receive a file from the host */

 #define HLLAPI_CMD_GETREVISION				2000	/**< Get lib3270 revision								*/

 typedef enum _hllapi_packet
 {
		HLLAPI_PACKET_CONNECT,
		HLLAPI_PACKET_DISCONNECT,
		HLLAPI_PACKET_GET_PROGRAM_MESSAGE,
		HLLAPI_PACKET_GET_TEXT_AT,
		HLLAPI_PACKET_SET_TEXT_AT,
		HLLAPI_PACKET_CMP_TEXT_AT,
		HLLAPI_PACKET_ENTER,
		HLLAPI_PACKET_PFKEY,
		HLLAPI_PACKET_PAKEY,
		HLLAPI_PACKET_SET_CURSOR_POSITION,
		HLLAPI_PACKET_GET_CURSOR_POSITION,
		HLLAPI_PACKET_INPUT_STRING,
		HLLAPI_PACKET_IS_CONNECTED,

		HLLAPI_PACKET_INVALID

 } HLLAPI_PACKET;



#ifdef _WIN32
	// http://www.mingw.org/wiki/Visual_Basic_DLL
	__declspec (dllexport) int __stdcall hllapi(const LPWORD func, LPSTR str, LPWORD length, LPWORD rc);

	__declspec (dllexport) DWORD __stdcall hllapi_init(LPSTR mode);
	__declspec (dllexport) DWORD __stdcall hllapi_deinit(void);

	__declspec (dllexport) DWORD __stdcall hllapi_get_revision(void);
	__declspec (dllexport) DWORD __stdcall hllapi_get_datadir(LPSTR datadir);

	__declspec (dllexport) DWORD __stdcall hllapi_connect(LPSTR uri, WORD wait);
	__declspec (dllexport) DWORD __stdcall hllapi_disconnect(void);
	__declspec (dllexport) DWORD __stdcall hllapi_get_message_id(void);
	__declspec (dllexport) DWORD __stdcall hllapi_is_connected(void);
	__declspec (dllexport) DWORD __stdcall hllapi_get_screen_at(WORD row, WORD col, LPSTR buffer);
	__declspec (dllexport) DWORD __stdcall hllapi_enter(void);
	__declspec (dllexport) DWORD __stdcall hllapi_set_text_at(WORD row, WORD col, LPSTR text);
	__declspec (dllexport) DWORD __stdcall hllapi_cmp_text_at(WORD row, WORD col, LPSTR text);
	__declspec (dllexport) DWORD __stdcall hllapi_wait_for_ready(WORD seconds);
	__declspec (dllexport) DWORD __stdcall hllapi_wait(WORD seconds);
	__declspec (dllexport) DWORD __stdcall hllapi_pfkey(WORD key);
	__declspec (dllexport) DWORD __stdcall hllapi_pakey(WORD key);

#else

	#error NOT IMPLEMENTED

#endif // _WIN32

#ifdef __cplusplus
}    /* end of extern "C" */
#endif

#endif // HLLAPI_H_INCLUDED
