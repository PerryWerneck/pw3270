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
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como sf.c e possui 964 linhas de código.
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
 *	sf.c
 *		This module handles 3270 structured fields.
 *
 */

#include "globals.h"
#include <errno.h>
#if !defined(_WIN32) /*[*/
#include <netinet/in.h>
#endif /*]*/
#include "3270ds.h"
// #include "appres.h"
#include "screen.h"
// #include "ctlr.h"
#include "resources.h"

#include "charsetc.h"
#include "ctlrc.h"

#if defined(X3270_FT)
	#include "ft_dftc.h"
#endif

#include "kybdc.h"
#include "screenc.h"
#include "seec.h"
#include "sf.h"
#include "tablesc.h"
#include "telnetc.h"
#include "trace_dsc.h"
#include "utilc.h"

#include "api.h"

// Externals
// extern unsigned char reply_mode;
// extern int      crm_nattr;
// extern unsigned char crm_attr[];

/* Screen globals. */
static int		  cw = 7;
int 			* char_width = &cw;

static int		  ch = 7;
int 			* char_height = &ch;

/* Globals */
static Boolean	  sfont = False;
Boolean			* standard_font = &sfont;


/* Statics */
static Boolean  qr_in_progress = False;
static enum pds sf_read_part(unsigned char buf[], unsigned buflen);
static enum pds sf_erase_reset(unsigned char buf[], int buflen);
static enum pds sf_set_reply_mode(unsigned char buf[], int buflen);
static enum pds sf_create_partition(unsigned char buf[], int buflen);
static enum pds sf_outbound_ds(unsigned char buf[], int buflen);
static void query_reply_start(void);
static void do_query_reply(unsigned char code);
static void query_reply_end(void);

typedef Boolean qr_multi_fn_t(unsigned *subindex, Boolean *more);

static qr_single_fn_t do_qr_summary, do_qr_usable_area, do_qr_alpha_part,
	do_qr_charsets, do_qr_color, do_qr_highlighting, do_qr_reply_modes,
	do_qr_imp_part, do_qr_null;


#if defined(X3270_DBCS) /*[*/
static qr_single_fn_t do_qr_dbcs_asia;
#endif /*]*/
#if defined(X3270_FT) /*[*/
static qr_single_fn_t do_qr_ddm;
#endif /*]*/

static struct reply {
	unsigned char code;
	qr_single_fn_t *single_fn;
	qr_multi_fn_t *multi_fn;
} replies[] = {
    { QR_SUMMARY,      do_qr_summary,      NULL },		/* 0x80 */
    { QR_USABLE_AREA,  do_qr_usable_area,  NULL },		/* 0x81 */
    { QR_ALPHA_PART,   do_qr_alpha_part,   NULL },		/* 0x84 */
    { QR_CHARSETS,     do_qr_charsets,     NULL },		/* 0x85 */
    { QR_COLOR,        do_qr_color,        NULL },		/* 0x86 */
    { QR_HIGHLIGHTING, do_qr_highlighting, NULL },		/* 0x87 */
    { QR_REPLY_MODES,  do_qr_reply_modes,  NULL },		/* 0x88 */
#if defined(X3270_DBCS) /*[*/
    { QR_DBCS_ASIA,    do_qr_dbcs_asia,    NULL },		/* 0x91 */
#endif /*]*/

#if defined(X3270_FT) /*[*/
    { QR_DDM,          do_qr_ddm,          NULL },		/* 0x95 */
#endif /*]*/

#ifndef ANDROID
    { QR_RPQNAMES,     do_qr_rpqnames,     NULL },		/* 0xa1 */
#endif // !ANDROID

    { QR_IMP_PART,     do_qr_imp_part,     NULL },		/* 0xa6 */

    /* QR_NULL must be last in the table */
    { QR_NULL,         do_qr_null,         NULL },		/* 0xff */
};

/*
 * NSR_ALL is the number of query replies supported, including NULL.
 * NSR is the number of query replies supported, except for NULL.
 */
#define NSR_ALL	(sizeof(replies)/sizeof(struct reply))
#define NSR	(NSR_ALL - 1)


/*
 * Process a 3270 Write Structured Field command
 */
enum pds
write_structured_field(unsigned char buf[], int buflen)
{
	unsigned short fieldlen;
	unsigned char *cp = buf;
	Boolean first = True;
	enum pds rv = PDS_OKAY_NO_OUTPUT;
	enum pds rv_this = PDS_OKAY_NO_OUTPUT;
	Boolean bad_cmd = False;

	/* Skip the WSF command itself. */
	cp++;
	buflen--;

	/* Interpret fields. */
	while (buflen > 0) {

		if (first)
			trace_ds(&h3270," ");
		else
			trace_ds(&h3270,"< WriteStructuredField ");
		first = False;

		/* Pick out the field length. */
		if (buflen < 2) {
			trace_ds(&h3270,"error: single byte at end of message\n");
			return rv ? rv : PDS_BAD_CMD;
		}
		fieldlen = (cp[0] << 8) + cp[1];
		if (fieldlen == 0)
			fieldlen = buflen;
		if (fieldlen < 3) {
			trace_ds(&h3270,"error: field length %d too small\n",fieldlen);
			return rv ? rv : PDS_BAD_CMD;
		}
		if ((int)fieldlen > buflen) {
			trace_ds(&h3270,"error: field length %d exceeds remaining message length %d\n",fieldlen, buflen);
			return rv ? rv : PDS_BAD_CMD;
		}

		/* Dispatch on the ID. */
		switch (cp[2]) {
		    case SF_READ_PART:
			trace_ds(&h3270,"ReadPartition");
			rv_this = sf_read_part(cp, (int)fieldlen);
			break;
		    case SF_ERASE_RESET:
			trace_ds(&h3270,"EraseReset");
			rv_this = sf_erase_reset(cp, (int)fieldlen);
			break;
		    case SF_SET_REPLY_MODE:
			trace_ds(&h3270,"SetReplyMode");
			rv_this = sf_set_reply_mode(cp, (int)fieldlen);
			break;
		    case SF_CREATE_PART:
			trace_ds(&h3270,"CreatePartition");
			rv_this = sf_create_partition(cp, (int)fieldlen);
			break;
		    case SF_OUTBOUND_DS:
			trace_ds(&h3270,"OutboundDS");
			rv_this = sf_outbound_ds(cp, (int)fieldlen);
			break;
#if defined(X3270_FT) /*[*/
		    case SF_TRANSFER_DATA:   /* File transfer data         */
			trace_ds(&h3270,"FileTransferData");
			ft_dft_data(&h3270,cp, (int)fieldlen);
			break;
#endif /*]*/
		    default:
			trace_ds(&h3270,"unsupported ID 0x%02x\n", cp[2]);
			rv_this = PDS_BAD_CMD;
			break;
		}

		/*
		 * Accumulate errors or output flags.
		 * One real ugliness here is that if we have already
		 * generated some output, then we have already positively
		 * acknowledged the request, so if we fail here, we have no
		 * way to return the error indication.
		 */
		if (rv_this < 0)
			bad_cmd = True;
		else
			rv |= rv_this;

		/* Skip to the next field. */
		cp += fieldlen;
		buflen -= fieldlen;
	}
	if (first)
		trace_ds(&h3270," (null)\n");

	if (bad_cmd && !rv)
		return PDS_BAD_CMD;
	else
		return rv;
}

static enum pds
sf_read_part(unsigned char buf[], unsigned buflen)
{
	unsigned char partition;
	unsigned i;
	int any = 0;
	const char *comma = "";

	if (buflen < 5)
	{
		trace_ds(&h3270," error: field length %d too small\n", buflen);
		return PDS_BAD_CMD;
	}

	partition = buf[3];
	trace_ds(&h3270,"(0x%02x)", partition);

	switch (buf[4]) {
	    case SF_RP_QUERY:
		trace_ds(&h3270," Query");
		if (partition != 0xff) {
			trace_ds(&h3270," error: illegal partition\n");
			return PDS_BAD_CMD;
		}
		trace_ds(&h3270,"\n");
		query_reply_start();
		for (i = 0; i < NSR; i++) {
#if defined(X3270_DBCS) /*[*/
			if (dbcs || replies[i].code != QR_DBCS_ASIA)
#endif /*]*/
				do_query_reply(replies[i].code);
		}
 		query_reply_end();
		break;
	    case SF_RP_QLIST:
		trace_ds(&h3270," QueryList ");
		if (partition != 0xff) {
			trace_ds(&h3270,"error: illegal partition\n");
			return PDS_BAD_CMD;
		}
		if (buflen < 6) {
			trace_ds(&h3270,"error: missing request type\n");
			return PDS_BAD_CMD;
		}
		query_reply_start();
		switch (buf[5]) {
		    case SF_RPQ_LIST:
			trace_ds(&h3270,"List(");
			if (buflen < 7) {
				trace_ds(&h3270,")\n");
				do_query_reply(QR_NULL);
			} else {
				for (i = 6; i < buflen; i++) {
					trace_ds(&h3270,"%s%s", comma,see_qcode(buf[i]));
					comma = ",";
				}
				trace_ds(&h3270,")\n");
				for (i = 0; i < NSR; i++) {
					if (memchr((char *)&buf[6],
						   (char)replies[i].code,
						   buflen-6)
#if defined(X3270_DBCS) /*[*/
						   && (dbcs ||
						       replies[i].code != QR_DBCS_ASIA)
#endif /*]*/
						   ) {
						do_query_reply(replies[i].code);
						any++;
					}
				}
				if (!any) {
					do_query_reply(QR_NULL);
				}
			}
			break;
		    case SF_RPQ_EQUIV:
			trace_ds(&h3270,"Equivlent+List(");
			for (i = 6; i < buflen; i++) {
				trace_ds(&h3270,"%s%s", comma, see_qcode(buf[i]));
				comma = ",";
			}
			trace_ds(&h3270,")\n");
			for (i = 0; i < NSR; i++)
#if defined(X3270_DBCS) /*[*/
				if (dbcs || replies[i].code != QR_DBCS_ASIA)
#endif /*]*/
					do_query_reply(replies[i].code);
			break;
		    case SF_RPQ_ALL:
			trace_ds(&h3270,"All\n");
			for (i = 0; i < NSR; i++)
#if defined(X3270_DBCS) /*[*/
				if (dbcs || replies[i].code != QR_DBCS_ASIA)
#endif /*]*/
					do_query_reply(replies[i].code);
			break;
		    default:
			trace_ds(&h3270,"unknown request type 0x%02x\n", buf[5]);
			return PDS_BAD_CMD;
		}
		query_reply_end();
		break;
	    case SNA_CMD_RMA:
		trace_ds(&h3270," ReadModifiedAll");
		if (partition != 0x00) {
			trace_ds(&h3270," error: illegal partition\n");
			return PDS_BAD_CMD;
		}
		trace_ds(&h3270,"\n");
		ctlr_read_modified(&h3270, AID_QREPLY, True);
		break;
	    case SNA_CMD_RB:
		trace_ds(&h3270," ReadBuffer");
		if (partition != 0x00) {
			trace_ds(&h3270," error: illegal partition\n");
			return PDS_BAD_CMD;
		}
		trace_ds(&h3270,"\n");
		ctlr_read_buffer(&h3270,AID_QREPLY);
		break;
	    case SNA_CMD_RM:
		trace_ds(&h3270," ReadModified");
		if (partition != 0x00) {
			trace_ds(&h3270," error: illegal partition\n");
			return PDS_BAD_CMD;
		}
		trace_ds(&h3270,"\n");
		ctlr_read_modified(&h3270, AID_QREPLY, False);
		break;
	    default:
		trace_ds(&h3270," unknown type 0x%02x\n", buf[4]);
		return PDS_BAD_CMD;
	}
	return PDS_OKAY_OUTPUT;
}

static enum pds
sf_erase_reset(unsigned char buf[], int buflen)
{
	if (buflen != 4)
	{
		trace_ds(&h3270," error: wrong field length %d\n", buflen);
		return PDS_BAD_CMD;
	}

	switch (buf[3])
	{
	case SF_ER_DEFAULT:
		trace_ds(&h3270," Default\n");
		ctlr_erase(&h3270,False);
		break;

	case SF_ER_ALT:
		trace_ds(&h3270," Alternate\n");
		ctlr_erase(&h3270,True);
		break;

	default:
		trace_ds(&h3270," unknown type 0x%02x\n", buf[3]);
		return PDS_BAD_CMD;
	}
	return PDS_OKAY_NO_OUTPUT;
}

static enum pds
sf_set_reply_mode(unsigned char buf[], int buflen)
{
	unsigned char partition;
	int i;
	const char *comma = "(";

	if (buflen < 5) {
		trace_ds(&h3270," error: wrong field length %d\n", buflen);
		return PDS_BAD_CMD;
	}

	partition = buf[3];
	trace_ds(&h3270,"(0x%02x)", partition);
	if (partition != 0x00) {
		trace_ds(&h3270," error: illegal partition\n");
		return PDS_BAD_CMD;
	}

	switch (buf[4]) {
	    case SF_SRM_FIELD:
		trace_ds(&h3270," Field\n");
		break;
	    case SF_SRM_XFIELD:
		trace_ds(&h3270," ExtendedField\n");
		break;
	    case SF_SRM_CHAR:
		trace_ds(&h3270," Character");
		break;
	    default:
		trace_ds(&h3270," unknown mode 0x%02x\n", buf[4]);
		return PDS_BAD_CMD;
	}
	h3270.reply_mode = buf[4];
	if (buf[4] == SF_SRM_CHAR) {
		h3270.crm_nattr = buflen - 5;
		for (i = 5; i < buflen; i++) {
			h3270.crm_attr[i - 5] = buf[i];
			trace_ds(&h3270,"%s%s", comma, see_efa_only(buf[i]));
			comma = ",";
		}
		trace_ds(&h3270,"%s\n", h3270.crm_nattr ? ")" : "");
	}
	return PDS_OKAY_NO_OUTPUT;
}

static enum pds
sf_create_partition(unsigned char buf[], int buflen)
{
	unsigned char pid;
	unsigned char uom;	/* unit of measure */
	unsigned char am;		/* addressing mode */
	unsigned char flags;	/* flags */
	unsigned short h;		/* height of presentation space */
	unsigned short w;		/* width of presentation space */
	unsigned short rv;	/* viewport origin row */
	unsigned short cv;	/* viewport origin column */
	unsigned short hv;	/* viewport height */
	unsigned short wv;	/* viewport width */
	unsigned short rw;	/* window origin row */
	unsigned short cw;	/* window origin column */
	unsigned short rs;	/* scroll rows */
	/* hole */
	unsigned short pw;	/* character cell point width */
	unsigned short ph;	/* character cell point height */

#if defined(X3270_TRACE) /*[*/
	static const char *bit4[16] = {
	    "0000", "0001", "0010", "0011",
	    "0100", "0101", "0110", "0111",
	    "1000", "1001", "1010", "1011",
	    "1100", "1101", "1110", "1111"
	};
#endif /*]*/

	if (buflen > 3) {
		trace_ds(&h3270,"(");

		/* Partition. */
		pid = buf[3];
		trace_ds(&h3270,"pid=0x%02x", pid);
		if (pid != 0x00) {
			trace_ds(&h3270,") error: illegal partition\n");
			return PDS_BAD_CMD;
		}
	} else
		pid = 0x00;

	if (buflen > 4) {
		uom = (buf[4] & 0xf0) >> 4;
		trace_ds(&h3270,",uom=B'%s'", bit4[uom]);
		if (uom != 0x0 && uom != 0x02) {
			trace_ds(&h3270,") error: illegal units\n");
			return PDS_BAD_CMD;
		}
		am = buf[4] & 0x0f;
		trace_ds(&h3270,",am=B'%s'", bit4[am]);
		if (am > 0x2) {
			trace_ds(&h3270,") error: illegal a-mode\n");
			return PDS_BAD_CMD;
		}
	} else {
		uom = 0;
		am = 0;
	}

	if (buflen > 5) {
		flags = buf[5];
		trace_ds(&h3270,",flags=0x%02x", flags);
	} else
		flags = 0;

	if (buflen > 7) {
		GET16(h, &buf[6]);
		trace_ds(&h3270,",h=%d", h);
	} else
		h = h3270.maxROWS;

	if (buflen > 9) {
		GET16(w, &buf[8]);
		trace_ds(&h3270,",w=%d", w);
	} else
		w = h3270.maxCOLS;

	if (buflen > 11) {
		GET16(rv, &buf[10]);
		trace_ds(&h3270,",rv=%d", rv);
	} else
		rv = 0;

	if (buflen > 13) {
		GET16(cv, &buf[12]);
		trace_ds(&h3270,",cv=%d", cv);
	} else
		cv = 0;

	if (buflen > 15) {
		GET16(hv, &buf[14]);
		trace_ds(&h3270,",hv=%d", hv);
	} else
		hv = (h > h3270.maxROWS)? h3270.maxROWS: h;

	if (buflen > 17) {
		GET16(wv, &buf[16]);
		trace_ds(&h3270,",wv=%d", wv);
	} else
		wv = (w > h3270.maxCOLS)? h3270.maxCOLS: w;

	if (buflen > 19) {
		GET16(rw, &buf[18]);
		trace_ds(&h3270,",rw=%d", rw);
	} else
		rw = 0;

	if (buflen > 21) {
		GET16(cw, &buf[20]);
		trace_ds(&h3270,",cw=%d", cw);
	} else
		cw = 0;

	if (buflen > 23) {
		GET16(rs, &buf[22]);
		trace_ds(&h3270,",rs=%d", rs);
	} else
		rs = (h > hv)? 1: 0;

	if (buflen > 27) {
		GET16(pw, &buf[26]);
		trace_ds(&h3270,",pw=%d", pw);
	} else
		pw = *char_width;

	if (buflen > 29) {
		GET16(ph, &buf[28]);
		trace_ds(&h3270,",ph=%d", ph);
	} else
		ph = *char_height;
	trace_ds(&h3270,")\n");

	cursor_move(&h3270,0);
	h3270.buffer_addr = 0;

	return PDS_OKAY_NO_OUTPUT;
}

static enum pds
sf_outbound_ds(unsigned char buf[], int buflen)
{
	enum pds rv;

	if (buflen < 5) {
		trace_ds(&h3270," error: field length %d too short\n", buflen);
		return PDS_BAD_CMD;
	}

	trace_ds(&h3270,"(0x%02x)", buf[3]);
	if (buf[3] != 0x00) {
		trace_ds(&h3270," error: illegal partition 0x%0x\n", buf[3]);
		return PDS_BAD_CMD;
	}

	switch (buf[4]) {
	    case SNA_CMD_W:
		trace_ds(&h3270," Write");
		if (buflen > 5) {
			if ((rv = ctlr_write(&h3270,&buf[4], buflen-4, False)) < 0)
				return rv;
		} else
			trace_ds(&h3270,"\n");
		break;
	    case SNA_CMD_EW:
		trace_ds(&h3270," EraseWrite");
		ctlr_erase(&h3270,h3270.screen_alt);
		if (buflen > 5) {
			if ((rv = ctlr_write(&h3270,&buf[4], buflen-4, True)) < 0)
				return rv;
		} else
			trace_ds(&h3270,"\n");
		break;
	    case SNA_CMD_EWA:
		trace_ds(&h3270," EraseWriteAlternate");
		ctlr_erase(&h3270,h3270.screen_alt);
		if (buflen > 5) {
			if ((rv = ctlr_write(&h3270,&buf[4], buflen-4, True)) < 0)
				return rv;
		} else
			trace_ds(&h3270,"\n");
		break;
	    case SNA_CMD_EAU:
		trace_ds(&h3270," EraseAllUnprotected\n");
		ctlr_erase_all_unprotected(&h3270);
		break;
	    default:
		trace_ds(&h3270," unknown type 0x%02x\n", buf[4]);
		return PDS_BAD_CMD;
	}
	return PDS_OKAY_NO_OUTPUT;
}

static void
query_reply_start(void)
{
	h3270.obptr = h3270.obuf;
	space3270out(&h3270,1);
	*h3270.obptr++ = AID_SF;
	qr_in_progress = True;
}

static void
do_query_reply(unsigned char code)
{
	int i;
	unsigned subindex = 0;
	Boolean more = False;

	/* Find the right entry in the reply table. */
	for (i = 0; i < NSR_ALL; i++) {
		if (replies[i].code == code)
			break;
	}
	if (i >= NSR_ALL ||
	    (replies[i].single_fn == NULL && replies[i].multi_fn == NULL))
		return;

	if (qr_in_progress) {
		trace_ds(&h3270,"> StructuredField\n");
		qr_in_progress = False;
	}

	do {
		int obptr0 = h3270.obptr - h3270.obuf;
		Boolean full = True;

		space3270out(&h3270,4);
		h3270.obptr += 2;	/* skip length for now */
		*h3270.obptr++ = SFID_QREPLY;
		*h3270.obptr++ = code;

		more = False;
		if (replies[i].single_fn)
			replies[i].single_fn();
		else
			full = replies[i].multi_fn(&subindex, &more);

		if (full) {
			int len;
			unsigned char *obptr_len;

			/* Fill in the length. */
			obptr_len = h3270.obuf + obptr0;
			len = (h3270.obptr - h3270.obuf) - obptr0;
			SET16(obptr_len, len);
		} else {
			/* Back over the header. */
			h3270.obptr -= 4;
		}
	} while (more);
}

static void
do_qr_null(void)
{
	trace_ds(&h3270,"> QueryReply(Null)\n");
}

static void
do_qr_summary(void)
{
	int i;
	const char *comma = "";

	trace_ds(&h3270,"> QueryReply(Summary(");
	space3270out(&h3270,NSR);
	for (i = 0; i < NSR; i++) {
#if defined(X3270_DBCS) /*[*/
		if (dbcs || replies[i].code != QR_DBCS_ASIA) {
#endif /*]*/
			trace_ds(&h3270,"%s%s", comma, see_qcode(replies[i].code));
			comma = ",";
			*h3270.obptr++ = replies[i].code;
#if defined(X3270_DBCS) /*[*/
		}
#endif /*]*/
	}
	trace_ds(&h3270,"))\n");
}

static void
do_qr_usable_area(void)
{
	unsigned short num, denom;

	trace_ds(&h3270,"> QueryReply(UsableArea)\n");
	space3270out(&h3270,19);
	*h3270.obptr++ = 0x01;				/* 12/14-bit addressing */
	*h3270.obptr++ = 0x00;				/* no special character features */
	SET16(h3270.obptr, h3270.maxCOLS);	/* usable width */
	SET16(h3270.obptr, h3270.maxROWS);	/* usable height */
	*h3270.obptr++ = 0x01;				/* units (mm) */
	num = display_widthMM();
	denom = display_width();
	while (!(num %2) && !(denom % 2)) {
		num /= 2;
		denom /= 2;
	}
	SET16(h3270.obptr, (int)num);	/* Xr numerator */
	SET16(h3270.obptr, (int)denom); /* Xr denominator */
	num = display_heightMM();
	denom = display_height();
	while (!(num %2) && !(denom % 2)) {
		num /= 2;
		denom /= 2;
	}
	SET16(h3270.obptr, (int)num);	/* Yr numerator */
	SET16(h3270.obptr, (int)denom); /* Yr denominator */
	*h3270.obptr++ = *char_width;	/* AW */
	*h3270.obptr++ = *char_height;/* AH */
	SET16(h3270.obptr, h3270.maxCOLS * h3270.maxROWS);	/* buffer, questionable */
}

static void
do_qr_color(void)
{
	int i;
	int color_max;

	trace_ds(&h3270,"> QueryReply(Color)\n");

	color_max = h3270.color8 ? 8: 16; /* report on 8 or 16 colors */

	space3270out(&h3270,4 + 2*15);
	*h3270.obptr++ = 0x00;					/* no options */
	*h3270.obptr++ = color_max; 			/* report on 8 or 16 colors */
	*h3270.obptr++ = 0x00;					/* default color: */
	*h3270.obptr++ = 0xf0 + COLOR_GREEN;	/*  green */
	for (i = 0xf1; i < 0xf1 + color_max - 1; i++) {
		*h3270.obptr++ = i;
		if (h3270.m3279)
			*h3270.obptr++ = i;
		else
			*h3270.obptr++ = 0x00;
	}

/*
#if !defined(X3270_DISPLAY)
	// Add background color.
	if (h3270.m3279) {
		space3270out(4);
		*obptr++ = 4;		// length
		*obptr++ = 0x02;	// background color
		*obptr++ = 0x00;	// attribute
		*obptr++ = 0xf0;	// default color
	}
#endif
*/
}

static void
do_qr_highlighting(void)
{
	trace_ds(&h3270,"> QueryReply(Highlighting)\n");
	space3270out(&h3270,11);
	*h3270.obptr++ = 5;					/* report on 5 pairs */
	*h3270.obptr++ = XAH_DEFAULT;		/* default: */
	*h3270.obptr++ = XAH_NORMAL;		/*  normal */
	*h3270.obptr++ = XAH_BLINK;			/* blink: */
	*h3270.obptr++ = XAH_BLINK;			/*  blink */
	*h3270.obptr++ = XAH_REVERSE;		/* reverse: */
	*h3270.obptr++ = XAH_REVERSE;		/*  reverse */
	*h3270.obptr++ = XAH_UNDERSCORE;	/* underscore: */
	*h3270.obptr++ = XAH_UNDERSCORE;	/*  underscore */
	*h3270.obptr++ = XAH_INTENSIFY;		/* intensify: */
	*h3270.obptr++ = XAH_INTENSIFY;		/*  intensify */
}

static void
do_qr_reply_modes(void)
{
	trace_ds(&h3270,"> QueryReply(ReplyModes)\n");
	space3270out(&h3270,3);
	*h3270.obptr++ = SF_SRM_FIELD;
	*h3270.obptr++ = SF_SRM_XFIELD;
	*h3270.obptr++ = SF_SRM_CHAR;
}

#if defined(X3270_DBCS) /*[*/
static void
do_qr_dbcs_asia(void)
{
	/* XXX: Should we support this, even when not in DBCS mode? */
	trace_ds(&h3270,"> QueryReply(DbcsAsia)\n");
	space3270out(7);
	*obptr++ = 0x00;	/* flags (none) */
	*obptr++ = 0x03;	/* field length 3 */
	*obptr++ = 0x01;	/* SI/SO supported */
	*obptr++ = 0x80;	/* character set ID 0x80 */
	*obptr++ = 0x03;	/* field length 3 */
	*obptr++ = 0x02;	/* input control */
	*obptr++ = 0x01;	/* creation supported */
}
#endif /*]*/

static void
do_qr_alpha_part(void)
{
	trace_ds(&h3270,"> QueryReply(AlphanumericPartitions)\n");
	space3270out(&h3270,4);
	*h3270.obptr++ = 0;		/* 1 partition */
	SET16(h3270.obptr, h3270.maxROWS * h3270.maxCOLS);	/* buffer space */
	*h3270.obptr++ = 0;		/* no special features */
}

static void
do_qr_charsets(void)
{
	trace_ds(&h3270,"> QueryReply(CharacterSets)\n");
	space3270out(&h3270,64);
#if defined(X3270_DBCS) /*[*/
	if (dbcs)
		*h3270.obptr++ = 0x8e;			/* flags: GE, CGCSGID, DBCS */
	else
#endif /*]*/
		*h3270.obptr++ = 0x82;			/* flags: GE, CGCSGID present */

	*h3270.obptr++ = 0x00;				/* more flags */
	*h3270.obptr++ = *char_width;		/* SDW */
	*h3270.obptr++ = *char_height;		/* SDW */
	*h3270.obptr++ = 0x00;				/* no load PS */
	*h3270.obptr++ = 0x00;
	*h3270.obptr++ = 0x00;
	*h3270.obptr++ = 0x00;
#if defined(X3270_DBCS) /*[*/
	if (dbcs)
		*h3270.obptr++ = 0x0b;	/* DL (11 bytes) */
	else
#endif /*]*/
		*h3270.obptr++ = 0x07;	/* DL (7 bytes) */

	*h3270.obptr++ = 0x00;		/* SET 0: */
#if defined(X3270_DBCS) /*[*/
	if (dbcs)
		*h3270.obptr++ = 0x00;	/*  FLAGS: non-load, single-
					    plane, single-bute */
	else
#endif /*]*/
		*h3270.obptr++ = 0x10;	/*  FLAGS: non-loadable,
					    single-plane, single-byte,
					    no compare */
	*h3270.obptr++ = 0x00;		/*  LCID 0 */
#if defined(X3270_DBCS) /*[*/
	if (dbcs) {
		*obptr++ = 0x00;	/*  SW 0 */
		*obptr++ = 0x00;	/*  SH 0 */
		*obptr++ = 0x00;	/*  SUBSN */
		*obptr++ = 0x00;	/*  SUBSN */
	}
#endif /*]*/
	SET32(h3270.obptr, cgcsgid);		/*  CGCSGID */
	if (!*standard_font) {
		/* special 3270 font, includes APL */
		*h3270.obptr++ = 0x01;/* SET 1: */
		if (h3270.apl_mode)
		    *h3270.obptr++ = 0x00;/*  FLAGS: non-loadable, single-plane,
					 single-byte, no compare */
		else
		    *h3270.obptr++ = 0x10;/*  FLAGS: non-loadable, single-plane,
					 single-byte, no compare */
		*h3270.obptr++ = 0xf1;/*  LCID */
#if defined(X3270_DBCS) /*[*/
		if (dbcs) {
			*obptr++ = 0x00;/*  SW 0 */
			*obptr++ = 0x00;/*  SH 0 */
			*obptr++ = 0x00;/*  SUBSN */
			*obptr++ = 0x00;/*  SUBSN */
		}
#endif /*]*/
		*h3270.obptr++ = 0x03;/*  CGCSGID: 3179-style APL2 */
		*h3270.obptr++ = 0xc3;
		*h3270.obptr++ = 0x01;
		*h3270.obptr++ = 0x36;
	}
#if defined(X3270_DBCS) /*[*/
	if (dbcs) {
		*obptr++ = 0x80;	/* SET 0x80: */
		*obptr++ = 0x20;	/*  FLAGS: DBCS */
		*obptr++ = 0xf8;	/*  LCID: 0xf8 */
		*obptr++ = *char_width * 2; /* SW */
		*obptr++ = *char_height; /* SH */
		*obptr++ = 0x41;	/*  SUBSN */
		*obptr++ = 0x7f;	/*  SUBSN */
		SET32(obptr, cgcsgid_dbcs); /* CGCSGID */
	}
#endif /*]*/
}

#if defined(X3270_FT) /*[*/
static void
do_qr_ddm(void)
{
	set_dft_buffersize(&h3270);

	trace_ds(&h3270,"> QueryReply(DistributedDataManagement)\n");
	space3270out(&h3270,8);
	SET16(h3270.obptr,0);						/* set reserved field to 0 */
	SET16(h3270.obptr, h3270.dft_buffersize);	/* set inbound length limit INLIM */
	SET16(h3270.obptr, h3270.dft_buffersize);	/* set outbound length limit OUTLIM */
	SET16(h3270.obptr, 0x0101);					/* NSS=01, DDMSS=01 */
}
#endif /*]*/

static void
do_qr_imp_part(void)
{
	trace_ds(&h3270,"> QueryReply(ImplicitPartition)\n");
	space3270out(&h3270,13);
	*h3270.obptr++ = 0x0;				/* reserved */
	*h3270.obptr++ = 0x0;
	*h3270.obptr++ = 0x0b;				/* length of display size */
	*h3270.obptr++ = 0x01;				/* "implicit partition size" */
	*h3270.obptr++ = 0x00;				/* reserved */
	SET16(h3270.obptr, 80);				/* implicit partition width */
	SET16(h3270.obptr, 24);				/* implicit partition height */
	SET16(h3270.obptr, h3270.maxCOLS);	/* alternate height */
	SET16(h3270.obptr, h3270.maxROWS);	/* alternate width */
}

static void
query_reply_end(void)
{
	net_output(&h3270);
	kybd_inhibit(&h3270,True);
}
