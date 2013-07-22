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

 #include <pw3270/class.h>

 #ifdef HAVE_SYSLOG
	#include <syslog.h>
 #endif // HAVE_SYSLOG

/*--[ Implement ]--------------------------------------------------------------------------------------------------*/

 namespace PW3270_NAMESPACE
 {
	session	* session::first						= 0;
	session	* session::last							= 0;
	session	* (*session::factory)(const char *name)	= 0;

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
		if(factory)
			return factory(name);

		if(name && *name)
			return create_remote(name);

		return create_local();
	}

	session	* session::start(const char *name)
	{
		return create(name);
	}

	session	* session::get_default(void)
	{
		if(first)
			return first;
		return create(NULL);
	}

	void session::set_plugin(session * (*factory)(const char *name))
	{
		session::factory = factory;
	}

	// Object settings
	void session::set_charset(const char *remote, const char *local)
	{
#ifdef HAVE_ICONV

		if(this->conv2Local != (iconv_t) (-1))
			iconv_close(this->conv2Local);

		if(this->conv2Host != (iconv_t) (-1))
			iconv_close(this->conv2Host);

		if(!remote)
			remote = this->get_charset();

		if(strcmp(local,remote))
		{
			// Local and remote charsets aren't the same, setup conversion
			conv2Local	= iconv_open(local, remote);
			conv2Host	= iconv_open(remote,local);
		}
		else
		{
			conv2Local = conv2Host = (iconv_t)(-1);
		}
#endif

	}

	const char * session::get_charset(void)
	{
		return "ISO-8859-1";
	}

	// 3270 methods
	string session::get_version(void)
	{
		return string(PACKAGE_VERSION);
	}

	string session::get_revision(void)
	{
		return string(PACKAGE_REVISION);
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

		while(time(0) < end)
		{
			if(!is_connected())
				return ENOTCONN;

			if(!cmp_text_at(row,col,key))
				return 0;

			iterate();
		}

		return ETIMEDOUT;
	}

	int session::set_copy(const char *text)
	{
		return EINVAL;
	}

	string * session::get_copy(void)
	{
		errno = EINVAL;
		return NULL;
	}

	string * session::get_clipboard(void)
	{
#if defined(WIN32)

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

		string *text = new string ( pszText );

		GlobalUnlock( hData );

		CloseClipboard();

		return text;

#else
		errno = EINVAL;
		return NULL;

#endif // WIN32
	}

	int session::set_clipboard(const char *text)
	{
#if defined(WIN32)
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

#endif // WIN32
	}


	int session::popup_dialog(LIB3270_NOTIFY id , const char *title, const char *message, const char *fmt, ...)
	{
		return -1;
	}

	string * session::file_chooser_dialog(GtkFileChooserAction action, const char *title, const char *extension, const char *filename)
	{
		return NULL;
	}

	string * session::get_3270_text(string *str)
	{
#ifdef HAVE_ICONV
		if(str && conv2Host != (iconv_t)(-1))
		{
			size_t				  in 		= str->length();
			size_t				  out 		= (in << 1);
			char				* ptr;
			char				* outBuffer = (char *) malloc(out);
			ICONV_CONST char	* inBuffer	= (ICONV_CONST char	*) str->c_str();

			memset(ptr=outBuffer,0,out);

			iconv(conv2Host,NULL,NULL,NULL,NULL);	// Reset state

			if(iconv(conv2Host,&inBuffer,&in,&ptr,&out) != ((size_t) -1))
				str->assign(outBuffer);

			free(outBuffer);
		}
#endif // HAVE_ICONV

		return str;
	}

	string * session::get_local_text(string *str)
	{
#ifdef HAVE_ICONV
		if(str && conv2Local != (iconv_t)(-1))
		{
			size_t				  in 		= str->length();
			size_t				  out 		= (in << 1);
			char				* ptr;
			char				* outBuffer = (char *) malloc(out);
			ICONV_CONST char	* inBuffer	= (ICONV_CONST char	*) str->c_str();

			memset(ptr=outBuffer,0,out);

			iconv(conv2Local,NULL,NULL,NULL,NULL);	// Reset state

			if(iconv(conv2Local,&inBuffer,&in,&ptr,&out) != ((size_t) -1))
				str->assign(outBuffer);

			free(outBuffer);
		}
#endif // HAVE_ICONV

		return str;
	}

	string * session::get_string_at(int row, int col, size_t sz)
	{
		string *str = this->get_text_at(row,col,sz);

		if(str)
			return this->get_local_text(str);

		return 0;
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

	int session::cmp_string_at(int row, int col, const char *text)
	{
		string	* str 	= get_3270_text(new string(text));
		int		  rc	= cmp_text_at(row,col,str->c_str());
		delete str;
		return rc;
	}

	int	session::wait_for_string_at(int row, int col, const char *key, int timeout)
	{
		string	* str 	= get_3270_text(new string(key));
		int		  rc	= wait_for_text_at(row,col,str->c_str(),timeout);
		delete str;
		return rc;
	}

	string * session::get_string(int baddr, size_t len)
	{
		return get_local_text(get_text(baddr,len));
	}

 }


