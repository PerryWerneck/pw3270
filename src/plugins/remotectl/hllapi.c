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

 static char *session_name = NULL;

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static int run_query(unsigned long func, char *string, unsigned short length, unsigned short *rc)
 {
 	int result = -1;

#ifdef WIN32
	char				PipeName[4096];

	if(length < 0 && string)
		length = strlen(string);

	snprintf(PipeName,4095,"\\\\.\\pipe\\%s",session_name);

	if(!WaitNamedPipe(PipeName,NMPWAIT_USE_DEFAULT_WAIT))
	{
		result = ETIMEDOUT;
	}
	else
	{
		HLLAPI_DATA	*buffer	= malloc(HLLAPI_MAXLENGTH+1);
		DWORD cbSize		= sizeof(HLLAPI_DATA) + length;
		HLLAPI_DATA *data	= malloc(cbSize+1);

		data->id		= HLLAPI_REQUEST_ID;
		data->func		= func;
		data->rc		= *rc;
		data->len		= length;

		if(string && length > 0)
			memcpy(data->string,string,length);

		if(!CallNamedPipe(PipeName,(LPVOID)data,cbSize,buffer,HLLAPI_MAXLENGTH,&cbSize,NMPWAIT_USE_DEFAULT_WAIT))
		{
			result = GetLastError();
		}
		else
		{
			int sz = length < buffer->len ? length : buffer->len;
			*rc = buffer->rc;

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

 static int set_session_name(const char *name)
 {
	if(!session_name)
		free(session_name);
	session_name = strdup(name);

	return 0;
 }

 LIB3270_EXPORT int hllapi(unsigned long func, char *str, unsigned short length, unsigned short *rc)
 {
 	int result = 1;
 	switch(func)
 	{
	case HLLAPI_CMD_CONNECTPS:
		result = set_session_name(str);
		break;

	default:
		if(!session_name)
		{
			if(set_session_name("pw3270"))
				return ENOENT;
		}
		result = run_query(func, str, length, rc);
 	}

 	return result;
 }


