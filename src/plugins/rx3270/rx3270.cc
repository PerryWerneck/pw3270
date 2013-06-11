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
 * Este programa está nomeado como rx3270.cc e possui - linhas de código.
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

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 static bool	  plugin		= false;
 static rx3270	* defSession	= NULL;

/*--[ Implement ]------------------------------------------------------------------------------------*/

rx3270::rx3270(const char *local, const char *remote)
{
#ifdef HAVE_ICONV

	if(strcmp(local,remote))
	{
		// Local and remote charsets aren't the same, setup conversion
		this->conv2Local = iconv_open(local, remote);
		this->conv2Host = iconv_open(remote,local);
	}
	else
	{
		this->conv2Local = this->conv2Host = (iconv_t)(-1);
	}
#endif

	if(!defSession)
		defSession = this;
}

rx3270::~rx3270()
{
#ifdef HAVE_ICONV

 	if(conv2Local != (iconv_t) (-1))
		iconv_close(conv2Local);

 	if(conv2Host != (iconv_t) (-1))
		iconv_close(conv2Host);
#endif


	if(defSession == this)
		defSession = NULL;
}

rx3270 * rx3270::create(const char *name)
{
	if(name && *name)
		return create_remote(name);
	return create_local();
}

char * rx3270::get_version(void)
{
	return strdup(PACKAGE_VERSION);
}

char * rx3270::get_revision(void)
{
	return strdup(PACKAGE_REVISION);
}

rx3270 * rx3270::get_default(void)
{
	if(defSession)
		return defSession;
	return create_local();
}

void rx3270::log(const char *fmt, ...)
{
	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	this->logva(fmt,arg_ptr);
	va_end(arg_ptr);
}

void rx3270::logva(const char *fmt, va_list args)
{
#ifdef HAVE_SYSLOG
	openlog(PACKAGE_NAME, LOG_NDELAY, LOG_USER);
	vsyslog(LOG_INFO,fmt,args);
	closelog();
#else
	vfprintf(stderr,fmt,args);
#endif
}

int rx3270::wait_for_text_at(int row, int col, const char *key, int timeout)
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

void rx3270::set_plugin(void)
{
	plugin = true;
}

int rx3270::set_copy(const char *text)
{
    return EINVAL;
}

char * rx3270::get_copy(void)
{
    errno = EINVAL;
    return NULL;
}




