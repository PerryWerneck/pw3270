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
 #include <stdarg.h>

 static rx3270 * factory_default(const char *type);

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 static rx3270	* defSession	                = NULL;
 static rx3270  * (*factory)(const char *type)  = factory_default;

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

    trace("%s",__FUNCTION__);
}

static rx3270 * factory_default(const char *type)
{
    trace("%s",__FUNCTION__);
	if(type && *type)
		return rx3270::create_remote(type);
	return rx3270::create_local();
}

rx3270 * rx3270::create(const char *type)
{
    return factory(type);
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

void rx3270::set_plugin(rx3270 * (*ptr)(const char *name))
{
    trace("%s factory=%p",__FUNCTION__,ptr);

	if(ptr)
        factory = ptr;
    else
        factory = factory_default;

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

char * rx3270::get_clipboard(void)
{
    errno = EINVAL;
    return NULL;
}

int rx3270::set_clipboard(const char *text)
{
    return EINVAL;
}

extern "C"
{
	static void memfree(void *ptr)
	{
		free(ptr);
	}
}

void rx3270::free(void *ptr)
{
    memfree(ptr);
}

int rx3270::popup_dialog(LIB3270_NOTIFY id , const char *title, const char *message, const char *fmt, ...)
{
    return -1;
}

char * rx3270::file_chooser_dialog(GtkFileChooserAction action, const char *title, const char *extension, const char *filename)
{
    return NULL;
}

