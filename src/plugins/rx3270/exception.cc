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

 #include "rx3270.h"

#ifdef HAVE_SYSLOG
 #include <syslog.h>
#endif // HAVE_SYSLOG

 #include <string.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

rx3270::exception::exception(const char *fmt, ...)
{
	char buffer[4096];
	va_list arg_ptr;

	va_start(arg_ptr, fmt);
	vsnprintf(buffer,4095,fmt,arg_ptr);
	va_end(arg_ptr);

	msg = strdup(buffer);
}

extern "C" {
	static void memfree(void *ptr)
	{
		free(ptr);
	}
}

rx3270::exception::~exception()
{
	memfree(msg);
}

const char * rx3270::exception::getMessage(void)
{
	return this->msg;
}

void rx3270::exception::logMessage(void)
{
#ifdef HAVE_SYSLOG
	openlog(PACKAGE_NAME, LOG_NDELAY, LOG_USER);
	syslog(LOG_INFO,"%s",this->getMessage());
	closelog();
#else
	fprintf(stderr,"%s",this->getMessage());
#endif
}

void rx3270::exception::RaiseException(RexxMethodContext *context)
{
	// TODO: Raise rexx exception
	trace("%s: %s",__FUNCTION__,this->getMessage());
    logMessage();
}

