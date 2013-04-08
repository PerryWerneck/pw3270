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
 * Este programa está nomeado como pluginmain.c e possui - linhas de código.
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

 #include "server.h" #include <pw3270/plugin.h>
 #include <pw3270/ipcpackets.h>
 #include <lib3270/actions.h>

/*--[ Defines ]--------------------------------------------------------------------------------------*/

 #pragma pack(1)

 typedef struct _pipe_source
 {
	GSource 			gsrc;
	HANDLE				hPipe;

	enum _PIPE_STATE
	{
		PIPE_STATE_WAITING,
		PIPE_STATE_READ,
		PIPE_STATE_PENDING_READ,
		PIPE_STATE_UNDEFINED,
	} 					state;

	OVERLAPPED			overlap;
	unsigned char		buffer[PIPE_BUFFER_LENGTH+1];
 } pipe_source;

 #pragma pack()


/*--[ Globals ]--------------------------------------------------------------------------------------*/

 static const gchar	  control_char	= '@';

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void IO_accept(pipe_source *source)
 {
	set_active(FALSE);

	if(ConnectNamedPipe(source->hPipe,&source->overlap))
	{
		popup_lasterror("%s",_( "Error in ConnectNamedPipe" ));
		return;
	}

	switch(GetLastError())
	{
	// The overlapped connection in progress.
	case ERROR_IO_PENDING:
		// trace("%s: ERROR_IO_PENDING",__FUNCTION__);
		source->state = PIPE_STATE_WAITING;
		break;

	// Client is already connected, so signal an event.
	case ERROR_PIPE_CONNECTED:
		trace("%s: ERROR_PIPE_CONNECTED",__FUNCTION__);
		set_active(TRUE);
		if(SetEvent(source->overlap.hEvent))
			break;

	// If an error occurs during the connect operation...
	default:
		popup_lasterror("%s", _( "ConnectNamedPipe failed" ));
	}

 }

 static gboolean IO_prepare(GSource *source, gint *timeout)
 {
	/*
 	 * Called before all the file descriptors are polled.
	 * If the source can determine that it is ready here
	 * (without waiting for the results of the poll() call)
	 * it should return TRUE.
	 *
	 * It can also return a timeout_ value which should be the maximum
	 * timeout (in milliseconds) which should be passed to the poll() call.
	 * The actual timeout used will be -1 if all sources returned -1,
	 * or it will be the minimum of all the timeout_ values
	 * returned which were >= 0.
	 *
	 */
	if(WaitForSingleObject(((pipe_source *) source)->overlap.hEvent,0) == WAIT_OBJECT_0)
		return TRUE;

	*timeout = 10;
	return FALSE;
 }

 static gboolean IO_check(GSource *source)
 {
	/*
 	 * Called after all the file descriptors are polled.
 	 * The source should return TRUE if it is ready to be dispatched.
	 * Note that some time may have passed since the previous prepare
	 * function was called, so the source should be checked again here.
	 *
	 */
	if(WaitForSingleObject(((pipe_source *) source)->overlap.hEvent,0) == WAIT_OBJECT_0)
		return TRUE;

	return FALSE;
 }

 static void send_text(pipe_source *source, char *text)
 {
 	struct hllapi_packet_text *pkt;
	DWORD szBlock;

	if(text)
	{
		szBlock = sizeof(struct hllapi_packet_text)+strlen(text);
		pkt = g_malloc0(szBlock);
		pkt->packet_id = 0;
		strcpy(pkt->text,text);
		lib3270_free(text);
	}
	else
	{
		szBlock = sizeof(struct hllapi_packet_text);
		pkt = g_malloc0(szBlock);
		pkt->packet_id = errno ? errno : -1;
	}

	WriteFile(source->hPipe,pkt,szBlock,&szBlock,NULL);

	g_free(pkt);
 }

 static void send_result(pipe_source *source, int rc)
 {
 	struct hllapi_packet_result pkt = { rc };
	DWORD wrote = sizeof(pkt);
	WriteFile(source->hPipe,&pkt,wrote,&wrote,NULL);
 }

 static void process_input(pipe_source *source, DWORD cbRead)
 {

 	trace("%s id=%d",__FUNCTION__,((struct hllapi_packet_query *) source->buffer)->packet_id);

	switch(((struct hllapi_packet_query *) source->buffer)->packet_id)
	{
	case HLLAPI_PACKET_CONNECT:
		send_result(source,lib3270_connect(	lib3270_get_default_session_handle(),
											((struct hllapi_packet_connect *) source->buffer)->hostname,
											((struct hllapi_packet_connect *) source->buffer)->wait));
		break;

	case HLLAPI_PACKET_DISCONNECT:
		send_result(source,lib3270_disconnect(lib3270_get_default_session_handle()));
		break;

	case HLLAPI_PACKET_GET_PROGRAM_MESSAGE:
		send_result(source,lib3270_get_program_message(lib3270_get_default_session_handle()));
		break;

	case HLLAPI_PACKET_IS_CONNECTED:
		send_result(source,lib3270_in_tn3270e(lib3270_get_default_session_handle()));
		break;

	case HLLAPI_PACKET_IS_READY:
		send_result(source,lib3270_is_ready(lib3270_get_default_session_handle()));
		break;

	case HLLAPI_PACKET_ENTER:
		send_result(source,lib3270_enter(lib3270_get_default_session_handle()));
		break;

	case HLLAPI_PACKET_PRINT:
		send_result(source,lib3270_print(lib3270_get_default_session_handle()));
		break;

	case HLLAPI_PACKET_ERASE_EOF:
		send_result(source,lib3270_eraseeof(lib3270_get_default_session_handle()));
		break;

	case HLLAPI_PACKET_PFKEY:
		send_result(source,lib3270_pfkey(	lib3270_get_default_session_handle(),
											((struct hllapi_packet_keycode *) source->buffer)->keycode));
		break;

	case HLLAPI_PACKET_PAKEY:
		send_result(source,lib3270_pakey(	lib3270_get_default_session_handle(),
											((struct hllapi_packet_keycode *) source->buffer)->keycode));
		break;

	case HLLAPI_PACKET_SET_CURSOR_POSITION:
		send_result(source,lib3270_set_cursor_position(	lib3270_get_default_session_handle(),
											((struct hllapi_packet_cursor *) source->buffer)->row,
											((struct hllapi_packet_cursor *) source->buffer)->col));
		break;

	case HLLAPI_PACKET_SET_TEXT_AT:
		send_result(source,lib3270_set_text_at(	lib3270_get_default_session_handle(),
											((struct hllapi_packet_text_at *) source->buffer)->row,
											((struct hllapi_packet_text_at *) source->buffer)->col,
											(unsigned char *) ((struct hllapi_packet_text_at *) source->buffer)->text));
		break;

	case HLLAPI_PACKET_GET_TEXT_AT:
		send_text(source,lib3270_get_text_at(	lib3270_get_default_session_handle(),
											((struct hllapi_packet_at *) source->buffer)->row,
											((struct hllapi_packet_at *) source->buffer)->col,
											((struct hllapi_packet_at *) source->buffer)->len));
		break;

	case HLLAPI_PACKET_GET_TEXT_AT_OFFSET:
		send_text(source,lib3270_get_text(	lib3270_get_default_session_handle(),
											((struct hllapi_packet_query_offset *) source->buffer)->addr,
											((struct hllapi_packet_query_offset *) source->buffer)->len));
		break;

	case HLLAPI_PACKET_CMP_TEXT_AT:
		send_result(source,lib3270_cmp_text_at(	lib3270_get_default_session_handle(),
											((struct hllapi_packet_text_at *) source->buffer)->row,
											((struct hllapi_packet_text_at *) source->buffer)->col,
											((struct hllapi_packet_text_at *) source->buffer)->text));
		break;

	case HLLAPI_PACKET_INPUT_STRING:
		send_result(source,lib3270_input_string(lib3270_get_default_session_handle(),
												(unsigned char *) ((struct hllapi_packet_text *) source->buffer)->text));
		break;

	case HLLAPI_PACKET_EMULATE_INPUT:
		send_result(source,lib3270_emulate_input(lib3270_get_default_session_handle(),
												(const char *) ((struct hllapi_packet_emulate_input *) source->buffer)->text,
												(int) ((struct hllapi_packet_emulate_input *) source->buffer)->len,
												(int) ((struct hllapi_packet_emulate_input *) source->buffer)->pasting));
		break;

	case HLLAPI_PACKET_SET_CURSOR:
		send_result(source,lib3270_set_cursor_address(lib3270_get_default_session_handle(),
												((struct hllapi_packet_addr *) source->buffer)->addr));
		break;

	case HLLAPI_PACKET_GET_CURSOR:
		send_result(source,lib3270_get_cursor_address(lib3270_get_default_session_handle()));
		break;

	case HLLAPI_PACKET_GET_CSTATE:
		send_result(source,lib3270_get_connection_state(lib3270_get_default_session_handle()));
		break;
	case HLLAPI_PACKET_SET_TOGGLE:
		send_result(source,lib3270_set_toggle(lib3270_get_default_session_handle(),
												((struct hllapi_packet_set *) source->buffer)->id,												((struct hllapi_packet_set *) source->buffer)->value));		break;
	default:
		send_result(source, EINVAL);
		g_message("Invalid remote request (id=%d)",source->buffer[0]);
	}

 }

 static void read_input_pipe(pipe_source *source)
 {
	DWORD cbRead	= 0;

	if(ReadFile(source->hPipe,source->buffer,PIPE_BUFFER_LENGTH,&cbRead,&source->overlap) && cbRead > 0)
		process_input(source,cbRead);

	// The read operation is still pending.
	switch(GetLastError())
	{
	case 0:
		break;

	case ERROR_IO_PENDING:
		// trace("%s: PIPE_STATE_PENDING_READ",__FUNCTION__);
		source->state = PIPE_STATE_PENDING_READ;
		break;

	case ERROR_PIPE_LISTENING:
		// trace("%s: ERROR_PIPE_LISTENING",__FUNCTION__);
		source->state = PIPE_STATE_READ;
		break;

	case ERROR_BROKEN_PIPE:
		trace("%s: ERROR_BROKEN_PIPE",__FUNCTION__);

		if(!DisconnectNamedPipe(source->hPipe))
		{
			set_active(FALSE);
			popup_lasterror("%s",_( "Error in DisconnectNamedPipe" ));
		}
		else
		{
			IO_accept(source);
		}
		break;

	case ERROR_PIPE_NOT_CONNECTED:
		trace("%s: ERROR_PIPE_NOT_CONNECTED",__FUNCTION__);
		set_active(FALSE);
		break;

	default:
		if(source->hPipe != INVALID_HANDLE_VALUE)
			popup_lasterror("%s",_( "Error receiving message from pipe" ) );
	}

 }

 static gboolean IO_dispatch(GSource *source, GSourceFunc callback, gpointer data)
 {
	/*
	 * Called to dispatch the event source,
	 * after it has returned TRUE in either its prepare or its check function.
	 * The dispatch function is passed in a callback function and data.
	 * The callback function may be NULL if the source was never connected
	 * to a callback using g_source_set_callback(). The dispatch function
	 * should call the callback function with user_data and whatever additional
	 * parameters are needed for this type of event source.
	 */
	BOOL	fSuccess;
	DWORD	cbRead	= 0;
//	DWORD	dwErr	= 0;

	fSuccess = GetOverlappedResult(((pipe_source *) source)->hPipe,&((pipe_source *) source)->overlap,&cbRead,FALSE );

	// trace("%s: source=%p data=%p Result=%s cbRead=%d",__FUNCTION__,source,data,fSuccess ? "Success" : "Unsuccess",(int) cbRead);

	switch(((pipe_source *) source)->state)
	{
	case PIPE_STATE_WAITING:
		if(fSuccess)
		{
			trace("Pipe connected (cbRet=%d)",(int) cbRead);
			set_active(TRUE);
			((pipe_source *) source)->state = PIPE_STATE_READ;
		}
		else
		{
			popup_lasterror("%s", _( "Pipe connection failed" ));
		}
		break;

	case PIPE_STATE_READ:
		// trace("Reading pipe (cbRead=%d)",(int) cbRead);
		read_input_pipe( (pipe_source *) source);
		break;

	case PIPE_STATE_PENDING_READ:
		if(fSuccess && cbRead > 0)
			process_input((pipe_source *) source,cbRead);
		((pipe_source *) source)->state = PIPE_STATE_READ;
		break;

	case PIPE_STATE_UNDEFINED:
		break;

//#ifdef DEBUG
//	default:
//		trace("%s: source=%p data=%p Unexpected mode %d",__FUNCTION__,source,data,((pipe_source *) source)->state);
//#endif
	}

	return TRUE;
 }

 static void IO_finalize(GSource *source)
 {
//	trace("%s: source=%p",__FUNCTION__,source);

	if( ((pipe_source *) source)->hPipe != INVALID_HANDLE_VALUE)
	{
		CloseHandle(((pipe_source *) source)->hPipe);
		((pipe_source *) source)->hPipe = INVALID_HANDLE_VALUE;
	}

 }

 static gboolean IO_closure(gpointer data)
 {
//	trace("%s: data=%p",__FUNCTION__,data);
	return 0;
 }

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

 LIB3270_EXPORT int pw3270_plugin_start(GtkWidget *window)
 {
	char id;

	for(id='A';id < 'Z';id++)
	{
		gchar	* pipename = g_strdup_printf("\\\\.\\pipe\\%s_%c",pw3270_get_session_name(window),id);
		gchar 	* ptr;
		HANDLE	  hPipe;

		for(ptr=pipename;*ptr;ptr++)
			*ptr = g_ascii_tolower(*ptr);

		hPipe = CreateNamedPipe(	TEXT(pipename),				// pipe name
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
			static GSourceFuncs pipe_source_funcs =
			{
				IO_prepare,
				IO_check,
				IO_dispatch,
				IO_finalize,
				IO_closure,
				NULL
			};
			pipe_source  	* source;
			gchar 			* session = g_strdup_printf("%s:%c",pw3270_get_session_name(window),id);

			pw3270_set_session_name(window,session);
			g_free(session);

			source = (pipe_source *) g_source_new(&pipe_source_funcs,sizeof(pipe_source));

			source->hPipe			= hPipe;
			source->state			= PIPE_STATE_WAITING;
			source->overlap.hEvent	= CreateEvent( NULL,TRUE,TRUE,NULL);

			g_source_attach((GSource *) source,NULL);
			IO_accept(source);

			return 0;
		}

	}

	popup_lasterror( "%s", _( "Can´t create remote control pipe" ));

	return -1;
 }

 LIB3270_EXPORT int pw3270_plugin_stop(GtkWidget *window)
 {

	return 0;
 }

/*
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

*/
