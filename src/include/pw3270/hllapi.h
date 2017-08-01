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
	#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

 #define HLLAPI_MAXLENGTH					4096

 /* Function codes - Reference http://www.ibm.com/support/knowledgecenter/SSEQ5Y_6.0.0/com.ibm.pcomm.doc/books/html/emulator_programming08.htm */
 #define HLLAPI_CMD_CONNECTPS				   1	/**< connect presentation space							*/
 #define HLLAPI_CMD_DISCONNECTPS			   2	/**< disconnect presentation space        				*/
 #define HLLAPI_CMD_INPUTSTRING				   3	/**< send string										*/
 #define HLLAPI_CMD_WAIT					   4	/**< Wait if the session is waiting for a host response	*/
 #define HLLAPI_CMD_COPYPS					   5	/**< Copies the contents of the presentation space into a string buffer. */
 #define HLLAPI_CMD_SEARCHPS				   6	/**< Search the presentation space for a specified string. */
 #define HLLAPI_CMD_QUERYCURSOR				   7	/**< Determines the location of the cursor in the presentation space. */
 #define HLLAPI_CMD_COPYPSTOSTR				   8	/**< Copy presentation space to string					*/
 #define HLLAPI_SET_SESSION_PARAMETERS		   9	/**< Lets you change certain default session options in EHLLAPI for all sessions. */
 #define HLLAPI_CMD_COPYSTRTOPS				  15	/**< Copies an ASCII string directly to a specified position in the presentation space. */
 #define HLLAPI_CMD_PAUSE					  18	/**< Waits for a specified amount of time. */
 #define HLLAPI_RESET_SYSTEM				  21	/**< Reinitializes EHLLAPI to its starting state. */
 #define HLLAPI_CMD_SETCURSOR				  40	/**< Places the cursor at a specified position in presentation space.*/
 #define HLLAPI_CMD_SENDFILE				  90	/**< Send file to the host */
 #define HLLAPI_CMD_RECEIVEFILE				  91	/**< Receive a file from the host */

 #define HLLAPI_CMD_GETREVISION				2000	/**< Get lib3270 revision								*/


 /* Standard Return Codes - http://ps-2.kev009.com/tl/techlib/manuals/adoclib/3270hcon/hconugd/retrncod.htm#C20819C058kmar */

 // 00	Either the function was successfully executed, or there were no host updates since the last call was issued.
 #define HLLAPI_STATUS_SUCCESS				   0	/**< Good return code */

 // 01	Either the presentation space short session ID was invalid, or the application is not connected.
 #define HLLAPI_STATUS_DISCONNECTED			   1	/**< The presentation space was not valid or not connected. */

 // 02	A parameter error occurred.
 #define HLLAPI_STATUS_BAD_PARAMETER		   2	/**< An incorrect option was specified. */

 // 04	The execution of the function was inhibited because the target presentation space was busy.
 // 	For example, X or XSYSTEM is displayed in the OIA for the 3270 terminal emulation.
 #define HLLAPI_STATUS_TIMEOUT				   4	/**< Timeout */

 // 05	The execution of the function was inhibited for some reason other than the reasons stated in return code 4.
 #define HLLAPI_STATUS_KEYBOARD_LOCKED		   5	/**< The keyboard is locked. */

 // 06	A data error occurred due to specification of an invalid parameter (for example, a length error causing truncation).
 // 07	The specified presentation space position was invalid.
 // 08	No prerequisite function was issued.

 // 09	A system error occurred.
 #define HLLAPI_STATUS_SYSTEM_ERROR			   9 	/**< A system error occurred */

 // 10	The function number is not supported by the emulation program.
 #define HLLAPI_STATUS_UNSUPPORTED			  10

 // 11	The resource that you requested is unavailable (for example, too many attempts have been made to connect to the same presentation space (PS) or another application is connected to the PS).
 #define HLLAPI_STATUS_UNAVAILABLE			  11	/**< Resource unavailable at this time */

 // 20	Invalid keystroke caused by ESC= option.
 // 21	OIA was updated.
 // 22	PS was updated.
 // 23	Both OIA and PS were updated.
 // 24	Either the string was not found, or the presentation space is unformatted.
 #define HLLAPI_STATUS_NOT_FOUND			  24	/**< String not found or unformatted presentation space */

 // 25	Keystrokes were not available on input queue.
 // 26	A host event occurred. See QUERY HOST UPDATE (24) for details.
 // 28	Field length was 0.
 // 31	Keystroke queue overflow. Keystrokes were lost.


 #define HLLAPI_STATUS_WAITING	HLLAPI_STATUS_TIMEOUT

 #if defined(WIN32)

	#include <windows.h>

	// http://www.mingw.org/wiki/Visual_Basic_DLL
	#define HLLAPI_API_CALL __declspec (dllexport) DWORD __stdcall

 #else

	// From wtypesbase.h
	typedef uint8_t BYTE;
	typedef uint16_t WORD;
	typedef unsigned int UINT;
	typedef int INT;
	typedef uint32_t LONG;
	typedef LONG WINBOOL;
	typedef uint32_t DWORD;
	typedef void *HANDLE;
	typedef WORD *LPWORD;
	typedef DWORD *LPDWORD;
	typedef char CHAR;
	typedef CHAR *LPSTR;
	typedef const CHAR *LPCSTR;
	typedef char WCHAR;
	typedef WCHAR TCHAR;
	typedef WCHAR *LPWSTR;
	typedef TCHAR *LPTSTR;
	typedef const WCHAR *LPCWSTR;
	typedef const TCHAR *LPCTSTR;
	typedef HANDLE *LPHANDLE;

	#define LPWORD				uint16_t *

	#define LPSTR				char *
	#define HANDLE				int

	#define HLLAPI_API_CALL		__attribute__((visibility("default"))) extern DWORD

 #endif // WIN32

	HLLAPI_API_CALL hllapi(const LPWORD func, LPSTR str, LPWORD length, LPWORD rc);

	HLLAPI_API_CALL hllapi_init(LPSTR mode);
	HLLAPI_API_CALL hllapi_deinit(void);

	HLLAPI_API_CALL hllapi_reset(void);

	HLLAPI_API_CALL hllapi_get_revision(void);
	HLLAPI_API_CALL hllapi_get_datadir(LPSTR datadir);

	HLLAPI_API_CALL hllapi_connect(LPSTR uri, WORD wait);
	HLLAPI_API_CALL hllapi_disconnect(void);
	HLLAPI_API_CALL hllapi_get_message_id(void);
	HLLAPI_API_CALL hllapi_is_connected(void);
	HLLAPI_API_CALL hllapi_get_state(void);
	HLLAPI_API_CALL hllapi_get_screen_at(WORD row, WORD col, LPSTR buffer);
	HLLAPI_API_CALL hllapi_get_screen(WORD pos, LPSTR buffer, WORD len);
	HLLAPI_API_CALL hllapi_enter(void);
	HLLAPI_API_CALL hllapi_set_text_at(WORD row, WORD col, LPSTR text);
	HLLAPI_API_CALL hllapi_cmp_text_at(WORD row, WORD col, LPSTR text);
	HLLAPI_API_CALL hllapi_find_text(LPSTR text);
    HLLAPI_API_CALL hllapi_emulate_input(const LPSTR buffer, WORD len, WORD pasting);
    HLLAPI_API_CALL hllapi_input_string(LPSTR buffer, WORD len);
	HLLAPI_API_CALL hllapi_wait_for_ready(WORD seconds);
	HLLAPI_API_CALL hllapi_wait_for_change(WORD seconds);
	HLLAPI_API_CALL hllapi_wait(WORD seconds);
	HLLAPI_API_CALL hllapi_pfkey(WORD key);
	HLLAPI_API_CALL hllapi_pakey(WORD key);

    HLLAPI_API_CALL hllapi_set_session_parameter(LPSTR param, WORD len, WORD value);

	HLLAPI_API_CALL hllapi_erase(void);
	HLLAPI_API_CALL hllapi_erase_eof(void);
	HLLAPI_API_CALL hllapi_erase_eol(void);
	HLLAPI_API_CALL hllapi_erase_input(void);

	HLLAPI_API_CALL hllapi_action(LPSTR buffer);

	HLLAPI_API_CALL hllapi_print(void);

	HLLAPI_API_CALL hllapi_init(LPSTR mode);
	HLLAPI_API_CALL hllapi_deinit(void);

	HLLAPI_API_CALL hllapi_get_revision(void);
	HLLAPI_API_CALL hllapi_get_datadir(LPSTR datadir);

	HLLAPI_API_CALL hllapi_connect(LPSTR uri, WORD wait);
	HLLAPI_API_CALL hllapi_disconnect(void);
	HLLAPI_API_CALL hllapi_get_message_id(void);
	HLLAPI_API_CALL hllapi_is_connected(void);
	HLLAPI_API_CALL hllapi_get_state(void);
	HLLAPI_API_CALL hllapi_get_screen_at(WORD row, WORD col, LPSTR buffer);
	HLLAPI_API_CALL hllapi_get_screen(WORD pos, LPSTR buffer, WORD len);
	HLLAPI_API_CALL hllapi_enter(void);
	HLLAPI_API_CALL hllapi_set_text_at(WORD row, WORD col, LPSTR text);
	HLLAPI_API_CALL hllapi_cmp_text_at(WORD row, WORD col, LPSTR text);
	HLLAPI_API_CALL hllapi_wait_for_ready(WORD seconds);
	HLLAPI_API_CALL hllapi_wait(WORD seconds);
	HLLAPI_API_CALL hllapi_pfkey(WORD key);
	HLLAPI_API_CALL hllapi_pakey(WORD key);
	HLLAPI_API_CALL hllapi_setcursor(WORD key);
	HLLAPI_API_CALL hllapi_getcursor();
	HLLAPI_API_CALL hllapi_get_cursor_address();
	HLLAPI_API_CALL hllapi_setcursor(WORD addr);
	HLLAPI_API_CALL hllapi_set_cursor_address(WORD addr);
	HLLAPI_API_CALL hllapi_erase_eof(void);
	HLLAPI_API_CALL hllapi_print(void);
	HLLAPI_API_CALL hllapi_set_unlock_delay(WORD ms);

#ifdef __cplusplus
}    /* end of extern "C" */
#endif

#endif // HLLAPI_H_INCLUDED
