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
 * Este programa está nomeado como local.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "rx3270.h"
 #include <string.h>

#if !defined WIN32
	#include <dlfcn.h>
	#include <stdio.h>
#endif

#ifdef HAVE_SYSLOG
	#include <syslog.h>
	#include <stdarg.h>
#endif // HAVE_SYSLOG

// http://msdn.microsoft.com/en-us/library/windows/desktop/ms684179(v=vs.85).aspx
#ifndef LOAD_LIBRARY_SEARCH_DEFAULT_DIRS
	#define LOAD_LIBRARY_SEARCH_DEFAULT_DIRS	0x00001000
#endif // LOAD_LIBRARY_SEARCH_DEFAULT_DIRS

#ifndef LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR
	#define LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR 	0x00000100
#endif // LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR

/*--[ Class definition ]-----------------------------------------------------------------------------*/

 class dynamic : public rx3270
 {
 public:
	dynamic();
	~dynamic();

	char			* get_version(void);
	LIB3270_CSTATE	  get_cstate(void);
	int				  disconnect(void);
	int				  connect(const char *uri, bool wait = true);
	bool			  is_connected(void);
	bool			  is_ready(void);

	int				  iterate(bool wait);
	int				  wait(int seconds);
	int				  wait_for_ready(int seconds);

	char			* get_text(int baddr, size_t len);
	char 			* get_text_at(int row, int col, size_t sz);
	int				  cmp_text_at(int row, int col, const char *text);
	int 			  set_text_at(int row, int col, const char *str);
    int               emulate_input(const char *str);

	int				  set_cursor_position(int row, int col);
	int               set_cursor_addr(int addr);
	int               get_cursor_addr(void);

	int 			  set_toggle(LIB3270_TOGGLE ix, bool value);

	int				  enter(void);
	int				  pfkey(int key);
	int				  pakey(int key);

	int               get_field_start(int baddr = -1);
	int               get_field_len(int baddr = -1);

 private:

	const char * 	(*_get_version)(void);
	LIB3270_CSTATE	(*_get_connection_state)(H3270 *h);
	int 			(*_disconnect)(H3270 *h);
	int 			(*_connect)(H3270 *h,const char *n, int wait);
	int 			(*_is_connected)(H3270 *h);
	void 			(*_main_iterate)(H3270 *h, int wait);
	int 			(*_wait)(H3270 *hSession, int seconds);
	int 			(*_enter)(H3270 *hSession);
	int 			(*_pfkey)(H3270 *hSession, int key);
	int 			(*_pakey)(H3270 *hSession, int key);
	int 			(*_wait_for_ready)(H3270 *hSession, int seconds);
	char * 			(*_get_text)(H3270 *h, int offset, int len);
	char *  		(*_get_text_at)(H3270 *h, int row, int col, int len);
	int 			(*_cmp_text_at)(H3270 *h, int row, int col, const char *text);
	int 			(*_set_text_at)(H3270 *h, int row, int col, const unsigned char *str);
	int 			(*_is_ready)(H3270 *h);
	int 			(*_set_cursor_position)(H3270 *h, int row, int col);
	int 			(*_set_toggle)(H3270 *h, LIB3270_TOGGLE ix, int value);
	int             (*_get_field_start)(H3270 *h, int baddr);
	int             (*_get_field_len)(H3270 *h, int baddr);
	int             (*_set_cursor_addr)(H3270 *h, int addr);
	int             (*_get_cursor_addr)(H3270 *h);
    int             (*_emulate_input)(H3270 *session, const char *s, int len, int pasting);

#ifdef WIN32
	HMODULE			  hModule;
#else
	void			* hModule;
#endif // WIN32

	H3270 			* hSession;

 };

/*--[ Globals ]--------------------------------------------------------------------------------------*/

/*--[ Implement ]------------------------------------------------------------------------------------*/

rx3270 * rx3270::create_local(void)
{
    trace("%s",__FUNCTION__);
	return new dynamic();
}

#ifdef WIN32
static int get_datadir(LPSTR datadir)
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
#endif // WIN32

extern "C"
{

static void loghandler(H3270 *session, const char *module, int rc, const char *fmt, va_list args)
{
#ifdef HAVE_SYSLOG
	openlog(PACKAGE_NAME, LOG_NDELAY, LOG_USER);
	vsyslog(LOG_INFO,fmt,args);
	closelog();
#endif // HAVE_SYSLOG
}

static void tracehandler(H3270 *session, const char *fmt, va_list args)
{
#ifdef HAVE_SYSLOG

	#define MAX_LOG_LENGTH 200

	static char	  line[MAX_LOG_LENGTH+1];
	char 		  temp[MAX_LOG_LENGTH];
	char		* ptr;
	size_t		  len = strlen(line);

	vsnprintf(temp,MAX_LOG_LENGTH-len,fmt,args);

	ptr = strchr(temp,'\n');
	if(!ptr)
	{
		strncat(line,temp,MAX_LOG_LENGTH);
		if(strlen(line) >= MAX_LOG_LENGTH)
		{
			openlog(PACKAGE_NAME, LOG_NDELAY, LOG_USER);
			syslog(LOG_INFO,line);
			closelog();
			*line = 0;
		}
		return;
	}

	*ptr = 0;
	strncat(line,temp,MAX_LOG_LENGTH);

	openlog(PACKAGE_NAME, LOG_NDELAY, LOG_USER);
	syslog(LOG_DEBUG,line);
	closelog();

	strncpy(line,ptr+1,MAX_LOG_LENGTH);

#endif // HAVE_SYSLOG
}

}

dynamic::dynamic()
{
 	H3270 * (*lib3270_new)(const char *);
 	void	(*set_log_handler)(void (*loghandler)(H3270 *, const char *, int, const char *, va_list));
	void 	(*set_trace_handler)( void (*handler)(H3270 *session, const char *fmt, va_list args) );

	struct _call
	{
		void 		**entry;
		const char 	* name;
	} call[] =
	{
		{ (void **) & lib3270_new,				"lib3270_session_new"				},
		{ (void **) & set_log_handler,			"lib3270_set_log_handler"			},
		{ (void **) & set_trace_handler,		"lib3270_set_trace_handler"			},
		{ (void **) & _get_version,				"lib3270_get_version"				},
		{ (void **) & _get_connection_state,	"lib3270_get_connection_state"		},
		{ (void **) & _disconnect,				"lib3270_disconnect"				},
		{ (void **) & _connect,					"lib3270_connect"					},
		{ (void **) & _is_connected,			"lib3270_in_tn3270e"				},
		{ (void **) & _main_iterate,			"lib3270_main_iterate"				},
		{ (void **) & _wait,					"lib3270_wait"						},
		{ (void **) & _enter,					"lib3270_enter"						},
		{ (void **) & _pfkey,					"lib3270_pfkey"						},
		{ (void **) & _pakey,					"lib3270_pakey"						},
		{ (void **) & _wait_for_ready,			"lib3270_wait_for_ready"			},
		{ (void **) & _get_text,				"lib3270_get_text"					},
		{ (void **) & _get_text_at,				"lib3270_get_text_at"				},
		{ (void **) & _cmp_text_at,				"lib3270_cmp_text_at"				},
		{ (void **) & _set_text_at,				"lib3270_set_string_at"				},
		{ (void **) & _is_ready,				"lib3270_is_ready"					},
		{ (void **) & _set_cursor_position,		"lib3270_set_cursor_position"		},
		{ (void **) & _set_toggle,				"lib3270_set_toggle"				},
		{ (void **) & _get_field_start,			"lib3270_get_field_start"			},
		{ (void **) & _get_field_len,			"lib3270_get_field_len"				},
		{ (void **) & _set_cursor_addr,			"lib3270_set_cursor_address"		},
		{ (void **) & _get_cursor_addr,			"lib3270_get_cursor_address"		},
		{ (void **) & _emulate_input,			"lib3270_emulate_input"	        	},

	};

// Load lib3270.dll
#ifdef WIN32
		static const char *dllname = "lib3270.dll." PACKAGE_VERSION;

		HMODULE		kernel;
		HANDLE		cookie		= NULL;
		DWORD		rc;
		HANDLE 		WINAPI (*AddDllDirectory)(PCWSTR NewDirectory);
		BOOL 	 	WINAPI (*RemoveDllDirectory)(HANDLE Cookie);
		UINT 		errorMode;
		char		datadir[4096];
		char		buffer[4096];

		kernel 				= LoadLibrary("kernel32.dll");
		AddDllDirectory		= (HANDLE WINAPI (*)(PCWSTR)) GetProcAddress(kernel,"AddDllDirectory");
		RemoveDllDirectory	= (BOOL WINAPI (*)(HANDLE)) GetProcAddress(kernel,"RemoveDllDirectory");

		// Notify user in case of error loading protocol DLL
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ms680621(v=vs.85).aspx
		errorMode = SetErrorMode(1);

		memset(datadir,' ',4095);
		datadir[4095] = 0;

		if(get_datadir(datadir))
		{
			trace("Datadir=[%s] AddDllDirectory=%p RemoveDllDirectory=%p\n",datadir,AddDllDirectory,RemoveDllDirectory);

			if(AddDllDirectory)
			{
				wchar_t	*path = (wchar_t *) malloc(4096*sizeof(wchar_t));
				mbstowcs(path, datadir, 4095);
				cookie = AddDllDirectory(path);
				free(path);
			}

#ifdef DEBUG
			snprintf(buffer,4096,"%s\\.bin\\Debug\\%s",datadir,dllname);
#else
			snprintf(buffer,4096,"%s\\%s",datadir,dllname);
#endif // DEBUG

			trace("Loading [%s] [%s]",buffer,datadir);
			hModule = LoadLibrary(buffer);

			trace("Module=%p rc=%d",hModule,(int) GetLastError());

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
			return;


#else
		dlerror();

		hModule = dlopen("lib3270.so." PACKAGE_VERSION, RTLD_NOW);
		if(!hModule)
		{
			fprintf(stderr,"Can't load lib3270\n%s\n",dlerror());
			fflush(stderr);
			return;
		}

#endif // WIN32

	// Load entry points
	for(unsigned int f = 0; f < (sizeof (call) / sizeof ((call)[0]));f++)
	{
#ifdef WIN32
		*call[f].entry = (void *) GetProcAddress(hModule,call[f].name);
#else
		*call[f].entry = dlsym(hModule,call[f].name);
#endif // WIN32

		if(!*call[f].entry)
		{
#ifndef WIN32
			fprintf(stderr,"Can't load lib3270::%s\n%s\n",call[f].name,dlerror());
			fflush(stderr);
			dlclose(hModule);
#else
			FreeLibrary(hModule);
#endif // !WIN32
			hModule = NULL;
			return;
		}
	}

	// Get Session handle, setup base callbacks
	set_log_handler(loghandler);
	set_trace_handler(tracehandler);
	this->hSession = lib3270_new("");

}

dynamic::~dynamic()
{
	static void	(*session_free)(void *h);

    trace("%s",__FUNCTION__);

	if(!hModule)
		return;

#ifdef WIN32

	session_free = (void (*)(void *h)) GetProcAddress(hModule,"lib3270_session_free");

	if(session_free)
		session_free(hSession);

	FreeLibrary(hModule);

#else

	session_free = (void (*)(void *h)) dlsym(hModule,"lib3270_session_free");

	if(session_free)
		session_free(hSession);

	dlclose(hModule);

#endif // WIN32

}

char * dynamic::get_version(void)
{
	if(!hModule)
		return NULL;
	return strdup(_get_version());
}

LIB3270_CSTATE dynamic::get_cstate(void)
{
	if(!hModule)
		return (LIB3270_CSTATE) -1;
	return _get_connection_state(hSession);
}

int dynamic::disconnect(void)
{
	if(!hModule)
		return -1;
	return _disconnect(hSession);
}

int dynamic::connect(const char *uri, bool wait)
{
	if(!hModule)
		return -1;
	return _connect(hSession,uri,(int) wait);
}

bool dynamic::is_connected(void)
{
	if(!hModule)
		return -1;
	return _is_connected(hSession) != 0;
}

bool dynamic::is_ready(void)
{
	if(!hModule)
		return -1;
	return _is_ready(hSession) != 0;
}

int dynamic::iterate(bool wait)
{
	if(!hModule)
		return -1;

	_main_iterate(hSession,wait);

	return 0;
}

int dynamic::wait(int seconds)
{
	if(!hModule)
		return -1;
	return _wait(hSession,seconds);
}

int dynamic::wait_for_ready(int seconds)
{
	if(!hModule)
		return -1;
	return _wait_for_ready(hSession,seconds);
}

char * dynamic::get_text(int offset, size_t len)
{
	if(!hModule)
		return NULL;
	return _get_text(hSession,offset,len);
}

char * dynamic::get_text_at(int row, int col, size_t sz)
{
	if(!hModule)
		return NULL;
	return _get_text_at(hSession,row,col,sz);
}

int dynamic::cmp_text_at(int row, int col, const char *text)
{
	if(!hModule)
		return 0;
	return _cmp_text_at(hSession,row,col,text);
}

int dynamic::set_text_at(int row, int col, const char *str)
{
	if(!hModule)
		return -1;
	return _set_text_at(hSession,row,col,(const unsigned char *) str);
}

int dynamic::set_cursor_position(int row, int col)
{
	if(!hModule)
		return -1;
	return _set_cursor_position(hSession,row,col);
}

int dynamic::enter(void)
{
	if(!hModule)
		return -1;
	return _enter(hSession);
}

int dynamic::pfkey(int key)
{
	if(!hModule)
		return -1;
	return _pfkey(hSession,key);
}

int dynamic::pakey(int key)
{
	if(!hModule)
		return -1;
	return _pakey(hSession,key);
}

int dynamic::set_toggle(LIB3270_TOGGLE ix, bool value)
{
	if(hModule)
		return _set_toggle(hSession,ix,(int) value);
	return -1;
}

int dynamic::get_field_start(int baddr)
{
    if(hModule)
        return _get_field_start(hSession,baddr);
    return -1;
}

int dynamic::get_field_len(int baddr)
{
    if(hModule)
        return _get_field_len(hSession,baddr);
    return -1;
}

int dynamic::set_cursor_addr(int addr)
{
    if(hModule)
        return _set_cursor_addr(hSession,addr);
    return -1;
}

int dynamic::get_cursor_addr(void)
{
    if(hModule)
        return _get_cursor_addr(hSession);
    return -1;
}

int dynamic::emulate_input(const char *str)
{
    if(hModule)
        return _emulate_input(hSession,str,-1,1);
    return -1;
}
