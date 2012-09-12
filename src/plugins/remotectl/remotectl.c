/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob o nome G3270.
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
 * Este programa está nomeado como remotectl.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 * Agradecimento:
 *
 * Hélio Passos
 *
 */

 #include "remotectl.h"
 #include <pw3270/plugin.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

#ifdef WIN32

 void popup_lasterror(const gchar *fmt, ...)
 {
 	char 		buffer[4096];
	va_list		arg_ptr;
	int			sz;
 	DWORD 		errcode = GetLastError();
 	char		*ptr;
    LPVOID 		lpMsgBuf = 0;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);

	for(ptr=lpMsgBuf;*ptr && *ptr != '\n';ptr++);
	*ptr = 0;

	va_start(arg_ptr, fmt);
	vsnprintf(buffer,4095,fmt,arg_ptr);
	va_end(arg_ptr);

	sz = strlen(buffer);
	snprintf(buffer+sz,4096-sz,": %s\n(rc=%d)",lpMsgBuf,(int) errcode);

	printf("%s\n",buffer);

#ifdef DEBUG
	fprintf(stderr,"%s\n",buffer);
	fflush(stderr);
#endif

	LocalFree(lpMsgBuf);
 }

#endif // WIN32

 LIB3270_EXPORT int pw3270_plugin_init(GtkWidget *window)
 {
#ifdef WIN32
	static const LPTSTR	lpszPipename	= TEXT("\\\\.\\pipe\\" PACKAGE_NAME );
	HANDLE				hPipe;

	trace("\n\n%s\n\n",__FUNCTION__);

	hPipe = CreateNamedPipe(	lpszPipename,				// pipe name
								PIPE_ACCESS_DUPLEX |		// read/write access
								FILE_FLAG_OVERLAPPED,		// overlapped mode
								PIPE_TYPE_MESSAGE |			// pipe type
								PIPE_READMODE_MESSAGE |		// pipe mode
								PIPE_WAIT,					// blocking mode
								1,							// number of instances
								PIPE_BUFFER_LENGTH,   		// output buffer size
								PIPE_BUFFER_LENGTH,			// input buffer size
								0,							// client time-out
								NULL);						// default security attributes


	if (hPipe == INVALID_HANDLE_VALUE)
	{
		popup_lasterror( _( "Can´t create pipe %s" ),lpszPipename);
		return -1;
	}

	init_source_pipe(hPipe);

#else

	#error Nao implementado

#endif // WIN32

	return 0;
 }

 LIB3270_EXPORT int pw3270_plugin_deinit(GtkWidget *window)
 {
#ifdef WIN32

#else

	#error Nao implementado

#endif // WIN32
	return 0;
 }


