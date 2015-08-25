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
 * Este programa está nomeado como ft_dft.c e possui 562 linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */


/*
 *	tc_dft.c
 *		File transfer: DFT-style data processing functions
 */

#include <lib3270.h>
#include "globals.h"

#if defined(X3270_FT) /*[*/

//#include "appres.h"
#include "3270ds.h"
#include "ft_dft_ds.h"

//#include "actionsc.h"
#include "kybdc.h"
#include "ft_dftc.h"
#include "ftc.h"
//#include "tablesc.h"
#include "telnetc.h"
#include "trace_dsc.h"
#include "utilc.h"

#include <errno.h>

// extern unsigned char aid;

/* Macros. */
#define OPEN_MSG	"FT:MSG"		/* Open request for message */
#define END_TRANSFER	"TRANS03"	/* Message for xfer complete */

#define DFT_MIN_BUF	256
#define DFT_MAX_BUF	32768

/* Typedefs. */
struct data_buffer
{
	char sf_length[2];			/**< SF length = 0x0023 */
	char sf_d0;					/**< 0xD0 */
	char sf_request_type[2];	/**< request type */
	char compress_indic[2];		/**< 0xc080 */
	char begin_data;			/**< 0x61 */
	char data_length[2];		/**< Data Length in 3270 byte order+5 */
	char data[256];				/**< The actual data */
};

static void dft_abort(H3270 *hSession, unsigned short code, const char *fmt, ...);

static void dft_close_request(H3270 *hSession);
static void dft_data_insert(H3270 *hSession, struct data_buffer *data_bufr);
static void dft_get_request(H3270 *hSession);
static void dft_insert_request(H3270 *hSession);
static void dft_open_request(H3270 *hSession, unsigned short len, unsigned char *cp);
static void dft_set_cur_req(H3270 *hSession);
static int  filter_len(char *s, register int len);

/**
 * Process a Transfer Data structured field from the host.
 *
 */
void ft_dft_data(H3270 *hSession, unsigned char *data, int length unused)
{
	struct data_buffer *data_bufr = (struct data_buffer *)data;
	unsigned short data_length, data_type;
	unsigned char *cp;

	if (lib3270_get_ft_state(hSession) == FT_NONE)
	{
		trace_ds(hSession," (no transfer in progress)\n");
		return;
	}

	/* Get the length. */
	cp = (unsigned char *)(data_bufr->sf_length);
	GET16(data_length, cp);

	/* Get the function type. */
	cp = (unsigned char *)(data_bufr->sf_request_type);
	GET16(data_type, cp);

	/* Handle the requests */
	switch (data_type)
	{
    case TR_OPEN_REQ:
		dft_open_request(hSession,data_length, cp);
		break;

    case TR_INSERT_REQ:	/* Insert Request */
		dft_insert_request(hSession);
		break;

    case TR_DATA_INSERT:
		dft_data_insert(hSession,data_bufr);
		break;

    case TR_SET_CUR_REQ:
		dft_set_cur_req(hSession);
		break;

    case TR_GET_REQ:
		dft_get_request(hSession);
		break;

	case TR_CLOSE_REQ:
		dft_close_request(hSession);
		break;

    default:
		trace_ds(hSession," Unsupported(0x%04x)\n", data_type);
		break;
	}
}

/* Process an Open request. */
static void dft_open_request(H3270 *hSession, unsigned short len, unsigned char *cp)
{
	H3270FT			* ft = get_ft_handle(hSession);
	char			* name = "?";
	char			  namebuf[8];
	char			* s;
	unsigned short	  recsz = 0;

	if (len == 0x23)
	{
		name = (char *)cp + 25;
	}
	else if (len == 0x29)
	{
		unsigned char *recszp;

		recszp = cp + 27;
		GET16(recsz, recszp);
		name = (char *)cp + 31;
	}
	else
	{
		dft_abort(hSession,TR_OPEN_REQ, "%s", _("Uknown DFT Open type from host") );
		return;
	}

	(void) memcpy(namebuf, name, 7);
	namebuf[7] = '\0';
	s = &namebuf[6];
	while (s >= namebuf && *s == ' ')
	{
		*s-- = '\0';
	}

	if (recsz)
	{
		trace_ds(hSession," Open('%s',recsz=%u)\n", namebuf, recsz);
	}
	else
	{
		trace_ds(hSession," Open('%s')\n", namebuf);
	}

	if (!strcmp(namebuf, OPEN_MSG))
		ft->message_flag = 1;
	else
	{
		ft->message_flag = 0;
		ft_running(ft,False);
	}

	ft->dft_eof = 0;
	ft->recnum = 1;

	/* Acknowledge the Open. */
	trace_ds(hSession,"> WriteStructuredField FileTransferData OpenAck\n");
	hSession->obptr = hSession->obuf;
	space3270out(hSession,6);
	*hSession->obptr++ = AID_SF;
	SET16(hSession->obptr, 5);
	*hSession->obptr++ = SF_TRANSFER_DATA;
	SET16(hSession->obptr, 9);
	net_output(hSession);
}

/* Process an Insert request. */
static void dft_insert_request(H3270 *hSession)
{
	trace_ds(hSession," Insert\n");
	/* Doesn't currently do anything. */
}

/* Process a Data Insert request. */
static void dft_data_insert(H3270 *hSession, struct data_buffer *data_bufr)
{
	/* Received a data buffer, get the length and process it */
	H3270FT			* ft = get_ft_handle(hSession);
	int				  my_length;
	unsigned char	* cp;

	if(!ft->message_flag && lib3270_get_ft_state(hSession) == FT_ABORT_WAIT)
	{
		dft_abort(hSession,TR_DATA_INSERT, "%s", _("Transfer cancelled by user") );
		return;
	}

	cp = (unsigned char *) (data_bufr->data_length);

	/* Get the data length in native format. */
	GET16(my_length, cp);

	/* Adjust for 5 extra count */
	my_length -= 5;

	trace_ds(hSession," Data(rec=%lu) %d bytes\n", ft->recnum, my_length);

	/*
	 * First, check to see if we have message data or file data.
	 * Message data will result in a popup.
	 */
	if (ft->message_flag)
	{
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
			ft_complete(hSession->ft,NULL);
		}
		else if (lib3270_get_ft_state(hSession) == FT_ABORT_SENT && ((H3270FT *) hSession->ft)->abort_string != CN)
		{
			lib3270_free(msgp);
			ft_complete(ft,ft->abort_string);
			lib3270_free(ft->abort_string);
		}
		else
		{
			ft_complete(hSession->ft,(char *)msgp);
			lib3270_free(msgp);
		}
	} else if (my_length > 0) {
		/* Write the data out to the file. */
		int rv = 1;

		if (ft->ascii_flag && ft->remap_flag)
		{
			/* Filter. */
			unsigned char *s = (unsigned char *)data_bufr->data;
			unsigned len = my_length;

			while (len--)
			{
				*s = ft->charset.ebc2asc[*s];
				s++;
			}
		}
		if (ft->ascii_flag && ft->cr_flag)
		{
			char *s = (char *)data_bufr->data;
			unsigned len = my_length;

			/* Delete CRs and ^Zs. */
			while (len) {
				unsigned l = filter_len(s, len);

				if (l)
				{
					rv = fwrite(s, l, (size_t)1,ft->local_file);
					if (rv == 0)
						break;
					ft->ft_length += l;
				}
				if (l < len)
					l++;
				s += l;
				len -= l;
			}
		} else {
			rv = fwrite((char *)data_bufr->data, my_length,(size_t)1, ft->local_file);
			ft->ft_length += my_length;
		}

		if (!rv) {
			/* write failed */
			dft_abort(hSession,TR_DATA_INSERT, _( "Error \"%s\" writing to local file (rc=%d)" ) , strerror(errno), errno);
		}

		/* Add up amount transferred. */
		ft_update_length(ft);
	}

	/* Send an acknowledgement frame back. */
	trace_ds(hSession,"> WriteStructuredField FileTransferData DataAck(rec=%lu)\n", ft->recnum);
	hSession->obptr = hSession->obuf;
	space3270out(hSession,12);
	*hSession->obptr++ = AID_SF;
	SET16(hSession->obptr, 11);
	*hSession->obptr++ = SF_TRANSFER_DATA;
	SET16(hSession->obptr, TR_NORMAL_REPLY);
	SET16(hSession->obptr, TR_RECNUM_HDR);
	SET32(hSession->obptr, ft->recnum);
	ft->recnum++;
	net_output(hSession);
}

/* Process a Set Cursor request. */
static void dft_set_cur_req(H3270 *hSession)
{
	trace_ds(hSession," SetCursor\n");
	/* Currently doesn't do anything. */
}

/* Process a Get request. */
static void dft_get_request(H3270 *hSession)
{
	int				  numbytes;
	size_t			  numread;
	size_t 			  total_read = 0;
	unsigned char	* bufptr;
	H3270FT 		* ft = get_ft_handle(hSession);

	trace_ds(hSession," Get\n");

	if (!ft->message_flag && lib3270_get_ft_state(hSession) == FT_ABORT_WAIT)
	{
		dft_abort(hSession,TR_GET_REQ, _( "Transfer cancelled by user" ) );
		return;
	}

	/* Read a buffer's worth. */
	set_dft_buffersize(hSession);
	space3270out(hSession,hSession->dft_buffersize);
	numbytes = hSession->dft_buffersize - 27; /* always read 5 bytes less than we're allowed */
	bufptr = hSession->obuf + 17;

	while (!ft->dft_eof && numbytes)
	{
		if (ft->ascii_flag && ft->cr_flag)
		{
			// ASCII text file

			int c;

			/* Read one byte and do CR/LF translation. */
			c = fgetc(ft->local_file);
			if (c == EOF)
			{
				break;
			}
			if (!ft->ft_last_cr && c == '\n')
			{
				if (numbytes < 2)
				{
					/*
					 * Not enough room to expand NL to
					 * CR/LF.
					 */
					ungetc(c, ft->local_file);
					break;
				}
				*bufptr++ = '\r';
				numbytes--;
				total_read++;
			}
			ft->ft_last_cr = (c == '\r') ? 1 : 0;
			*bufptr++ = ft->remap_flag ? ft->charset.asc2ebc[c]: c;
			numbytes--;
			total_read++;
		}
		else
		{
			/* Binary read. */
			numread = fread(bufptr, 1, numbytes, ft->local_file);
			if (numread <= 0)
			{
				lib3270_write_log(hSession,"Error %s reading source file (rc=%d)",strerror(errno),errno);
				break;
			}

			if (ft->ascii_flag && ft->remap_flag)
			{
				// Remap charset
				unsigned char *s = bufptr;
				int i = numread;

				while (i)
				{
					*s = ft->charset.asc2ebc[*s];
					s++;
					i--;
				}
			}
			bufptr 		+= numread;
			numbytes	-= numread;
			total_read	+= numread;
		}

		if (feof(ft->local_file) || ferror(ft->local_file))
		{
			break;
		}
	}

	/* Check for read error. */
	if (ferror(((H3270FT *) hSession->ft)->local_file))
	{
		dft_abort(hSession,TR_GET_REQ, _( "Error \"%s\" reading from local file (rc=%d)" ), strerror(errno), errno);
		return;
	}

	/* Set up SF header for Data or EOF. */
	hSession->obptr = hSession->obuf;
	*hSession->obptr++ = AID_SF;
	hSession->obptr += 2;	/* skip SF length for now */
	*hSession->obptr++ = SF_TRANSFER_DATA;

	if (total_read)
	{
		trace_ds(hSession,"> WriteStructuredField FileTransferData Data(rec=%lu) %d bytes\n",(unsigned long) ft->recnum, (int) total_read);
		SET16(hSession->obptr, TR_GET_REPLY);
		SET16(hSession->obptr, TR_RECNUM_HDR);
		SET32(hSession->obptr, ft->recnum);
		ft->recnum++;
		SET16(hSession->obptr, TR_NOT_COMPRESSED);
		*hSession->obptr++ = TR_BEGIN_DATA;
		SET16(hSession->obptr, total_read + 5);
		hSession->obptr += total_read;

		ft->ft_length += total_read;

		if (feof(ft->local_file))
		{
			ft->dft_eof = 1;
		}

	}
	else
	{
		trace_ds(hSession,"> WriteStructuredField FileTransferData EOF\n");
		*hSession->obptr++ = HIGH8(TR_GET_REQ);
		*hSession->obptr++ = TR_ERROR_REPLY;
		SET16(hSession->obptr, TR_ERROR_HDR);
		SET16(hSession->obptr, TR_ERR_EOF);

		ft->dft_eof = 1;
	}

	/* Set the SF length. */
	bufptr = hSession->obuf + 1;
	SET16(bufptr, hSession->obptr - (hSession->obuf + 1));

	/* Save the data. */
	ft->dft_savebuf_len = hSession->obptr - hSession->obuf;
	if (ft->dft_savebuf_len > ft->dft_savebuf_max)
	{
		ft->dft_savebuf_max = ft->dft_savebuf_len;
		Replace(ft->dft_savebuf, (unsigned char *)lib3270_malloc(ft->dft_savebuf_max));
	}
	(void) memcpy(ft->dft_savebuf, hSession->obuf, ft->dft_savebuf_len);
	hSession->aid = AID_SF;

	/* Write the data. */
	net_output(hSession);
	ft_update_length(get_ft_handle(hSession));
}

/* Process a Close request. */
static void dft_close_request(H3270 *hSession)
{
	/*
	 * Recieved a close request from the system.
	 * Return a close acknowledgement.
	 */
	trace_ds(hSession," Close\n");
	trace_ds(hSession,"> WriteStructuredField FileTransferData CloseAck\n");
	hSession->obptr = hSession->obuf;
	space3270out(hSession,6);
	*hSession->obptr++ = AID_SF;
	SET16(hSession->obptr, 5);	/* length */
	*hSession->obptr++ = SF_TRANSFER_DATA;
	SET16(hSession->obptr, TR_CLOSE_REPLY);
	net_output(hSession);
}

/* Abort a transfer. */
static void dft_abort(H3270 *hSession, unsigned short code, const char *fmt, ...)
{
	H3270FT *ft = (H3270FT *) hSession->ft;
	va_list args;

	lib3270_free(ft->abort_string);

	va_start(args, fmt);
	ft->abort_string = lib3270_vsprintf(fmt, args);
	va_end(args);

	trace_ds(hSession,"> WriteStructuredField FileTransferData Error\n");

	hSession->obptr = hSession->obuf;
	space3270out(hSession,10);
	*hSession->obptr++ = AID_SF;
	SET16(hSession->obptr, 9);	/* length */
	*hSession->obptr++ = SF_TRANSFER_DATA;
	*hSession->obptr++ = HIGH8(code);
	*hSession->obptr++ = TR_ERROR_REPLY;
	SET16(hSession->obptr, TR_ERROR_HDR);
	SET16(hSession->obptr, TR_ERR_CMDFAIL);
	net_output(hSession);

	/* Update the pop-up and state. */
	ft_aborting(ft);
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

/**
 * Processes a Read Modified command when there is upload data pending.
 */
void dft_read_modified(H3270 *hSession)
{
	H3270FT	*ft = get_ft_handle(hSession);

	if(ft->dft_savebuf_len)
	{
		trace_ds(hSession,"> WriteStructuredField FileTransferData\n");
		hSession->obptr = hSession->obuf;
		space3270out(hSession,ft->dft_savebuf_len);
		memcpy(hSession->obptr, ft->dft_savebuf, ft->dft_savebuf_len);
		hSession->obptr += ft->dft_savebuf_len;
		net_output(hSession);
	}
}

/**
 * Update the buffersize for generating a Query Reply.
 */
void set_dft_buffersize(H3270 *hSession)
{
	if (hSession->dft_buffersize == 0)
		hSession->dft_buffersize = DFT_BUF;

	if (hSession->dft_buffersize > DFT_MAX_BUF)
		hSession->dft_buffersize = DFT_MAX_BUF;

	if (hSession->dft_buffersize < DFT_MIN_BUF)
		hSession->dft_buffersize = DFT_MIN_BUF;
}


#endif /*]*/
