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
 #include <time.h>
 #include <lib3270/actions.h>

#ifdef HAVE_SYSLOG
 #include <syslog.h>
#endif // HAVE_SYSLOG

 #include <string.h>

#if defined WIN32
	BOOL WINAPI DllMain(HANDLE hinst, DWORD dwcallpurpose, LPVOID lpvResvd);
	static int librx3270_loaded(void);
	static int librx3270_unloaded(void);
#else

	int librx3270_loaded(void) __attribute__((constructor));
	int librx3270_unloaded(void) __attribute__((destructor));
#endif

/*--[ Globals ]--------------------------------------------------------------------------------------*/

//	LIB3270_EXPORT RexxRoutineEntry rx3270_functions[];
//	LIB3270_EXPORT RexxPackageEntry rx3270_package_entry;

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
	return 0;
}

int librx3270_unloaded(void)
{
	return 0;
}

// now build the actual entry list
RexxRoutineEntry rx3270_functions[] =
{
	REXX_TYPED_ROUTINE(rx3270version, 				rx3270version),
	REXX_TYPED_ROUTINE(rx3270QueryCState,			rx3270QueryCState),
	REXX_TYPED_ROUTINE(rx3270Disconnect,			rx3270Disconnect),
	REXX_TYPED_ROUTINE(rx3270Connect,				rx3270Connect),
	REXX_TYPED_ROUTINE(rx3270isConnected,			rx3270isConnected),
	REXX_TYPED_ROUTINE(rx3270WaitForEvents,			rx3270WaitForEvents),
	REXX_TYPED_ROUTINE(rx3270Sleep,					rx3270Sleep),
	REXX_TYPED_ROUTINE(rx3270SendENTERKey,			rx3270SendENTERKey),
	REXX_TYPED_ROUTINE(rx3270SendPFKey,				rx3270SendPFKey),
	REXX_TYPED_ROUTINE(rx3270SendPAKey,				rx3270SendPAKey),
	REXX_TYPED_ROUTINE(rx3270WaitForTerminalReady,	rx3270WaitForTerminalReady),
	REXX_TYPED_ROUTINE(rx3270WaitForStringAt,		rx3270WaitForStringAt),
	REXX_TYPED_ROUTINE(rx3270GetStringAt,			rx3270GetStringAt),
	REXX_TYPED_ROUTINE(rx3270IsTerminalReady,		rx3270IsTerminalReady),
	REXX_TYPED_ROUTINE(rx3270queryStringAt,			rx3270queryStringAt),
	REXX_TYPED_ROUTINE(rx3270SetStringAt,			rx3270SetStringAt),

	// rx3270Popup

	REXX_LAST_METHOD()
};

RexxMethodEntry rx3270_methods[] =
{
	REXX_METHOD(rx3270_method_version,			rx3270_method_version			),
	REXX_METHOD(rx3270_method_revision,			rx3270_method_revision			),
    REXX_METHOD(rx3270_method_init,				rx3270_method_init				),
    REXX_METHOD(rx3270_method_uninit,			rx3270_method_uninit			),
    REXX_METHOD(rx3270_method_connect,			rx3270_method_connect			),
    REXX_METHOD(rx3270_method_disconnect,		rx3270_method_disconnect		),
    REXX_METHOD(rx3270_method_sleep,			rx3270_method_sleep				),
    REXX_METHOD(rx3270_method_is_connected, 	rx3270_method_is_connected		),
    REXX_METHOD(rx3270_method_is_ready, 		rx3270_method_is_ready			),
    REXX_METHOD(rx3270_method_wait_for_ready, 	rx3270_method_wait_for_ready	),
    REXX_METHOD(rx3270_method_set_cursor, 		rx3270_method_set_cursor		),
    REXX_METHOD(rx3270_method_set_cursor, 		rx3270_method_get_cursor_addr	),
    REXX_METHOD(rx3270_method_set_cursor, 		rx3270_method_set_cursor_addr	),
    REXX_METHOD(rx3270_method_enter, 			rx3270_method_enter				),
    REXX_METHOD(rx3270_method_pfkey, 			rx3270_method_pfkey				),
    REXX_METHOD(rx3270_method_pakey, 			rx3270_method_pakey				),
    REXX_METHOD(rx3270_method_get_text,			rx3270_method_get_text			),
    REXX_METHOD(rx3270_method_get_text_at, 		rx3270_method_get_text_at		),
    REXX_METHOD(rx3270_method_set_text_at, 		rx3270_method_set_text_at		),
    REXX_METHOD(rx3270_method_cmp_text_at, 		rx3270_method_cmp_text_at		),
    REXX_METHOD(rx3270_method_event_trace, 		rx3270_method_event_trace		),
    REXX_METHOD(rx3270_method_screen_trace, 	rx3270_method_screen_trace		),
    REXX_METHOD(rx3270_method_ds_trace, 		rx3270_method_ds_trace			),
    REXX_METHOD(rx3270_method_set_option,		rx3270_method_set_option		),
    REXX_METHOD(rx3270_method_test,				rx3270_method_test				),
    REXX_METHOD(rx3270_method_wait_for_text_at,	rx3270_method_wait_for_text_at	),

    REXX_METHOD(rx3270_method_get_field_len,	rx3270_method_get_field_len	    ),
    REXX_METHOD(rx3270_method_get_field_start,	rx3270_method_get_field_start	),
    REXX_METHOD(rx3270_method_get_next_unprotected, rx3270_method_get_next_unprotected  ),
    REXX_METHOD(rx3270_method_get_selection,	rx3270_method_get_selection	    ),
    REXX_METHOD(rx3270_method_set_selection,	rx3270_method_set_selection 	),
    REXX_METHOD(rx3270_method_get_clipboard,	rx3270_method_get_clipboard	    ),

    REXX_METHOD(rx3270_method_get_cursor_addr,	rx3270_method_get_cursor_addr   ),
    REXX_METHOD(rx3270_method_set_cursor_addr,	rx3270_method_set_cursor_addr   ),
    REXX_METHOD(rx3270_method_input_text,   	rx3270_method_input_text	    ),

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
    rx3270_methods							// no methods in rx3270.
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


