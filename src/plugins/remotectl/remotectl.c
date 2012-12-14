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

#ifndef ETIMEDOUT
	#define ETIMEDOUT 1238
#endif // ETIMEDOUT

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 static const gchar	  control_char	= '@';

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
												NMPWAIT_USE_DEFAULT_WAIT,	// client time-out
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

	set_active(FALSE);

	return 0;
 }

 static void cmd_connectps(QUERY *qry)
 {
 	g_message("%s","HLLAPI ConnectPS request received");
	request_status(qry,v3270_set_script(pw3270_get_terminal_widget(NULL),'H',TRUE));
 }

 static void cmd_disconnectps(QUERY *qry)
 {
 	g_message("%s","HLLAPI DisconnectPS request received");
 	request_status(qry,0);
 }

 static void cmd_getrevision(QUERY *qry)
 {
	request_complete(qry,0,lib3270_get_revision());
 }

 static void cmd_setcursor(QUERY *qry)
 {
 	int rc = ENOTCONN;

	if(lib3270_connected(qry->hSession))
	{
		trace("%s: pos=%d row=%d col=%d",__FUNCTION__,rc,rc/80,rc%80);
		lib3270_set_cursor_address(qry->hSession,qry->pos -1);
		rc = 0;
	}

 	request_status(qry,rc);
 }

 static void cmd_sendstring(QUERY *qry)
 {
	gchar		* text;
	GError		* error			= NULL;
	gsize		  bytes_read;
	gsize		  bytes_written;
	const gchar	* charset;

	if(!lib3270_connected(qry->hSession))
	{
		request_status(qry,ENOTCONN);
		return;
	}

	g_get_charset(&charset);

	text = g_convert(qry->text,qry->length,lib3270_get_charset(qry->hSession),charset,&bytes_read,&bytes_written,&error);
	if(text)
	{
		int rc = 0;

		if(strchr(text,control_char))
		{
			// Convert control char
			gchar	* buffer = text;
			char	* ptr;

			for(ptr = strchr(text,control_char);ptr;ptr = strchr(buffer,control_char))
			{
				*(ptr++) = 0;

				lib3270_emulate_input(qry->hSession,buffer,-1,0);

				switch(*(ptr++))
				{
				case 'P':	// Print
					rc = pw3270_print(pw3270_get_terminal_widget(NULL), NULL, GTK_PRINT_OPERATION_ACTION_PRINT, PW3270_SRC_ALL);
					break;

				case 'E':	// Enter
					lib3270_enter(qry->hSession);
					break;

				case 'F':	// Erase EOF
					lib3270_eraseeof(qry->hSession);
					break;

				case '1':	// PF1
					lib3270_pfkey(qry->hSession,1);
					break;

				case '2':	// PF2
					lib3270_pfkey(qry->hSession,2);
					break;

				case '3':	// PF3
					lib3270_pfkey(qry->hSession,3);
					break;

				case '4':	// PF4
					lib3270_pfkey(qry->hSession,4);
					break;

				case '5':	// PF5
					lib3270_pfkey(qry->hSession,5);
					break;

				case '6':	// PF6
					lib3270_pfkey(qry->hSession,6);
					break;

				case '7':	// PF7
					lib3270_pfkey(qry->hSession,7);
					break;

				case '8':	// PF8
					lib3270_pfkey(qry->hSession,8);
					break;

				case '9':	// PF9
					lib3270_pfkey(qry->hSession,9);
					break;

				case 'a':	// PF10
					lib3270_pfkey(qry->hSession,10);
					break;

				case 'b':	// PF11
					lib3270_pfkey(qry->hSession,11);
					break;

				case 'c':	// PF12
					lib3270_pfkey(qry->hSession,12);
					break;
				}

			}

			lib3270_emulate_input(qry->hSession,buffer,-1,0);

		}
		else
		{
			lib3270_emulate_input(qry->hSession,text,strlen(text),0);
		}
		g_free(text);

		request_status(qry,rc);

		return;
	}

	request_complete(qry, error->code, error->message);
	g_error_free(error);

 }

 struct wait
 {
 	QUERY	* qry;
 	time_t	  end;
 };

 static gboolean do_wait(struct wait *w)
 {
	if(lib3270_get_program_message(w->qry->hSession) == LIB3270_MESSAGE_NONE)
	{
		request_status(w->qry,0);
		return FALSE;
	}

	if(time(0) > w->end)
	{
		trace("%s: TIMEOUT",__FUNCTION__);
		request_status(w->qry,ETIMEDOUT);
		return FALSE;
	}

	return TRUE;
 }

 static void cmd_wait(QUERY *qry)
 {
 	struct wait *w;

	if(lib3270_get_program_message(qry->hSession) == LIB3270_MESSAGE_NONE)
	{
		request_status(qry,0);
		return;
	}

	w 		= g_malloc0(sizeof(struct wait));
	w->qry	= qry;
	w->end	= time(0)+pw3270_get_integer(pw3270_get_toplevel(),"hllapi","wait",2);

	g_timeout_add_full(G_PRIORITY_DEFAULT, (guint) 300, (GSourceFunc) do_wait, w, g_free);
 }

 static void cmd_copypstostr(QUERY *qry)
 {
	int 			  rows;
	int 			  cols;
	unsigned short	* attr;
	unsigned char	* text;
	int				  rc;
	unsigned char	* buffer;
	size_t 			  length;

	if(!lib3270_connected(qry->hSession))
	{
		request_status(qry,ENOTCONN);
		return;
	}

	lib3270_get_screen_size(qry->hSession,&rows,&cols);

	if(qry->pos < 1 || (qry->pos+qry->length) >= (rows*cols))
	{
		request_status(qry,EINVAL);
		return;
	}

	qry->pos--;

	length	= (qry->length * sizeof(unsigned short)) + qry->length + 2;
	text	= buffer = g_malloc0(length+1);
	attr 	= (unsigned short *) (text+qry->length+1);

	trace("%s: pos=%d length=%d",__FUNCTION__,qry->pos,qry->length);
	rc = lib3270_get_contents(qry->hSession,qry->pos,qry->pos+(qry->length-1),text,attr);

	if(rc)
	{
		request_status(qry,rc);
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

		local = g_convert((const gchar *) text,-1,charset,lib3270_get_charset(qry->hSession),&bytes_read,&bytes_written,&error);

		if(!local)
		{
			request_complete(qry,error->code,error->message);
			g_error_free(error);
		}
		else
		{
			strncpy((char *) text,(const char *) local,qry->length);

			trace("response: [%s] len=%d",buffer,length);
			request_buffer(qry,0,length,buffer);
			g_free(local);
		}
	}

	g_free(buffer);
 }

 static void cmd_querycursor(QUERY *qry)
 {
	request_value(qry,0,lib3270_get_cursor_address(qry->hSession));
 }

 void enqueue_request(QUERY *qry)
 {
	static const struct _cmd
	{
		int cmd;
		void (*exec)(QUERY *qry);
	} cmd[] =
	{
		{ HLLAPI_CMD_CONNECTPS,		cmd_connectps		},	// 1
		{ HLLAPI_CMD_DISCONNECTPS,	cmd_disconnectps	},	// 2
		{ HLLAPI_CMD_INPUTSTRING,	cmd_sendstring		},	// 3
		{ HLLAPI_CMD_WAIT,			cmd_wait			},	// 4
//		{ HLLAPI_CMD_COPYPS,							},	// 5
//		{ HLLAPI_CMD_SEARCHPS,							},	// 6
		{ HLLAPI_CMD_QUERYCURSOR,	cmd_querycursor		},	// 7

		{ HLLAPI_CMD_COPYPSTOSTR,	cmd_copypstostr 	},	// 8

//		{ HLLAPI_CMD_COPYSTRTOPS				  		},	// 15

		{ HLLAPI_CMD_SETCURSOR,		cmd_setcursor		},	// 40

//		{ HLLAPI_CMD_SENDFILE				  			},	// 90
//		{ HLLAPI_CMD_RECEIVEFILE				  		},


		{ HLLAPI_CMD_GETREVISION,	cmd_getrevision 	},
	};



	int f;

	trace("HLLAPI function %d",(int) qry->cmd);

	qry->hSession = lib3270_get_default_session_handle();

	for(f=0;f<G_N_ELEMENTS(cmd);f++)
	{
		if(cmd[f].cmd == qry->cmd)
		{
			cmd[f].exec(qry);
			return;
		}
	}

	g_warning("Unexpected HLLAPI function %d",(int) qry->cmd);
 	request_status(qry,EINVAL);
 }

 G_GNUC_INTERNAL void set_active(gboolean on)
 {
	v3270_set_script(pw3270_get_terminal_widget(NULL),'H',on);
 }


