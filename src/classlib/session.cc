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
 * Este programa está nomeado como exception.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <stdarg.h>
 #include <stdio.h>

 #include <pw3270/class.h>

 #ifdef HAVE_SYSLOG
	#include <syslog.h>
 #endif // HAVE_SYSLOG

/*--[ Implement ]--------------------------------------------------------------------------------------------------*/

 namespace pw3270
 {
	session	* session::first						= 0;
	session	* session::last							= 0;
	session	* (*session::factory)(const char *name)	= 0;

	session::session()
	{
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
	void session::set_charset(const char *charset)
	{

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



 }


