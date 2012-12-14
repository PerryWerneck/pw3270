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

 #define HLLAPI_REQUEST_QUERY			0x01
 #define HLLAPI_RESPONSE_VALUE			0x02
 #define HLLAPI_RESPONSE_TEXT			0x03

 #define HLLAPI_MAXLENGTH				32768

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

 #pragma pack(1)
 typedef struct _hllapi_data
 {
		unsigned char	id;			/**< Request id */
		unsigned long	func;		/**< Function number */
		unsigned short	rc;			/**< Short argument/return code */
		unsigned int	value;		/**< Requested value */
		char			string[1];	/**< String argument */
 } HLLAPI_DATA;
 #pragma pack()

#ifdef _WIN32
 // http://www.mingw.org/wiki/Visual_Basic_DLL
 __declspec (dllexport) int __stdcall hllapi(const LPWORD func, LPSTR str, LPWORD length, LPWORD rc);
#else
 LIB3270_EXPORT int hllapi(const unsigned long *func, char *str, unsigned short *length, unsigned short *rc);
#endif // _WIN32

#ifdef __cplusplus
}    /* end of extern "C" */
#endif

#endif // HLLAPI_H_INCLUDED
