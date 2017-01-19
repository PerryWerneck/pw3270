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
 #include <string.h>

 #include <pw3270/class.h>

/*--[ Implement ]--------------------------------------------------------------------------------------------------*/

 namespace PW3270_NAMESPACE
 {

	exception::exception(int syscode)
	{
		snprintf(this->msg,4095,"%s",strerror(syscode));
	}

	exception::exception(const char *fmt, ...)
	{
		va_list arg_ptr;
		va_start(arg_ptr, fmt);
		vsnprintf(this->msg,4095,fmt,arg_ptr);
		va_end(arg_ptr);
	}

#ifdef WIN32
	exception::exception(DWORD error, const char *fmt, ...)
	{
		LPVOID 		  lpMsgBuf = 0;
		char		* ptr;
		size_t		  szPrefix;

		va_list arg_ptr;
		va_start(arg_ptr, fmt);
		vsnprintf(this->msg,4095,fmt,arg_ptr);
		va_end(arg_ptr);

		szPrefix = strlen(this->msg);

		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS, NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);

		for(ptr= (char *) lpMsgBuf;*ptr && *ptr != '\n';ptr++);
		*ptr = 0;

		snprintf(this->msg+szPrefix,4095-szPrefix,": %s (rc=%d)",(char *) lpMsgBuf,(int) error);

		LocalFree(lpMsgBuf);

	}
#else
	exception::exception(int error, const char *fmt, ...)
	{
		size_t		  szPrefix;

		va_list arg_ptr;
		va_start(arg_ptr, fmt);
		vsnprintf(this->msg,4095,fmt,arg_ptr);
		va_end(arg_ptr);

		szPrefix = strlen(this->msg);

		snprintf(this->msg+szPrefix,4095-szPrefix,": %s (rc=%d)",strerror(error),(int) error);

	}
#endif // WIN32

	const char * exception::what() const throw()
	{
		return this->msg;
	}

 }
