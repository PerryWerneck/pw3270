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

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 HMODULE 	  hModule	= NULL;
 H3270		* hSession	= NULL;

 static H3270			* (*session_new)(const char *model)											= NULL;
 static void			  (*session_free)(H3270 *h)													= NULL;
 static const char		* (*get_revision)(void)														= NULL;
 static int				  (*host_connect)(H3270 *h,const char *n, int wait)							= NULL;
 static int 			  (*wait_for_ready)(H3270 *h, int seconds)									= NULL;
 static void 			  (*host_disconnect)(H3270 *h)												= NULL;
 static int 			  (*script_sleep)(H3270 *h, int seconds)									= NULL;
 static LIB3270_MESSAGE	  (*get_message)(H3270 *h)													= NULL;
 static char 			* (*get_text)(H3270 *h, int row, int col, int len)							= NULL;
 static void  			* (*release_memory)(void *p)												= NULL;
 static int  			  (*action_enter)(H3270 *h)													= NULL;
 static int 			  (*set_text_at)(H3270 *h, int row, int col, const unsigned char *str)		= NULL;
 static int 			  (*cmp_text_at)(H3270 *h, int row, int col, const char *text)				= NULL;
 static int				  (*pfkey)(H3270 *hSession, int key)										= NULL;
 static int				  (*pakey)(H3270 *hSession, int key)										= NULL;

 static const struct _entry_point
 {
	void		**call;
	const char	* name;
 } entry_point[] =
 {
	{ (void **) &session_new,		"lib3270_session_new" 			},
	{ (void **) &session_free,		"lib3270_session_free"			},
	{ (void **) &get_revision,		"lib3270_get_revision"			},
	{ (void **) &host_connect,		"lib3270_connect"				},
	{ (void **) &host_disconnect,	"lib3270_disconnect"			},
	{ (void **) &wait_for_ready,	"lib3270_wait_for_ready"		},
	{ (void **) &script_sleep,		"lib3270_wait"					},
	{ (void **) &get_message,		"lib3270_get_program_message"	},
	{ (void **) &get_text,			"lib3270_get_text_at"			},
	{ (void **) &release_memory,	"lib3270_free"					},
	{ (void **) &action_enter,		"lib3270_enter"					},
	{ (void **) &set_text_at,		"lib3270_set_string_at"			},
	{ (void **) &cmp_text_at,		"lib3270_cmp_text_at"			},
	{ (void **) &pfkey,				"lib3270_pfkey"					},
	{ (void **) &pakey,				"lib3270_pakey"					},

	{ NULL, NULL }
 };


/*--[ Implement ]------------------------------------------------------------------------------------*/

 __declspec (dllexport) int __stdcall hllapi_init(LPSTR mode)
 {
 	if(!mode)
		return EINVAL;

	if(hModule)
		return EBUSY;

	if(!*mode)
	{
		// Direct mode, load lib3270.dll, get pointers to the calls
		int f;

		hModule = LoadLibrary("lib3270.dll");
		if(hModule == NULL)
			return GetLastError();

		// Get library entry pointers
		for(f=0;entry_point[f].name;f++)
		{
			void *ptr = (void *) GetProcAddress(hModule,entry_point[f].name);
			if(!ptr)
			{
				fprintf(stderr,"Can´t load \"%s\"\n",entry_point[f].name);
				hllapi_deinit();
				return ENOENT;
			}
			*entry_point[f].call = ptr;
		}

		// Get session handle
		hSession = session_new("");

		return 0;
	}

	// Set entry points to pipe based calls


 	return -1;
 }

 __declspec (dllexport) int __stdcall hllapi_deinit(void)
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

 __declspec (dllexport) int __stdcall hllapi_get_revision(LPDWORD rc)
 {
	if(!get_revision)
		return EINVAL;

	*rc = (DWORD) atoi(get_revision());

 	return 0;
 }

 __declspec (dllexport) int __stdcall hllapi_connect(LPSTR uri)
 {
 	if(!(host_connect && hSession && uri))
		return EINVAL;

 	return host_connect(hSession,uri,0);
 }

 __declspec (dllexport) int __stdcall hllapi_disconnect(LPWORD rc)
 {
 	if(!(host_disconnect && hSession))
		return EINVAL;

	host_disconnect(hSession);

 	return 0;
 }

 __declspec (dllexport) int __stdcall hllapi_wait_for_ready(LPWORD rc, WORD seconds)
 {
 	if(!(wait_for_ready && hSession))
		return EINVAL;

	*rc = (WORD) wait_for_ready(hSession,(int) seconds);

 	return 0;
 }

 __declspec (dllexport) int __stdcall hllapi_wait(LPWORD rc, WORD seconds)
 {
 	if(!(script_sleep && hSession))
		return EINVAL;

	*rc = (WORD) script_sleep(hSession,(int) seconds);

 	return 0;

 }

 __declspec (dllexport) int __stdcall hllapi_get_message_id(LPWORD rc)
 {
	if(!(get_message && hSession))
		return EINVAL;
	*rc = get_message(hSession);
 	return 0;
 }

 __declspec (dllexport) int __stdcall hllapi_get_screen_at(WORD row, WORD col, LPSTR buffer)
 {
	char	* text;
	int		  len = strlen(buffer);

	if(!(get_text && release_memory && hSession))
		return EINVAL;

	text = get_text(hSession,row,col,len);

	if(!text)
		return EINVAL;

	strncpy(buffer,text,len);
	release_memory(text);

 	return 0;
 }

 __declspec (dllexport) int __stdcall hllapi_enter(LPWORD rc)
 {
	if(!(action_enter && hSession))
		return EINVAL;

	*rc = (WORD) action_enter(hSession);

 	return 0;
 }

 __declspec (dllexport) int __stdcall hllapi_set_text_at(LPWORD rc, WORD row, WORD col, LPSTR text)
 {
	if(!(set_text_at && hSession))
		return EINVAL;

	*rc = (WORD) set_text_at(hSession,row,col,(const unsigned char *) text);

 	return 0;
 }

 __declspec (dllexport) int __stdcall hllapi_cmp_text_at(LPWORD rc, WORD row, WORD col, LPSTR text)
 {
	if(!(cmp_text_at && hSession))
		return EINVAL;

	*rc = (WORD) cmp_text_at(hSession,row,col,(const char *) text);

 	return 0;
 }

 __declspec (dllexport) int __stdcall hllapi_pfkey(LPWORD rc, WORD key)
 {
	if(!(pfkey && hSession))
		return EINVAL;

	*rc = (WORD) pfkey(hSession,key);

 	return 0;
 }

 __declspec (dllexport) int __stdcall hllapi_pakey(LPWORD rc, WORD key)
 {
	if(!(pfkey && hSession))
		return EINVAL;

	*rc = (WORD) pakey(hSession,key);

 	return 0;
 }
