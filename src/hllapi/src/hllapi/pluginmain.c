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

 #include "server.h"

#ifdef _WIN32
	#include <windows.h>
#else
	#error HLLAPI is designed for windows.
#endif // _WIN32

 #include <pw3270/plugin.h>
 #include <v3270.h>
 #include <pw3270/ipcpackets.h>
 #include <lib3270/actions.h>
 #include <lib3270/charset.h>

/*--[ Defines ]--------------------------------------------------------------------------------------*/

 #pragma pack(1)

 enum PIPE_STATE
 {
	PIPE_STATE_WAITING,
	PIPE_STATE_READ,
	PIPE_STATE_PENDING_READ,
	PIPE_STATE_UNDEFINED
 };

 typedef struct _pipe_source
 {
	GSource 			gsrc;
	HANDLE				hPipe;

	enum PIPE_STATE		state;

	OVERLAPPED			overlap;
	unsigned char		buffer[PIPE_BUFFER_LENGTH+1];
 } pipe_source;

 #pragma pack()


/*--[ Globals ]--------------------------------------------------------------------------------------*/

//  static const gchar	  control_char	= '@';

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
	int f;

	if(text)
	{
		szBlock = sizeof(struct hllapi_packet_text)+strlen(text);
		pkt = (struct hllapi_packet_text *) g_malloc0(szBlock);
		pkt->packet_id = 0;
		strcpy(pkt->text,text);
		lib3270_free(text);
	}
	else
	{
		szBlock = sizeof(struct hllapi_packet_text);
		pkt = (struct hllapi_packet_text *) g_malloc0(szBlock);
		pkt->packet_id = errno ? errno : -1;
	}

	trace("szBlock=%d text=\"%s\"",szBlock, ( (struct hllapi_packet_text *) pkt)->text);
	for(f=0;f< (int) szBlock;f++)
	{
		trace("rsp(%d)= %d \"%s\"",f,* (((char *) pkt)+f),((char *) pkt)+f);
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

 static int do_file_transfer(struct hllapi_packet_file_transfer * source)
 {
 	/*
 	const gchar	* local		= (const char *) source->text;
 	const gchar	* remote	= (const char *) (local+strlen(local)+1);

	return v3270_transfer_file(	v3270_get_default_widget(),
								(LIB3270_FT_OPTION) source->options,
								local,
								remote,
								source->lrecl,
								source->blksize,
								source->primspace,
								source->secspace,
								source->dft );
	*/
	return EINVAL;
 }

 static void get_host(pipe_source *source) {
	send_text(source,strdup(lib3270_get_url(lib3270_get_default_session_handle())));
 }

 static void process_input(pipe_source *source, DWORD cbRead)
 {
	const struct hllapi_packet_query * query = ((struct hllapi_packet_query *) source->buffer);

 	trace("%s id=%d",__FUNCTION__,query->packet_id);

	switch(query->packet_id)
	{
	case HLLAPI_PACKET_CONNECT:
		send_result(source,lib3270_reconnect(	lib3270_get_default_session_handle(),
											((struct hllapi_packet_connect *) source->buffer)->wait));
		break;

	case HLLAPI_PACKET_CONNECT_URL:
		send_result(source,lib3270_connect_url(lib3270_get_default_session_handle(),(const char *) (query+1),0));
		break;

	case HLLAPI_PACKET_SET_HOST:
		send_result(source,lib3270_set_url(lib3270_get_default_session_handle(),
											((struct hllapi_packet_text *) source->buffer)->text));
		break;

	case HLLAPI_PACKET_GET_HOST:
		get_host(source);
		break;

	case HLLAPI_PACKET_DISCONNECT:
		send_result(source,lib3270_disconnect(lib3270_get_default_session_handle()));
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
		send_result(source,lib3270_print_all(lib3270_get_default_session_handle()));
		break;

	case HLLAPI_PACKET_ERASE:
		send_result(source,lib3270_erase(lib3270_get_default_session_handle()));
		break;

	case HLLAPI_PACKET_ERASE_EOF:
		send_result(source,lib3270_eraseeof(lib3270_get_default_session_handle()));
		break;

	case HLLAPI_PACKET_ERASE_EOL:
		send_result(source,lib3270_eraseeol(lib3270_get_default_session_handle()));
		break;

	case HLLAPI_PACKET_ERASE_INPUT:
		send_result(source,lib3270_eraseinput(lib3270_get_default_session_handle()));
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
		send_text(source,lib3270_get_string_at(	lib3270_get_default_session_handle(),
											((struct hllapi_packet_at *) source->buffer)->row,
											((struct hllapi_packet_at *) source->buffer)->col,
											((struct hllapi_packet_at *) source->buffer)->len,
											((struct hllapi_packet_at *) source->buffer)->lf));
		break;

	case HLLAPI_PACKET_GET_TEXT_AT_OFFSET:
		send_text(source,lib3270_get_string_at_address(	lib3270_get_default_session_handle(),
											((struct hllapi_packet_query_offset *) source->buffer)->addr,
											((struct hllapi_packet_query_offset *) source->buffer)->len,
											((struct hllapi_packet_query_offset *) source->buffer)->lf));
		break;

	case HLLAPI_PACKET_CMP_TEXT_AT:
		send_result(source,lib3270_cmp_text_at(	lib3270_get_default_session_handle(),
											((struct hllapi_packet_text_at *) source->buffer)->row,
											((struct hllapi_packet_text_at *) source->buffer)->col,
											((struct hllapi_packet_text_at *) source->buffer)->text,
											((struct hllapi_packet_text_at *) source->buffer)->lf));
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

	case HLLAPI_PACKET_GET_WIDTH:
		send_result(source,lib3270_get_width(lib3270_get_default_session_handle()));
		break;

	case HLLAPI_PACKET_GET_HEIGHT:
		send_result(source,lib3270_get_height(lib3270_get_default_session_handle()));
		break;

	case HLLAPI_PACKET_GET_LENGTH:
		send_result(source,lib3270_get_length(lib3270_get_default_session_handle()));
		break;

	case HLLAPI_PACKET_GET_PROGRAM_MESSAGE:
		send_result(source,lib3270_get_program_message(lib3270_get_default_session_handle()));
		break;

	case HLLAPI_PACKET_GET_SSL_STATE:
		send_result(source,lib3270_get_secure(lib3270_get_default_session_handle()));
		break;

	case HLLAPI_PACKET_SET_UNLOCK_DELAY:
		lib3270_set_unlock_delay(lib3270_get_default_session_handle(),(unsigned short) ((struct hllapi_packet_set_int *) source->buffer)->value);
		send_result(source,0);
		break;

	case HLLAPI_PACKET_SET_TOGGLE:
		send_result(source,lib3270_set_toggle(lib3270_get_default_session_handle(),
												(LIB3270_TOGGLE) ((struct hllapi_packet_set *) source->buffer)->id,
												((struct hllapi_packet_set *) source->buffer)->value));
		break;

    case HLLAPI_PACKET_FIELD_START:
		send_result(source,lib3270_get_field_start(lib3270_get_default_session_handle(),
												((struct hllapi_packet_addr *) source->buffer)->addr));
		break;


    case HLLAPI_PACKET_FIELD_LEN:
		send_result(source,lib3270_get_field_len(lib3270_get_default_session_handle(),
												((struct hllapi_packet_addr *) source->buffer)->addr));
		break;

    case HLLAPI_PACKET_NEXT_UNPROTECTED:
		send_result(source,lib3270_get_next_unprotected(lib3270_get_default_session_handle(),
												((struct hllapi_packet_addr *) source->buffer)->addr));
		break;

    case HLLAPI_PACKET_IS_PROTECTED:
		send_result(source,lib3270_get_is_protected(lib3270_get_default_session_handle(),
												((struct hllapi_packet_addr *) source->buffer)->addr));
		break;

    case HLLAPI_PACKET_IS_PROTECTED_AT:
		send_result(source,lib3270_get_is_protected_at(	lib3270_get_default_session_handle(),
												((struct hllapi_packet_query_at *) source->buffer)->row,
												((struct hllapi_packet_query_at *) source->buffer)->col));
		break;

	case HLLAPI_PACKET_QUIT:
		gtk_main_quit();
		send_result(source,0);
		break;

	case HLLAPI_PACKET_SET_HOST_CHARSET:
		send_result(source,lib3270_set_host_charset(	lib3270_get_default_session_handle(),
														(const char *) ((struct hllapi_packet_set_text *) source->buffer)->text));
		break;

	case HLLAPI_PACKET_ASC2EBC:
		send_text(source,(char *) lib3270_asc2ebc(
								lib3270_get_default_session_handle(),
								(unsigned char *) ((struct hllapi_packet_set_text *) source->buffer)->text,-1
								));
		break;

	case HLLAPI_PACKET_EBC2ASC:
		send_text(source,(char *) lib3270_ebc2asc(
								lib3270_get_default_session_handle(),
								(unsigned char *) ((struct hllapi_packet_set_text *) source->buffer)->text,-1
								));
		break;

	case HLLAPI_PACKET_FILE_TRANSFER:
		send_result(source,do_file_transfer((struct hllapi_packet_file_transfer *) source));
		break;

	case HLLAPI_PACKET_GET_HOST_CHARSET:
		trace("%s","HLLAPI_PACKET_GET_HOST_CHARSET");
		send_text(source,(char *) lib3270_get_host_charset(lib3270_get_default_session_handle()));
		break;

	case HLLAPI_PACKET_ACTION:
		send_result(source,lib3270_action(lib3270_get_default_session_handle(),
								(const char *) ((struct hllapi_packet_text *) source->buffer)->text));
		break;


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

	for(ptr= (char *) lpMsgBuf;*ptr && *ptr != '\n';ptr++);
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

 LIB3270_EXPORT int pw3270_plugin_start(GtkWidget *window, GtkWidget *terminal)
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

 LIB3270_EXPORT int pw3270_plugin_stop(GtkWidget *window, GtkWidget *terminal)
 {

	return 0;
 }

 G_GNUC_INTERNAL void set_active(gboolean on)
 {
 	trace("%s(%s)",__FUNCTION__,on ? "Active" : "Inactive");
	// v3270_set_script(v3270_get_default_widget(),'H');
 }
