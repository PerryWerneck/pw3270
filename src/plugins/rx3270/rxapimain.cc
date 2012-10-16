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
 * Este programa está nomeado como rxapimain.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 /*
  *
  * Reference:
  *
  * http://www.oorexx.org/docs/rexxpg/x2950.htm
  *
  */

 #include "rx3270.h"

#if defined WIN32
	BOOL WINAPI DllMain(HANDLE hinst, DWORD dwcallpurpose, LPVOID lpvResvd);
	static int librx3270_loaded(void);
	static int librx3270_unloaded(void);
#else
	int librx3270_loaded(void) __attribute__((constructor));
	int librx3270_unloaded(void) __attribute__((destructor));
#endif
/*--[ Implement ]------------------------------------------------------------------------------------*/

#if defined WIN32
BOOL WINAPI DllMain(HANDLE hinst, DWORD dwcallpurpose, LPVOID lpvResvd)
{
//	Trace("%s - Library %s",__FUNCTION__,(dwcallpurpose == DLL_PROCESS_ATTACH) ? "Loaded" : "Unloaded");

    switch(dwcallpurpose)
    {
	case DLL_PROCESS_ATTACH:
		librx3270_loaded();
		break;

	case DLL_PROCESS_DETACH:
		librx3270_unloaded();
		break;
    }
    return TRUE;
}
#endif

int librx3270_loaded(void)
{
	trace("%s %04x %s %s",__FUNCTION__,REXX_CURRENT_INTERPRETER_VERSION,__DATE__,__TIME__);
	return 0;
}

int librx3270_unloaded(void)
{
	trace("%s",__FUNCTION__);
	return 0;
}

RexxRoutine0(CSTRING, rx3270version)
{
	return lib3270_get_version();
}

RexxRoutine0(CSTRING, rx3270QueryCState)
{
	return "";
}

RexxRoutine0(int, rx3270Disconnect)
{
	return 0;
}

RexxRoutine2(int, rx3270Connect, CSTRING, hostname, int, timeout)
{
	return 0;
}


// now build the actual entry list
RexxRoutineEntry rx3270_functions[] =
{
	REXX_TYPED_ROUTINE(rx3270version, 		rx3270version),
	REXX_TYPED_ROUTINE(rx3270QueryCState,	rx3270QueryCState),
	REXX_TYPED_ROUTINE(rx3270Disconnect,	rx3270Disconnect),
	REXX_TYPED_ROUTINE(rx3270Connect,		rx3270Connect),
	REXX_LAST_METHOD()
};

RexxPackageEntry rx3270_package_entry =
{
    STANDARD_PACKAGE_HEADER
    REXX_CURRENT_INTERPRETER_VERSION,		// anything after 4.0.0 will work
    "pw3270",								// name of the package
    PACKAGE_VERSION,						// package information
    NULL,									// no load/unload functions
    NULL,
    rx3270_functions,						// the exported functions
    NULL									// no methods in rx3270.
};

// package loading stub.
/*
OOREXX_GET_PACKAGE(rx3270);
*/

BEGIN_EXTERN_C()
LIB3270_EXPORT RexxPackageEntry * RexxEntry RexxGetPackage(void)
{
	return &rx3270_package_entry;
}
END_EXTERN_C()



