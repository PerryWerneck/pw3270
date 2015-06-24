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
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como module.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#if defined WIN32

	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms684179(v=vs.85).aspx
	#ifndef LOAD_LIBRARY_SEARCH_DEFAULT_DIRS
		#define LOAD_LIBRARY_SEARCH_DEFAULT_DIRS	0x00001000
	#endif // LOAD_LIBRARY_SEARCH_DEFAULT_DIRS

	#ifndef LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR
		#define LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR 	0x00000100
	#endif // LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR

	#include <windows.h>

#else

	#include <dlfcn.h>

#endif

#include <pw3270/class.h>

/*---[ Implement ]----------------------------------------------------------------------------------*/


namespace PW3270_NAMESPACE
{

#ifdef WIN32
	int module::get_datadir(LPSTR datadir)
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

	module::module(const char *name, const char *version) throw (std::exception)
	{
		string dllname = name;

#ifdef WIN32

		dllname += ".dll";
		if(version)
		{
			dllname += ".";
			dllname += version;
		}

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
			snprintf(buffer,4096,"%s\\.bin\\Debug\\%s",datadir,dllname.c_str());
#else
			snprintf(buffer,4096,"%s\\%s",datadir,dllname.c_str());
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
			hModule = LoadLibrary(dllname.c_str());
			rc = GetLastError();
		}

		SetErrorMode(errorMode);

		trace("%s hModule=%p rc=%d",dllname.c_str(),hModule,(int) rc);

		if(cookie && RemoveDllDirectory)
			RemoveDllDirectory(cookie);

		if(kernel)
			FreeLibrary(kernel);

		if(!hModule)
			throw exception("Can't load %s: %s",dllname.c_str(),session::win32_strerror(rc));

#else
		dllname += ".so";
		if(version)
		{
			dllname += ".";
			dllname += version;
		}

		dlerror();

		hModule = dlopen(dllname.c_str(), RTLD_NOW);
		if(!hModule)
			throw exception("Can't load lib3270: %s",dllname.c_str());

#endif // WIN32


	}

	module::~module()
	{
#ifdef WIN32
		FreeLibrary(hModule);
#else
		dlclose(hModule);
#endif // WIN32
	}


	void * module::get_symbol(const char *name)
	{
		void *symbol;

#ifdef WIN32

		symbol = (void *) GetProcAddress(hModule,name);

		if(!symbol)
			throw exception("Can't load symbol %s",name);

#else
		symbol = dlsym(hModule,name);

		if(!symbol)
			throw exception("Can't load symbol %s dlerror was \"%s\"",name,dlerror());

#endif // WIN32

		return symbol;
	}

}

