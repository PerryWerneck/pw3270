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
#include "private.h"
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
	const char *message = NULL;

	if(session->state == state)
		return;

	// State changed, notify

	for(f = 0; f < sizeof(msg)/sizeof(msg[0]);f++)
	{
		if(msg[f].state == state)
		{
			message = msg[f].text;
			break;
		}
	}

	session->state = state;

	ft_message(session,message);
	session->cbk.state_changed(session->host,state,message,session->user_data);


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
	if(!ft)
		return EINVAL;

	if (ft->state == LIB3270_FT_STATE_RUNNING)
	{
		set_ft_state(ft,LIB3270_FT_STATE_ABORT_WAIT);
		ft->cbk.aborting(hSession,ft->user_data);
		return 0;
	}

	if(!force)
		return EBUSY;

	// Impatient user or hung host -- just clean up.
	ft_failed(ft, N_("Cancelled by user") );

	return 0;
 }

 static void def_complete(H3270 *hSession, unsigned long length,double kbytes_sec,const char *msg, void *userdata)
 {
 	hSession->ft->cbk.message(hSession,msg,hSession->ft->user_data);
 }

 static void def_failed(H3270 *hSession, unsigned long length,double kbytes_sec,const char *msg, void *userdata)
 {
 	hSession->ft->cbk.complete(hSession,length,kbytes_sec,msg,userdata);
 }

 static void def_message(H3270 *hSession, const char *msg, void *userdata)
 {
	lib3270_write_log(hSession,"ft","%s",msg);
 }

 static void def_update(H3270 *hSession, unsigned long current, unsigned long length, double kbytes_sec, void *userdata)
 {

 }

 static void def_running(H3270 *hSession, int is_cut, void *userdata)
 {

 }

 static void def_aborting(H3270 *hSession, void *userdata)
 {

 }

 static void def_state_changed(H3270 *hSession, LIB3270_FT_STATE state, const char *text, void *userdata)
 {

 }

 static H3270FT * ft_creation_failed(H3270 *session, int rc, const char **dest, const char *message) {

	errno = rc;

	if(!dest) {
		// Nao tem destino para a mensagem, apresenta
		lib3270_popup_dialog(session, LIB3270_NOTIFY_ERROR, _( "Request failed" ), _( "Can't start file transfer." ), "%s", message);
	} else {
		*dest = message;
	}

	return NULL;
 }

 LIB3270_EXPORT H3270FT * lib3270_ft_new(H3270 *session, LIB3270_FT_OPTION flags, const char *local, const char *remote, int lrecl, int blksize, int primspace, int secspace, int dft, const char **message)
 {
	static const unsigned short asc2ft[256] =
	{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0xd5, 0x5c, 0xe5, 0xd8, 0x5f,
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x5d, 0x7d, 0x7e, 0x7f,
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x00,
        0xa0, 0xd2, 0x5b, 0xd9, 0xd0, 0xda, 0x7c, 0xdd, 0xe3, 0xdc, 0xcb, 0xc4, 0x5e, 0xe8, 0xd7, 0xe4,
        0xca, 0xc9, 0xf4, 0xfa, 0xe6, 0xd1, 0xde, 0xdb, 0xce, 0xee, 0xcc, 0xc5, 0xdf, 0xe0, 0xe1, 0xd3,
        0xb4, 0xb5, 0xb2, 0xb6, 0xb3, 0xb7, 0xcf, 0xb8, 0xbe, 0xbb, 0xbc, 0xbd, 0xc2, 0xbf, 0xc0, 0xc1,
        0xd4, 0xb9, 0xf7, 0xf8, 0xf5, 0xf9, 0xf6, 0xe7, 0xc3, 0xfd, 0xfe, 0xfb, 0xfc, 0xe2, 0xd6, 0xb1,
        0xa3, 0xa4, 0xa1, 0xa5, 0xa2, 0xa6, 0xcd, 0xa7, 0xac, 0xa9, 0xaa, 0xab, 0xb0, 0xad, 0xae, 0xaf,
        0xc6, 0xa8, 0xeb, 0xec, 0xe9, 0xed, 0xea, 0x9f, 0xba, 0xf1, 0xf2, 0xef, 0xf0, 0xc7, 0xc8, 0xf3
	};

	static const unsigned short ft2asc[256] =
	{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0xa2, 0x5c, 0x7c, 0xac, 0x5f,
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0xa6, 0x7d, 0x7e, 0x7f,
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0xf7,
        0xa0, 0xe2, 0xe4, 0xe0, 0xe1, 0xe3, 0xe5, 0xe7, 0xf1, 0xe9, 0xea, 0xeb, 0xe8, 0xed, 0xee, 0xef,
        0xec, 0xdf, 0xc2, 0xc4, 0xc0, 0xc1, 0xc3, 0xc5, 0xc7, 0xd1, 0xf8, 0xc9, 0xca, 0xcb, 0xc8, 0xcd,
        0xce, 0xcf, 0xcc, 0xd8, 0xab, 0xbb, 0xf0, 0xfd, 0xfe, 0xb1, 0xb0, 0xaa, 0xba, 0xe6, 0xb8, 0xc6,
        0xa4, 0xb5, 0xa1, 0xbf, 0xd0, 0x5b, 0xde, 0xae, 0x5e, 0xa3, 0xa5, 0xb7, 0xa9, 0xa7, 0xb6, 0xbc,
        0xbd, 0xbe, 0xdd, 0xa8, 0xaf, 0x5d, 0xb4, 0xd7, 0xad, 0xf4, 0xf6, 0xf2, 0xf3, 0xf5, 0xb9, 0xfb,
        0xfc, 0xf9, 0xfa, 0xff, 0xb2, 0xd4, 0xd6, 0xd2, 0xd3, 0xd5, 0xb3, 0xdb, 0xdc, 0xd9, 0xda, 0xff
	};

 	H3270FT				* ftHandle		= (H3270FT *) session->ft;
 	FILE				* ft_local_file	= NULL;
 	int                   f;
// 	unsigned long		  length		= 0L;

//	trace("%s(%s)",__FUNCTION__,local);
	if(!lib3270_connected(session))
	{
		return ft_creation_failed(session,ENOTCONN,message,_( "Disconnected from host." ));
	}

	if(ftHandle)
	{
		return ft_creation_failed(session,EBUSY,message,_( "File transfer is already active in this session." ));
	}

	// Check remote file
	if(!*remote)
	{
		return ft_creation_failed(session,EINVAL,message,_( "The remote file name is invalid." ));
	}

	// Open local file
#ifdef _WIN32
	ft_local_file = fopen(local,(flags & LIB3270_FT_OPTION_RECEIVE) ? ((flags & LIB3270_FT_OPTION_APPEND) ? "ab" : "wb") : "rb");
#else
	ft_local_file = fopen(local,(flags & LIB3270_FT_OPTION_RECEIVE) ? ((flags & LIB3270_FT_OPTION_APPEND) ? "a" : "w") : "r");
#endif // _WIN32

	if(!ft_local_file)
	{
		return ft_creation_failed(session,errno,message,strerror(errno));
	}

	// Set options
	session->dft_buffersize = dft;
	set_dft_buffersize(session);

	// Create & Initialize ft control structure.
	ftHandle = lib3270_malloc(sizeof(H3270FT)+strlen(local)+strlen(remote)+3);

	ftHandle->host				= session;

	ftHandle->ft_last_cr		= 0;

	ftHandle->ascii_flag		= (flags & LIB3270_FT_OPTION_ASCII)	? 1 : 0;
	ftHandle->cr_flag   		= (flags & LIB3270_FT_OPTION_CRLF)	? 1 : 0;
	ftHandle->remap_flag		= (flags & LIB3270_FT_OPTION_REMAP)	? 1 : 0;
	ftHandle->unix_text			= (flags & LIB3270_FT_OPTION_UNIX)	? 1 : 0;
	ftHandle->ft_is_cut  		= 0;
	ftHandle->flags				= flags;
	ftHandle->local_file		= ft_local_file;
	ftHandle->state				= LIB3270_FT_STATE_AWAIT_ACK;
	ftHandle->lrecl				= lrecl;
	ftHandle->blksize			= blksize;
	ftHandle->primspace			= primspace;
	ftHandle->secspace			= secspace;
	ftHandle->dft				= dft;
	ftHandle->quadrant			= -1;

	ftHandle->cbk.complete 		= def_complete;
	ftHandle->cbk.failed		= def_failed;
	ftHandle->cbk.message 		= def_message;
	ftHandle->cbk.update 		= def_update;
	ftHandle->cbk.running 		= def_running;
	ftHandle->cbk.aborting 		= def_aborting;
	ftHandle->cbk.state_changed	= def_state_changed;


	// Setup file transfer charset.
	memcpy(&ftHandle->charset,&session->charset,sizeof(struct lib3270_charset));

	// Copy "IND$FILE" charset.
	for(f=0;f<0x0100;f++)
    {
        ftHandle->charset.ebc2asc[f] = ft2asc[f];
        ftHandle->charset.asc2ebc[f] = asc2ft[f];
	}

	// Setup files.
	ftHandle->local			= (char *) (ftHandle+1);
	strcpy((char *) ftHandle->local,local);

	ftHandle->remote		= ftHandle->local + strlen(ftHandle->local)+1;
	strcpy((char *) ftHandle->remote,remote);

	session->ft				= ftHandle;

 	return ftHandle;
 }

 LIB3270_EXPORT void lib3270_ft_set_user_data(H3270 *hSession, void *ptr)
 {
 	H3270FT * ft;

	CHECK_SESSION_HANDLE(hSession);

	ft = get_ft_handle(hSession);
	if(!ft)
		return;

	ft->user_data = ptr;

 }

 LIB3270_EXPORT void * lib3270_ft_get_user_data(H3270 *hSession)
 {
 	H3270FT * ft;

	CHECK_SESSION_HANDLE(hSession);

	ft = get_ft_handle(hSession);
	if(!ft)
		return NULL;

	return ft->user_data;
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
					"\nReceiving file %s (%s %s %s %s)\n",
					ft->local,
					ft->ascii_flag	?	"ASCII" 	: "BINARY",
					ft->cr_flag 	?	"CRLF" 		: "NOCRLF",
					ft->remap_flag	?	"REMAP" 	: "NOREMAP",
					ft->unix_text	?	"LF Only"	: "CR/LF"
				);
	}
	else
	{
		// Sending file
		if(fseek(ft->local_file,0L,SEEK_END) < 0)
		{
			ft_failed(ft,N_( "Can't get file size" ));
			return errno ? errno : -1;
		}

		ft->length = ftell(ft->local_file);

		lib3270_write_dstrace(
				ft->host,
				"\nSending file %s (%ld bytes %s %s %s %s)\n",
				ft->local,
				ft->length,
				ft->ascii_flag	?	"ASCII" 	: "BINARY",
				ft->cr_flag		?	"CRLF"		: "NOCRLF",
				ft->remap_flag	?	"REMAP" 	: "NOREMAP",
				ft->unix_text	?	"LF only"	: "CR/LF"
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
		ft_failed(ft,N_( "Unable to send file-transfer request" ));
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
	ft->cbk.message(ft->host,msg,ft->user_data);
}

static double finish(H3270FT *ft)
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

	return kbytes_sec;
}

void ft_complete(H3270FT *ft, const char *errmsg)
{
	ft->cbk.complete(ft->host,ft->ft_length,finish(ft),errmsg ? errmsg : N_("Transfer complete"),ft->user_data);
}

void ft_failed(H3270FT *ft, const char *errmsg)
{
	ft->cbk.failed(ft->host,ft->ft_length,finish(ft),errmsg ? errmsg : N_("Transfer failed"),ft->user_data);
}

LIB3270_EXPORT int lib3270_ft_destroy(H3270 *hSession)
{
	H3270FT *session;

	CHECK_SESSION_HANDLE(hSession);

	session = (H3270FT *) hSession->ft;
	if(!session)
		return EINVAL;

	if (session->state != LIB3270_FT_STATE_NONE)
	{
		lib3270_ft_cancel(hSession,1);
	}

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

	session->cbk.update(session->host,session->ft_length,session->length,kbytes_sec,session->user_data);

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

	ft->cbk.running(ft->host,is_cut,ft->user_data);

	ft_update_length(ft);

}

LIB3270_EXPORT struct lib3270_ft_callbacks * lib3270_get_ft_callbacks(H3270 *session, unsigned short sz)
{

	CHECK_SESSION_HANDLE(session);

	if(sz != sizeof(struct lib3270_ft_callbacks))
		return NULL;

	if(session->ft)
		return &(session->ft->cbk);

	return NULL;

}


// Process a protocol-generated abort.
void ft_aborting(H3270FT *h)
{
	if (h->state == FT_RUNNING || h->state == FT_ABORT_WAIT)
	{
		set_ft_state(h,FT_ABORT_SENT);
		h->cbk.message(h->host,N_("Aborting..."),h->user_data);
		h->cbk.aborting(h->host,h->user_data);
	}
}

/* Process a disconnect abort. */
static void ft_connected(H3270 *hSession, int ignored, void *dunno)
{
	if (!CONNECTED && lib3270_get_ft_state(hSession) != LIB3270_FT_STATE_NONE)
		ft_failed(get_ft_handle(hSession),_("Host disconnected, transfer cancelled"));
}

/* Process an abort from no longer being in 3270 mode. */
static void ft_in3270(H3270 *hSession, int ignored, void *dunno)
{
	if (!IN_3270 && lib3270_get_ft_state(hSession) != LIB3270_FT_STATE_NONE)
		ft_failed(get_ft_handle(hSession),_("Not in 3270 mode, transfer cancelled"));
}

LIB3270_EXPORT LIB3270_FT_STATE lib3270_get_ft_state(H3270 *session)
{
	CHECK_SESSION_HANDLE(session);

	if(!session->ft)
		return LIB3270_FT_STATE_NONE;

	return ((H3270FT *) session->ft)->state;
}
