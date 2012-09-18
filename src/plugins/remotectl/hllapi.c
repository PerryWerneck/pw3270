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
		return EBUSY;

	snprintf(PipeName,4095,"\\\\.\\pipe\\%s",name);

	if(!WaitNamedPipe(PipeName,NMPWAIT_USE_DEFAULT_WAIT))
		return ETIMEDOUT;

	hPipe = CreateFile(PipeName,GENERIC_WRITE|GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);

	if(hPipe == INVALID_HANDLE_VALUE)
		return GetLastError();

	if(!SetNamedPipeHandleState(hPipe,&dwMode,NULL,NULL))
		return GetLastError();

#else

	#error Not implemented

#endif // WIN32

	return 0;
 }


 static int run_query(unsigned long func, const char *arg, char *string, unsigned short length, unsigned short *rc)
 {
 	int result = -1;

#ifdef WIN32

	if(length < 0 && string)
		length = strlen(string);

	if(hPipe == INVALID_HANDLE_VALUE)
	{
		result = EPERM;
	}
	else
	{
		HLLAPI_DATA	*buffer	= malloc(HLLAPI_MAXLENGTH+1);
		DWORD cbSize		= sizeof(HLLAPI_DATA) + length;
		HLLAPI_DATA *data	= malloc(cbSize+1);

		memset(buffer,0,HLLAPI_MAXLENGTH);

		data->id		= HLLAPI_REQUEST_ID;
		data->func		= func;
		data->rc		= *rc;
		data->len		= length;

		if(length > 0)
		{
			memset(data->string,0,length);
			if(arg)
				strncpy(data->string,arg,length);
		}

		memset(buffer,0,HLLAPI_MAXLENGTH);
		if(!TransactNamedPipe(hPipe,(LPVOID) data,cbSize,buffer,HLLAPI_MAXLENGTH,&cbSize,NULL))
		{
			result = GetLastError();
		}
		else
		{
			int sz = length < buffer->len ? length : buffer->len;

			*rc = buffer->rc;

			trace("%s: Query rc=%d",__FUNCTION__,(int) buffer->rc);

			if(string && sz > 0)
				memcpy(string,buffer->string,sz);

			result = 0;
		}

		free(data);
		free(buffer);
	}

#else

	#error NOT IMPLEMENTED

#endif // WIN32

	return result;
 }

 LIB3270_EXPORT int hllapi(const unsigned long *func, char *str, unsigned short *length, unsigned short *rc)
 {
 	int 	  result = 1;
 	char	* arg;

	if(!length || *length > HLLAPI_MAXLENGTH)
		return EINVAL;

	if(length > 0)
	{
		arg = malloc(*length+1);
		strncpy(arg,str,(int) *length);
		arg[(size_t) *length] = 0;
	}
	else
	{
		arg = malloc(1);
		*arg = 0;
	}

/*
#ifdef DEBUG
	freopen("hllapi.log","a",stderr);
#endif // DEBUG
*/

 	switch(*func)
 	{
	case HLLAPI_CMD_CONNECTPS:
		result = cmd_connect_ps(arg);
		if(!result)
		{
			result = run_query(*func, arg, str, *length, rc);
			if(result || rc)
			{
				CloseHandle(hPipe);
				hPipe = INVALID_HANDLE_VALUE;
			}
		}
		break;

	case HLLAPI_CMD_DISCONNECTPS:
#ifdef WIN32
		if(hPipe == INVALID_HANDLE_VALUE)
		{
			result = EINVAL;
		}
		else
		{
			result = run_query(*func, arg, str, *length, rc);
			CloseHandle(hPipe);
			hPipe = INVALID_HANDLE_VALUE;
		}
#endif // WIN32
		break;

	default:
		result = run_query(*func, arg, str, *length, rc);
 	}

 	if(result && length && *length && str)
		strncpy(str,strerror(result),*length);

	free(arg);
 	return result;
 }


