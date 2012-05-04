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

static void ft_connected(H3270 *session, int ignored, H3270FT *ft);
static void ft_in3270(H3270 *session, int ignored unused, H3270FT *ft);

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

#define CHECK_FT_HANDLE(x) if(!x) x = ftsession;

enum ft_state ft_state = FT_NONE;		// File transfer state
// char *ft_local_filename;				// Local file to transfer to/from
Boolean ft_last_cr = False;				// CR was last char in local file
Boolean ascii_flag = True;				// Convert to ascii
Boolean cr_flag = True;					// Add crlf to each line
Boolean remap_flag = True;				// Remap ASCII<->EBCDIC
unsigned long ft_length = 0;			// Length of transfer
static Boolean ft_is_cut;				// File transfer is CUT-style

static struct timeval starting_time;	// Starting time

static const struct filetransfer_callbacks	*callbacks = NULL;		// Callbacks to main application

#define snconcat(x,s,fmt,...) snprintf(x+strlen(x),s-strlen(x),fmt,__VA_ARGS__)

#define set_ft_state(x) ft_state = x

/*---[ Implement ]-------------------------------------------------------------------------------------------------------*/

 void ft_init(H3270FT *h)
 {
	/* Register for state changes. */

	CHECK_FT_HANDLE(h);

	lib3270_register_schange(h->host, ST_CONNECT, ( void (*)(H3270 *, int, void *)) ft_connected, h);
	lib3270_register_schange(h->host, ST_3270_MODE, ( void (*)(H3270 *, int, void *)) ft_in3270, h);
 }

 enum ft_state QueryFTstate(void)
 {
 	return ft_state;
 }

/*
 int RegisterFTCallbacks(const struct filetransfer_callbacks *cbk)
 {
 	if(!(cbk && cbk->sz == sizeof(struct filetransfer_callbacks)) )
		return EINVAL;

	callbacks = cbk;

	return 0;
 }
*/

 enum ft_state GetFileTransferState(void)
 {
	return ft_state;
 }

 LIB3270_EXPORT int lib3270_ft_cancel(H3270FT *ft, int force)
 {
	if (ft_state == FT_RUNNING)
	{
		set_ft_state(FT_ABORT_WAIT);
		if(callbacks && callbacks->aborting)
			callbacks->aborting();
		return 0;
	}

	if(!force)
		return EBUSY;

	// Impatient user or hung host -- just clean up.
	ft_complete(ft, _("Cancelled by user") );

	return ECANCELED;
 }

 LIB3270_EXPORT H3270FT * lib3270_ft_start(H3270 *session, LIB3270_FT_OPTION flags, const char *local, const char *remote, int lrecl, int blksize, int primspace, int secspace, int dft, const char **msg)
 {
 	H3270FT				* ftHandle		= NULL;
 	static const char	* rec			= "fvu";
 	static const char	* un[]			= { "tracks", "cylinders", "avblock" };

 	unsigned short		  recfm			= (flags & FT_RECORD_FORMAT_MASK) >> 8;
 	unsigned short		  units			= (flags & FT_ALLOCATION_UNITS_MASK) >> 12;

 	FILE				* ft_local_file	= NULL;

 	char 				  op[4096];
 	char				  buffer[4096];

	unsigned int		  flen;

	Trace("%s(%s)",__FUNCTION__,local);

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

	ascii_flag = ((flags & LIB3270_FT_OPTION_ASCII) != 0);
	cr_flag    = ((flags & LIB3270_FT_OPTION_CRLF) != 0);
	remap_flag = ((flags & LIB3270_FT_OPTION_ASCII) != 0);

	lib3270_write_log(session, "%s file \"%s\"",(flags & LIB3270_FT_OPTION_RECEIVE) ? "Receiving" : "Sending", local);

 	/* Build the ind$file command */
 	snprintf(op,4095,"%s%s%s",
						(flags & LIB3270_FT_OPTION_ASCII) 	? " ASCII"	: "",
						(flags & LIB3270_FT_OPTION_CRLF) 	? " CRLF"	: "",
						(flags & LIB3270_FT_OPTION_APPEND)	? " APPEND"	: ""
			);

	if(!(flags & LIB3270_FT_OPTION_RECEIVE))
	{
		if(flags & LIB3270_FT_OPTION_TSO)
		{
			// TSO Host
			if(recfm > 0)
			{
				snconcat(op,4096," recfm(%c)",rec[recfm-1]);

				if(lrecl > 0)
					snconcat(op,4096," lrecl(%d)",lrecl);

				if(blksize > 0)
					snconcat(op,4096," blksize(%d)", blksize);
			}

			if(units > 0)
			{
				snconcat(op,4096," %s",un[units-1]);

				if(primspace > 0)
				{
					snconcat(op,4096," space(%d",primspace);
					if(secspace)
						snconcat(op,4096,",%d",secspace);
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

				if(lrecl > 0)
					snconcat(op,4096," lrecl %d",lrecl);

			}
		}
	}

	snprintf(buffer,4095,"%s %s %s",	"IND$FILE",
										(flags & LIB3270_FT_OPTION_RECEIVE) ? "GET" : "PUT",
										remote );

	if(*op)
	{
		if(flags & LIB3270_FT_OPTION_TSO)
			snconcat(buffer,4095," %s",op+1);
		else
			snconcat(buffer,4095," (%s)",op+1);
	}

	snconcat(buffer,4095,"%s","\n");

	// Erase the line and enter the command.
	flen = kybd_prime();
	if (!flen || flen < strlen(buffer) - 1)
	{
		lib3270_write_log(session, "Unable to send command \"%s\" (flen=%d szBuffer=%d)",buffer,flen,strlen(buffer));
		fclose(ft_local_file);
		*msg  = _( "Unable to send file-transfer request" );
		errno = EINVAL;
		return NULL;
	}

	Trace("Command: \"%s\"",buffer);

	(void) lib3270_emulate_input(NULL, buffer, strlen(buffer), False);

	// Get this thing started.
	set_ft_state(FT_AWAIT_ACK);

	ft_last_cr = False;
	ft_is_cut  = False;

	ftHandle = malloc(sizeof(H3270FT));
	memset(ftHandle,0,sizeof(H3270FT));

	ftHandle->sz 			= sizeof(H3270FT);
	ftHandle->host			= session;
	ftHandle->ft_local_file	= ft_local_file;

 	return ftsession = ftHandle;
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

	trace("%s",__FUNCTION__);

	// Close the local file.
	if(session->ft_local_file)
	{
		fclose(session->ft_local_file);
		session->ft_local_file = NULL;
	}

	// Clean up the state.
	set_ft_state(FT_NONE);

	ft_update_length(session);

	if(callbacks && callbacks->complete)
		callbacks->complete(errmsg,ft_length,kbytes_sec,ft_is_cut ? "CUT" : "DFT");

	if(session == ftsession)
		ftsession = NULL;

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

	Trace("%s",__FUNCTION__);

	if(callbacks && callbacks->update)
		callbacks->update(ft_length,kbytes_sec);

}

// Process a transfer acknowledgement.
void ft_running(H3270FT *h, Boolean is_cut)
{
	trace("%s",__FUNCTION__);

	CHECK_FT_HANDLE(h);

	ft_is_cut = is_cut;
	ft_length = 0;

	(void) gettimeofday(&starting_time, (struct timezone *)NULL);

	if (ft_state == FT_AWAIT_ACK)
		set_ft_state(FT_RUNNING);

	if(callbacks && callbacks->running)
		callbacks->running(is_cut);

	ft_update_length(h);

}

// Process a protocol-generated abort.
void ft_aborting(H3270FT *h)
{
	Trace("%s",__FUNCTION__);

	CHECK_FT_HANDLE(h);

	if (ft_state == FT_RUNNING || ft_state == FT_ABORT_WAIT)
		set_ft_state(FT_ABORT_SENT);

	if(callbacks && callbacks->aborting)
		callbacks->aborting();

}

/* Process a disconnect abort. */
static void ft_connected(H3270 *session, int ignored, H3270FT *ft)
{
	CHECK_FT_HANDLE(ft);

	if (!CONNECTED && ft_state != FT_NONE)
		ft_complete(ft,_("Host disconnected, transfer cancelled"));
}

/* Process an abort from no longer being in 3270 mode. */
static void ft_in3270(H3270 *session, int ignored, H3270FT *ft)
{
	CHECK_FT_HANDLE(ft);

	if (!IN_3270 && ft_state != FT_NONE)
		ft_complete(ft,_("Not in 3270 mode, transfer cancelled"));
}

