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
 #include <stdio.h>
 #include <lib3270/log.h>

/*--[ Globals ]--------------------------------------------------------------------------------------*/

#ifdef WIN32

 static HANDLE	hPipe = INVALID_HANDLE_VALUE;

#endif // WIN32

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static int cmd_connect_ps(const char *name)
 {
#ifdef WIN32

	static DWORD dwMode = PIPE_READMODE_MESSAGE;
	char PipeName[4096];

	if(hPipe != INVALID_HANDLE_VALUE)
		return 0;

	snprintf(PipeName,4095,"\\\\.\\pipe\\%s",name);

	if(!WaitNamedPipe(PipeName,NMPWAIT_USE_DEFAULT_WAIT))
		return ENOENT;

	hPipe = CreateFile(PipeName,GENERIC_WRITE|GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);

	if(hPipe == INVALID_HANDLE_VALUE)
		return GetLastError();

	if(!SetNamedPipeHandleState(hPipe,&dwMode,NULL,NULL))
		return GetLastError();

	trace("Pipe %ld open",(unsigned long) hPipe);
#else

	#error Not implemented

#endif // WIN32

	return 0;
 }


 static char * run_query(unsigned long func, const char *arg, size_t *length, unsigned short *rc)
 {
	char *outBuffer = NULL;

#ifdef WIN32

	if(hPipe == INVALID_HANDLE_VALUE)
	{
		trace("%s: Invalid pipe handle",__FUNCTION__);
		*rc = EPERM;
	}
	else
	{
		HLLAPI_DATA	*buffer	= malloc(HLLAPI_MAXLENGTH+1);
		DWORD cbSize		= sizeof(HLLAPI_DATA) + *length;
		HLLAPI_DATA *data	= malloc(cbSize+1);

		memset(buffer,0,HLLAPI_MAXLENGTH);

		data->id		= HLLAPI_REQUEST_QUERY;
		data->func		= func;
		data->rc		= *rc;
		data->value		= *length;

		if(arg && *length > 0)
			memcpy(data->string,arg,*length);

		memset(buffer,0,HLLAPI_MAXLENGTH);

		if(!TransactNamedPipe(hPipe,(LPVOID) data,cbSize,buffer,HLLAPI_MAXLENGTH,&cbSize,NULL))
		{
			trace("Error %d in TransactNamedPipe",(int) GetLastError());
			*rc = GetLastError();
		}
		else
		{
			*rc		= buffer->rc;
			*length = buffer->value;

			trace("buffer->id=%d buffer->value=%d rc=%d",buffer->id,buffer->value,buffer->rc);

			if(buffer->value > 0 && buffer->id == HLLAPI_RESPONSE_TEXT)
			{
				outBuffer = malloc(buffer->value+1);
				memcpy(outBuffer,buffer->string,buffer->value);
				outBuffer[buffer->value] = 0;

				trace("outBuffer=[%s]",outBuffer);
			}
		}

		free(data);
		free(buffer);
	}

#else

	#error NOT IMPLEMENTED

#endif // WIN32

	return outBuffer;
 }

 static void copyString(char *str, unsigned short *length, const char *msg)
 {
	size_t len = strlen(msg);

	if(len > *length)
		len = *length;
	else
		*length = len;

	memcpy(str,msg,*length);
 }

#ifdef _WIN32
 __declspec (dllexport) int __stdcall hllapi(LPWORD func, LPSTR buffer, LPWORD length, LPWORD rc)
#else
 LIB3270_EXPORT int hllapi(const unsigned long *func, char *buffer, unsigned short *length, unsigned short *rc)
#endif // _WIN32
 {
 	char	* inBuffer	= NULL;
 	char	* outBuffer	= NULL;
	size_t	  szOutBuffer;

	if(*length < 0 || *length > HLLAPI_MAXLENGTH)
	{
		*rc = EINVAL;
		return 0;
	}

	szOutBuffer = (size_t) *length;

	// Copy input argument
	if(*length)
	{
		inBuffer = malloc(*length+1);
		memcpy(inBuffer,buffer,*length);
		inBuffer[*length] = 0;
	}

	// Clear output buffer
	memset(buffer,' ',*length);
	buffer[*length] = 0;

 	switch(*func)
 	{
	case HLLAPI_CMD_CONNECTPS:
		*rc = cmd_connect_ps(inBuffer);
		if(!*rc)
		{
			outBuffer = run_query(*func, inBuffer, &szOutBuffer, rc);
			if(*rc)
			{
				trace("Closing pipe rc=%d",*rc);
				CloseHandle(hPipe);
				hPipe = INVALID_HANDLE_VALUE;
			}
		}
		break;

	case HLLAPI_CMD_DISCONNECTPS:
#ifdef WIN32
		if(hPipe == INVALID_HANDLE_VALUE)
		{
			*rc = EINVAL;
		}
		else
		{
			outBuffer = run_query(*func, inBuffer, &szOutBuffer, rc);
			CloseHandle(hPipe);
			hPipe = INVALID_HANDLE_VALUE;
		}
#endif // WIN32
		break;

	default:
		trace("Calling function %d",(int) *func);
		outBuffer = run_query(*func, inBuffer, &szOutBuffer, rc);
 	}

	if(*rc)
		copyString(buffer,length,strerror(*rc));
	else if(outBuffer)
	{
		if(szOutBuffer < *length)
			*length = szOutBuffer;

		copyString(buffer,length,outBuffer);
	}
	if(outBuffer)
		free(outBuffer);

	free(inBuffer);
 	return 0;
 }


