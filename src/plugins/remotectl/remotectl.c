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
 #include <pw3270.h>
 #include <pw3270/plugin.h>
 #include <pw3270/v3270.h>
 #include <lib3270/actions.h>
 #include <errno.h>
 #include <string.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static const gchar control_char = '@';

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
	char id;

	for(id='A';id < 'Z';id++)
	{
		gchar	* pipename	= g_strdup_printf("\\\\.\\pipe\\%s%c",pw3270_get_session_name(window),id);

		HANDLE	  hPipe		= CreateNamedPipe(	TEXT(pipename),				// pipe name
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

		trace("%s = %p",pipename,hPipe);
		g_free(pipename);

		if(hPipe != INVALID_HANDLE_VALUE)
		{
			gchar *session = g_strdup_printf("%s:%c",pw3270_get_session_name(window),id);
			pw3270_set_session_name(window,session);
			g_free(session);

			init_source_pipe(hPipe);
			return 0;
		}

	}

	popup_lasterror( "%s", _( "Can´t create remote control pipe" ));
	return -1;

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

 static int cmd_connectps(H3270 *hSession, unsigned short rc, char *string, unsigned short length)
 {
 	g_message("%s","HLLAPI ConnectPS request received");
	return v3270_set_script(pw3270_get_terminal_widget(NULL),'H',1);
 }

 static int cmd_disconnectps(H3270 *hSession, unsigned short rc, char *string, unsigned short length)
 {
 	g_message("%s","HLLAPI DisconnectPS request received");
	return 0;
 }

 static int cmd_getrevision(H3270 *hSession, unsigned short rc, char *string, unsigned short length)
 {
	strncpy(string,lib3270_get_revision(),length);
	return 0;
 }

 static int cmd_setcursor(H3270 *hSession, unsigned short rc, char *string, unsigned short length)
 {
	if(!lib3270_connected(hSession))
		return ENOTCONN;

	trace("%s: pos=%d row=%d col=%d",__FUNCTION__,rc,rc/80,rc%80);
	lib3270_set_cursor_address(hSession,(int) rc);
	return 0;
 }

 static int cmd_sendstring(H3270 *hSession, unsigned short dunno, char *buffer, unsigned short length)
 {
	gchar		* text;
	GError		* error			= NULL;
	gsize		  bytes_read;
	gsize		  bytes_written;
	const gchar	* charset;
	int 		  rc = -1;

	if(!lib3270_connected(hSession))
		return ENOTCONN;

	g_get_charset(&charset);

	text = g_convert(buffer,length,lib3270_get_charset(hSession),charset,&bytes_read,&bytes_written,&error);
	if(text)
	{
		if(strchr(text,control_char))
		{
			// Convert control char
			gchar	* buffer = text;
			char	* ptr;

			for(ptr = strchr(text,control_char);ptr;ptr = strchr(buffer,control_char))
			{
				*(ptr++) = 0;

				lib3270_emulate_input(hSession,buffer,-1,0);

				switch(*(ptr++))
				{
				case 'P':	// Print
					break;

				case 'E':	// Enter
					lib3270_enter(hSession);
					break;

				case '1':	// PF1
					lib3270_pfkey(hSession,1);
					break;

				case '2':	// PF2
					lib3270_pfkey(hSession,2);
					break;

				case '3':	// PF3
					lib3270_pfkey(hSession,3);
					break;

				case '4':	// PF4
					lib3270_pfkey(hSession,4);
					break;

				case '5':	// PF5
					lib3270_pfkey(hSession,5);
					break;

				case '6':	// PF6
					lib3270_pfkey(hSession,6);
					break;

				case '7':	// PF7
					lib3270_pfkey(hSession,7);
					break;

				case '8':	// PF8
					lib3270_pfkey(hSession,8);
					break;

				case '9':	// PF9
					lib3270_pfkey(hSession,9);
					break;

				case 'a':	// PF10
					lib3270_pfkey(hSession,10);
					break;

				case 'b':	// PF11
					lib3270_pfkey(hSession,11);
					break;

				case 'c':	// PF12
					lib3270_pfkey(hSession,12);
					break;
				}

			}

			lib3270_emulate_input(hSession,buffer,-1,0);

		}
		else
		{
			lib3270_emulate_input(hSession,text,strlen(text),0);
		}
		g_free(text);
		rc = 0;
	}
	else
	{
		strncpy(buffer,error->message,length);
		rc = error->code;
		g_error_free(error);
	}

	return rc;
 }

 static int cmd_wait(H3270 *hSession, unsigned short rc, char *text, unsigned short length)
 {
	return lib3270_wait_for_ready(hSession,pw3270_get_integer(pw3270_get_toplevel(),"hllapi","wait",2));
 }

 static int cmd_copypstostr(H3270 *hSession, unsigned short pos, char *outBuff, unsigned short length)
 {
	int 			  rows;
	int 			  cols;
	unsigned short	* attr;
	unsigned char	* text;
	int				  rc;

	lib3270_get_screen_size(hSession,&rows,&cols);

	if(pos < 1 || (pos+length) >= (rows*cols))
		return EINVAL;

	pos--;

	attr = g_new0(unsigned short, length+0);
	text = g_new0(unsigned char, length+1);

	trace("%s: pos=%d length=%d",__FUNCTION__,pos,length);
	rc = lib3270_get_contents(hSession,pos,pos+(length-1),text,attr);

	if(rc)
	{
		strncpy(outBuff,strerror(rc),length);
	}
	else
	{
		const gchar		* charset;
		gchar		 	* local;
		gsize			  bytes_read;
		gsize			  bytes_written;
		GError			* error				= NULL;

		trace("Text: [%s]",text);

		g_get_charset(&charset);

		local = g_convert((const gchar *) text,length,charset,lib3270_get_charset(hSession),&bytes_read,&bytes_written,&error);

		g_free(attr);
		g_free(text);

		if(!local)
		{
			rc = error->code;
			strncpy(outBuff,error->message,length);
			g_error_free(error);
		}
		else
		{
			strncpy(outBuff,(const char *) local,length);
			g_free(local);
		}
	}
	return rc;
 }

 int run_hllapi(unsigned long function, char *string, unsigned short length, unsigned short rc)
 {
	static const struct _cmd
	{
		unsigned long function;
		int (*exec)(H3270 *hSession, unsigned short rc, char *string, unsigned short length);
	} cmd[] =
	{
		{ HLLAPI_CMD_CONNECTPS,		cmd_connectps		},
		{ HLLAPI_CMD_DISCONNECTPS,	cmd_disconnectps	},
		{ HLLAPI_CMD_INPUTSTRING,	cmd_sendstring		},
		{ HLLAPI_CMD_WAIT,			cmd_wait			},
		{ HLLAPI_CMD_SETCURSOR,		cmd_setcursor		},
		{ HLLAPI_CMD_GETREVISION,	cmd_getrevision 	},
		{ HLLAPI_CMD_COPYPSTOSTR,	cmd_copypstostr 	}
	};
	int f;

	trace("HLLAPI function %d",(int) function);

	for(f=0;f<G_N_ELEMENTS(cmd);f++)
	{
		if(cmd[f].function == function)
			return cmd[f].exec(lib3270_get_default_session_handle(),rc,string,length);
	}

	g_warning("Unexpected HLLAPI function %d",(int) function);
 	return EINVAL;
 }

