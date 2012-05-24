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
 * Este programa está nomeado como XtGlue.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */

#error Deprecated

/* glue for missing Xt code */
#if defined(_WIN32)
	#include <winsock2.h>
	#include <windows.h>
#endif // WIN32

#include "globals.h"
#include "api.h"
// #include <malloc.h>

#if defined(_WIN32)
//	#include "appres.h"
	#include "trace_dsc.h"
	#include "xioc.h"
#endif

#include "utilc.h"

#include <stdio.h>
// #include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "X11keysym.h"

#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>

#include <lib3270.h>

#if defined(_WIN32) /*[*/

	#include <winsock2.h>
	#include <windows.h>
	#include <ws2tcpip.h>

#else /*][*/

	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>

	#if defined(SEPARATE_SELECT_H) /*[*/
		#include <sys/select.h>
	#endif /*]*/
#endif /*]*/

#include "resolverc.h"

/*---[ Implement external calls ]---------------------------------------------------------------------------*/

/*
const char * KeysymToString(KeySym k)
{
	int i;

	for (i = 0; latin1[i].name != (char *)NULL; i++) {
		if (latin1[i].keysym == k)
			return latin1[i].name;
	}
	return (char *)NULL;
}
*/
