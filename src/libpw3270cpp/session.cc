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
 * Este programa está nomeado como session.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <stdarg.h>
 #include <stdio.h>
 #include <string.h>
 #include <malloc.h>

 #include "private.h"

#ifndef WIN32
 #include <unistd.h>
#endif // !WIN32

#ifdef HAVE_SYSLOG
	#include <syslog.h>
#endif // HAVE_SYSLOG


/*--[ Implement ]--------------------------------------------------------------------------------------------------*/

 using namespace PW3270_NAMESPACE;

#if defined(linux)
 static void onLoad() __attribute__((constructor));
 static void onUnLoad() __attribute__((destructor));

 static void onLoad()
 {
	session::init();
 }

 static void onUnLoad()
 {
	session::deinit();
 }

#endif // linux

#ifdef _WIN32

 BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
 {
	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		session::init();
		break;

	case DLL_PROCESS_DETACH:
		session::deinit();
		break;
	}

	return TRUE;
 }

#endif // _WIN32


 namespace PW3270_NAMESPACE {

	session	* session::first						= nullptr;
	session	* session::last							= nullptr;
	session	* (*session::factory)(const char *name)	= nullptr;

	static recursive_mutex	mtx;

	inline void lock()
	{
		mtx.lock();
	}

	inline void unlock()
	{
		mtx.unlock();
	}

	void session::init()
	{
		trace("Loading %s objects",PACKAGE_NAME);
	}

	void session::deinit()
	{
		trace("Unloading %s objects",PACKAGE_NAME);
		while(first)
		{
			delete first;
		}

	}

	session::session()
	{

#ifdef HAVE_ICONV
		this->conv2Local	= (iconv_t) (-1);
		this->conv2Host		= (iconv_t) (-1);
#endif

		if(first)
		{
			prev		= last;
			next		= 0;
			last->next	= this;
			last		= this;
		}
		else
		{
			prev  = next = 0;
			first = last = this;
		}

	}

	session::~session()
	{
#ifdef HAVE_ICONV

		if(this->conv2Local != (iconv_t) (-1))
			iconv_close(this->conv2Local);

		if(this->conv2Host != (iconv_t) (-1))
			iconv_close(this->conv2Host);

#endif

		if(prev)
			prev->next = next;
		else
			first = next;

		if(next)
			next->prev = prev;
		else
			last = prev;
	}

	// Factory methods and settings
	session	* session::create(const char *name)
	{
		session	*rc = nullptr;

		trace("%s(%s)",__FUNCTION__,name);

		lock();

		try
		{
			if(factory)
				rc = factory(name);
			else if(name && *name)
				rc = create_remote(name);
			else
				rc = create_local();

		}
		catch(std::exception &e)
		{
			unlock();
			throw exception("%s",e.what());
		}

		unlock();

		return rc;
	}

	session	* session::start(const char *name)
	{
		return create(name);
	}

	bool session::has_default(void)
	{
		return first != nullptr;
	}

	session	* session::get_default(void)
	{
		if(first)
			return first;
		return create(NULL);
	}

	void session::set_plugin(session * (*factory)(const char *name))
	{
		trace("%s(%p)",__FUNCTION__,factory);
		session::factory = factory;
	}

	// Object settings
	void session::set_display_charset(const char *remote, const char *local)
	{
		trace("%s(%s,%s)",__FUNCTION__,remote,local);

#ifdef HAVE_ICONV

		string display_charset = this->get_display_charset();

		if(this->conv2Local != (iconv_t) (-1))
			iconv_close(this->conv2Local);

		if(this->conv2Host != (iconv_t) (-1))
			iconv_close(this->conv2Host);

		if(!remote)
			remote = display_charset.c_str();

		trace("%s remote=%s local=%s",__FUNCTION__,remote,local);

		if(strcmp(local,remote))
		{
			// Local and remote charsets aren't the same, setup conversion
			conv2Local	= iconv_open(local, remote);
			conv2Host	= iconv_open(remote,local);
		}
		else
		{
			// Same charset, doesn't convert
			conv2Local = conv2Host = (iconv_t)(-1);
		}

#else

		#error aqui
		throw exception("%s",strerror(ENOSUP));


#endif

	}

	string session::get_display_charset(void)
	{
		return string(get_encoding());
	}

	const char * session::get_encoding(void)
	{
		return "ISO-8859-1";
	}

	// 3270 methods
	const string session::get_version(void)
	{
		return string(PACKAGE_VERSION);
	}

	const string session::get_revision(void)
	{
#ifdef PACKAGE_REVISION
		return string(PACKAGE_REVISION);
#else
		return string(STRINGIZE_VALUE_OF(BUILD_DATE));
#endif // PACKAGE_REVISION
	}

	void session::log(const char *fmt, ...)
	{
		va_list arg_ptr;
		va_start(arg_ptr, fmt);
		this->logva(fmt,arg_ptr);
		va_end(arg_ptr);
	}

	void session::logva(const char *fmt, va_list args)
	{
	#ifdef HAVE_SYSLOG
		openlog(PACKAGE_NAME, LOG_NDELAY, LOG_USER);
		vsyslog(LOG_INFO,fmt,args);
		closelog();
	#else
		vfprintf(stderr,fmt,args);
	#endif
	}

	int session::wait_for_text_at(int row, int col, const char *key, int timeout)
	{
		time_t end = time(0)+timeout;

		trace("%s(%d,%d,%s,%d)",__FUNCTION__,row,col,key,timeout);

		iterate(false);
		while(time(0) < end)
		{
			trace("Aguardar %d segundos por \"%s\" @%d,%d (%s)",(int) (end - time(0)),key,row,col,get_text_at(row,col,strlen(key)).c_str());

			int rc = wait_for_ready(end - time(0));
			if(rc) {
				return rc;
			}

			if(!cmp_text_at(row,col,key)) {
				return 0;
			}

			iterate(true);

		}

		trace("Tela:\n%s\n", ((string) *this).c_str());

		return ETIMEDOUT;
	}

	int session::set_copy(const char *text)
	{
		return EINVAL;
	}

	string session::get_copy(void)
	{
		errno = EINVAL;
		return string();
	}

	string session::get_clipboard(void)
	{
#if defined(_WIN32)

		if (! OpenClipboard(0))
		{
			throw exception(GetLastError(),"%s","Can´t open system clipboard");
			return NULL;
		}

		HANDLE hData = GetClipboardData(CF_TEXT);
		if(!hData)
		{
			throw exception(GetLastError(),"%s","Can´t get clipboard data");
			return NULL;
		}

		char * pszText = static_cast<char*>( GlobalLock(hData) );
		if(!pszText)
		{
			throw exception(GetLastError(),"%s","Can´t lock clipboard");
			return NULL;
		}

		string text = string ( pszText );

		GlobalUnlock( hData );

		CloseClipboard();

		return text;

#else
		errno = EINVAL;
		return NULL;

#endif // _WIN32
	}

	int session::set_clipboard(const char *text)
	{
#if defined(_WIN32)
		if (! OpenClipboard(0))
		{
			throw exception(GetLastError(),"%s","Can´t open system clipboard");
			return -1;
		}

		EmptyClipboard();

        size_t size = strlen(text)+1;
		HGLOBAL hClipboardData = GlobalAlloc(GMEM_MOVEABLE , size);

		strcpy((char *) GlobalLock(hClipboardData), text);

		if(!SetClipboardData(CF_TEXT, hClipboardData))
		{
			GlobalUnlock(hClipboardData);
			CloseClipboard();
			throw exception(GetLastError(),"%s","Can´t set system clipboard");
		}

		GlobalUnlock(hClipboardData);
		CloseClipboard();

		return 0;
#else

		return EINVAL;

#endif // _WIN32
	}


	int session::popup_dialog(LIB3270_NOTIFY id , const char *title, const char *message, const char *fmt, ...)
	{
		return -1;
	}

	string session::file_chooser_dialog(int action, const char *title, const char *extension, const char *filename)
	{
		return string("");
	}

	string session::get_3270_text(const char *str)
	{
		string rc;

#ifdef HAVE_ICONV
		size_t in = strlen(str);

		if(in && conv2Host != (iconv_t)(-1))
		{
			size_t				  out 		= (in << 1);
			char				* ptr;
			char				* outBuffer = (char *) malloc(out);
			ICONV_CONST char	* inBuffer	= (ICONV_CONST char	*) str;

			memset(ptr=outBuffer,0,out);

			iconv(conv2Host,NULL,NULL,NULL,NULL);	// Reset state

			if(iconv(conv2Host,&inBuffer,&in,&ptr,&out) == ((size_t) -1)) {
				rc.assign(str);
			} else {
				rc.assign(outBuffer);
			}

			free(outBuffer);
		} else {
			rc.assign(str);
		}
#else
		rc.assign(str);
#endif // HAVE_ICONV

		trace("%s(\"%s\")=\"%s\"",__FUNCTION__,str,rc.c_str());

		return rc;
	}

	string session::get_local_text(const char *str)
	{
		string rc;

#ifdef HAVE_ICONV
		size_t in = strlen(str);

		if(in && conv2Local != (iconv_t)(-1))
		{
			size_t				  out 		= (in << 1);
			char				* ptr;
			char				* outBuffer = (char *) malloc(out);
			ICONV_CONST char	* inBuffer	= (ICONV_CONST char	*) str;

			memset(ptr=outBuffer,0,out);

			iconv(conv2Local,NULL,NULL,NULL,NULL);	// Reset state

			if(iconv(conv2Local,&inBuffer,&in,&ptr,&out) != ((size_t) -1))
				rc.assign(outBuffer);

			free(outBuffer);
		}
		else
		{
			char * text = strdup(str);
			for(char *ptr = text;*ptr;ptr++)
			{
				if(*ptr < ' ' || *ptr > 128)
				{
					*ptr = '?';
				}
			}
			rc = text;
			free(text);
		}
#else
		char * text = strdup(str);
		for(char *ptr = text;*ptr;ptr++)
		{
			if(*ptr < ' ' || *ptr > 128)
			{
				*ptr = '?';
			}
		}
		rc = text;
		free(text);
#endif // HAVE_ICONV

		return rc;
	}

	string session::get_string_at(int row, int col, size_t sz, bool lf)
	{
		return this->get_local_text(this->get_text_at(row,col,sz,lf).c_str());
	}

	int session::set_string_at(int row, int col, const char *str)
	{
		if(!str)
			return -1;

#ifdef HAVE_ICONV
		if(conv2Host != (iconv_t)(-1))
		{
			size_t				  in 		= strlen(str);
			size_t				  out 		= (in << 1);
			char				* ptr;
			char				* outBuffer = (char *) malloc(out);
			ICONV_CONST char	* inBuffer	= (ICONV_CONST char	*) str;

			memset(ptr=outBuffer,0,out);

			iconv(conv2Host,NULL,NULL,NULL,NULL);	// Reset state

			if(iconv(conv2Host,&inBuffer,&in,&ptr,&out) != ((size_t) -1))
			{
				int rc = this->set_text_at(row,col,outBuffer);
				free(outBuffer);
				return rc;
			}

			free(outBuffer);
		}
#endif // HAVE_ICONV

		return this->set_text_at(row,col,str);

	}

	int session::input_string(const char *str)
	{
		if(!str)
			return -1;

#ifdef HAVE_ICONV
		if(conv2Host != (iconv_t)(-1))
		{
			size_t				  in 		= strlen(str);
			size_t				  out 		= (in << 1);
			char				* ptr;
			char				* outBuffer = (char *) malloc(out);
			ICONV_CONST char	* inBuffer	= (ICONV_CONST char	*) str;

			memset(ptr=outBuffer,0,out);

			iconv(conv2Host,NULL,NULL,NULL,NULL);	// Reset state

			if(iconv(conv2Host,&inBuffer,&in,&ptr,&out) != ((size_t) -1))
			{
				int rc = this->emulate_input(outBuffer);
				free(outBuffer);
				return rc;
			}

			free(outBuffer);
		}
#endif // HAVE_ICONV

		return this->emulate_input(str);

	}

	int session::cmp_string_at(int row, int col, const char *text, bool lf)
	{
		return cmp_text_at(row,col,get_3270_text(text).c_str(),lf);
	}

	int	session::wait_for_string_at(int row, int col, const char *key, int timeout)
	{
		return wait_for_text_at(row,col,get_3270_text(key).c_str(),timeout);
	}

	string session::get_string(int baddr, size_t len, bool lf)
	{
		return get_local_text(get_text(baddr,len,lf).c_str());
	}

	string session::asc2ebc(string &str)
	{
		size_t			sz 			= str.size();
		unsigned char	buffer[sz+1];

		memcpy(buffer,str.c_str(),sz);
		return string(asc2ebc(buffer,sz));
	}

	string session::ebc2asc(string &str)
	{
		size_t			sz 			= str.size();
		unsigned char	buffer[sz+1];
		memcpy(buffer,str.c_str(),sz);
		return string(ebc2asc(buffer,sz));
	}

	int session::file_transfer(LIB3270_FT_OPTION options, const char *local, const char *remote, int lrecl, int blksize, int primspace, int secspace, int dft)
	{
		log("Can't transfer %s: File transfer is unavailable", local ? local : "file");
		return EINVAL;
	}

	int session::set_host(const char *host)
	{
		return set_url(host);
	}

	int session::connect(const char *url, time_t wait)
	{
		int rc = 0;

		if(url && *url)
		{
			set_url(url);
		}

		rc = connect();
		trace("%s: connect=%d wait=%u",__FUNCTION__,rc,(unsigned int) wait);

		if(!rc && wait)
		{
			rc = wait_for_ready(wait);
		}

		return rc;
	}

#ifdef _WIN32
	string	session::win32_strerror(int e)
	{
		static char buffer[4096];

		memset(buffer,0,sizeof(buffer));

		if(FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,NULL,e,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),buffer,sizeof(buffer),NULL) == 0)
		{
			snprintf(buffer,4095,"Windows error %d", e);
		}

		for(size_t f=0;f<sizeof(buffer);f++)
		{
			if(buffer[f] < ' ')
			{
				buffer[f] = 0;
				break;
			}
		}

		return string(buffer);
	}
#endif // _WIN32

	int session::erase(int mode) {

		switch(mode) {
		case 0:
			return erase();

		case 1:
			return erase_eof();

		case 2:
			return erase_eol();

		case 3:
			return erase_input();

		}

		return -1;
	}

	string session::get_contents(bool lf)
	{
		string	rc = "";
		int		rows = get_height();
		int		cols = get_width();

		for(int r = 0; r < rows; r++)
		{
			rc += get_string_at(r+1,1,cols).c_str();
			if(lf) {
				rc += "\n";
			}
		}

		return rc;
	}

	size_t session::find_string(const char *str, bool lf) {

		int rc = 0;

		try
		{
			size_t pos = get_contents(lf).find(str);

			if(pos != string::npos) {
				rc = ((int) pos) + 1;
			}

		}
		catch(std::exception &e)
		{
			rc = 0;
		}

		return rc;

	}


	string session::get_session_name(void) const
	{
		return string();
	}

	int	session::close(void) {
		return 0;
	}

	/**
	 * @brief Define após quantos segundos uma sessão IDLE será cancelada.
	 *
	 * @param timeout Nº de segundos a esperar em sessão inativa.
	 *
	 */
	void session::set_timeout(time_t timeout) {
	}

	/**
	 * @brief Define após quantos segundos uma sessão offline será cancelada.
	 *
	 * @param timeout Nº de segundos a esperar em sessão offline.
	 *
	 */
	void session::set_autoclose(time_t timeout) {
	}

 }


