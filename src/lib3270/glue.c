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
 * Este programa está nomeado como glue.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */


/*
 *	glue.c
 *		A displayless 3270 Terminal Emulator
 *		Glue for missing parts.
 */



#include "globals.h"

#if !defined(_WIN32) /*[*/
	#include <sys/wait.h>
#else
	#include <windows.h>
#endif /*]*/

#include <signal.h>
#include <errno.h>
#include <stdarg.h>

#include "3270ds.h"
#include "resources.h"

//#include "actionsc.h"
#include "ansic.h"
// #include "charsetc.h"
#include "ctlrc.h"
// #include "gluec.h"
#include "hostc.h"
// #include "keymapc.h"
#include "kybdc.h"
//#include "macrosc.h"
#include "popupsc.h"
#include "screenc.h"
// #include "selectc.h"
#include "tablesc.h"
#include "telnetc.h"
#include "togglesc.h"
#include "trace_dsc.h"
#include "utilc.h"
// #include "idlec.h"
// #include "printerc.h"

#if defined(X3270_FT)
	#include "ftc.h"
#endif

#if defined(_WIN32) /*[*/
#include "winversc.h"
#endif /*]*/

// #include "session.h"

#if defined WIN32
	BOOL WINAPI DllMain(HANDLE hinst, DWORD dwcallpurpose, LPVOID lpvResvd);
#else
	int lib3270_loaded(void) __attribute__((constructor));
	int lib3270_unloaded(void) __attribute__((destructor));
#endif

 #define LAST_ARG	"--"

/*---[ Globals ]--------------------------------------------------------------------------------------------------------------*/

#if defined WIN32

BOOL WINAPI DllMain(HANDLE hinst, DWORD dwcallpurpose, LPVOID lpvResvd)
{
//	Trace("%s - Library %s",__FUNCTION__,(dwcallpurpose == DLL_PROCESS_ATTACH) ? "Loaded" : "Unloaded");

    if(dwcallpurpose == DLL_PROCESS_ATTACH)
		get_version_info();

    return TRUE;
}

#else

int lib3270_loaded(void)
{
    return 0;
}

int lib3270_unloaded(void)
{
    return 0;
}

#endif


#ifdef DEBUG
extern void lib3270_initialize(void)
{
}
#endif


