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
 * Este programa está nomeado como calls.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <windows.h>
 #include <lib3270.h>
 #include <malloc.h>
 #include <string.h>
 #include <errno.h>
 #include <pw3270/hllapi.h>
 #include <stdio.h>
 #include <lib3270/log.h>
 #include "client.h"

 #undef trace
 #define trace( fmt, ... )	{ FILE *out = fopen("c:\\Users\\Perry\\hllapi.log","a"); if(out) { fprintf(out, "%s(%d) " fmt "\n", __FILE__, __LINE__, __VA_ARGS__ ); fclose(out); } }

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 HMODULE 	  hModule	= NULL;
 void		* hSession	= NULL;


 static void			* (*session_new)(const char *model)											= NULL;
 static void			  (*session_free)(void *h)													= NULL;
 static const char		* (*get_revision)(void)														= NULL;
 static int				  (*host_connect)(void *h,const char *n, int wait)							= NULL;
 static int				  (*host_is_connected)(void *h)												= NULL;
 static int 			  (*wait_for_ready)(void *h, int seconds)									= NULL;
 static void 			  (*host_disconnect)(void *h)												= NULL;
 static int 			  (*script_sleep)(void *h, int seconds)										= NULL;
 static LIB3270_MESSAGE	  (*get_message)(void *h)													= NULL;
 static char 			* (*get_text)(void *h, int row, int col, int len)							= NULL;
 static char 			* (*get_text_at_offset)(void *h, int offset, int len)						= NULL;

 static void  			* (*release_memory)(void *p)												= NULL;
 static int  			  (*action_enter)(void *h)													= NULL;
 static int 			  (*set_text_at)(void *h, int row, int col, const unsigned char *str)		= NULL;
 static int 			  (*cmp_text_at)(void *h, int row, int col, const char *text)				= NULL;
 static int				  (*pfkey)(void *hSession, int key)											= NULL;
 static int				  (*pakey)(void *hSession, int key)											= NULL;
 static int 			  (*getcursor)(void *hSession)												= NULL;
 static int 			  (*setcursor)(void *hSession, int baddr)									= NULL;
 static int 			  (*emulate_input)(void *hSession, const char *s, int len, int pasting)		= NULL;
 static int				  (*erase_eof)(void *hSession)												= NULL;
 static int 			  (*do_print)(void *h)														= NULL;

 static const struct _entry_point
 {
	void		**call;
	void		* pipe;
	const char	* name;
 } entry_point[] =
 {
	{ (void **) &session_new,			(void *) hllapi_pipe_init,				"lib3270_session_new" 			},
	{ (void **) &session_free,			(void *) hllapi_pipe_deinit,			"lib3270_session_free"			},
	{ (void **) &get_revision,			(void *) hllapi_pipe_get_revision,		"lib3270_get_revision"			},
	{ (void **) &host_connect,			(void *) hllapi_pipe_connect, 			"lib3270_connect"				},
	{ (void **) &host_disconnect,		(void *) hllapi_pipe_disconnect, 		"lib3270_disconnect"			},
	{ (void **) &host_is_connected,		(void *) hllapi_pipe_is_connected, 		"lib3270_in_tn3270e"			},
	{ (void **) &wait_for_ready,		(void *) hllapi_pipe_wait_for_ready, 	"lib3270_wait_for_ready"		},
	{ (void **) &script_sleep,			(void *) hllapi_pipe_sleep, 			"lib3270_wait"					},
	{ (void **) &get_message,			(void *) hllapi_pipe_get_message, 		"lib3270_get_program_message"	},
	{ (void **) &get_text,				(void *) hllapi_pipe_get_text_at, 		"lib3270_get_text_at"			},
	{ (void **) &release_memory,		(void *) hllapi_pipe_release_memory,	"lib3270_free"					},
	{ (void **) &action_enter,			(void *) hllapi_pipe_enter, 			"lib3270_enter"					},
	{ (void **) &set_text_at,			(void *) hllapi_pipe_set_text_at, 		"lib3270_set_string_at"			},
	{ (void **) &cmp_text_at,			(void *) hllapi_pipe_cmp_text_at, 		"lib3270_cmp_text_at"			},
	{ (void **) &pfkey,					(void *) hllapi_pipe_pfkey, 			"lib3270_pfkey"					},
	{ (void **) &pakey,					(void *) hllapi_pipe_pakey, 			"lib3270_pakey"					},
	{ (void **) &setcursor,				(void *) hllapi_pipe_setcursor,			"lib3270_set_cursor_address"	},
	{ (void **) &getcursor,				(void *) hllapi_pipe_getcursor,			"lib3270_get_cursor_address"	},
	{ (void **) &get_text_at_offset, 	(void *) hllapi_pipe_get_text,			"lib3270_get_text"				},
	{ (void **) &emulate_input,		 	(void *) hllapi_pipe_emulate_input,		"lib3270_emulate_input"			},
	{ (void **) &erase_eof,				(void *) hllapi_pipe_erase_eof,			"lib3270_eraseeof"				},
	{ (void **) &do_print,				(void *) hllapi_pipe_print,				"lib3270_print"					},

	{ NULL, NULL, NULL }
 };

// http://msdn.microsoft.com/en-us/library/windows/desktop/ms684179(v=vs.85).aspx
#ifndef LOAD_LIBRARY_SEARCH_DEFAULT_DIRS
	#define LOAD_LIBRARY_SEARCH_DEFAULT_DIRS	0x00001000
#endif // LOAD_LIBRARY_SEARCH_DEFAULT_DIRS

#ifndef LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR
	#define LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR 	0x00000100
#endif // LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR

/*--[ Implement ]------------------------------------------------------------------------------------*/

 __declspec (dllexport) DWORD __stdcall hllapi_init(LPSTR mode)
 {
 	if(!mode)
		return HLLAPI_STATUS_SYSTEM_ERROR;

	trace("%s(%s)",__FUNCTION__,(char *) mode);

	if(mode && *mode)
	{
		// Get pointers to the pipe based calls
		int f;

		trace("%s: Loading pipe based calls",__FUNCTION__);
		for(f=0;entry_point[f].name;f++)
			*entry_point[f].call = entry_point[f].pipe;

	}
	else
	{
		// Direct mode, load lib3270.dll, get pointers to the calls
		static const char *dllname = "lib3270.dll." PACKAGE_VERSION;

		int 		f;
		HMODULE		kernel;
		HANDLE		cookie		= NULL;
		DWORD		rc;
		HANDLE 		(*AddDllDirectory)(PCWSTR NewDirectory);
		BOOL 	 	(*RemoveDllDirectory)(HANDLE Cookie);
		UINT 		errorMode;
		char		datadir[4096];

		trace("hModule=%p",hModule);
		if(hModule)
			return -EBUSY;

		kernel 				= LoadLibrary("kernel32.dll");
		AddDllDirectory		= (HANDLE (*)(PCWSTR)) GetProcAddress(kernel,"AddDllDirectory");
		RemoveDllDirectory	= (BOOL (*)(HANDLE)) GetProcAddress(kernel,"RemoveDllDirectory");

		// Notify user in case of error loading protocol DLL
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ms680621(v=vs.85).aspx
		errorMode = SetErrorMode(1);

		memset(datadir,' ',4095);
		datadir[4095] = 0;

		if(hllapi_get_datadir(datadir))
		{
			char	buffer[4096];
			wchar_t	path[4096];

			mbstowcs(path, datadir, 4095);
			trace("Datadir=[%s] AddDllDirectory=%p RemoveDllDirectory=%p\n",datadir,AddDllDirectory,RemoveDllDirectory);
			if(AddDllDirectory)
				cookie = AddDllDirectory(path);

#ifdef DEBUG
			snprintf(buffer,4096,"%s\\.bin\\Debug\\%s",datadir,dllname);
#else
			snprintf(buffer,4096,"%s\\%s",datadir,dllname);
#endif // DEBUG

			hModule = LoadLibrary(buffer);

			trace("%s hModule=%p rc=%d",buffer,hModule,(int) GetLastError());

			if(hModule == NULL)
			{
				// Enable DLL error popup and try again with full path
				SetErrorMode(0);
				hModule = LoadLibraryEx(buffer,NULL,LOAD_LIBRARY_SEARCH_DEFAULT_DIRS|LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
			}

			rc = GetLastError();

			trace("%s hModule=%p rc=%d",buffer,hModule,(int) rc);
		}
		else
		{
			hModule = LoadLibrary(dllname);
			rc = GetLastError();
		}

		SetErrorMode(errorMode);

		trace("%s hModule=%p rc=%d",dllname,hModule,(int) rc);

		if(cookie && RemoveDllDirectory)
			RemoveDllDirectory(cookie);

		if(kernel)
			FreeLibrary(kernel);

		if(!hModule)
			return rc;

		// Get library entry pointers
		for(f=0;entry_point[f].name;f++)
		{
			void *ptr = (void *) GetProcAddress(hModule,entry_point[f].name);

			trace("%d %s=%p\n",f,entry_point[f].name,ptr);

			if(!ptr)
			{
				trace("Can´t load \"%s\"\n",entry_point[f].name);
				hllapi_deinit();
				return -ENOENT;
			}
			*entry_point[f].call = ptr;
		}

	}
	// Get session handle
	hSession = session_new((const char *) mode);
	trace("%s ok hSession=%p\n",__FUNCTION__,hSession);

 	return hSession ? 0 : -1;
 }

 __declspec (dllexport) DWORD __stdcall hllapi_deinit(void)
 {
 	int f;

	// Release session
 	if(hSession && session_free)
		session_free(hSession);

	for(f=0;entry_point[f].name;f++)
		*entry_point[f].call = NULL;

 	if(hModule != NULL)
 	{
		FreeLibrary(hModule);
		hModule = NULL;
 	}

 	return 0;
 }

 __declspec (dllexport) DWORD __stdcall hllapi_get_revision(void)
 {
	if(!get_revision)
		return 0;
	return (DWORD) atoi(get_revision());
 }

 __declspec (dllexport) DWORD __stdcall hllapi_connect(LPSTR uri, WORD wait)
 {
 	if(!(host_connect && hSession && uri))
		return HLLAPI_STATUS_SYSTEM_ERROR;

 	return host_connect(hSession,uri,wait);
 }

 __declspec (dllexport) DWORD __stdcall hllapi_is_connected(void)
 {
 	if(!(host_is_connected && hSession))
		return HLLAPI_STATUS_SYSTEM_ERROR;

 	return host_is_connected(hSession);
 }

 __declspec (dllexport) DWORD __stdcall hllapi_get_state(void)
 {
	switch(hllapi_get_message_id())
	{
	case LIB3270_MESSAGE_NONE:				/* 0 - No message */
		return HLLAPI_STATUS_SUCCESS;

	case LIB3270_MESSAGE_DISCONNECTED:		/* 4 - Disconnected from host */
		return HLLAPI_STATUS_DISCONNECTED;

	case LIB3270_MESSAGE_MINUS:
	case LIB3270_MESSAGE_PROTECTED:
	case LIB3270_MESSAGE_NUMERIC:
	case LIB3270_MESSAGE_OVERFLOW:
	case LIB3270_MESSAGE_INHIBIT:
	case LIB3270_MESSAGE_KYBDLOCK:
		return HLLAPI_STATUS_KEYBOARD_LOCKED;

	case LIB3270_MESSAGE_SYSWAIT:
	case LIB3270_MESSAGE_TWAIT:
	case LIB3270_MESSAGE_AWAITING_FIRST:
	case LIB3270_MESSAGE_X:
	case LIB3270_MESSAGE_RESOLVING:
	case LIB3270_MESSAGE_CONNECTING:
		return HLLAPI_STATUS_WAITING;
	}

	return HLLAPI_STATUS_SYSTEM_ERROR;
 }

 __declspec (dllexport) DWORD __stdcall hllapi_disconnect(void)
 {
 	if(!(host_disconnect && hSession))
		return HLLAPI_STATUS_SYSTEM_ERROR;

	host_disconnect(hSession);

	return 0;
 }

 __declspec (dllexport) DWORD __stdcall hllapi_wait_for_ready(WORD seconds)
 {
 	if(!(wait_for_ready && hSession))
		return HLLAPI_STATUS_SYSTEM_ERROR;

	trace("%s seconds=%d\n", __FUNCTION__, (int) seconds);

	return (DWORD) wait_for_ready(hSession,(int) seconds);
 }

 __declspec (dllexport) DWORD __stdcall hllapi_wait(WORD seconds)
 {
 	if(!(script_sleep && hSession))
		return HLLAPI_STATUS_SYSTEM_ERROR;

	return (DWORD) script_sleep(hSession,(int) seconds);
 }

 __declspec (dllexport) DWORD __stdcall hllapi_get_message_id(void)
 {
	if(!(get_message && hSession))
		return HLLAPI_STATUS_SYSTEM_ERROR;
	return (DWORD) get_message(hSession);
 }

 __declspec (dllexport) DWORD __stdcall hllapi_get_screen_at(WORD row, WORD col, LPSTR buffer)
 {
	char	* text;
	int		  len;

	if(!(get_text && release_memory && hSession))
		return HLLAPI_STATUS_SYSTEM_ERROR;

	trace("%s row=%d col=%d buffer=%p",__FUNCTION__,row,col,buffer);
	len = strlen(buffer);

	trace(" len=%d",len);

	text = get_text(hSession,row,col,len);

	trace(" text=%p errno=%d %s\n",text,errno,strerror(errno));

	if(!text)
	{
		int rc = hllapi_get_state();
		return rc == HLLAPI_STATUS_SUCCESS ? -1 : rc;
	}

	strncpy(buffer,text,len);
	release_memory(text);

	trace("text:\n%s\n",buffer);

 	return HLLAPI_STATUS_SUCCESS;
 }

 __declspec (dllexport) DWORD __stdcall hllapi_enter(void)
 {
	if(!(action_enter && hSession))
		return HLLAPI_STATUS_SYSTEM_ERROR;

	return (DWORD) action_enter(hSession);
 }

 __declspec (dllexport) DWORD __stdcall hllapi_set_text_at(WORD row, WORD col, LPSTR text)
 {
	if(!(set_text_at && hSession))
		return HLLAPI_STATUS_SYSTEM_ERROR;

	return (DWORD) set_text_at(hSession,row,col,(const unsigned char *) text);
 }

 __declspec (dllexport) DWORD __stdcall hllapi_cmp_text_at(WORD row, WORD col, LPSTR text)
 {
	if(!(cmp_text_at && hSession))
		return HLLAPI_STATUS_SYSTEM_ERROR;

	return (DWORD) cmp_text_at(hSession,row,col,(const char *) text);
 }

 __declspec (dllexport) DWORD __stdcall hllapi_pfkey(WORD key)
 {
	if(!(pfkey && hSession))
		return HLLAPI_STATUS_SYSTEM_ERROR;

	return (DWORD) pfkey(hSession,key);
 }

 __declspec (dllexport) DWORD __stdcall hllapi_pakey(WORD key)
 {
	if(!(pfkey && hSession))
		return HLLAPI_STATUS_SYSTEM_ERROR;

	return (DWORD) pakey(hSession,key);
 }

 __declspec (dllexport) DWORD __stdcall hllapi_get_datadir(LPSTR datadir)
 {
	HKEY 			hKey	= 0;
 	unsigned long	datalen = strlen(datadir);

	*datadir = 0;

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Software\\pw3270",0,KEY_QUERY_VALUE,&hKey) == ERROR_SUCCESS)
	{
		unsigned long datatype;					// #defined in winnt.h (predefined types 0-11)
		if(RegQueryValueExA(hKey,"datadir",NULL,&datatype,(LPBYTE) datadir,&datalen) != ERROR_SUCCESS)
			*datadir = 0;
		RegCloseKey(hKey);
	}

	return *datadir;
 }

 __declspec (dllexport) DWORD __stdcall hllapi_setcursor(WORD pos)
 {
	if(!(setcursor && hSession))
		return HLLAPI_STATUS_SYSTEM_ERROR;
	trace("%s(%d)",__FUNCTION__,pos);
	return setcursor(hSession,pos-1);
 }

 __declspec (dllexport) DWORD __stdcall hllapi_getcursor()
 {
	if(!(getcursor && hSession))
		return HLLAPI_STATUS_SYSTEM_ERROR;
	return getcursor(hSession)+1;
 }

 __declspec (dllexport) DWORD __stdcall hllapi_get_screen(WORD offset, LPSTR buffer, WORD len)
 {
	size_t	  szBuffer = strlen(buffer);
	char	* text;

	if(len < szBuffer && len > 0)
		szBuffer = len;

	text = hllapi_get_string(offset, szBuffer);
	if(!text)
	{
		int rc = hllapi_get_state();
		return rc == HLLAPI_STATUS_SUCCESS ? -1 : rc;
	}

	memcpy(buffer,text,len);

	hllapi_free(text);

	return HLLAPI_STATUS_SUCCESS;
 }

 __declspec (dllexport) DWORD __stdcall hllapi_emulate_input(LPSTR buffer, WORD len, WORD pasting)
 {

	if(!(emulate_input && hSession))
		return HLLAPI_STATUS_DISCONNECTED;
	trace("%s(%s)",__FUNCTION__,(char *) buffer);

	if(buffer && *buffer)
		emulate_input(hSession, buffer, len, pasting);

	return HLLAPI_STATUS_SUCCESS;
 }

 __declspec (dllexport) DWORD __stdcall hllapi_erase_eof(void)
 {
	if(!erase_eof && hSession)
		return HLLAPI_STATUS_SYSTEM_ERROR;
	trace("%s",__FUNCTION__);
	return erase_eof(hSession);
 }

 __declspec (dllexport) DWORD __stdcall hllapi_print(void)
 {
	if(!(do_print && hSession))
		return HLLAPI_STATUS_SYSTEM_ERROR;

	return do_print(hSession);
 }

 char * hllapi_get_string(int offset, size_t len)
 {
 	if(!(get_text_at_offset && hSession))
		return NULL;

	return get_text_at_offset(hSession,offset-1,len);
 }

 void hllapi_free(void *p)
 {
	if(release_memory)
		release_memory(p);
 }
