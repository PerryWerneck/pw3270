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
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como ft.c e possui 2143 linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */

#include <lib3270/config.h>
#include <lib3270.h>
#include <lib3270/filetransfer.h>
#include "globals.h"

#include <errno.h>
#include <malloc.h>

#include "appres.h"
#include "actionsc.h"
#include "ft_cutc.h"
#include "ft_dftc.h"
#include "ftc.h"
#include "hostc.h"
/*
#if defined(C3270) || defined(WC3270)
#include "icmdc.h"
#endif
*/
#include "kybdc.h"
#include "objects.h"
#include "popupsc.h"
#include "screenc.h"
#include "tablesc.h"
#include "telnetc.h"
#include "utilc.h"

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

// Globals.
H3270FT *ftsession = NULL;

// enum ft_state ft_state = FT_NONE;		// File transfer state
// char *ft_local_filename;				// Local file to transfer to/from
Boolean ft_last_cr = False;				// CR was last char in local file
Boolean ascii_flag = True;				// Convert to ascii
Boolean cr_flag = True;					// Add crlf to each line
Boolean remap_flag = True;				// Remap ASCII<->EBCDIC
unsigned long ft_length = 0;			// Length of transfer
static Boolean ft_is_cut;				// File transfer is CUT-style

static struct timeval starting_time;	// Starting time

// static const struct filetransfer_callbacks	*callbacks = NULL;		// Callbacks to main application

#define snconcat(x,s,fmt,...) snprintf(x+strlen(x),s-strlen(x),fmt,__VA_ARGS__)

static void set_ft_state(H3270FT *session, LIB3270_FT_STATE state);


/*---[ Implement ]-------------------------------------------------------------------------------------------------------*/

 static void set_ft_state(H3270FT *session, LIB3270_FT_STATE state)
 {
	CHECK_FT_HANDLE(session);

	if(session->state == state)
		return;

	session->state = state;
	session->state_changed(session,state);

 }

 void ft_init(H3270 *session)
 {
	/* Register for state changes. */
	lib3270_register_schange(session, LIB3270_STATE_CONNECT, ( void (*)(H3270 *, int, void *)) ft_connected, NULL);
	lib3270_register_schange(session, LIB3270_STATE_3270_MODE, ( void (*)(H3270 *, int, void *)) ft_in3270, NULL);
 }

// enum ft_state QueryFTstate(void)
// {
// 	return ft_state;
// }

/*
 int RegisterFTCallbacks(const struct filetransfer_callbacks *cbk)
 {
 	if(!(cbk && cbk->sz == sizeof(struct filetransfer_callbacks)) )
		return EINVAL;

	callbacks = cbk;

	return 0;
 }
*/

/*
 enum ft_state GetFileTransferState(void)
 {
	return ft_state;
 }
*/

 LIB3270_EXPORT int lib3270_ft_cancel(H3270FT *ft, int force)
 {
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

 static void def_complete(H3270FT *ft,unsigned long length,double kbytes_sec,const char *mode)
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


 LIB3270_EXPORT H3270FT * lib3270_ft_new(H3270 *session, LIB3270_FT_OPTION flags, const char *local, const char *remote, int lrecl, int blksize, int primspace, int secspace, int dft, const char **msg)
 {
 	H3270FT				* ftHandle		= NULL;
 	FILE				* ft_local_file	= NULL;
 	unsigned long		  ft_length		= 0L;

//	trace("%s(%s)",__FUNCTION__,local);
	if(!lib3270_connected(session))
	{
		*msg  = N_( "Disconnected from host" );
		errno = EINVAL;
		return NULL;
	}

	if(session->ft)
	{
		*msg  = N_( "File transfer is already active in this session" );
		errno = EBUSY;
		return NULL;
	}

	if(ftsession)
	{
		*msg  = N_( "File transfer is already active" );
		errno = EBUSY;
		return NULL;
	}

	// Check remote file
	if(!*remote)
	{
		*msg  = N_( "The remote file name is invalid" );
		errno = EINVAL;
		return NULL;
	}

	// Open local file
	ft_local_file = fopen(local,(flags & LIB3270_FT_OPTION_RECEIVE) ? ((flags & LIB3270_FT_OPTION_APPEND) ? "a" : "w") : "r");

	if(!ft_local_file)
	{
		*msg = N_( "Can't open local file" );
		return NULL;
	}

	// Set options
	dft_buffersize = dft;
	set_dft_buffersize();

	// Initialize ft control structure.
	ft_last_cr = False;
	ft_is_cut  = False;

	ftHandle = lib3270_malloc(sizeof(H3270FT)+strlen(local)+strlen(remote)+3);

	ftHandle->sz 			= sizeof(H3270FT);
	ftHandle->host			= session;
	ftHandle->flags			= flags;
	ftHandle->local_file	= ft_local_file;
	ftHandle->length		= ft_length;
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

	ftHandle->local			= (char *) (ftHandle+1);
	strcpy((char *) ftHandle->local,local);

	ftHandle->remote		= ftHandle->local + strlen(ftHandle->local)+1;
	strcpy((char *) ftHandle->remote,remote);

	session->ft				= ftHandle;

 	return ftsession = ftHandle;
 }

 LIB3270_EXPORT int lib3270_ft_start(H3270FT *ft)
 {
 	static const char	* rec			= "fvu";
 	static const char	* un[]			= { "tracks", "cylinders", "avblock" };

 	char 				  op[4096];
 	char				  buffer[4096];
	unsigned int		  flen;
 	unsigned short		  recfm;
 	unsigned short		  units;

	CHECK_FT_HANDLE(ft);

 	recfm		= (ft->flags & FT_RECORD_FORMAT_MASK) >> 8;
 	units		= (ft->flags & FT_ALLOCATION_UNITS_MASK) >> 12;
	ascii_flag	= ((ft->flags & LIB3270_FT_OPTION_ASCII) != 0);
	cr_flag   	= ((ft->flags & LIB3270_FT_OPTION_CRLF) != 0);
	remap_flag	= ((ft->flags & LIB3270_FT_OPTION_ASCII) != 0);

	if(ft->flags & LIB3270_FT_OPTION_RECEIVE)
	{
		// Receiving file
		lib3270_write_log(ft->host,"ft","Receiving file %s",ft->local);
	}
	else
	{
		// Sending file
		if(fseek(ft->local_file,0L,SEEK_END) < 0)
		{
			ft_complete(ft,N_( "Can't get file size" ));
			return errno ? errno : -1;
		}

		ft_length = ftell(ft->local_file);

		lib3270_write_log(ft->host,"ft","Sending file %s (%ld bytes)",ft->local,ft_length);
		rewind(ft->local_file);
	}

 	/* Build the ind$file command */
 	snprintf(op,4095,"%s%s%s",
						(ft->flags & LIB3270_FT_OPTION_ASCII) 	? " ASCII"	: "",
						(ft->flags & LIB3270_FT_OPTION_CRLF) 	? " CRLF"	: "",
						(ft->flags & LIB3270_FT_OPTION_APPEND)	? " APPEND"	: ""
			);

	if(!(ft->flags & LIB3270_FT_OPTION_RECEIVE))
	{
		if(ft->flags & LIB3270_FT_OPTION_TSO)
		{
			// TSO Host
			if(recfm > 0)
			{
				snconcat(op,4096," recfm(%c)",rec[recfm-1]);

				if(ft->lrecl > 0)
					snconcat(op,4096," lrecl(%d)",ft->lrecl);

				if(ft->blksize > 0)
					snconcat(op,4096," blksize(%d)", ft->blksize);
			}

			if(units > 0)
			{
				snconcat(op,4096," %s",un[units-1]);

				if(ft->primspace > 0)
				{
					snconcat(op,4096," space(%d",ft->primspace);
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
				snconcat(op,4096," recfm %c",rec[recfm-1]);

				if(ft->lrecl > 0)
					snconcat(op,4096," lrecl %d",ft->lrecl);

			}
		}
	}

	snprintf(buffer,4095,"%s %s %s",	"IND$FILE",
										(ft->flags & LIB3270_FT_OPTION_RECEIVE) ? "GET" : "PUT",
										ft->remote );

	if(*op)
	{
		if(ft->flags & LIB3270_FT_OPTION_TSO)
			snconcat(buffer,4095," %s",op+1);
		else
			snconcat(buffer,4095," (%s)",op+1);
	}

	snconcat(buffer,4095,"%s","\n");

	// Erase the line and enter the command.
	flen = kybd_prime();
	if (!flen || flen < strlen(buffer) - 1)
	{
		lib3270_write_log(ft->host, "Unable to send command \"%s\" (flen=%d szBuffer=%d)",buffer,flen,strlen(buffer));
		ft_complete(ft,N_( "Unable to send file-transfer request" ));
		return errno = EINVAL;
	}

	trace_event("Sending FT request:\n%s\n",buffer);

	lib3270_emulate_input(ft->host, buffer, strlen(buffer), False);

	if(ft->flags & LIB3270_FT_OPTION_RECEIVE)
		ft->message(ft,N_( "Waiting for GET response" ));
	else
		ft->message(ft,N_( "Waiting for PUT response" ));

	return 0;

 }


/* External entry points called by ft_dft and ft_cut. */

/**
 * Pop up a message, end the transfer, release resources.
 *
 */
void ft_complete(H3270FT *session, const char *errmsg)
{
	double kbytes_sec = 0;
	struct timeval	t1;

	CHECK_FT_HANDLE(session);

	(void) gettimeofday(&t1, (struct timezone *)NULL);
	kbytes_sec = (double)ft_length / 1024.0 /
		((double)(t1.tv_sec - starting_time.tv_sec) +
		 (double)(t1.tv_usec - starting_time.tv_usec) / 1.0e6);

	// Close the local file.
	if(session->local_file)
	{
		fclose(session->local_file);
		session->local_file = NULL;
	}

	// Clean up the state.
	set_ft_state(session,FT_NONE);

	ft_update_length(session);

	session->message(session,errmsg);
	session->complete(session,ft_length,kbytes_sec,ft_is_cut ? "CUT" : "DFT");

}

LIB3270_EXPORT void lib3270_ft_destroy(H3270FT *session)
{
	CHECK_FT_HANDLE(session);

	if (session->state != LIB3270_FT_STATE_NONE)
		lib3270_ft_cancel(session,1);

	if(session->local_file)
	{
		fclose(session->local_file);
		session->local_file = NULL;
	}

	if(session == ftsession)
		ftsession = NULL;

	if(session->host)
		session->host->ft = NULL;

	free(session);

}

// Update the bytes-transferred count on the progress pop-up.
void ft_update_length(H3270FT *session)
{
	double kbytes_sec = 0;

	CHECK_FT_HANDLE(session);

	if(ft_length > 1024.0)
	{
		struct timeval	t1;

		(void) gettimeofday(&t1, (struct timezone *)NULL);
		kbytes_sec = (double)ft_length / 1024.0 /
			((double)(t1.tv_sec - starting_time.tv_sec) +
			 (double)(t1.tv_usec - starting_time.tv_usec) / 1.0e6);
	}

	session->update(session,ft_length,session->length,kbytes_sec);

}

// Process a transfer acknowledgement.
void ft_running(H3270FT *h, Boolean is_cut)
{
	CHECK_FT_HANDLE(h);

	ft_is_cut = is_cut;
	ft_length = 0;

	(void) gettimeofday(&starting_time, (struct timezone *)NULL);

	if (h->state == FT_AWAIT_ACK)
		set_ft_state(h,FT_RUNNING);

	h->running(h,is_cut);

	ft_update_length(h);

}

// Process a protocol-generated abort.
void ft_aborting(H3270FT *h)
{
	CHECK_FT_HANDLE(h);

	if (h->state == FT_RUNNING || h->state == FT_ABORT_WAIT)
		set_ft_state(h,FT_ABORT_SENT);

	h->aborting(h);

}

/* Process a disconnect abort. */
static void ft_connected(H3270 *session, int ignored, void *dunno)
{
	if (!CONNECTED && lib3270_get_ft_state(session) != LIB3270_FT_STATE_NONE)
		ft_complete(session->ft,_("Host disconnected, transfer cancelled"));
}

/* Process an abort from no longer being in 3270 mode. */
static void ft_in3270(H3270 *session, int ignored, void *dunno)
{
	if (!IN_3270 && lib3270_get_ft_state(session) != LIB3270_FT_STATE_NONE)
		ft_complete(session->ft,_("Not in 3270 mode, transfer cancelled"));
}

LIB3270_EXPORT LIB3270_FT_STATE lib3270_get_ft_state(H3270 *session)
{
	CHECK_SESSION_HANDLE(session);

	if(!session->ft)
		return LIB3270_FT_STATE_NONE;

	return ((H3270FT *) session->ft)->state;
}
