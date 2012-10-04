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
 * Este programa está nomeado como pipesource.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

 #include <pw3270.h>
 #include <pw3270/v3270.h>

#ifdef WIN32

 #include <windows.h>
 #include <stdarg.h>
 #include "remotectl.h"

/*---[ Defines ]----------------------------------------------------------------------------*/

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

/*---[ GSource ]----------------------------------------------------------------------------*/

static void wait_for_client(pipe_source *source)
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
	{
		// trace("%s: source=%p",__FUNCTION__,source);
		return TRUE;
	}

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

 static void process_input(pipe_source *source, DWORD cbRead)
 {
	HLLAPI_DATA * data	= (HLLAPI_DATA *) source->buffer;
 	QUERY 		* qry	= g_malloc0(sizeof(QUERY)+cbRead+1);

	qry->hPipe	= source->hPipe;
 	qry->text	= (const gchar *) (qry+1);

	if(data->id == 0x01)
	{
		// HLLAPI query
		qry->cmd	= (int) data->func;
		qry->pos	= (int) data->rc;
		qry->length	= data->len;
		memcpy((gchar *)(qry->text),data->string,qry->length);
	}
	else
	{
		qry->cmd = -1;
	}

	enqueue_request(qry);
 }

 void request_complete(QUERY *qry, int rc, const gchar *text)
 {
 	request_buffer(qry,rc,strlen(text),(const gpointer) text);
 }

 void request_status(QUERY *qry, int rc)
 {
 	if(rc)
	{
		const gchar *msg = strerror(rc);
		request_buffer(qry, rc, strlen(msg), (const gpointer) msg);
	}
	else
	{
		request_buffer(qry, rc, 0, NULL);
	}
/*
	HLLAPI_DATA data;

	memset(&data,0,sizeof(data));

	data.id		= 0x01;
	data.func	= qry->cmd;
	data.rc		= rc;

	trace("rc=%d",rc);

#ifdef WIN32
	{
		DWORD wrote = sizeof(data);
		WriteFile(qry->hPipe,&data,wrote,&wrote,NULL);
	}
#endif // WIN32

 	g_free(qry);
*/
 }

 void request_buffer(QUERY *qry, int rc, size_t szBuffer, const gpointer buffer)
 {
 	size_t			sz;
	HLLAPI_DATA 	*data;

	if(buffer)
	{
		sz 			= sizeof(HLLAPI_DATA)+szBuffer;
		data		= g_malloc0(sz);
		data->len	= szBuffer;
		memcpy(data->string,buffer,szBuffer);
	}
	else
	{
		sz		= sizeof(HLLAPI_DATA);
		data	= g_malloc0(sz);
	}

	data->id	= 0x01;
	data->func	= qry->cmd;
	data->rc	= rc;

	trace("rc=%d data->len=%d",rc,(int) data->len);

#ifdef WIN32
	{
		DWORD wrote = sz;
		WriteFile(qry->hPipe,data,wrote,&wrote,NULL);
		trace("Wrote=%d len=%d",(int) wrote, (int) sz);
	}
#endif // WIN32

	g_free(data);
 	g_free(qry);

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
			wait_for_client(source);
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


 void init_source_pipe(HANDLE hPipe)
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

	pipe_source *source = (pipe_source *) g_source_new(&pipe_source_funcs,sizeof(pipe_source));

	source->hPipe			= hPipe;
	source->state			= PIPE_STATE_WAITING;
	source->overlap.hEvent	= CreateEvent( NULL,TRUE,TRUE,NULL);

	g_source_attach((GSource *) source,NULL);

	wait_for_client(source);

	return;
 }

#endif // WIN32
