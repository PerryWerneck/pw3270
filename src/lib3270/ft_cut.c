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
 * Este programa está nomeado como ft_cut.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

/**
 *	@file ft_cut.c
 *
 *		File transfer, data movement logic, CUT version
 */

#include <errno.h>

#include "globals.h"

#if defined(X3270_FT) /*[*/

#include "3270ds.h"
//#include "actionsc.h"
#include "ctlrc.h"
#include "ft_cutc.h"
#include "ft_cut_ds.h"
#include "ftc.h"
#include "kybdc.h"
#include "popupsc.h"
#include "tablesc.h"
#include "telnetc.h"
#include "trace_dsc.h"
#include "utilc.h"

// static Boolean cut_xfer_in_progress = 0;

/* Data stream conversion tables. */

#define NQ			4		/* number of quadrants */
#define NE			77		/* number of elements per quadrant */
#define OTHER_2		2		/* "OTHER 2" quadrant (includes NULL) */
#define XLATE_NULL	0xc1	/* translation of NULL */

static const char alphas[NE + 1] = " ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789%&_()<+,-./:>?";

static const struct
{
	unsigned char selector;
	unsigned char xlate[NE];
} conv[NQ] = {
    {	0x5e,	/* ';' */
	{ 0x40,0xc1,0xc2,0xc3, 0xc4,0xc5,0xc6,0xc7, 0xc8,0xc9,0xd1,0xd2,
	  0xd3,0xd4,0xd5,0xd6, 0xd7,0xd8,0xd9,0xe2, 0xe3,0xe4,0xe5,0xe6,
	  0xe7,0xe8,0xe9,0x81, 0x82,0x83,0x84,0x85, 0x86,0x87,0x88,0x89,
	  0x91,0x92,0x93,0x94, 0x95,0x96,0x97,0x98, 0x99,0xa2,0xa3,0xa4,
	  0xa5,0xa6,0xa7,0xa8, 0xa9,0xf0,0xf1,0xf2, 0xf3,0xf4,0xf5,0xf6,
	  0xf7,0xf8,0xf9,0x6c, 0x50,0x6d,0x4d,0x5d, 0x4c,0x4e,0x6b,0x60,
	  0x4b,0x61,0x7a,0x6e, 0x6f }
    },
    {	0x7e,	/* '=' */
	{ 0x20,0x41,0x42,0x43, 0x44,0x45,0x46,0x47, 0x48,0x49,0x4a,0x4b,
	  0x4c,0x4d,0x4e,0x4f, 0x50,0x51,0x52,0x53, 0x54,0x55,0x56,0x57,
	  0x58,0x59,0x5a,0x61, 0x62,0x63,0x64,0x65, 0x66,0x67,0x68,0x69,
	  0x6a,0x6b,0x6c,0x6d, 0x6e,0x6f,0x70,0x71, 0x72,0x73,0x74,0x75,
	  0x76,0x77,0x78,0x79, 0x7a,0x30,0x31,0x32, 0x33,0x34,0x35,0x36,
	  0x37,0x38,0x39,0x25, 0x26,0x27,0x28,0x29, 0x2a,0x2b,0x2c,0x2d,
	  0x2e,0x2f,0x3a,0x3b, 0x3f }
    },
    {	0x5c,	/* '*' */
	{ 0x00,0x00,0x01,0x02, 0x03,0x04,0x05,0x06, 0x07,0x08,0x09,0x0a,
	  0x0b,0x0c,0x0d,0x0e, 0x0f,0x10,0x11,0x12, 0x13,0x14,0x15,0x16,
	  0x17,0x18,0x19,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	  0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,
	  0x00,0x00,0x00,0x00, 0x00,0x3c,0x3d,0x3e, 0x00,0xfa,0xfb,0xfc,
	  0xfd,0xfe,0xff,0x7b, 0x7c,0x7d,0x7e,0x7f, 0x1a,0x1b,0x1c,0x1d,
	  0x1e,0x1f,0x00,0x00, 0x00 }
    },
    {	0x7d,	/* '\'' */
	{ 0x00,0xa0,0xa1,0xea, 0xeb,0xec,0xed,0xee, 0xef,0xe0,0xe1,0xaa,
	  0xab,0xac,0xad,0xae, 0xaf,0xb0,0xb1,0xb2, 0xb3,0xb4,0xb5,0xb6,
	  0xb7,0xb8,0xb9,0x80, 0x00,0xca,0xcb,0xcc, 0xcd,0xce,0xcf,0xc0,
	  0x00,0x8a,0x8b,0x8c, 0x8d,0x8e,0x8f,0x90, 0x00,0xda,0xdb,0xdc,
	  0xdd,0xde,0xdf,0xd0, 0x00,0x00,0x21,0x22, 0x23,0x24,0x5b,0x5c,
	  0x00,0x5e,0x5f,0x00, 0x9c,0x9d,0x9e,0x9f, 0xba,0xbb,0xbc,0xbd,
	  0xbe,0xbf,0x9a,0x9b, 0x00 }
    }
};
static const char table6[] = "abcdefghijklmnopqrstuvwxyz&-.,:+ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";

// static int quadrant = -1;
// static unsigned long expanded_length;
// static char *saved_errmsg = CN;

#define XLATE_NBUF	LIB3270_XLATE_NBUF
// static int xlate_buffered = 0;			/* buffer count */
// static int xlate_buf_ix = 0;			/* buffer index */
// static unsigned char xlate_buf[XLATE_NBUF];	/* buffer */

static void cut_control_code(H3270 *hSession);
static void cut_data_request(H3270 *hSession);
static void cut_retransmit(H3270 *hSession);
static void cut_data(H3270 *hSession);

static void cut_ack(H3270 *hSession);
static void cut_abort(H3270 *hSession, unsigned short code, const char *fmt, ...) printflike(3,4);

static unsigned from6(unsigned char c);
static int xlate_getc(H3270FT *ft);

/**
 * Convert a buffer for uploading (host->local). Overwrites the buffer.
 *
 * If there is a conversion error, calls cut_abort() and returns -1.
 *
 * @return the length of the converted data.
 */
static int upload_convert(H3270 *hSession, unsigned char *buf, int len)
{
	unsigned char	* ob0 = buf;
	unsigned char	* ob = ob0;
	H3270FT			* ft = get_ft_handle(hSession);

	while (len--)
	{
		unsigned char c = *buf++;
		char *ixp;
		int ix;
		// int oq = -1;

	    retry:
		if (ft->quadrant < 0)
		{
			/* Find the quadrant. */
			for (ft->quadrant = 0; ft->quadrant < NQ; ft->quadrant++)
			{
				if (c == conv[ft->quadrant].selector)
					break;
			}
			if (ft->quadrant >= NQ)
			{
				cut_abort(hSession,SC_ABORT_XMIT, "%s", _("Data conversion error"));
				return -1;
			}
			continue;
		}

		/* Make sure it's in a valid range. */
		if (c < 0x40 || c > 0xf9)
		{
			cut_abort(hSession,SC_ABORT_XMIT, "%s", _("Data conversion error"));
			return -1;
		}

		/* Translate to a quadrant index. */
		ixp = strchr(alphas, ebc2asc[c]);
		if (ixp == (char *)NULL)
		{
			/* Try a different quadrant. */
			// oq = quadrant;
			ft->quadrant = -1;
			goto retry;
		}
		ix = ixp - alphas;

		/*
		 * See if it's mapped by that quadrant, handling NULLs
		 * specially.
		 */
		if (ft->quadrant != OTHER_2 && c != XLATE_NULL && !conv[ft->quadrant].xlate[ix])
		{
			/* Try a different quadrant. */
//			oq = quadrant;
			ft->quadrant = -1;
			goto retry;
		}

		/* Map it. */
		c = conv[ft->quadrant].xlate[ix];
		if (ft->ascii_flag && ft->cr_flag && (c == '\r' || c == 0x1a))
			continue;
		if (ft->ascii_flag && ft->remap_flag)
			c = ft2asc[c];
		*ob++ = c;
	}

	return ob - ob0;
}

/**
 * Convert a buffer for downloading (local->host).
 */
static int download_convert(H3270FT *ft, unsigned const char *buf, unsigned len, unsigned char *xobuf)
{
	unsigned char	* ob0 = xobuf;
	unsigned char	* ob = ob0;

	while (len--)
	{
		unsigned char c = *buf++;
		unsigned char *ixp;
		unsigned ix;
		int oq;

		/* Handle nulls separately. */
		if (!c)
		{
			if (ft->quadrant != OTHER_2)
			{
				ft->quadrant = OTHER_2;
				*ob++ = conv[ft->quadrant].selector;
			}
			*ob++ = XLATE_NULL;
			continue;
		}

		/* Translate. */
		if (ft->ascii_flag && ft->remap_flag)
			c = asc2ft[c];

		/* Quadrant already defined. */
		if (ft->quadrant >= 0) {
			ixp = (unsigned char *)memchr(conv[ft->quadrant].xlate, c, NE);
			if (ixp != (unsigned char *)NULL)
			{
				ix = ixp - conv[ft->quadrant].xlate;
				*ob++ = asc2ebc[(int)alphas[ix]];
				continue;
			}
		}

		/* Locate a quadrant. */
		oq = ft->quadrant;
		for (ft->quadrant = 0; ft->quadrant < NQ; ft->quadrant++)
		{
			if (ft->quadrant == oq)
				continue;

			ixp = (unsigned char *)memchr(conv[ft->quadrant].xlate, c, NE);

			if (ixp == (unsigned char *)NULL)
				continue;
			ix = ixp - conv[ft->quadrant].xlate;
			*ob++ = conv[ft->quadrant].selector;
			*ob++ = asc2ebc[(int)alphas[ix]];
			break;
		}
		if (ft->quadrant >= NQ)
		{
			ft->quadrant = -1;
			fprintf(stderr, "Oops\n");
			continue;
		}
	}
	return ob - ob0;
}

/*
 * Main entry point from ctlr.c.
 * We have received what looks like an appropriate message from the host.
 */
void ft_cut_data(H3270 *hSession)
{
	switch (hSession->ea_buf[O_FRAME_TYPE].cc)
	{
    case FT_CONTROL_CODE:
		cut_control_code(hSession);
		break;

    case FT_DATA_REQUEST:
		cut_data_request(hSession);
		break;

    case FT_RETRANSMIT:
		cut_retransmit(hSession);
		break;

    case FT_DATA:
		cut_data(hSession);
		break;

    default:
		trace_ds(hSession,"< FT unknown 0x%02x\n", hSession->ea_buf[O_FRAME_TYPE].cc);
		cut_abort(hSession,SC_ABORT_XMIT, "%s", _("Unknown frame type from host"));
		break;
	}
}

/*
 * Process a control code from the host.
 */
static void cut_control_code(H3270 *hSession)
{
	H3270FT			* ft	= get_ft_handle(hSession);
	unsigned short	  code;
	char 			* buf;
	char			* bp;
	int				  i;

	trace_ds(hSession,"< FT CONTROL_CODE ");
	code = (hSession->ea_buf[O_CC_STATUS_CODE].cc << 8) | hSession->ea_buf[O_CC_STATUS_CODE + 1].cc;

	switch (code)
	{
	case SC_HOST_ACK:
		trace_ds(hSession,"HOST_ACK\n");
		hSession->cut_xfer_in_progress = 1;
		ft->expanded_length = 0;
		ft->quadrant = -1;
		ft->xlate_buffered = 0;
		cut_ack(hSession);
		ft_running(hSession->ft,True);
		break;

	case SC_XFER_COMPLETE:
		trace_ds(hSession,"XFER_COMPLETE\n");
		cut_ack(hSession);
		hSession->cut_xfer_in_progress = 0;
		ft_complete(ft,N_( "Complete" ) );
		break;

	case SC_ABORT_FILE:
	case SC_ABORT_XMIT:
		trace_ds(hSession,"ABORT\n");
		hSession->cut_xfer_in_progress = 0;
		cut_ack(hSession);

		if (lib3270_get_ft_state(hSession) == FT_ABORT_SENT && ft->saved_errmsg != CN)
		{
			buf = ft->saved_errmsg;
			ft->saved_errmsg = CN;
		}
		else
		{
			bp = buf = lib3270_malloc(81);

			for (i = 0; i < 80; i++)
				*bp++ = ebc2asc[hSession->ea_buf[O_CC_MESSAGE + i].cc];

			*bp-- = '\0';

			while (bp >= buf && *bp == ' ')
				*bp-- = '\0';

			if (bp >= buf && *bp == '$')
				*bp-- = '\0';

			while (bp >= buf && *bp == ' ')
				*bp-- = '\0';

			if (!*buf)
				strcpy(buf, N_( "Transfer cancelled by host" ) );
		}
		ft_complete(hSession->ft,buf);
		lib3270_free(buf);
		break;

	default:
		trace_ds(hSession,"unknown 0x%04x\n", code);
		cut_abort(hSession,SC_ABORT_XMIT, "%s", _("Unknown FT control code from host"));
		break;
	}
}

/*
 * Process a data request from the host.
 */
static void cut_data_request(H3270 *hSession)
{
	H3270FT			* ft	= get_ft_handle(hSession);
	unsigned char	  seq	= hSession->ea_buf[O_DR_FRAME_SEQ].cc;
	int				  count;
	unsigned char	  cs;
	int				  c;
	int				  i;
	unsigned char	  attr;

	trace_ds(hSession,"< FT DATA_REQUEST %u\n", from6(seq));
	if (lib3270_get_ft_state(hSession) == FT_ABORT_WAIT)
	{
		cut_abort(hSession,SC_ABORT_FILE,"%s", N_("Transfer cancelled by user") );
		return;
	}


	/* Copy data into the screen buffer. */
	count = 0;
	while (count < O_UP_MAX && (c = xlate_getc(hSession->ft)) != EOF)
	{
		ctlr_add(hSession,O_UP_DATA + count, c, 0);
		count++;
	}

	/* Check for errors. */
	if (ferror(((H3270FT *) hSession->ft)->local_file))
	{
		int j;

		/* Clean out any data we may have written. */
		for (j = 0; j < count; j++)
			ctlr_add(hSession,O_UP_DATA + j, 0, 0);

		/* Abort the transfer. */
		cut_abort(hSession,SC_ABORT_FILE,_( "Error \"%s\" reading from local file (rc=%d)" ), strerror(errno), errno);
		return;
	}

	/* Send special data for EOF. */
	if (!count && feof(((H3270FT *) hSession->ft)->local_file))
	{
		ctlr_add(hSession,O_UP_DATA, EOF_DATA1, 0);
		ctlr_add(hSession,O_UP_DATA+1, EOF_DATA2, 0);
		count = 2;
	}

	/* Compute the other fields. */
	ctlr_add(hSession,O_UP_FRAME_SEQ, seq, 0);
	cs = 0;
	for (i = 0; i < count; i++)
		cs ^= hSession->ea_buf[O_UP_DATA + i].cc;

	ctlr_add(hSession,O_UP_CSUM, asc2ebc[(int)table6[cs & 0x3f]], 0);
	ctlr_add(hSession,O_UP_LEN, asc2ebc[(int)table6[(count >> 6) & 0x3f]], 0);
	ctlr_add(hSession,O_UP_LEN+1, asc2ebc[(int)table6[count & 0x3f]], 0);

	/* XXX: Change the data field attribute so it doesn't display. */
	attr = hSession->ea_buf[O_DR_SF].fa;
	attr = (attr & ~FA_INTENSITY) | FA_INT_ZERO_NSEL;
	ctlr_add_fa(hSession,O_DR_SF, attr, 0);

	/* Send it up to the host. */
	trace_ds(hSession,"> FT DATA %u\n", from6(seq));
	ft_update_length(ft);
	ft->expanded_length += count;

	lib3270_enter(hSession);
}

/*
 * (Improperly) process a retransmit from the host.
 */
static void  cut_retransmit(H3270 *hSession)
{
	trace_ds(hSession,"< FT RETRANSMIT\n");
	cut_abort(hSession,SC_ABORT_XMIT,"%s",_("Transmission error"));
}

/*
 * Convert an encoded integer.
 */
static unsigned
from6(unsigned char c)
{
	char *p;

	c = ebc2asc[c];
	p = strchr(table6, c);
	if (p == CN)
		return 0;
	return p - table6;
}

/*
 * Process data from the host.
 */
static void cut_data(H3270 *hSession)
{
	H3270FT *ft = get_ft_handle(hSession);
	static unsigned char cvbuf[O_RESPONSE - O_DT_DATA];
	unsigned short raw_length;
	int conv_length;
	register int i;

	trace_ds(hSession,"< FT DATA\n");
	if (ft->state == LIB3270_FT_STATE_ABORT_WAIT)
	{
		cut_abort(hSession,SC_ABORT_FILE,"%s",_("Transfer cancelled by user"));
		return;
	}

	/* Copy and convert the data. */
	raw_length = from6(hSession->ea_buf[O_DT_LEN].cc) << 6 |
		     from6(hSession->ea_buf[O_DT_LEN + 1].cc);

	if ((int)raw_length > O_RESPONSE - O_DT_DATA)
	{
		cut_abort(hSession,SC_ABORT_XMIT,"%s",_("Illegal frame length"));
		return;
	}

	for (i = 0; i < (int)raw_length; i++)
		cvbuf[i] = hSession->ea_buf[O_DT_DATA + i].cc;

	if (raw_length == 2 && cvbuf[0] == EOF_DATA1 && cvbuf[1] == EOF_DATA2)
	{
		trace_ds(hSession,"< FT EOF\n");
		cut_ack(hSession);
		return;
	}

	conv_length = upload_convert(hSession, cvbuf, raw_length);
	if (conv_length < 0)
		return;

	/* Write it to the file. */
	if (fwrite((char *)cvbuf, conv_length, 1, ft->local_file) == 0)
	{
		cut_abort(hSession,SC_ABORT_FILE,_( "Error \"%s\" writing to local file (rc=%d)" ),strerror(errno),errno);
	}
	else
	{
		ft->ft_length += conv_length;
		ft_update_length(ft);
		cut_ack(hSession);
	}
}

/*
 * Acknowledge a host command.
 */
static void cut_ack(H3270 *hSession)
{
	trace_ds(hSession,"> FT ACK\n");
	lib3270_enter(hSession);
}

/*
 * Abort a transfer in progress.
 */
static void cut_abort(H3270 *hSession, unsigned short reason, const char *fmt, ...)
{
	H3270FT	* ft = get_ft_handle(hSession);
	va_list	  args;

	if(ft->saved_errmsg)
		lib3270_free(ft->saved_errmsg);

	/* Save the error message. */
	va_start(args, fmt);
	ft->saved_errmsg = lib3270_vsprintf(fmt, args);
	va_end(args);

	/* Send the abort sequence. */
	ctlr_add(hSession,RO_FRAME_TYPE, RFT_CONTROL_CODE, 0);
	ctlr_add(hSession,RO_FRAME_SEQ, hSession->ea_buf[O_DT_FRAME_SEQ].cc, 0);
	ctlr_add(hSession,RO_REASON_CODE, HIGH8(reason), 0);
	ctlr_add(hSession,RO_REASON_CODE+1, LOW8(reason), 0);
	trace_ds(hSession,"> FT CONTROL_CODE ABORT\n");

	lib3270_pfkey(hSession,2);

	/* Update the in-progress pop-up. */
	ft_aborting(ft);
}

/**
 * Get the next translated character from the local file.
 *
 * @return the character (in EBCDIC), or EOF.
 */
static int xlate_getc(H3270FT *ft)
{
	int r;
	int c;
	unsigned char cc;
	unsigned char cbuf[4];
	int nc;

	/* If there is a data buffered, return it. */
	if (ft->xlate_buffered)
	{
		r = ft->xlate_buf[ft->xlate_buf_ix];
		ft->xlate_buf_ix++;
		ft->xlate_buffered--;
		return r;
	}

	/* Get the next byte from the file. */
	c = fgetc(ft->local_file);
	if (c == EOF)
		return c;
	ft->ft_length++;

	/* Expand it. */
	if (ft->ascii_flag && ft->cr_flag && !ft->ft_last_cr && c == '\n')
	{
		nc = download_convert(ft,(unsigned const char *)"\r", 1, cbuf);
	}
	else
	{
		nc = 0;
		ft->ft_last_cr = (c == '\r') ? 1 : 0;
	}

	/* Convert it. */
	cc = (unsigned char)c;
	nc += download_convert(ft,&cc, 1, &cbuf[nc]);

	/* Return it and buffer what's left. */
	r = cbuf[0];
	if (nc > 1)
	{
		int i;

		for (i = 1; i < nc; i++)
			ft->xlate_buf[ft->xlate_buffered++] = cbuf[i];
		ft->xlate_buf_ix = 0;
	}
	return r;
}

#endif /*]*/
