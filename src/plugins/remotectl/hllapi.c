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
 * Este programa está nomeado como hllapi.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <lib3270.h>
 #include <malloc.h>
 #include <string.h>
 #include <errno.h>
 #include <pw3270/hllapi.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 LIB3270_EXPORT int hllapi(unsigned long func, const char *string, unsigned short length, unsigned short *rc)
 {
 	int result = -1;

#ifdef WIN32

	static DWORD		dwMode			= PIPE_READMODE_MESSAGE;
	static const LPTSTR	lpszPipeName	= TEXT( "\\\\.\\pipe\\pw3270" );
	HANDLE				hPipe			= INVALID_HANDLE_VALUE;

	if(length < 0 && string)
		length = strlen(string);

	if(!WaitNamedPipe(lpszPipeName,10000))
		return ETIMEDOUT;

	hPipe = CreateFile(	lpszPipeName,   					// pipe name
						GENERIC_WRITE|GENERIC_READ,			// Read/Write access
						0,              					// no sharing
						NULL,           					// default security attributes
						OPEN_EXISTING,  					// opens existing pipe
						0,									// Attributes
						NULL);          					// no template file

	if(hPipe == INVALID_HANDLE_VALUE)
		return -1;

	if(!SetNamedPipeHandleState(hPipe,&dwMode,NULL,NULL))
	{
		result = -1;
	}
	else
	{
		HLLAPI_DATA	*buffer	= malloc(HLLAPI_MAXLENGTH+1);
		DWORD cbSize		= 0;
		HLLAPI_DATA *data	= malloc(sizeof(HLLAPI_DATA) + length);

		data->id		= HLLAPI_REQUEST_ID;
		data->func		= func;
		data->rc		= *rc;
		data->len		= length;

		if(string && length > 0)
			memcpy(data->string,string,length);

		if(!TransactNamedPipe(hPipe,(LPVOID) data,sizeof(HLLAPI_DATA) + length,buffer,HLLAPI_MAXLENGTH,&cbSize,NULL))
		{
			result = -1;
		}
		else
		{
			*rc = buffer->rc;

			if(string && length > 0)
				memcpy(string,buffer->string,length);

			result = 0;
		}

		free(data);
		free(buffer);
	}

	CloseHandle(hPipe);

#else

	#error NOT IMPLEMENTED

#endif // WIN32

	return result;
 }
