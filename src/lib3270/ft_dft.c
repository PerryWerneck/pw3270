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
 * Este programa está nomeado como ft_dft.c e possui 562 linhas de código.
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


/*
 *	tc_dft.c
 *		File transfer: DFT-style data processing functions
 */

#include <lib3270.h>
#include "globals.h"

#if defined(X3270_FT) /*[*/

#include "appres.h"
#include "3270ds.h"
#include "ft_dft_ds.h"

#include "actionsc.h"
#include "kybdc.h"
#include "ft_dftc.h"
#include "ftc.h"
#include "tablesc.h"
#include "telnetc.h"
#include "trace_dsc.h"
#include "utilc.h"

#include <errno.h>
// #include <malloc.h>

extern unsigned char aid;

/* Macros. */
#define OPEN_MSG	"FT:MSG"	/* Open request for message */
#define END_TRANSFER	"TRANS03"	/* Message for xfer complete */

#define DFT_MIN_BUF	256
#define DFT_MAX_BUF	32768

/* Typedefs. */
struct data_buffer {
	char sf_length[2];		/* SF length = 0x0023 */
	char sf_d0;			/* 0xD0 */
	char sf_request_type[2];	/* request type */
	char compress_indic[2];		/* 0xc080 */
	char begin_data;		/* 0x61 */
	char data_length[2];		/* Data Length in 3270 byte order+5 */
	char data[256];			/* The actual data */
};

/* Globals. */
int dft_buffersize = 0;			/* Buffer size (LIMIN, LIMOUT) */

/* Statics. */
static Boolean message_flag = False;	/* Open Request for msg received */
static int dft_eof;
static unsigned long recnum;
static char *abort_string = CN;
static unsigned char *dft_savebuf = NULL;
static int dft_savebuf_len = 0;
static int dft_savebuf_max = 0;

static void dft_abort(unsigned short code, const char *fmt, ...);

static void dft_close_request(void);
static void dft_data_insert(struct data_buffer *data_bufr);
static void dft_get_request(void);
static void dft_insert_request(void);
static void dft_open_request(unsigned short len, unsigned char *cp);
static void dft_set_cur_req(void);
static int filter_len(char *s, register int len);

/* Process a Transfer Data structured field from the host. */
void
ft_dft_data(unsigned char *data, int length unused)
{
	struct data_buffer *data_bufr = (struct data_buffer *)data;
	unsigned short data_length, data_type;
	unsigned char *cp;

	if (lib3270_get_ft_state(&h3270) == FT_NONE)
	{
		trace_ds(" (no transfer in progress)\n");
		return;
	}

	/* Get the length. */
	cp = (unsigned char *)(data_bufr->sf_length);
	GET16(data_length, cp);

	/* Get the function type. */
	cp = (unsigned char *)(data_bufr->sf_request_type);
	GET16(data_type, cp);

	/* Handle the requests */
	switch (data_type) {
	    case TR_OPEN_REQ:
		dft_open_request(data_length, cp);
		break;
	    case TR_INSERT_REQ:	/* Insert Request */
		dft_insert_request();
		break;
	    case TR_DATA_INSERT:
		dft_data_insert(data_bufr);
		break;
	    case TR_SET_CUR_REQ:
		dft_set_cur_req();
		break;
	    case TR_GET_REQ:
		dft_get_request();
		break;
	    case TR_CLOSE_REQ:
		dft_close_request();
		break;
	    default:
		trace_ds(" Unsupported(0x%04x)\n", data_type);
		break;
	}
}

/* Process an Open request. */
static void
dft_open_request(unsigned short len, unsigned char *cp)
{
	char *name = "?";
	char namebuf[8];
	char *s;
	unsigned short recsz = 0;

	if (len == 0x23) {
		name = (char *)cp + 25;
	} else if (len == 0x29) {
		unsigned char *recszp;

		recszp = cp + 27;
		GET16(recsz, recszp);
		name = (char *)cp + 31;
	} else {
		dft_abort(TR_OPEN_REQ, "%s", _("Uknown DFT Open type from host") );
		return;
	}

	(void) memcpy(namebuf, name, 7);
	namebuf[7] = '\0';
	s = &namebuf[6];
	while (s >= namebuf && *s == ' ') {
		*s-- = '\0';
	}
	if (recsz) {
		trace_ds(" Open('%s',recsz=%u)\n", namebuf, recsz);
	} else {
		trace_ds(" Open('%s')\n", namebuf);
	}

	if (!strcmp(namebuf, OPEN_MSG))
		message_flag = True;
	else {
		message_flag = False;
		ft_running(NULL,False);
	}
	dft_eof = False;
	recnum = 1;

	/* Acknowledge the Open. */
	trace_ds("> WriteStructuredField FileTransferData OpenAck\n");
	obptr = obuf;
	space3270out(6);
	*obptr++ = AID_SF;
	SET16(obptr, 5);
	*obptr++ = SF_TRANSFER_DATA;
	SET16(obptr, 9);
	net_output();
}

/* Process an Insert request. */
static void
dft_insert_request(void)
{
	trace_ds(" Insert\n");
	/* Doesn't currently do anything. */
}

/* Process a Data Insert request. */
static void
dft_data_insert(struct data_buffer *data_bufr)
{
	/* Received a data buffer, get the length and process it */
	int my_length;
	unsigned char *cp;

	if(!message_flag && lib3270_get_ft_state(&h3270) == FT_ABORT_WAIT)
	{
		dft_abort(TR_DATA_INSERT, "%s", _("Transfer cancelled by user") );
		return;
	}

	cp = (unsigned char *) (data_bufr->data_length);

	/* Get the data length in native format. */
	GET16(my_length, cp);

	/* Adjust for 5 extra count */
	my_length -= 5;

	trace_ds(" Data(rec=%lu) %d bytes\n", recnum, my_length);

	/*
	 * First, check to see if we have message data or file data.
	 * Message data will result in a popup.
	 */
	if (message_flag) {
		/* Data is from a message */
		unsigned char *msgp;
		unsigned char *dollarp;

		/* Get storage to copy the message. */
		msgp = (unsigned char *)lib3270_malloc(my_length + 1);

		/* Copy the message. */
		memcpy(msgp, data_bufr->data, my_length);

		/* Null terminate the string. */
		dollarp = (unsigned char *)memchr(msgp, '$', my_length);
		if (dollarp != NULL)
			*dollarp = '\0';
		else
			*(msgp + my_length) = '\0';

		/* If transfer completed ok, use our msg. */
		if (memcmp(msgp, END_TRANSFER, strlen(END_TRANSFER)) == 0) {
			lib3270_free(msgp);
			ft_complete(NULL,NULL);
		} else if (lib3270_get_ft_state(&h3270) == FT_ABORT_SENT && abort_string != CN) {
			lib3270_free(msgp);
			ft_complete(NULL,abort_string);
			Replace(abort_string, CN);
		} else {
			ft_complete(NULL,(char *)msgp);
			lib3270_free(msgp);
		}
	} else if (my_length > 0) {
		/* Write the data out to the file. */
		int rv = 1;

		if (ascii_flag && remap_flag) {
			/* Filter. */
			unsigned char *s = (unsigned char *)data_bufr->data;
			unsigned len = my_length;

			while (len--) {
				*s = ft2asc[*s];
				s++;
			}
		}
		if (ascii_flag && cr_flag) {
			char *s = (char *)data_bufr->data;
			unsigned len = my_length;

			/* Delete CRs and ^Zs. */
			while (len) {
				unsigned l = filter_len(s, len);

				if (l)
				{
					rv = fwrite(s, l, (size_t)1,((H3270FT *) h3270.ft)->ft_local_file);
					if (rv == 0)
						break;
					ft_length += l;
				}
				if (l < len)
					l++;
				s += l;
				len -= l;
			}
		} else {
			rv = fwrite((char *)data_bufr->data, my_length,(size_t)1, ((H3270FT *) h3270.ft)->ft_local_file);
			ft_length += my_length;
		}

		if (!rv) {
			/* write failed */
			dft_abort(TR_DATA_INSERT, _( "Error \"%s\" writing to local file (rc=%d)" ) , strerror(errno), errno);
		}

		/* Add up amount transferred. */
		ft_update_length((H3270FT *) h3270.ft);
	}

	/* Send an acknowledgement frame back. */
	trace_ds("> WriteStructuredField FileTransferData DataAck(rec=%lu)\n", recnum);
	obptr = obuf;
	space3270out(12);
	*obptr++ = AID_SF;
	SET16(obptr, 11);
	*obptr++ = SF_TRANSFER_DATA;
	SET16(obptr, TR_NORMAL_REPLY);
	SET16(obptr, TR_RECNUM_HDR);
	SET32(obptr, recnum);
	recnum++;
	net_output();
}

/* Process a Set Cursor request. */
static void
dft_set_cur_req(void)
{
	trace_ds(" SetCursor\n");
	/* Currently doesn't do anything. */
}

/* Process a Get request. */
static void
dft_get_request(void)
{
	int numbytes;
	size_t numread;
	size_t total_read = 0;
	unsigned char *bufptr;

	trace_ds(" Get\n");

	if (!message_flag && lib3270_get_ft_state(&h3270) == FT_ABORT_WAIT) {
		dft_abort(TR_GET_REQ, _( "Transfer cancelled by user" ) );
		return;
	}

	/* Read a buffer's worth. */
	set_dft_buffersize();
	space3270out(dft_buffersize);
	numbytes = dft_buffersize - 27; /* always read 5 bytes less than we're allowed */
	bufptr = obuf + 17;
	while (!dft_eof && numbytes) {
		if (ascii_flag && cr_flag) {
			int c;

			/* Read one byte and do CR/LF translation. */
			c = fgetc(((H3270FT *) h3270.ft)->ft_local_file);
			if (c == EOF) {
				break;
			}
			if (!ft_last_cr && c == '\n') {
				if (numbytes < 2) {
					/*
					 * Not enough room to expand NL to
					 * CR/LF.
					 */
					ungetc(c, ((H3270FT *) h3270.ft)->ft_local_file);
					break;
				}
				*bufptr++ = '\r';
				numbytes--;
				total_read++;
			}
			ft_last_cr = (c == '\r');
			*bufptr++ = remap_flag? asc2ft[c]: c;
			numbytes--;
			total_read++;
		} else {
			/* Binary read. */
			numread = fread(bufptr, 1, numbytes, ((H3270FT *) h3270.ft)->ft_local_file);
			if (numread <= 0) {
				break;
			}
			if (ascii_flag && remap_flag) {
				unsigned char *s = bufptr;
				int i = numread;

				while (i) {
					*s = asc2ft[*s];
					s++;
					i--;
				}
			}
			bufptr += numread;
			numbytes -= numread;
			total_read += numread;
		}
		if (feof(((H3270FT *) h3270.ft)->ft_local_file) || ferror(((H3270FT *) h3270.ft)->ft_local_file)) {
			break;
		}
	}

	/* Check for read error. */
	if (ferror(((H3270FT *) h3270.ft)->ft_local_file))
	{
		dft_abort(TR_GET_REQ, _( "Error \"%s\" reading from local file (rc=%d)" ), strerror(errno), errno);
		return;
	}

	/* Set up SF header for Data or EOF. */
	obptr = obuf;
	*obptr++ = AID_SF;
	obptr += 2;	/* skip SF length for now */
	*obptr++ = SF_TRANSFER_DATA;

	if (total_read) {
		trace_ds("> WriteStructuredField FileTransferData Data(rec=%lu) %d bytes\n",
		    (unsigned long) recnum, (int) total_read);
		SET16(obptr, TR_GET_REPLY);
		SET16(obptr, TR_RECNUM_HDR);
		SET32(obptr, recnum);
		recnum++;
		SET16(obptr, TR_NOT_COMPRESSED);
		*obptr++ = TR_BEGIN_DATA;
		SET16(obptr, total_read + 5);
		obptr += total_read;

		ft_length += total_read;

		if (feof(((H3270FT *) h3270.ft)->ft_local_file))
		{
			dft_eof = True;
		}

	} else {
		trace_ds("> WriteStructuredField FileTransferData EOF\n");
		*obptr++ = HIGH8(TR_GET_REQ);
		*obptr++ = TR_ERROR_REPLY;
		SET16(obptr, TR_ERROR_HDR);
		SET16(obptr, TR_ERR_EOF);

		dft_eof = True;
	}

	/* Set the SF length. */
	bufptr = obuf + 1;
	SET16(bufptr, obptr - (obuf + 1));

	/* Save the data. */
	dft_savebuf_len = obptr - obuf;
	if (dft_savebuf_len > dft_savebuf_max) {
		dft_savebuf_max = dft_savebuf_len;
		Replace(dft_savebuf, (unsigned char *)lib3270_malloc(dft_savebuf_max));
	}
	(void) memcpy(dft_savebuf, obuf, dft_savebuf_len);
	aid = AID_SF;

	/* Write the data. */
	net_output();
	ft_update_length((H3270FT *) h3270.ft);
}

/* Process a Close request. */
static void
dft_close_request(void)
{
	/*
	 * Recieved a close request from the system.
	 * Return a close acknowledgement.
	 */
	trace_ds(" Close\n");
	trace_ds("> WriteStructuredField FileTransferData CloseAck\n");
	obptr = obuf;
	space3270out(6);
	*obptr++ = AID_SF;
	SET16(obptr, 5);	/* length */
	*obptr++ = SF_TRANSFER_DATA;
	SET16(obptr, TR_CLOSE_REPLY);
	net_output();
}

/* Abort a transfer. */
static void dft_abort(unsigned short code, const char *fmt, ...)
{
	va_list args;

	if(abort_string)
		lib3270_free(abort_string);

	va_start(args, fmt);
	abort_string = lib3270_vsprintf(fmt, args);
	va_end(args);

	trace_ds("> WriteStructuredField FileTransferData Error\n");

	obptr = obuf;
	space3270out(10);
	*obptr++ = AID_SF;
	SET16(obptr, 9);	/* length */
	*obptr++ = SF_TRANSFER_DATA;
	*obptr++ = HIGH8(code);
	*obptr++ = TR_ERROR_REPLY;
	SET16(obptr, TR_ERROR_HDR);
	SET16(obptr, TR_ERR_CMDFAIL);
	net_output();

	/* Update the pop-up and state. */
	ft_aborting((H3270FT *) h3270.ft);
}

/* Returns the number of bytes in s, limited by len, that aren't CRs or ^Zs. */
static int
filter_len(char *s, register int len)
{
	register char *t = s;

	while (len && *t != '\r' && *t != 0x1a) {
		len--;
		t++;
	}
	return t - s;
}

/* Processes a Read Modified command when there is upload data pending. */
void
dft_read_modified(void)
{
	if (dft_savebuf_len) {
		trace_ds("> WriteStructuredField FileTransferData\n");
		obptr = obuf;
		space3270out(dft_savebuf_len);
		memcpy(obptr, dft_savebuf, dft_savebuf_len);
		obptr += dft_savebuf_len;
		net_output();
	}
}

/* Update the buffersize for generating a Query Reply. */
void
set_dft_buffersize(void)
{
	if (dft_buffersize == 0) {
		dft_buffersize = appres.dft_buffer_size;
		if (dft_buffersize == 0)
			dft_buffersize = DFT_BUF;
	}
	if (dft_buffersize > DFT_MAX_BUF)
		dft_buffersize = DFT_MAX_BUF;
	if (dft_buffersize < DFT_MIN_BUF)
		dft_buffersize = DFT_MIN_BUF;
}


#endif /*]*/
