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
 * Este programa está nomeado como ft.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

#include <lib3270/config.h>
#include <lib3270.h>
#include <lib3270/filetransfer.h>
#include "globals.h"
#include <lib3270/trace.h>

#include <errno.h>

#ifdef HAVE_MALLOC_H
	#include <malloc.h>
#endif // HAVE_MALLOC_H

#include "ft_cutc.h"
#include "ft_dftc.h"
#include "ftc.h"
#include "hostc.h"
#include "kybdc.h"
#include "popupsc.h"
#include "screenc.h"
// #include "tablesc.h"
#include "telnetc.h"
#include "utilc.h"
#include "trace_dsc.h"

static void ft_connected(H3270 *session, int ignored, void *unused);
static void ft_in3270(H3270 *session, int ignored unused, void *unused);

/* Macros. */
#define eos(s)	strchr((s), '\0')

#if defined(X3270_DISPLAY)
#define FILE_WIDTH	300	// width of file name widgets
#define MARGIN		3	// distance from margins to widgets
#define CLOSE_VGAP	0	// distance between paired toggles
#define FAR_VGAP	10	// distance between single toggles and groups
#define BUTTON_GAP	5	// horizontal distance between buttons
#define COLUMN_GAP	40	// distance between columns
#endif

#define BN	(Boolean *)NULL

#define snconcat(x,s,fmt,...) snprintf(x+strlen(x),s-strlen(x),fmt,__VA_ARGS__)

static void set_ft_state(H3270FT *session, LIB3270_FT_STATE state);


/*---[ Implement ]-------------------------------------------------------------------------------------------------------*/

 H3270FT * get_ft_handle(H3270 *hSession)
 {
 	if(!hSession->ft)
	{
		popup_an_error(hSession,_( "Unexpected call to %s: No active filetransfer" ),__FUNCTION__);
	}
 	return hSession->ft;
 }

 static void set_ft_state(H3270FT *session, LIB3270_FT_STATE state)
 {
 	static const struct _msg
 	{
 		LIB3270_FT_STATE 	  state;
 		const char			* text;
 	} msg[] =
 	{
		{ LIB3270_FT_STATE_NONE,		N_( "No transfer in progress" )				},
		{ LIB3270_FT_STATE_RUNNING,		N_( "Ack received, data flowing" )			},
		{ LIB3270_FT_STATE_ABORT_WAIT,	N_( "Awaiting chance to send an abort" )	},
		{ LIB3270_FT_STATE_ABORT_SENT,	N_( "Abort sent; awaiting response" )		},
	};

	int f;

	if(session->state == state)
		return;
	session->state = state;
	session->state_changed(session,state);

	for(f = 0; f < sizeof(msg)/sizeof(msg[0]);f++)
	{
		if(msg[f].state == state)
		{
			ft_message(session,msg[f].text);
			break;
		}
	}

 }

 void ft_init(H3270 *hSession)
 {
	/* Register for state changes. */
	lib3270_register_schange(hSession, LIB3270_STATE_CONNECT, ( void (*)(H3270 *, int, void *)) ft_connected, NULL);
	lib3270_register_schange(hSession, LIB3270_STATE_3270_MODE, ( void (*)(H3270 *, int, void *)) ft_in3270, NULL);
 }

 LIB3270_EXPORT int lib3270_ft_cancel(H3270 *hSession, int force)
 {
 	H3270FT *ft;

	CHECK_SESSION_HANDLE(hSession);

	ft = get_ft_handle(hSession);

	if (ft->state == LIB3270_FT_STATE_RUNNING)
	{
		set_ft_state(ft,LIB3270_FT_STATE_ABORT_WAIT);
		ft->aborting(ft);
		return 0;
	}

	if(!force)
		return EBUSY;

	// Impatient user or hung host -- just clean up.
	ft_complete(ft, N_("Cancelled by user") );

	return 0;
 }

 static void def_complete(H3270FT *ft,unsigned long length,double kbytes_sec)
 {
 }

 static void def_message(H3270FT *ft, const char *errmsg)
 {
	lib3270_write_log(ft->host,"ft","%s",errmsg);
 }

 static void def_update(H3270FT *ft, unsigned long current, unsigned long length, double kbytes_sec)
 {

 }

 static void def_running(H3270FT *ft, int is_cut)
 {

 }

 static void def_aborting(H3270FT *ft)
 {

 }

 static void def_state_changed(H3270FT *ft, LIB3270_FT_STATE state)
 {

 }


 LIB3270_EXPORT H3270FT * lib3270_ft_new(H3270 *session, LIB3270_FT_OPTION flags, const char *local, const char *remote, int lrecl, int blksize, int primspace, int secspace, int dft)
 {
 	H3270FT				* ftHandle		= (H3270FT *) session->ft;
 	FILE				* ft_local_file	= NULL;
// 	unsigned long		  length		= 0L;

//	trace("%s(%s)",__FUNCTION__,local);
	if(!lib3270_connected(session))
	{
		lib3270_popup_dialog(	session,
								LIB3270_NOTIFY_ERROR,
								_( "Request failed" ),
								_( "Can't start file transfer." ),
								"%s",
								_( "Disconnected from host." ));
		errno = ENOTCONN;
		return NULL;
	}

	if(ftHandle)
	{
		lib3270_popup_dialog(	session,
								LIB3270_NOTIFY_ERROR,
								_( "Request failed" ),
								_( "Can't start file transfer." ),
								"%s",
								_( "File transfer is already active in this session." ));

		errno = EBUSY;
		return NULL;
	}

	// Check remote file
	if(!*remote)
	{
		lib3270_popup_dialog(	session,
								LIB3270_NOTIFY_ERROR,
								_( "Request failed" ),
								_( "Can't start file transfer." ),
								"%s",
								_( "The remote file name is invalid." ));
		errno = EINVAL;
		return NULL;
	}

	// Open local file
#ifdef _WIN32

	char 	openmode[5] = "";
	int		opensize = 0;

	openmode[opensize++] = (flags & LIB3270_FT_OPTION_RECEIVE)	? 'r' : 'w';
	openmode[opensize++] = (flags & LIB3270_FT_OPTION_CRLF)		? 't' : 'b';

	// É para adicionar?
	if(flags & LIB3270_FT_OPTION_APPEND) {
		openmode[opensize++] = 'a';
	}

	openmode[opensize++] = 0;

	debug("Opening \"%s\" with \"%s\"",local,openmode);

	ft_local_file = fopen(local,openmode);

#else
	ft_local_file = fopen(local,(flags & LIB3270_FT_OPTION_RECEIVE) ? ((flags & LIB3270_FT_OPTION_APPEND) ? "a" : "w") : "r");
#endif // _WIN32

	if(!ft_local_file)
	{
		lib3270_popup_dialog(	session,
								LIB3270_NOTIFY_ERROR,
								_( "Request failed" ),
								_( "Can't open local file." ),
								"%s",
								strerror(errno));
		return NULL;
	}

	// Set options
	session->dft_buffersize = dft;
	set_dft_buffersize(session);

	// Create & Initialize ft control structure.
	ftHandle = lib3270_malloc(sizeof(H3270FT)+strlen(local)+strlen(remote)+3);

	ftHandle->sz 			= sizeof(H3270FT);
	ftHandle->host			= session;

	ftHandle->ft_last_cr	= 0;

	ftHandle->ascii_flag	= (flags & LIB3270_FT_OPTION_ASCII)	? 1 : 0;
	ftHandle->cr_flag   	= (flags & LIB3270_FT_OPTION_CRLF)	? 1 : 0;
	ftHandle->remap_flag	= (flags & LIB3270_FT_OPTION_REMAP)	? 1 : 0;
	ftHandle->ft_is_cut  	= 0;
	ftHandle->flags			= flags;
	ftHandle->local_file	= ft_local_file;
	ftHandle->state			= LIB3270_FT_STATE_AWAIT_ACK;
	ftHandle->complete 		= def_complete;
	ftHandle->message 		= def_message;
	ftHandle->update 		= def_update;
	ftHandle->running 		= def_running;
	ftHandle->aborting 		= def_aborting;
	ftHandle->state_changed	= def_state_changed;
	ftHandle->lrecl			= lrecl;
	ftHandle->blksize		= blksize;
	ftHandle->primspace		= primspace;
	ftHandle->secspace		= secspace;
	ftHandle->dft			= dft;
	ftHandle->quadrant		= -1;

	memcpy(&ftHandle->charset,&session->charset,sizeof(struct lib3270_charset));

	ftHandle->local			= (char *) (ftHandle+1);
	strcpy((char *) ftHandle->local,local);

	ftHandle->remote		= ftHandle->local + strlen(ftHandle->local)+1;
	strcpy((char *) ftHandle->remote,remote);

	session->ft				= ftHandle;

 	return ftHandle;
 }

 LIB3270_EXPORT int lib3270_ft_start(H3270 *hSession)
 {
 	static const char	* rec			= "FVU";
 	static const char	* un[]			= { "tracks", "cylinders", "avblock" };

 	char 				  op[4096];
 	char				  buffer[4096];
	unsigned int		  flen;
 	unsigned short		  recfm;
 	unsigned short		  units;
 	H3270FT 			* ft;

	CHECK_SESSION_HANDLE(hSession);

	ft = get_ft_handle(hSession);
	if(!ft)
		return EINVAL;

 	recfm			= (ft->flags & FT_RECORD_FORMAT_MASK) >> 8;
 	units			= (ft->flags & FT_ALLOCATION_UNITS_MASK) >> 12;
	ft->ascii_flag	= (ft->flags & LIB3270_FT_OPTION_ASCII)	? 1 : 0;
	ft->cr_flag   	= (ft->flags & LIB3270_FT_OPTION_CRLF)	? 1 : 0;
	ft->remap_flag	= (ft->flags & LIB3270_FT_OPTION_REMAP)	? 1 : 0;

	if(ft->flags & LIB3270_FT_OPTION_RECEIVE)
	{
		// Receiving file
		lib3270_write_dstrace(
					ft->host,
					"\nReceiving file %s (%s %s %s)\n",
					ft->local,
					ft->ascii_flag	?	"ASCII" : "BINARY",
					ft->cr_flag 	?	"CRLF" 	: "NOCRLF",
					ft->remap_flag	?	"REMAP" : "NOREMAP"
				);
	}
	else
	{
		// Sending file
		if(fseek(ft->local_file,0L,SEEK_END) < 0)
		{
			ft_complete(ft,N_( "Can't get file size" ));
			return errno ? errno : -1;
		}

		ft->length = ftell(ft->local_file);

		lib3270_write_dstrace(
				ft->host,
				"\nSending file %s (%ld bytes %s %s %s)\n",
				ft->local,
				ft->length,
				ft->ascii_flag	?	"ASCII" : "BINARY",
				ft->cr_flag		?	"CRLF"	: "NOCRLF",
				ft->remap_flag	?	"REMAP" : "NOREMAP"
			);

		rewind(ft->local_file);
	}

 	/* Build the ind$file command */
 	snprintf(op,4095,"%s%s%s",
						(ft->flags & LIB3270_FT_OPTION_ASCII) 	? " ASCII"	: "",
						(ft->flags & LIB3270_FT_OPTION_CRLF) 	? " CRLF"	: "",
						(ft->flags & LIB3270_FT_OPTION_APPEND)	? " APPEND"	: ""
			);

	trace("tso=%s",hSession->options & LIB3270_OPTION_TSO ? "yes" : "No");

	if(!(ft->flags & LIB3270_FT_OPTION_RECEIVE))
	{
		// Sending file

		if(hSession->options & LIB3270_OPTION_TSO)
		{
			// TSO Host
			if(recfm > 0)
			{
				snconcat(op,4096," RECFM(%c)",rec[recfm-1]);

				if(ft->lrecl > 0)
					snconcat(op,4096," LRECL(%d)",ft->lrecl);

				if(ft->blksize > 0)
					snconcat(op,4096," BLKSIZE(%d)", ft->blksize);
			}

			if(units > 0)
			{
				snconcat(op,4096," %s",un[units-1]);

				if(ft->primspace > 0)
				{
					snconcat(op,4096," SPACE(%d",ft->primspace);
					if(ft->secspace)
						snconcat(op,4096,",%d",ft->secspace);
					snconcat(op,4096,"%s",")");
				}
			}
		}
		else
		{
			// VM Host
			if(recfm > 0)
			{
				snconcat(op,4096," RECFM %c",rec[recfm-1]);

				if(ft->lrecl > 0)
					snconcat(op,4096," LRECL %d",ft->lrecl);

			}
		}
	}

	snprintf(buffer,4095,"%s %s %s",	"IND$FILE",
										(ft->flags & LIB3270_FT_OPTION_RECEIVE) ? "GET" : "PUT",
										ft->remote );

	if(*op)
	{
		if(hSession->options & LIB3270_OPTION_TSO)
			snconcat(buffer,4095," %s",op+1);
		else
			snconcat(buffer,4095," (%s",op+1);
	}

	snconcat(buffer,4095,"%s","\n");
	trace_ds(hSession,"\n%s file using %s",(ft->flags & LIB3270_FT_OPTION_RECEIVE) ? "Receiving" : "Sending",buffer);

	// Erase the line and enter the command.
	flen = kybd_prime(ft->host);
	if (!flen || flen < strlen(buffer) - 1)
	{
		lib3270_write_log(ft->host, "Unable to send command \"%s\" (flen=%d szBuffer=%d)",buffer,flen,strlen(buffer));
		ft_complete(ft,N_( "Unable to send file-transfer request" ));
		return errno = EINVAL;
	}

	lib3270_trace_event(hSession,"Sending FT request:\n%s\n",buffer);

	lib3270_emulate_input(ft->host, buffer, strlen(buffer), False);

	if(ft->flags & LIB3270_FT_OPTION_RECEIVE)
		ft_message(ft,N_( "Waiting for GET response" ));
	else
		ft_message(ft,N_( "Waiting for PUT response" ));

	return 0;

 }


/* External entry points called by ft_dft and ft_cut. */
void ft_message(H3270FT *ft, const char *msg)
{
	lib3270_trace_event(ft->host,"%s\n",msg);
	ft->message(ft,msg);
}

/**
 * Pop up a message, end the transfer, release resources.
 *
 */
void ft_complete(H3270FT *ft, const char *errmsg)
{
	double			  kbytes_sec = 0;
	struct timeval	  t1;

	(void) gettimeofday(&t1, (struct timezone *)NULL);
	kbytes_sec = (double) ft->ft_length / 1024.0 /
		((double)(t1.tv_sec - ft->starting_time.tv_sec) +
		 (double)(t1.tv_usec - ft->starting_time.tv_usec) / 1.0e6);

	// Close the local file.
	if(ft->local_file)
	{
		fclose(ft->local_file);
		ft->local_file = NULL;
	}

	// Clean up the state.
	set_ft_state(ft,FT_NONE);

	ft_update_length(ft);

	ft->complete(ft,ft->ft_length,kbytes_sec);

	ft_message(ft,errmsg ? errmsg : N_("Transfer complete"));


}

LIB3270_EXPORT int lib3270_ft_destroy(H3270 *hSession)
{
	H3270FT *session;

	CHECK_SESSION_HANDLE(hSession);

	session = (H3270FT *) hSession->ft;
	if(!session)
		return EINVAL;

	if (session->state != LIB3270_FT_STATE_NONE)
		lib3270_ft_cancel(hSession,1);

	if(session->local_file)
	{
		fclose(session->local_file);
		session->local_file = NULL;
	}

	hSession->ft = NULL;

	lib3270_free(session);

	return 0;
}

// Update the bytes-transferred count on the progress pop-up.
void ft_update_length(H3270FT *session)
{
	double kbytes_sec = 0;

	if(session->ft_length > 1024.0)
	{
		struct timeval	t1;

		(void) gettimeofday(&t1, (struct timezone *)NULL);
		kbytes_sec = (double)session->ft_length / 1024.0 /
			((double)(t1.tv_sec - session->starting_time.tv_sec) +
			 (double)(t1.tv_usec - session->starting_time.tv_usec) / 1.0e6);
	}

	session->update(session,session->ft_length,session->length,kbytes_sec);

}

/**
 * Process a transfer acknowledgement.
 *
 */
void ft_running(H3270FT *ft, Boolean is_cut)
{
	ft->ft_is_cut = is_cut ? 1 : 0;
	ft->ft_length = 0;

	gettimeofday(&ft->starting_time, (struct timezone *)NULL);

	if (ft->state == FT_AWAIT_ACK)
		set_ft_state(ft,FT_RUNNING);

	ft->running(ft,is_cut);

	ft_update_length(ft);

}

// Process a protocol-generated abort.
void ft_aborting(H3270FT *h)
{
	if (h->state == FT_RUNNING || h->state == FT_ABORT_WAIT)
	{
		set_ft_state(h,FT_ABORT_SENT);
		h->message(h,N_("Aborting..."));
		h->aborting(h);
	}
}

/* Process a disconnect abort. */
static void ft_connected(H3270 *hSession, int ignored, void *dunno)
{
	if (!CONNECTED && lib3270_get_ft_state(hSession) != LIB3270_FT_STATE_NONE)
		ft_complete(get_ft_handle(hSession),_("Host disconnected, transfer cancelled"));
}

/* Process an abort from no longer being in 3270 mode. */
static void ft_in3270(H3270 *hSession, int ignored, void *dunno)
{
	if (!IN_3270 && lib3270_get_ft_state(hSession) != LIB3270_FT_STATE_NONE)
		ft_complete(get_ft_handle(hSession),_("Not in 3270 mode, transfer cancelled"));
}

LIB3270_EXPORT LIB3270_FT_STATE lib3270_get_ft_state(H3270 *session)
{
	CHECK_SESSION_HANDLE(session);

	if(!session->ft)
		return LIB3270_FT_STATE_NONE;

	return ((H3270FT *) session->ft)->state;
}
