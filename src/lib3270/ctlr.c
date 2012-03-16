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
 * Este programa está nomeado como ctlr.c e possui - linhas de código.
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
 *	ctlr.c
 *		This module handles interpretation of the 3270 data stream and
 *		maintenance of the 3270 device state.  It was split out from
 *		screen.c, which handles X operations.
 *
 */

#include "globals.h"
#include <errno.h>
#include "3270ds.h"
#include "appres.h"
#include "ctlr.h"
#include "screen.h"
#include "resources.h"

#include "ctlrc.h"
#include "ftc.h"
#include "ft_cutc.h"
#include "ft_dftc.h"
#include "hostc.h"
#include "kybdc.h"
// #include "macrosc.h"
#include "popupsc.h"
#include "screenc.h"
#include "scrollc.h"
#include "seec.h"
// #include "selectc.h"
#include "sfc.h"
#include "statusc.h"
#include "tablesc.h"
#include "telnetc.h"
#include "trace_dsc.h"
#include "utilc.h"
#include "widec.h"
#include "screenc.h"

/* Externals: kybd.c */
extern unsigned char aid;

/* Globals */
// int				ROWS, COLS;
//int				maxROWS		= 0;
//int				maxCOLS		= 0;
// int				cursor_addr;

int				buffer_addr;
// Boolean         screen_alt = False;	/* alternate screen? */
// Boolean         is_altbuffer = False;

// struct ea      *ea_buf	= NULL;		/* 3270 device buffer */
//										/* ea_buf[-1] is the dummy default field attribute */

// Boolean         formatted = False;	/* set in screen_disp */
unsigned char	reply_mode = SF_SRM_FIELD;
int				crm_nattr = 0;
unsigned char	crm_attr[16];
Boolean			dbcs = False;

/* Statics */
// static struct ea *aea_buf;	/* alternate 3270 extended attribute buffer */
static unsigned char *zero_buf;	// empty buffer, for area clears
static void set_formatted(H3270 *session);
static void ctlr_blanks(void);
static Boolean  trace_primed = False;
static unsigned char default_fg;
static unsigned char default_bg;
static unsigned char default_gr;
static unsigned char default_cs;
static unsigned char default_ic;
static void	ctlr_half_connect(H3270 *session, int ignored, void *dunno);
static void	ctlr_connect(H3270 *session, int ignored, void *dunno);
static int	sscp_start;
static void ticking_stop(H3270 *session);
static void ctlr_add_ic(int baddr, unsigned char ic);
static void changed(H3270 *session, int bstart, int bend);

/*
 * code_table is used to translate buffer addresses and attributes to the 3270
 * datastream representation
 */
static unsigned char	code_table[64] = {
	0x40, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
	0xC8, 0xC9, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
	0x50, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
	0xD8, 0xD9, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
	0xE8, 0xE9, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
	0xF8, 0xF9, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
};

#define IsBlank(c)	((c == EBC_null) || (c == EBC_space))


#define ALL_CHANGED	if(IN_ANSI) changed(&h3270,0,h3270.rows*h3270.cols);
#define REGION_CHANGED(f, l) if(IN_ANSI) changed(&h3270,f,l)
#define ONE_CHANGED(n)	if(IN_ANSI) changed(&h3270,n,n+1);

#define DECODE_BADDR(c1, c2) \
	((((c1) & 0xC0) == 0x00) ? \
	(((c1) & 0x3F) << 8) | (c2) : \
	(((c1) & 0x3F) << 6) | ((c2) & 0x3F))

#define ENCODE_BADDR(ptr, addr) { \
	if ((addr) > 0xfff) { \
		*(ptr)++ = ((addr) >> 8) & 0x3F; \
		*(ptr)++ = (addr) & 0xFF; \
	} else { \
		*(ptr)++ = code_table[((addr) >> 6) & 0x3F]; \
		*(ptr)++ = code_table[(addr) & 0x3F]; \
	} \
    }


/*
 * Initialize the emulated 3270 hardware.
 */
void ctlr_init(H3270 *session, unsigned cmask unused)
{
	/* Register callback routines. */
	lib3270_register_schange(session,ST_HALF_CONNECT, ctlr_half_connect, 0);
	lib3270_register_schange(session,ST_CONNECT, ctlr_connect, 0);
	lib3270_register_schange(session,ST_3270_MODE, ctlr_connect, 0);
}
/*
 * Reinitialize the emulated 3270 hardware.
 */
void ctlr_reinit(H3270 *session, unsigned cmask)
{
//	static struct ea *real_ea_buf = NULL;
//	static struct ea *real_aea_buf = NULL;

	if (cmask & MODEL_CHANGE)
	{
		/* Allocate buffers */
		struct ea *tmp;
		size_t sz = (session->maxROWS * session->maxCOLS);


		session->buffer[0] = tmp = lib3270_calloc(sizeof(struct ea),sz+1, session->buffer[0]);
		session->ea_buf = tmp + 1;

		session->buffer[1] = tmp = lib3270_calloc(sizeof(struct ea),sz+1,session->buffer[1]);
		session->aea_buf = tmp + 1;

		session->text = lib3270_calloc(sizeof(struct lib3270_text),sz,session->text);

		Replace(zero_buf, (unsigned char *)Calloc(sizeof(struct ea),sz));

		session->cursor_addr	= 0;
		buffer_addr 		 	= 0;
	}
}

/**
 * Get current 3270 model.
 *
 * @param session selected 3270 session.
 * @return Current model number.
 */
int lib3270_get_model(H3270 *session)
{
	return session->model_num;
}

/**
 * Deal with the relationships between model numbers and rows/cols.
 *
 * @param model	New model (updates model name)
 */
int	lib3270_set_model(H3270 *session, int model)
{
	if(CONNECTED)
		return EBUSY;

	ctlr_set_rows_cols(session,model,session->ov_cols,session->ov_rows);
	ctlr_reinit(session,MODEL_CHANGE);

	return 0;
}

void ctlr_set_rows_cols(H3270 *session, int mn, int ovc, int ovr)
{
	static const struct _sz
	{
		unsigned char cols;
		unsigned char rows;
	} sz[] =
	{
		{  80, 24 },	// 2
		{  80, 32 },	// 3
		{  80, 43 },	// 4
		{ 132, 27 }		// 5
	};

	int idx = mn -2;

	if(idx < 0 || idx >= (sizeof(sz)/sizeof(struct _sz)))
	{
		idx = 2;
		popup_an_error(NULL,"Unknown model: %d - Defaulting to 4 (%dx%d)", mn, sz[idx].cols,sz[idx].rows);
		mn  = 4;
	}

	update_model_info(session,mn,sz[idx].cols,sz[idx].rows);

	// Apply oversize.
	session->ov_cols = 0;
	session->ov_rows = 0;
	if (ovc != 0 || ovr != 0)
	{
		if (ovc <= 0 || ovr <= 0)
			popup_an_error(NULL,"Invalid %s %dx%d:\nNegative or zero",ResOversize, ovc, ovr);
		else if (ovc * ovr >= 0x4000)
			popup_an_error(NULL,"Invalid %s %dx%d:\nToo big",ResOversize, ovc, ovr);
		else if (ovc > 0 && ovc < session->maxCOLS)
			popup_an_error(NULL,"Invalid %s cols (%d):\nLess than model %d cols (%d)",ResOversize, ovc, session->model_num, session->maxCOLS);
		else if (ovr > 0 && ovr < session->maxROWS)
			popup_an_error(NULL,"Invalid %s rows (%d):\nLess than model %d rows (%d)",ResOversize, ovr, session->model_num, session->maxROWS);
		else
			update_model_info(session,mn,session->ov_cols = ovc,session->ov_rows = ovr);
	}

	set_viewsize(session,sz[idx].rows,sz[idx].cols);

	/*
	// Make sure that the current rows/cols are still 24x80.
	session->cols = 80;
	session->rows = 24;
	session->screen_alt = False;
	*/

}


/*
 * Set the formatted screen flag.  A formatted screen is a screen that
 * has at least one field somewhere on it.
 */
static void set_formatted(H3270 *session)
{
	register int baddr;

	CHECK_SESSION_HANDLE(session);

	session->formatted = False;
	baddr = 0;
	do
	{
		if(session->ea_buf[baddr].fa)
		{
			session->formatted = True;
			break;
		}
		INC_BA(baddr);
	} while (baddr != 0);
}

/*
 * Called when a host is half connected.
 */
static void ctlr_half_connect(H3270 *session, int ignored unused, void *dunno)
{
	ticking_start(session,True);
}


/*
 * Called when a host connects, disconnects, or changes ANSI/3270 modes.
 */
static void ctlr_connect(H3270 *session, int ignored unused, void *dunno)
{
	ticking_stop(session);
	status_untiming(session);

	if (session->ever_3270)
		session->ea_buf[-1].fa = FA_PRINTABLE | FA_MODIFY;
	else
		session->ea_buf[-1].fa = FA_PRINTABLE | FA_PROTECT;
	if (!IN_3270 || (IN_SSCP && (kybdlock & KL_OIA_TWAIT)))
	{
		kybdlock_clr(KL_OIA_TWAIT, "ctlr_connect");
		status_reset(session);
	}

	default_fg = 0x00;
	default_bg = 0x00;
	default_gr = 0x00;
	default_cs = 0x00;
	default_ic = 0x00;
	reply_mode = SF_SRM_FIELD;
	crm_nattr = 0;
}



LIB3270_EXPORT int lib3270_field_addr(H3270 *h, int baddr)
{
	int sbaddr;

	CHECK_SESSION_HANDLE(h);

	if (!h->formatted)
		return -1;

	sbaddr = baddr;
	do
	{
		if(h->ea_buf[baddr].fa)
			return baddr;
		DEC_BA(baddr);
	} while (baddr != sbaddr);

	return -1;
}

/*
 * Get Field width
 */
int lib3270_field_length(H3270 *h, int baddr)
{
	int saddr;
	int addr;
	int width = 0;

	CHECK_SESSION_HANDLE(h);

	addr = find_field_attribute(h,baddr);

	if(addr < 0)
		return -1;

	saddr = addr;
	INC_BA(addr);
	do {
		if(h->ea_buf[addr].fa)
			return width;
		INC_BA(addr);
		width++;
	} while (addr != saddr);

	return -1;

}

/*
 * Find the field attribute for the given buffer address.  Return its address
 * rather than its value.
 */
unsigned char get_field_attribute(H3270 *h, int baddr)
{
	CHECK_SESSION_HANDLE(h);
	return h->ea_buf[find_field_attribute(h,baddr)].fa;
}

/*
 * Find the field attribute for the given buffer address, bounded by another
 * buffer address.  Return the attribute in a parameter.
 *
 * Returns True if an attribute is found, False if boundary hit.
 */
Boolean
get_bounded_field_attribute(register int baddr, register int bound,
    unsigned char *fa_out)
{
	int	sbaddr;

	if (!h3270.formatted) {
		*fa_out = h3270.ea_buf[-1].fa;
		return True;
	}

	sbaddr = baddr;
	do {
		if (h3270.ea_buf[baddr].fa) {
			*fa_out = h3270.ea_buf[baddr].fa;
			return True;
		}
		DEC_BA(baddr);
	} while (baddr != sbaddr && baddr != bound);

	/* Screen is unformatted (and 'formatted' is inaccurate). */
	if (baddr == sbaddr) {
		*fa_out = h3270.ea_buf[-1].fa;
		return True;
	}

	/* Wrapped to boundary. */
	return False;
}

/*
 * Given the address of a field attribute, return the address of the
 * extended attribute structure.
 */
struct ea *
fa2ea(int baddr)
{
	return &h3270.ea_buf[baddr];
}

/*
 * Find the next unprotected field.  Returns the address following the
 * unprotected attribute byte, or 0 if no nonzero-width unprotected field
 * can be found.
 */
int
next_unprotected(int baddr0)
{
	register int baddr, nbaddr;

	nbaddr = baddr0;
	do {
		baddr = nbaddr;
		INC_BA(nbaddr);
		if (h3270.ea_buf[baddr].fa &&
		    !FA_IS_PROTECTED(h3270.ea_buf[baddr].fa) &&
		    !h3270.ea_buf[nbaddr].fa)
			return nbaddr;
	} while (nbaddr != baddr0);
	return 0;
}

/*
 * Perform an erase command, which may include changing the (virtual) screen
 * size.
 */
void ctlr_erase(H3270 *session, int alt)
{
	CHECK_SESSION_HANDLE(session);

	kybd_inhibit(False);

	ctlr_clear(session,True);
	screen_erase(session);

	if(alt == session->screen_alt)
		return;

	if (alt)
	{
		/* Going from 24x80 to maximum. */
		screen_disp(session);
		set_viewsize(session,session->maxROWS,session->maxCOLS);
	}
	else
	{
		/* Going from maximum to 24x80. */
		if (session->maxROWS > 24 || session->maxCOLS > 80)
		{
			if (visible_control)
			{
				ctlr_blanks();
				screen_disp(session);
			}
			set_viewsize(session,24,80);
		}
	}

	session->screen_alt = alt;
}


/*
 * Interpret an incoming 3270 command.
 */
enum pds
process_ds(unsigned char *buf, int buflen)
{
	enum pds rv;

	if (!buflen)
		return PDS_OKAY_NO_OUTPUT;

	scroll_to_bottom();

	trace_ds("< ");

	switch (buf[0]) {	/* 3270 command */
	case CMD_EAU:	/* erase all unprotected */
	case SNA_CMD_EAU:
		trace_ds("EraseAllUnprotected\n");
		ctlr_erase_all_unprotected();
		return PDS_OKAY_NO_OUTPUT;
		break;
	case CMD_EWA:	/* erase/write alternate */
	case SNA_CMD_EWA:
		trace_ds("EraseWriteAlternate");
		ctlr_erase(NULL,True);
		if ((rv = ctlr_write(buf, buflen, True)) < 0)
			return rv;
		return PDS_OKAY_NO_OUTPUT;
		break;
	case CMD_EW:	/* erase/write */
	case SNA_CMD_EW:
		trace_ds("EraseWrite");
		ctlr_erase(NULL,False);
		if ((rv = ctlr_write(buf, buflen, True)) < 0)
			return rv;
		return PDS_OKAY_NO_OUTPUT;
		break;
	case CMD_W:	/* write */
	case SNA_CMD_W:
		trace_ds("Write");
		if ((rv = ctlr_write(buf, buflen, False)) < 0)
			return rv;
		return PDS_OKAY_NO_OUTPUT;
		break;
	case CMD_RB:	/* read buffer */
	case SNA_CMD_RB:
		trace_ds("ReadBuffer\n");
		ctlr_read_buffer(aid);
		return PDS_OKAY_OUTPUT;
		break;
	case CMD_RM:	/* read modifed */
	case SNA_CMD_RM:
		trace_ds("ReadModified\n");
		ctlr_read_modified(aid, False);
		return PDS_OKAY_OUTPUT;
		break;
	case CMD_RMA:	/* read modifed all */
	case SNA_CMD_RMA:
		trace_ds("ReadModifiedAll\n");
		ctlr_read_modified(aid, True);
		return PDS_OKAY_OUTPUT;
		break;
	case CMD_WSF:	/* write structured field */
	case SNA_CMD_WSF:
		trace_ds("WriteStructuredField");
		return write_structured_field(buf, buflen);
		break;
	case CMD_NOP:	/* no-op */
		trace_ds("NoOp\n");
		return PDS_OKAY_NO_OUTPUT;
		break;
	default:
		/* unknown 3270 command */
		popup_an_error(NULL,"Unknown 3270 Data Stream command: 0x%X\n",buf[0]);
		return PDS_BAD_CMD;
	}
}

/*
 * Functions to insert SA attributes into the inbound data stream.
 */
static void
insert_sa1(unsigned char attr, unsigned char value, unsigned char *currentp, Boolean *anyp)
{
	if (value == *currentp)
		return;
	*currentp = value;
	space3270out(3);
	*obptr++ = ORDER_SA;
	*obptr++ = attr;
	*obptr++ = value;
	if (*anyp)
		trace_ds("'");
	trace_ds(" SetAttribute(%s)", see_efa(attr, value));
	*anyp = False;
}

/*
 * Translate an internal character set number to a 3270DS characte set number.
 */
static unsigned char
host_cs(unsigned char cs)
{
	switch (cs & CS_MASK) {
	case CS_APL:
	case CS_LINEDRAW:
	    return 0xf0 | (cs & CS_MASK);
	case CS_DBCS:
	    return 0xf8;
	default:
	    return 0;
	}
}

static void
insert_sa(int baddr, unsigned char *current_fgp, unsigned char *current_bgp,
	unsigned char *current_grp, unsigned char *current_csp, Boolean *anyp)
{
	if (reply_mode != SF_SRM_CHAR)
		return;

	if (memchr((char *)crm_attr, XA_FOREGROUND, crm_nattr))
		insert_sa1(XA_FOREGROUND, h3270.ea_buf[baddr].fg, current_fgp, anyp);
	if (memchr((char *)crm_attr, XA_BACKGROUND, crm_nattr))
		insert_sa1(XA_BACKGROUND, h3270.ea_buf[baddr].bg, current_bgp, anyp);
	if (memchr((char *)crm_attr, XA_HIGHLIGHTING, crm_nattr)) {
		unsigned char gr;

		gr = h3270.ea_buf[baddr].gr;
		if (gr)
			gr |= 0xf0;
		insert_sa1(XA_HIGHLIGHTING, gr, current_grp, anyp);
	}
	if (memchr((char *)crm_attr, XA_CHARSET, crm_nattr)) {
		insert_sa1(XA_CHARSET, host_cs(h3270.ea_buf[baddr].cs), current_csp,anyp);
	}
}


/*
 * Process a 3270 Read-Modified command and transmit the data back to the
 * host.
 */
void
ctlr_read_modified(unsigned char aid_byte, Boolean all)
{
	register int	baddr, sbaddr;
	Boolean		send_data = True;
	Boolean		short_read = False;
	unsigned char	current_fg = 0x00;
	unsigned char	current_bg = 0x00;
	unsigned char	current_gr = 0x00;
	unsigned char	current_cs = 0x00;

	if (IN_SSCP && aid_byte != AID_ENTER)
		return;

#if defined(X3270_FT) /*[*/
	if (aid_byte == AID_SF) {
		dft_read_modified();
		return;
	}
#endif /*]*/

	trace_ds("> ");
	obptr = obuf;

	switch (aid_byte) {

	    case AID_SYSREQ:			/* test request */
			space3270out(4);
			*obptr++ = 0x01;	/* soh */
			*obptr++ = 0x5b;	/*  %  */
			*obptr++ = 0x61;	/*  /  */
			*obptr++ = 0x02;	/* stx */
			trace_ds("SYSREQ");
		break;

	    case AID_PA1:			/* short-read AIDs */
	    case AID_PA2:
	    case AID_PA3:
	    case AID_CLEAR:
			if (!all)
				short_read = True;
			/* fall through... */

	    case AID_SELECT:			/* No data on READ MODIFIED */
			if (!all)
				send_data = False;
			/* fall through... */

	    default:				/* ordinary AID */
		if (!IN_SSCP) {
			space3270out(3);
			*obptr++ = aid_byte;
			trace_ds("%s",see_aid(aid_byte));
			if (short_read)
			    goto rm_done;
			ENCODE_BADDR(obptr, h3270.cursor_addr);
			trace_ds("%s",rcba(h3270.cursor_addr));
		} else {
			space3270out(1);	/* just in case */
		}
		break;
	}

	baddr = 0;
	if (h3270.formatted) {
		/* find first field attribute */
		do {
			if (h3270.ea_buf[baddr].fa)
				break;
			INC_BA(baddr);
		} while (baddr != 0);
		sbaddr = baddr;
		do {
			if (FA_IS_MODIFIED(h3270.ea_buf[baddr].fa)) {
				Boolean	any = False;

				INC_BA(baddr);
				space3270out(3);
				*obptr++ = ORDER_SBA;
				ENCODE_BADDR(obptr, baddr);
				trace_ds(" SetBufferAddress%s (Cols: %d Rows: %d)", rcba(baddr), h3270.cols, h3270.rows);
				while (!h3270.ea_buf[baddr].fa) {

					if (send_data &&
					    h3270.ea_buf[baddr].cc) {
						insert_sa(baddr,
						    &current_fg,
						    &current_bg,
						    &current_gr,
						    &current_cs,
						    &any);
						if (h3270.ea_buf[baddr].cs & CS_GE) {
							space3270out(1);
							*obptr++ = ORDER_GE;
							if (any)
								trace_ds("'");
							trace_ds(" GraphicEscape");
							any = False;
						}
						space3270out(1);
						*obptr++ = h3270.ea_buf[baddr].cc;
						if (!any)
							trace_ds(" '");
						trace_ds("%s",
						    see_ebc(ea_buf[baddr].cc));
						any = True;
					}
					INC_BA(baddr);
				}
				if (any)
					trace_ds("'");
			}
			else {	/* not modified - skip */
				do {
					INC_BA(baddr);
				} while (!h3270.ea_buf[baddr].fa);
			}
		} while (baddr != sbaddr);
	} else {
		Boolean	any = False;
		int nbytes = 0;

		/*
		 * If we're in SSCP-LU mode, the starting point is where the
		 * host left the cursor.
		 */
		if (IN_SSCP)
			baddr = sscp_start;

		do {
			if (h3270.ea_buf[baddr].cc) {
				insert_sa(baddr,
				    &current_fg,
				    &current_bg,
				    &current_gr,
				    &current_cs,
				    &any);
				if (h3270.ea_buf[baddr].cs & CS_GE) {
					space3270out(1);
					*obptr++ = ORDER_GE;
					if (any)
						trace_ds("' ");
					trace_ds(" GraphicEscape ");
					any = False;
				}
				space3270out(1);
				*obptr++ = h3270.ea_buf[baddr].cc;
				if (!any)
					trace_ds("%s","'");
				trace_ds("%s",see_ebc(ea_buf[baddr].cc));
				any = True;
				nbytes++;
			}
			INC_BA(baddr);

			/*
			 * If we're in SSCP-LU mode, end the return value at
			 * 255 bytes, or where the screen wraps.
			 */
			if (IN_SSCP && (nbytes >= 255 || !baddr))
				break;
		} while (baddr != 0);
		if (any)
			trace_ds("'");
	}

    rm_done:
	trace_ds("\n");
	net_output();
}

/*
 * Process a 3270 Read-Buffer command and transmit the data back to the
 * host.
 */
void
ctlr_read_buffer(unsigned char aid_byte)
{
	register int	baddr;
	unsigned char	fa;
	Boolean		any = False;
	int		attr_count = 0;
	unsigned char	current_fg = 0x00;
	unsigned char	current_bg = 0x00;
	unsigned char	current_gr = 0x00;
	unsigned char	current_cs = 0x00;

#if defined(X3270_FT) /*[*/
	if (aid_byte == AID_SF) {
		dft_read_modified();
		return;
	}
#endif /*]*/

	trace_ds("> ");
	obptr = obuf;

	space3270out(3);
	*obptr++ = aid_byte;
	ENCODE_BADDR(obptr, h3270.cursor_addr);
	trace_ds("%s%s", see_aid(aid_byte), rcba(h3270.cursor_addr));

	baddr = 0;
	do {
		if (h3270.ea_buf[baddr].fa) {
			if (reply_mode == SF_SRM_FIELD) {
				space3270out(2);
				*obptr++ = ORDER_SF;
			} else {
				space3270out(4);
				*obptr++ = ORDER_SFE;
				attr_count = obptr - obuf;
				*obptr++ = 1; /* for now */
				*obptr++ = XA_3270;
			}
			fa = h3270.ea_buf[baddr].fa & ~FA_PRINTABLE;
			*obptr++ = code_table[fa];
			if (any)
				trace_ds("'");
			trace_ds(" StartField%s%s%s",
			    (reply_mode == SF_SRM_FIELD) ? "" : "Extended",
			    rcba(baddr), see_attr(fa));
			if (reply_mode != SF_SRM_FIELD) {
				if (h3270.ea_buf[baddr].fg) {
					space3270out(2);
					*obptr++ = XA_FOREGROUND;
					*obptr++ = h3270.ea_buf[baddr].fg;
					trace_ds("%s", see_efa(XA_FOREGROUND,
					    ea_buf[baddr].fg));
					(*(obuf + attr_count))++;
				}
				if (h3270.ea_buf[baddr].bg) {
					space3270out(2);
					*obptr++ = XA_BACKGROUND;
					*obptr++ = h3270.ea_buf[baddr].bg;
					trace_ds("%s", see_efa(XA_BACKGROUND,
					    ea_buf[baddr].bg));
					(*(obuf + attr_count))++;
				}
				if (h3270.ea_buf[baddr].gr) {
					space3270out(2);
					*obptr++ = XA_HIGHLIGHTING;
					*obptr++ = h3270.ea_buf[baddr].gr | 0xf0;
					trace_ds("%s", see_efa(XA_HIGHLIGHTING,
					    h3270.ea_buf[baddr].gr | 0xf0));
					(*(obuf + attr_count))++;
				}
				if (h3270.ea_buf[baddr].cs & CS_MASK) {
					space3270out(2);
					*obptr++ = XA_CHARSET;
					*obptr++ = host_cs(h3270.ea_buf[baddr].cs);
					trace_ds("%s", see_efa(XA_CHARSET,
					    host_cs(ea_buf[baddr].cs)));
					(*(obuf + attr_count))++;
				}
			}
			any = False;
		} else {
			insert_sa(baddr,
			    &current_fg,
			    &current_bg,
			    &current_gr,
			    &current_cs,
			    &any);
			if (h3270.ea_buf[baddr].cs & CS_GE) {
				space3270out(1);
				*obptr++ = ORDER_GE;
				if (any)
					trace_ds("'");
				trace_ds(" GraphicEscape");
				any = False;
			}
			space3270out(1);
			*obptr++ = h3270.ea_buf[baddr].cc;
			if (h3270.ea_buf[baddr].cc <= 0x3f ||
			    h3270.ea_buf[baddr].cc == 0xff) {
				if (any)
					trace_ds("'");

				trace_ds(" %s", see_ebc(ea_buf[baddr].cc));
				any = False;
			} else {
				if (!any)
					trace_ds(" '");
				trace_ds("%s", see_ebc(ea_buf[baddr].cc));
				any = True;
			}
		}
		INC_BA(baddr);
	} while (baddr != 0);
	if (any)
		trace_ds("'");

	trace_ds("\n");
	net_output();
}

#if defined(X3270_TRACE) /*[*/
/*
 * Construct a 3270 command to reproduce the current state of the display.
 */
void
ctlr_snap_buffer(void)
{
	register int	baddr = 0;
	int		attr_count;
	unsigned char	current_fg = 0x00;
	unsigned char	current_bg = 0x00;
	unsigned char	current_gr = 0x00;
	unsigned char	current_cs = 0x00;
	unsigned char   av;

	space3270out(2);
	*obptr++ = h3270.screen_alt ? CMD_EWA : CMD_EW;
	*obptr++ = code_table[0];

	do {
		if (ea_buf[baddr].fa) {
			space3270out(4);
			*obptr++ = ORDER_SFE;
			attr_count = obptr - obuf;
			*obptr++ = 1; /* for now */
			*obptr++ = XA_3270;
			*obptr++ = code_table[ea_buf[baddr].fa & ~FA_PRINTABLE];
			if (ea_buf[baddr].fg) {
				space3270out(2);
				*obptr++ = XA_FOREGROUND;
				*obptr++ = ea_buf[baddr].fg;
				(*(obuf + attr_count))++;
			}
			if (ea_buf[baddr].bg) {
				space3270out(2);
				*obptr++ = XA_BACKGROUND;
				*obptr++ = ea_buf[baddr].fg;
				(*(obuf + attr_count))++;
			}
			if (ea_buf[baddr].gr) {
				space3270out(2);
				*obptr++ = XA_HIGHLIGHTING;
				*obptr++ = ea_buf[baddr].gr | 0xf0;
				(*(obuf + attr_count))++;
			}
			if (ea_buf[baddr].cs & CS_MASK) {
				space3270out(2);
				*obptr++ = XA_CHARSET;
				*obptr++ = host_cs(ea_buf[baddr].cs);
				(*(obuf + attr_count))++;
			}
		} else {
			av = ea_buf[baddr].fg;
			if (current_fg != av) {
				current_fg = av;
				space3270out(3);
				*obptr++ = ORDER_SA;
				*obptr++ = XA_FOREGROUND;
				*obptr++ = av;
			}
			av = ea_buf[baddr].bg;
			if (current_bg != av) {
				current_bg = av;
				space3270out(3);
				*obptr++ = ORDER_SA;
				*obptr++ = XA_BACKGROUND;
				*obptr++ = av;
			}
			av = ea_buf[baddr].gr;
			if (av)
				av |= 0xf0;
			if (current_gr != av) {
				current_gr = av;
				space3270out(3);
				*obptr++ = ORDER_SA;
				*obptr++ = XA_HIGHLIGHTING;
				*obptr++ = av;
			}
			av = ea_buf[baddr].cs & CS_MASK;
			if (av)
				av = host_cs(av);
			if (current_cs != av) {
				current_cs = av;
				space3270out(3);
				*obptr++ = ORDER_SA;
				*obptr++ = XA_CHARSET;
				*obptr++ = av;
			}
			if (ea_buf[baddr].cs & CS_GE) {
				space3270out(1);
				*obptr++ = ORDER_GE;
			}
			space3270out(1);
			*obptr++ = ea_buf[baddr].cc;
		}
		INC_BA(baddr);
	} while (baddr != 0);

	space3270out(4);
	*obptr++ = ORDER_SBA;
	ENCODE_BADDR(obptr, h3270.cursor_addr);
	*obptr++ = ORDER_IC;
}

/*
 * Construct a 3270 command to reproduce the reply mode.
 * Returns a Boolean indicating if one is necessary.
 */
Boolean
ctlr_snap_modes(void)
{
	int i;

	if (!IN_3270 || reply_mode == SF_SRM_FIELD)
		return False;

	space3270out(6 + crm_nattr);
	*obptr++ = CMD_WSF;
	*obptr++ = 0x00;	/* implicit length */
	*obptr++ = 0x00;
	*obptr++ = SF_SET_REPLY_MODE;
	*obptr++ = 0x00;	/* partition 0 */
	*obptr++ = reply_mode;
	if (reply_mode == SF_SRM_CHAR)
		for (i = 0; i < crm_nattr; i++)
			*obptr++ = crm_attr[i];
	return True;
}
#endif /*]*/


/*
 * Process a 3270 Erase All Unprotected command.
 */
void
ctlr_erase_all_unprotected(void)
{
	register int	baddr, sbaddr;
	unsigned char	fa;
	Boolean		f;

	kybd_inhibit(False);

	if (h3270.formatted) {
		/* find first field attribute */
		baddr = 0;
		do {
			if (h3270.ea_buf[baddr].fa)
				break;
			INC_BA(baddr);
		} while (baddr != 0);
		sbaddr = baddr;
		f = False;
		do {
			fa = h3270.ea_buf[baddr].fa;
			if (!FA_IS_PROTECTED(fa)) {
				mdt_clear(baddr);
				do {
					INC_BA(baddr);
					if (!f) {
						cursor_move(baddr);
						f = True;
					}
					if (!h3270.ea_buf[baddr].fa) {
						ctlr_add(baddr, EBC_null, 0);
					}
				} while (!h3270.ea_buf[baddr].fa);
			}
			else {
				do {
					INC_BA(baddr);
				} while (!h3270.ea_buf[baddr].fa);
			}
		} while (baddr != sbaddr);
		if (!f)
			cursor_move(0);
	} else {
		ctlr_clear(&h3270,True);
	}
	aid = AID_NO;
	do_reset(False);
	ALL_CHANGED;
}



/*
 * Process a 3270 Write command.
 */
enum pds
ctlr_write(unsigned char buf[], int buflen, Boolean erase)
{
	register unsigned char	*cp;
	register int	baddr;
	unsigned char	current_fa;
	Boolean		last_cmd;
	Boolean		last_zpt;
	Boolean		wcc_keyboard_restore, wcc_sound_alarm;
	Boolean		ra_ge;
	int		i;
	unsigned char	na;
	int		any_fa;
	unsigned char	efa_fg;
	unsigned char	efa_bg;
	unsigned char	efa_gr;
	unsigned char	efa_cs;
	unsigned char	efa_ic;
	const char	*paren = "(";
	enum { NONE, ORDER, SBA, TEXT, NULLCH } previous = NONE;
	enum pds	rv = PDS_OKAY_NO_OUTPUT;
	int		fa_addr;
	Boolean		add_dbcs;
	unsigned char	add_c1, add_c2 = 0;
	enum dbcs_state	d;
	enum dbcs_why	why = DBCS_FIELD;
	Boolean		aborted = False;
#if defined(X3270_DBCS) /*[*/
	char		mb[16];
#endif /*]*/

#define END_TEXT0	{ if (previous == TEXT) trace_ds("'"); }
#define END_TEXT(cmd)	{ END_TEXT0; trace_ds(" %s", cmd); }

/* XXX: Should there be a ctlr_add_cs call here? */
#define START_FIELD(fa) { \
			current_fa = fa; \
			ctlr_add_fa(buffer_addr, fa, 0); \
			ctlr_add_cs(buffer_addr, 0); \
			ctlr_add_fg(buffer_addr, 0); \
			ctlr_add_bg(buffer_addr, 0); \
			ctlr_add_gr(buffer_addr, 0); \
			ctlr_add_ic(buffer_addr, 0); \
			trace_ds("%s",see_attr(fa)); \
			h3270.formatted = True; \
		}

	kybd_inhibit(False);

	if (buflen < 2)
		return PDS_BAD_CMD;

	default_fg = 0;
	default_bg = 0;
	default_gr = 0;
	default_cs = 0;
	default_ic = 0;
	trace_primed = True;
	buffer_addr = h3270.cursor_addr;
	if (WCC_RESET(buf[1])) {
		if (erase)
			reply_mode = SF_SRM_FIELD;
		trace_ds("%sreset", paren);
		paren = ",";
	}
	wcc_sound_alarm = WCC_SOUND_ALARM(buf[1]);
	if (wcc_sound_alarm) {
		trace_ds("%salarm", paren);
		paren = ",";
	}
	wcc_keyboard_restore = WCC_KEYBOARD_RESTORE(buf[1]);
	if (wcc_keyboard_restore)
		ticking_stop(NULL);
	if (wcc_keyboard_restore) {
		trace_ds("%srestore", paren);
		paren = ",";
	}

	if (WCC_RESET_MDT(buf[1])) {
		trace_ds("%sresetMDT", paren);
		paren = ",";
		baddr = 0;
		if (appres.modified_sel)
			ALL_CHANGED;
		do {
			if (h3270.ea_buf[baddr].fa) {
				mdt_clear(baddr);
			}
			INC_BA(baddr);
		} while (baddr != 0);
	}
	if (strcmp(paren, "("))
		trace_ds(")");

	last_cmd = True;
	last_zpt = False;
	current_fa = get_field_attribute(&h3270,buffer_addr);

#define ABORT_WRITEx { \
	rv = PDS_BAD_ADDR; \
	aborted = True; \
	break; \
}
#define ABORT_WRITE(s) { \
	trace_ds(" [" s "; write aborted]\n"); \
	ABORT_WRITEx; \
} \

	for (cp = &buf[2]; !aborted && cp < (buf + buflen); cp++) {
		switch (*cp) {
		case ORDER_SF:	/* start field */
			END_TEXT("StartField");
			if (previous != SBA)
				trace_ds("%s",rcba(buffer_addr));
			previous = ORDER;
			cp++;		/* skip field attribute */
			START_FIELD(*cp);
			ctlr_add_fg(buffer_addr, 0);
			ctlr_add_bg(buffer_addr, 0);
			INC_BA(buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;
		case ORDER_SBA:	/* set buffer address */
			cp += 2;	/* skip buffer address */
			buffer_addr = DECODE_BADDR(*(cp-1), *cp);
			END_TEXT("SetBufferAddress");
			previous = SBA;
			trace_ds("%s",rcba(buffer_addr));
			if (buffer_addr >= h3270.cols * h3270.rows) {
				ABORT_WRITE("invalid SBA address");
			}
			current_fa = get_field_attribute(&h3270,buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;
		case ORDER_IC:	/* insert cursor */
			END_TEXT("InsertCursor");
			if (previous != SBA)
				trace_ds("%s",rcba(buffer_addr));
			previous = ORDER;
			cursor_move(buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;
		case ORDER_PT:	/* program tab */
			END_TEXT("ProgramTab");
			previous = ORDER;
			/*
			 * If the buffer address is the field attribute of
			 * of an unprotected field, simply advance one
			 * position.
			 */
			if (h3270.ea_buf[buffer_addr].fa &&
			    !FA_IS_PROTECTED(h3270.ea_buf[buffer_addr].fa)) {
				INC_BA(buffer_addr);
				last_zpt = False;
				last_cmd = True;
				break;
			}
			/*
			 * Otherwise, advance to the first position of the
			 * next unprotected field.
			 */
			baddr = next_unprotected(buffer_addr);
			if (baddr < buffer_addr)
				baddr = 0;
			/*
			 * Null out the remainder of the current field -- even
			 * if protected -- if the PT doesn't follow a command
			 * or order, or (honestly) if the last order we saw was
			 * a null-filling PT that left the buffer address at 0.
			 * XXX: There's some funky DBCS rule here.
			 */
			if (!last_cmd || last_zpt) {
				trace_ds("(nulling)");
				while ((buffer_addr != baddr) &&
				       (!h3270.ea_buf[buffer_addr].fa)) {
					ctlr_add(buffer_addr, EBC_null, 0);
					ctlr_add_cs(buffer_addr, 0);
					ctlr_add_fg(buffer_addr, 0);
					ctlr_add_bg(buffer_addr, 0);
					ctlr_add_gr(buffer_addr, 0);
					ctlr_add_ic(buffer_addr, 0);
					INC_BA(buffer_addr);
				}
				if (baddr == 0)
					last_zpt = True;
			} else
				last_zpt = False;
			buffer_addr = baddr;
			last_cmd = True;
			break;
		case ORDER_RA:	/* repeat to address */
			END_TEXT("RepeatToAddress");
			cp += 2;	/* skip buffer address */
			baddr = DECODE_BADDR(*(cp-1), *cp);
			trace_ds("%s",rcba(baddr));
			cp++;		/* skip char to repeat */
			add_dbcs = False;
			ra_ge = False;
			previous = ORDER;
#if defined(X3270_DBCS) /*[*/
			if (dbcs) {
				d = ctlr_lookleft_state(buffer_addr, &why);
				if (d == DBCS_RIGHT) {
					ABORT_WRITE("RA over right half of DBCS character");
				}
				if (default_cs == CS_DBCS || d == DBCS_LEFT) {
					add_dbcs = True;
				}
			}
			if (add_dbcs) {
				if ((baddr - buffer_addr) % 2) {
					ABORT_WRITE("DBCS RA with odd length");
				}
				add_c1 = *cp;
				cp++;
				if (cp >= buf + buflen) {
					ABORT_WRITE("missing second half of DBCS character");
				}
				add_c2 = *cp;
				if (add_c1 == EBC_null) {
					switch (add_c2) {
					case EBC_null:
					case EBC_nl:
					case EBC_em:
					case EBC_ff:
					case EBC_cr:
					case EBC_dup:
					case EBC_fm:
						break;
					default:
						trace_ds(" [invalid DBCS RA control character X'%02x%02x'; write aborted]",
							add_c1, add_c2);
						ABORT_WRITEx;
					}
				} else if (add_c1 < 0x40 || add_c1 > 0xfe ||
					   add_c2 < 0x40 || add_c2 > 0xfe) {
					trace_ds(" [invalid DBCS RA character X'%02x%02x'; write aborted]",
						add_c1, add_c2);
					ABORT_WRITEx;
			       }
			       dbcs_to_mb(add_c1, add_c2, mb);
			       trace_ds_nb("'%s'", mb);
			} else
#endif /*]*/
			{
				if (*cp == ORDER_GE) {
					ra_ge = True;
					trace_ds("GraphicEscape");
					cp++;
				}
				add_c1 = *cp;
				if (add_c1)
					trace_ds("'");
				trace_ds("%s", see_ebc(add_c1));
				if (add_c1)
					trace_ds("'");
			}
			if (baddr >= h3270.cols * h3270.rows) {
				ABORT_WRITE("invalid RA address");
			}
			do {
				if (add_dbcs) {
					ctlr_add(buffer_addr, add_c1,
					    default_cs);
				} else {
					if (ra_ge)
						ctlr_add(buffer_addr, add_c1,
						    CS_GE);
					else if (default_cs)
						ctlr_add(buffer_addr, add_c1,
						    default_cs);
					else
						ctlr_add(buffer_addr, add_c1,
						    0);
				}
				ctlr_add_fg(buffer_addr, default_fg);
				ctlr_add_gr(buffer_addr, default_gr);
				ctlr_add_ic(buffer_addr, default_ic);
				INC_BA(buffer_addr);
				if (add_dbcs) {
					ctlr_add(buffer_addr, add_c2,
					    default_cs);
					ctlr_add_fg(buffer_addr, default_fg);
					ctlr_add_bg(buffer_addr, default_bg);
					ctlr_add_gr(buffer_addr, default_gr);
					ctlr_add_ic(buffer_addr, default_ic);
					INC_BA(buffer_addr);
				}
			} while (buffer_addr != baddr);
			current_fa = get_field_attribute(&h3270,buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;
		case ORDER_EUA:	/* erase unprotected to address */
			cp += 2;	/* skip buffer address */
			baddr = DECODE_BADDR(*(cp-1), *cp);
			END_TEXT("EraseUnprotectedAll");
			if (previous != SBA)
				trace_ds("%s",rcba(baddr));
			previous = ORDER;
			if (baddr >= h3270.cols * h3270.rows) {
				ABORT_WRITE("invalid EUA address");
			}
			d = ctlr_lookleft_state(buffer_addr, &why);
			if (d == DBCS_RIGHT) {
				ABORT_WRITE("EUA overwriting right half of DBCS character");
			}
			d = ctlr_lookleft_state(baddr, &why);
			if (d == DBCS_LEFT) {
				ABORT_WRITE("EUA overwriting left half of DBCS character");
			}
			do {
				if (h3270.ea_buf[buffer_addr].fa)
					current_fa = h3270.ea_buf[buffer_addr].fa;
				else if (!FA_IS_PROTECTED(current_fa)) {
					ctlr_add(buffer_addr, EBC_null,
					    CS_BASE);
				}
				INC_BA(buffer_addr);
			} while (buffer_addr != baddr);
			current_fa = get_field_attribute(&h3270,buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;
		case ORDER_GE:	/* graphic escape */
			/* XXX: DBCS? */
			END_TEXT("GraphicEscape ");
			cp++;		/* skip char */
			previous = ORDER;
			if (*cp)
				trace_ds("'");
			trace_ds("%s", see_ebc(*cp));
			if (*cp)
				trace_ds("'");
			ctlr_add(buffer_addr, *cp, CS_GE);
			ctlr_add_fg(buffer_addr, default_fg);
			ctlr_add_bg(buffer_addr, default_bg);
			ctlr_add_gr(buffer_addr, default_gr);
			ctlr_add_ic(buffer_addr, default_ic);
			INC_BA(buffer_addr);
			current_fa = get_field_attribute(&h3270,buffer_addr);
			last_cmd = False;
			last_zpt = False;
			break;
		case ORDER_MF:	/* modify field */
			END_TEXT("ModifyField");
			if (previous != SBA)
				trace_ds("%s",rcba(buffer_addr));
			previous = ORDER;
			cp++;
			na = *cp;
			if (h3270.ea_buf[buffer_addr].fa) {
				for (i = 0; i < (int)na; i++) {
					cp++;
					if (*cp == XA_3270) {
						trace_ds(" 3270");
						cp++;
						ctlr_add_fa(buffer_addr, *cp,
							h3270.ea_buf[buffer_addr].cs);
						trace_ds("%s",see_attr(*cp));
					} else if (*cp == XA_FOREGROUND) {
						trace_ds("%s",
						    see_efa(*cp,
							*(cp + 1)));
						cp++;
						if (appres.m3279)
							ctlr_add_fg(buffer_addr, *cp);
					} else if (*cp == XA_BACKGROUND) {
						trace_ds("%s",
						    see_efa(*cp,
							*(cp + 1)));
						cp++;
						if (appres.m3279)
							ctlr_add_bg(buffer_addr, *cp);
					} else if (*cp == XA_HIGHLIGHTING) {
						trace_ds("%s",
						    see_efa(*cp,
							*(cp + 1)));
						cp++;
						ctlr_add_gr(buffer_addr, *cp & 0x0f);
					} else if (*cp == XA_CHARSET) {
						int cs = 0;

						trace_ds("%s",
						    see_efa(*cp,
							*(cp + 1)));
						cp++;
						if (*cp == 0xf1)
							cs = CS_APL;
						else if (*cp == 0xf8)
							cs = CS_DBCS;
						ctlr_add_cs(buffer_addr, cs);
					} else if (*cp == XA_ALL) {
						trace_ds("%s",
						    see_efa(*cp,
							*(cp + 1)));
						cp++;
					} else if (*cp == XA_INPUT_CONTROL) {
						trace_ds("%s",
						    see_efa(*cp,
							*(cp + 1)));
						ctlr_add_ic(buffer_addr,
						    (*(cp + 1) == 1));
						cp++;
					} else {
						trace_ds("%s[unsupported]", see_efa(*cp, *(cp + 1)));
						cp++;
					}
				}
				INC_BA(buffer_addr);
			} else
				cp += na * 2;
			last_cmd = True;
			last_zpt = False;
			break;
		case ORDER_SFE:	/* start field extended */
			END_TEXT("StartFieldExtended");
			if (previous != SBA)
				trace_ds("%s",rcba(buffer_addr));
			previous = ORDER;
			cp++;	/* skip order */
			na = *cp;
			any_fa = 0;
			efa_fg = 0;
			efa_bg = 0;
			efa_gr = 0;
			efa_cs = 0;
			efa_ic = 0;
			for (i = 0; i < (int)na; i++) {
				cp++;
				if (*cp == XA_3270) {
					trace_ds(" 3270");
					cp++;
					START_FIELD(*cp);
					any_fa++;
				} else if (*cp == XA_FOREGROUND) {
					trace_ds("%s", see_efa(*cp, *(cp + 1)));
					cp++;
					if (appres.m3279)
						efa_fg = *cp;
				} else if (*cp == XA_BACKGROUND) {
					trace_ds("%s", see_efa(*cp, *(cp + 1)));
					cp++;
					if (appres.m3279)
						efa_bg = *cp;
				} else if (*cp == XA_HIGHLIGHTING) {
					trace_ds("%s", see_efa(*cp, *(cp + 1)));
					cp++;
					efa_gr = *cp & 0x07;
				} else if (*cp == XA_CHARSET) {
					trace_ds("%s", see_efa(*cp, *(cp + 1)));
					cp++;
					if (*cp == 0xf1)
						efa_cs = CS_APL;
					else if (dbcs && (*cp == 0xf8))
						efa_cs = CS_DBCS;
					else
						efa_cs = CS_BASE;
				} else if (*cp == XA_ALL) {
					trace_ds("%s", see_efa(*cp, *(cp + 1)));
					cp++;
				} else if (*cp == XA_INPUT_CONTROL) {
					trace_ds("%s", see_efa(*cp, *(cp + 1)));
					if (dbcs)
					    efa_ic = (*(cp + 1) == 1);
					cp++;
				} else {
					trace_ds("%s[unsupported]", see_efa(*cp, *(cp + 1)));
					cp++;
				}
			}
			if (!any_fa)
				START_FIELD(0);
			ctlr_add_cs(buffer_addr, efa_cs);
			ctlr_add_fg(buffer_addr, efa_fg);
			ctlr_add_bg(buffer_addr, efa_bg);
			ctlr_add_gr(buffer_addr, efa_gr);
			ctlr_add_ic(buffer_addr, efa_ic);
			INC_BA(buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;
		case ORDER_SA:	/* set attribute */
			END_TEXT("SetAttribute");
			previous = ORDER;
			cp++;
			if (*cp == XA_FOREGROUND)  {
				trace_ds("%s", see_efa(*cp, *(cp + 1)));
				if (appres.m3279)
					default_fg = *(cp + 1);
			} else if (*cp == XA_BACKGROUND)  {
				trace_ds("%s", see_efa(*cp, *(cp + 1)));
				if (appres.m3279)
					default_bg = *(cp + 1);
			} else if (*cp == XA_HIGHLIGHTING)  {
				trace_ds("%s", see_efa(*cp, *(cp + 1)));
				default_gr = *(cp + 1) & 0x0f;
			} else if (*cp == XA_ALL)  {
				trace_ds("%s", see_efa(*cp, *(cp + 1)));
				default_fg = 0;
				default_bg = 0;
				default_gr = 0;
				default_cs = 0;
				default_ic = 0;
			} else if (*cp == XA_CHARSET) {
				trace_ds("%s", see_efa(*cp, *(cp + 1)));
				switch (*(cp + 1)) {
				case 0xf1:
				    default_cs = CS_APL;
				    break;
				case 0xf8:
				    default_cs = CS_DBCS;
				    break;
				default:
				    default_cs = CS_BASE;
				    break;
				}
			} else if (*cp == XA_INPUT_CONTROL) {
				trace_ds("%s", see_efa(*cp, *(cp + 1)));
				if (*(cp + 1) == 1)
					default_ic = 1;
				else
					default_ic = 0;
			} else
				trace_ds("%s[unsupported]",
				    see_efa(*cp, *(cp + 1)));
			cp++;
			last_cmd = True;
			last_zpt = False;
			break;
		case FCORDER_SUB:	/* format control orders */
		case FCORDER_DUP:
		case FCORDER_FM:
		case FCORDER_FF:
		case FCORDER_CR:
		case FCORDER_NL:
		case FCORDER_EM:
		case FCORDER_EO:
			END_TEXT(see_ebc(*cp));
			previous = ORDER;
			d = ctlr_lookleft_state(buffer_addr, &why);
			if (default_cs == CS_DBCS || d != DBCS_NONE) {
				ABORT_WRITE("invalid format control order in DBCS field");
			}
			ctlr_add(buffer_addr, *cp, default_cs);
			ctlr_add_fg(buffer_addr, default_fg);
			ctlr_add_bg(buffer_addr, default_bg);
			ctlr_add_gr(buffer_addr, default_gr);
			ctlr_add_ic(buffer_addr, default_ic);
			INC_BA(buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;
		case FCORDER_SO:
			/* Look left for errors. */
			END_TEXT(see_ebc(*cp));
			d = ctlr_lookleft_state(buffer_addr, &why);
			if (d == DBCS_RIGHT) {
				ABORT_WRITE("SO overwriting right half of DBCS character");
			}
			if (d != DBCS_NONE && why == DBCS_FIELD) {
				ABORT_WRITE("SO in DBCS field");
			}
			if (d != DBCS_NONE && why == DBCS_SUBFIELD) {
				ABORT_WRITE("double SO");
			}
			/* All is well. */
			previous = ORDER;
			ctlr_add(buffer_addr, *cp, default_cs);
			ctlr_add_fg(buffer_addr, default_fg);
			ctlr_add_bg(buffer_addr, default_bg);
			ctlr_add_gr(buffer_addr, default_gr);
			ctlr_add_ic(buffer_addr, default_ic);
			INC_BA(buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;
		case FCORDER_SI:
			/* Look left for errors. */
			END_TEXT(see_ebc(*cp));
			d = ctlr_lookleft_state(buffer_addr, &why);
			if (d == DBCS_RIGHT) {
				ABORT_WRITE("SI overwriting right half of DBCS character");
			}
			if (d != DBCS_NONE && why == DBCS_FIELD) {
				ABORT_WRITE("SI in DBCS field");
			}
			fa_addr = find_field_attribute(&h3270,buffer_addr);
			baddr = buffer_addr;
			DEC_BA(baddr);
			while (!aborted &&
			       ((fa_addr >= 0 && baddr != fa_addr) ||
			        (fa_addr < 0 && baddr != h3270.rows*h3270.cols - 1))) {
				if (h3270.ea_buf[baddr].cc == FCORDER_SI) {
					ABORT_WRITE("double SI");
				}
				if (h3270.ea_buf[baddr].cc == FCORDER_SO)
					break;
				DEC_BA(baddr);
			}
			if (aborted)
				break;
			if (h3270.ea_buf[baddr].cc != FCORDER_SO) {
				ABORT_WRITE("SI without SO");
			}
			/* All is well. */
			previous = ORDER;
			ctlr_add(buffer_addr, *cp, default_cs);
			ctlr_add_fg(buffer_addr, default_fg);
			ctlr_add_bg(buffer_addr, default_bg);
			ctlr_add_gr(buffer_addr, default_gr);
			ctlr_add_ic(buffer_addr, default_ic);
			INC_BA(buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;
		case FCORDER_NULL:	/* NULL or DBCS control char */
			previous = NULLCH;
			add_dbcs = False;
			d = ctlr_lookleft_state(buffer_addr, &why);
			if (d == DBCS_RIGHT) {
				ABORT_WRITE("NULL overwriting right half of DBCS character");
			}
			if (d != DBCS_NONE || default_cs == CS_DBCS) {
				add_c1 = EBC_null;
				cp++;
				if (cp >= buf + buflen) {
					ABORT_WRITE("missing second half of DBCS character");
				}
				add_c2 = *cp;
				switch (add_c2) {
				case EBC_null:
				case EBC_nl:
				case EBC_em:
				case EBC_ff:
				case EBC_cr:
				case EBC_dup:
				case EBC_fm:
					/* DBCS control code */
					END_TEXT(see_ebc(add_c2));
					add_dbcs = True;
					break;
				case ORDER_SF:
				case ORDER_SFE:
					/* Dead position */
					END_TEXT("DeadNULL");
					cp--;
					break;
				default:
					trace_ds(" [invalid DBCS control character X'%02x%02x'; write aborted]",
						add_c1, add_c2);
					ABORT_WRITEx;
					break;
				}
				if (aborted)
					break;
			} else {
				END_TEXT("NULL");
				add_c1 = *cp;
			}
			ctlr_add(buffer_addr, add_c1, default_cs);
			ctlr_add_fg(buffer_addr, default_fg);
			ctlr_add_bg(buffer_addr, default_bg);
			ctlr_add_gr(buffer_addr, default_gr);
			ctlr_add_ic(buffer_addr, default_ic);
			INC_BA(buffer_addr);
			if (add_dbcs) {
				ctlr_add(buffer_addr, add_c2, default_cs);
				ctlr_add_fg(buffer_addr, default_fg);
				ctlr_add_bg(buffer_addr, default_bg);
				ctlr_add_gr(buffer_addr, default_gr);
				ctlr_add_ic(buffer_addr, default_ic);
				INC_BA(buffer_addr);
			}
			last_cmd = False;
			last_zpt = False;
			break;
		default:	/* enter character */
			if (*cp <= 0x3F) {
				END_TEXT("UnsupportedOrder");
				trace_ds("(%02X)", *cp);
				previous = ORDER;
				last_cmd = True;
				last_zpt = False;
				break;
			}
			if (previous != TEXT)
				trace_ds(" '");
			previous = TEXT;
#if defined(X3270_DBCS) /*[*/
			add_dbcs = False;
			d = ctlr_lookleft_state(buffer_addr, &why);
			if (d == DBCS_RIGHT) {
				ABORT_WRITE("overwriting right half of DBCS character");
			}
			if (d != DBCS_NONE || default_cs == CS_DBCS) {
				add_c1 = *cp;
				cp++;
				if (cp >= buf + buflen) {
					ABORT_WRITE("missing second half of DBCS character");
				}
				add_c2 = *cp;
				if (add_c1 < 0x40 || add_c1 > 0xfe ||
				    add_c2 < 0x40 || add_c2 > 0xfe) {
					trace_ds(" [invalid DBCS character X'%02x%02x'; write aborted]",
						add_c1, add_c2);
					ABORT_WRITEx;
			       }
			       add_dbcs = True;
			       dbcs_to_mb(add_c1, add_c2, mb);
			       trace_ds_nb("%s", mb);
			} else {
#endif /*]*/
				add_c1 = *cp;
				trace_ds("%s", see_ebc(*cp));
#if defined(X3270_DBCS) /*[*/
			}
#endif /*]*/
			ctlr_add(buffer_addr, add_c1, default_cs);
			ctlr_add_fg(buffer_addr, default_fg);
			ctlr_add_bg(buffer_addr, default_bg);
			ctlr_add_gr(buffer_addr, default_gr);
			ctlr_add_ic(buffer_addr, default_ic);
			INC_BA(buffer_addr);
#if defined(X3270_DBCS) /*[*/
			if (add_dbcs) {
				ctlr_add(buffer_addr, add_c2, default_cs);
				ctlr_add_fg(buffer_addr, default_fg);
				ctlr_add_bg(buffer_addr, default_bg);
				ctlr_add_gr(buffer_addr, default_gr);
				ctlr_add_ic(buffer_addr, default_ic);
				INC_BA(buffer_addr);
			}
#endif /*]*/
			last_cmd = False;
			last_zpt = False;
			break;
		}
	}
	set_formatted(&h3270);
	END_TEXT0;
	trace_ds("\n");
	if (wcc_keyboard_restore) {
		aid = AID_NO;
		do_reset(False);
	} else if (kybdlock & KL_OIA_TWAIT) {
		kybdlock_clr(KL_OIA_TWAIT, "ctlr_write");
		status_syswait();
	}
	if (wcc_sound_alarm)
		lib3270_ring_bell(NULL);

	/* Set up the DBCS state. */
	if (ctlr_dbcs_postprocess() < 0 && rv == PDS_OKAY_NO_OUTPUT)
		rv = PDS_BAD_ADDR;

	trace_primed = False;

	ps_process();

	/* Let a script go. */
//	sms_host_output();

	/* Tell 'em what happened. */
	return rv;
}

#undef START_FIELDx
#undef START_FIELD0
#undef START_FIELD
#undef END_TEXT0
#undef END_TEXT
#undef ABORT_WRITEx
#undef ABORT_WRITE

/*
 * Write SSCP-LU data, which is quite a bit dumber than regular 3270
 * output.
 */
void
ctlr_write_sscp_lu(unsigned char buf[], int buflen)
{
	int i;
	unsigned char *cp = buf;
	int s_row;
	unsigned char c;
//	int baddr;

	/*
	 * The 3174 Functionl Description says that anything but NL, NULL, FM,
	 * or DUP is to be displayed as a graphic.  However, to deal with
	 * badly-behaved hosts, we filter out SF, IC and SBA sequences, and
	 * we display other control codes as spaces.
	 */

	trace_ds("SSCP-LU data\n");
	for (i = 0; i < buflen; cp++, i++) {
		switch (*cp) {
		case FCORDER_NL:
			/*
			 * Insert NULLs to the end of the line and advance to
			 * the beginning of the next line.
			 */
			s_row = buffer_addr / h3270.cols;
			while ((buffer_addr / h3270.cols) == s_row) {
				ctlr_add(buffer_addr, EBC_null, default_cs);
				ctlr_add_fg(buffer_addr, default_fg);
				ctlr_add_bg(buffer_addr, default_bg);
				ctlr_add_gr(buffer_addr, default_gr);
				ctlr_add_ic(buffer_addr, default_ic);
				INC_BA(buffer_addr);
			}
			break;

		case ORDER_SF:
			/* Some hosts forget they're talking SSCP-LU. */
			cp++;
			i++;
			trace_ds(" StartField%s %s [translated to space]\n",rcba(buffer_addr), see_attr(*cp));
			ctlr_add(buffer_addr, EBC_space, default_cs);
			ctlr_add_fg(buffer_addr, default_fg);
			ctlr_add_bg(buffer_addr, default_bg);
			ctlr_add_gr(buffer_addr, default_gr);
			ctlr_add_ic(buffer_addr, default_ic);
			INC_BA(buffer_addr);
			break;
		case ORDER_IC:
			trace_ds(" InsertCursor%s [ignored]\n",
			    rcba(buffer_addr));
			break;
		case ORDER_SBA:
//			baddr = DECODE_BADDR(*(cp+1), *(cp+2));
			trace_ds(" SetBufferAddress%s [ignored]\n", rcba(DECODE_BADDR(*(cp+1), *(cp+2))));
			cp += 2;
			i += 2;
			break;

		case ORDER_GE:
			cp++;
			if (++i >= buflen)
				break;
			if (*cp <= 0x40)
				c = EBC_space;
			else
				c = *cp;
			ctlr_add(buffer_addr, c, CS_GE);
			ctlr_add_fg(buffer_addr, default_fg);
			ctlr_add_bg(buffer_addr, default_bg);
			ctlr_add_gr(buffer_addr, default_gr);
			ctlr_add_ic(buffer_addr, default_ic);
			INC_BA(buffer_addr);
			break;

		default:
			ctlr_add(buffer_addr, *cp, default_cs);
			ctlr_add_fg(buffer_addr, default_fg);
			ctlr_add_bg(buffer_addr, default_bg);
			ctlr_add_gr(buffer_addr, default_gr);
			ctlr_add_ic(buffer_addr, default_ic);
			INC_BA(buffer_addr);
			break;
		}
	}
	cursor_move(buffer_addr);
	sscp_start = buffer_addr;

	/* Unlock the keyboard. */
	aid = AID_NO;
	do_reset(False);

	/* Let a script go. */
//	sms_host_output();
}

#if defined(X3270_DBCS) /*[*/

/*
 * Determine the DBCS state of a buffer location strictly by looking left.
 * Used only to validate write operations.
 * Returns only DBCS_LEFT, DBCS_RIGHT or DBCS_NONE.
 * Also returns whether the location is part of a DBCS field (SFE with the
 *  DBCS character set), DBCS subfield (to the right of an SO within a non-DBCS
 *  field), or DBCS attribute (has the DBCS character set extended attribute
 *  within a non-DBCS field).
 *
 * This function should be used only to determine the legality of adding a
 * DBCS or SBCS character at baddr.
 */
enum dbcs_state
ctlr_lookleft_state(int baddr, enum dbcs_why *why)
{
	int faddr;
	int fdist;
	int xaddr;
	Boolean si = False;
#define	AT_END(f, b) \
	(((f) < 0 && (b) == ROWS*COLS - 1) || \
	 ((f) >= 0 && (b) == (f)))

	 /* If we're not in DBCS state, everything is DBCS_NONE. */
	 if (!dbcs)
		return DBCS_NONE;

	/* Find the field attribute, if any. */
	faddr = find_field_attribute(baddr);

	/*
	 * First in precedence is a DBCS field.
	 * DBCS SA and SO/SI inside a DBCS field are errors, but are considered
	 * defective DBCS characters.
	 */
	if (ea_buf[faddr].cs == CS_DBCS) {
		*why = DBCS_FIELD;
		fdist = (baddr + ROWS*COLS) - faddr;
		return (fdist % 2)? DBCS_LEFT: DBCS_RIGHT;
	}

	/*
	 * The DBCS attribute takes precedence next.
	 * SO and SI can appear within such a region, but they are single-byte
	 * characters which effectively split it.
	 */
	if (ea_buf[baddr].cs == CS_DBCS) {
		if (ea_buf[baddr].cc == EBC_so || ea_buf[baddr].cc == EBC_si)
			return DBCS_NONE;
		xaddr = baddr;
		while (!AT_END(faddr, xaddr) &&
		       ea_buf[xaddr].cs == CS_DBCS &&
		       ea_buf[xaddr].cc != EBC_so &&
		       ea_buf[xaddr].cc != EBC_si) {
			DEC_BA(xaddr);
		}
		*why = DBCS_ATTRIBUTE;
		fdist = (baddr + ROWS*COLS) - xaddr;
		return (fdist % 2)? DBCS_LEFT: DBCS_RIGHT;
	}

	/*
	 * Finally, look for a SO not followed by an SI.
	 */
	xaddr = baddr;
	DEC_BA(xaddr);
	while (!AT_END(faddr, xaddr)) {
		if (ea_buf[xaddr].cc == EBC_si)
			si = True;
		else if (ea_buf[xaddr].cc == EBC_so) {
			if (si)
				si = False;
			else {
				*why = DBCS_SUBFIELD;
				fdist = (baddr + ROWS*COLS) - xaddr;
				return (fdist % 2)? DBCS_LEFT: DBCS_RIGHT;
			}
		}
		DEC_BA(xaddr);
	}

	/* Nada. */
	return DBCS_NONE;
}

static Boolean
valid_dbcs_char(unsigned char c1, unsigned char c2)
{
	if (c1 >= 0x40 && c1 < 0xff && c2 >= 0x40 && c2 < 0xff)
		return True;
	if (c1 != 0x00 || c2 < 0x40 || c2 >= 0xff)
		return False;
	switch (c2) {
	case EBC_null:
	case EBC_nl:
	case EBC_em:
	case EBC_ff:
	case EBC_cr:
	case EBC_dup:
	case EBC_fm:
		return True;
	default:
		return False;
	}
}

/*
 * Post-process DBCS state in the buffer.
 * This has two purposes:
 *
 * - Required post-processing validation, per the data stream spec, which can
 *   cause the write operation to be rejected.
 * - Setting up the value of the all the db fields in ea_buf.
 *
 * This function is called at the end of every 3270 write operation, and also
 * after each batch of NVT write operations.  It could also be called after
 * significant keyboard operations, but that might be too expensive.
 *
 * Returns 0 for success, -1 for failure.
 */
int
ctlr_dbcs_postprocess(void)
{
	int baddr;		/* current buffer address */
	int faddr0;		/* address of first field attribute */
	int faddr;		/* address of current field attribute */
	int last_baddr;		/* last buffer address to search */
	int pbaddr = -1;	/* previous buffer address */
	int dbaddr = -1;	/* first data position of current DBCS (sub-)
				   field */
	Boolean so = False, si = False;
	Boolean dbcs_field = False;
	int rc = 0;

	/* If we're not in DBCS mode, do nothing. */
	if (!dbcs)
		return 0;

	/*
	 * Find the field attribute for location 0.  If unformatted, it's the
	 * dummy at -1.  Also compute the starting and ending points for the
	 * scan: the first location after that field attribute.
	 */
	faddr0 = find_field_attribute(0);
	baddr = faddr0;
	INC_BA(baddr);
	if (faddr0 < 0)
		last_baddr = 0;
	else
		last_baddr = faddr0;
	faddr = faddr0;
	dbcs_field = (ea_buf[faddr].cs & CS_MASK) == CS_DBCS;

	do {
		if (ea_buf[baddr].fa) {
			faddr = baddr;
			ea_buf[faddr].db = DBCS_NONE;
			dbcs_field = (ea_buf[faddr].cs & CS_MASK) == CS_DBCS;
			if (dbcs_field) {
				dbaddr = baddr;
				INC_BA(dbaddr);
			} else {
				dbaddr = -1;
			}
			/*
			 * An SI followed by a field attribute shouldn't be
			 * displayed with a wide cursor.
			 */
			if (pbaddr >= 0 && ea_buf[pbaddr].db == DBCS_SI)
				ea_buf[pbaddr].db = DBCS_NONE;
		} else {
			switch (ea_buf[baddr].cc) {
			case EBC_so:
			    /* Two SO's or SO in DBCS field are invalid. */
			    if (so || dbcs_field) {
				    trace_ds("DBCS postprocess: invalid SO "
					"found at %s\n", rcba(baddr));
				    rc = -1;
			    } else {
				    dbaddr = baddr;
				    INC_BA(dbaddr);
			    }
			    ea_buf[baddr].db = DBCS_NONE;
			    so = True;
			    si = False;
			    break;
			case EBC_si:
			    /* Two SI's or SI in DBCS field are invalid. */
			    if (si || dbcs_field) {
				    trace_ds("Postprocess: Invalid SO found "
					"at %s\n", rcba(baddr));
				    rc = -1;
				    ea_buf[baddr].db = DBCS_NONE;
			    } else {
				    ea_buf[baddr].db = DBCS_SI;
			    }
			    dbaddr = -1;
			    si = True;
			    so = False;
			    break;
			default:
			    /* Non-base CS in DBCS subfield is invalid. */
			    if (so && ea_buf[baddr].cs != CS_BASE) {
				    trace_ds("DBCS postprocess: invalid "
					"character set found at %s\n",
					rcba(baddr));
				    rc = -1;
				    ea_buf[baddr].cs = CS_BASE;
			    }
			    if ((ea_buf[baddr].cs & CS_MASK) == CS_DBCS) {
				    /*
				     * Beginning or continuation of an SA DBCS
				     * subfield.
				     */
				    if (dbaddr < 0) {
					    dbaddr = baddr;
				    }
			    } else if (!so && !dbcs_field) {
				    /*
				     * End of SA DBCS subfield.
				     */
				    dbaddr = -1;
			    }
			    if (dbaddr >= 0) {
				    /*
				     * Turn invalid characters into spaces,
				     * silently.
				     */
				    if ((baddr + ROWS*COLS - dbaddr) % 2) {
					    if (!valid_dbcs_char(
							ea_buf[pbaddr].cc,
							ea_buf[baddr].cc)) {
						    ea_buf[pbaddr].cc =
							EBC_space;
						    ea_buf[baddr].cc =
							EBC_space;
					    }
					    MAKE_RIGHT(baddr);
				    } else {
					    MAKE_LEFT(baddr);
				    }
			    } else
				    ea_buf[baddr].db = DBCS_NONE;
			    break;
			}
		}

		/*
		 * Check for dead positions.
		 * Turn them into NULLs, silently.
		 */
		if (pbaddr >= 0 &&
		    IS_LEFT(ea_buf[pbaddr].db) &&
		    !IS_RIGHT(ea_buf[baddr].db) &&
		    ea_buf[pbaddr].db != DBCS_DEAD) {
			if (!ea_buf[baddr].fa) {
				trace_ds("DBCS postprocess: dead position "
				    "at %s\n", rcba(pbaddr));
				rc = -1;
			}
			ea_buf[pbaddr].cc = EBC_null;
			ea_buf[pbaddr].db = DBCS_DEAD;
		}

		/* Check for SB's, which follow SIs. */
		if (pbaddr >= 0 && ea_buf[pbaddr].db == DBCS_SI)
			ea_buf[baddr].db = DBCS_SB;

		/* Save this position as the previous and increment. */
		pbaddr = baddr;
		INC_BA(baddr);

	} while (baddr != last_baddr);

	return rc;
}
#endif /*]*/

/*
 * Process pending input.
 */
void
ps_process(void)
{
	while (run_ta())
		;
//	sms_continue();

#if defined(X3270_FT) /*[*/
	/* Process file transfers. */
	if (ft_state != FT_NONE &&      /* transfer in progress */
	    h3270.formatted &&          /* screen is formatted */
	    !h3270.screen_alt &&        /* 24x80 screen */
	    !kybdlock &&                /* keyboard not locked */
	    /* magic field */
	    h3270.ea_buf[1919].fa && FA_IS_SKIP(h3270.ea_buf[1919].fa)) {
		ft_cut_data();
	}
#endif /*]*/
}

/*
 * Tell me if there is any data on the screen.
 */
Boolean
ctlr_any_data(void)
{
	register int i;

	for (i = 0; i < h3270.rows*h3270.cols; i++) {
		if (!IsBlank(h3270.ea_buf[i].cc))
			return True;
	}
	return False;
}

/*
 * Clear the text (non-status) portion of the display.  Also resets the cursor
 * and buffer addresses and extended attributes.
 */
void
ctlr_clear(H3270 *session, Boolean can_snap)
{
	/* Snap any data that is about to be lost into the trace file. */
	if (ctlr_any_data()) {
#if defined(X3270_TRACE) /*[*/
		if (can_snap && !trace_skipping && toggled(SCREEN_TRACE))
			trace_screen();
#endif /*]*/
		scroll_save(session->maxROWS, ever_3270 ? False : True);
	}
#if defined(X3270_TRACE) /*[*/
	trace_skipping = False;
#endif /*]*/

	/* Clear the screen. */
	(void) memset((char *)session->ea_buf, 0, session->rows*session->cols*sizeof(struct ea));
	cursor_move(0);
	buffer_addr = 0;
	// unselect(0, ROWS*COLS);
	session->formatted = False;
	default_fg = 0;
	default_bg = 0;
	default_gr = 0;
	default_ic = 0;
	sscp_start = 0;

//	ALL_CHANGED;
	screen_erase(session);

}

/*
 * Fill the screen buffer with blanks.
 */
static void
ctlr_blanks(void)
{
	int baddr;

	for (baddr = 0; baddr < h3270.rows*h3270.cols; baddr++) {
		if (!h3270.ea_buf[baddr].fa)
			h3270.ea_buf[baddr].cc = EBC_space;
	}
	cursor_move(0);
	buffer_addr = 0;
	// unselect(0, ROWS*COLS);
	h3270.formatted = False;
	ALL_CHANGED;
}


/*
 * Change a character in the 3270 buffer.
 * Removes any field attribute defined at that location.
 */
void
ctlr_add(int baddr, unsigned char c, unsigned char cs)
{
	unsigned char oc = 0;

	if(h3270.ea_buf[baddr].fa || ((oc = h3270.ea_buf[baddr].cc) != c || h3270.ea_buf[baddr].cs != cs))
	{
		if (trace_primed && !IsBlank(oc))
		{
#if defined(X3270_TRACE) /*[*/
			if (toggled(SCREEN_TRACE))
				trace_screen();
#endif /*]*/
			scroll_save(session->maxROWS, False);
			trace_primed = False;
		}
		/*
		if (SELECTED(baddr))
			unselect(baddr, 1);
		*/
		h3270.ea_buf[baddr].cc = c;
		h3270.ea_buf[baddr].cs = cs;
		h3270.ea_buf[baddr].fa = 0;
		ONE_CHANGED(baddr);
	}
}

/*
 * Set a field attribute in the 3270 buffer.
 */
void
ctlr_add_fa(int baddr, unsigned char fa, unsigned char cs)
{
	/* Put a null in the display buffer. */
	ctlr_add(baddr, EBC_null, cs);

	/*
	 * Store the new attribute, setting the 'printable' bits so that the
	 * value will be non-zero.
	 */
	h3270.ea_buf[baddr].fa = FA_PRINTABLE | (fa & FA_MASK);
}

/*
 * Change the character set for a field in the 3270 buffer.
 */
void
ctlr_add_cs(int baddr, unsigned char cs)
{
	if (h3270.ea_buf[baddr].cs != cs)
	{
		/*
		if (SELECTED(baddr))
			unselect(baddr, 1);
		*/
		h3270.ea_buf[baddr].cs = cs;
		ONE_CHANGED(baddr);
	}
}

/*
 * Change the graphic rendition of a character in the 3270 buffer.
 */
void
ctlr_add_gr(int baddr, unsigned char gr)
{
	if (h3270.ea_buf[baddr].gr != gr)
	{
		h3270.ea_buf[baddr].gr = gr;
		if (gr & GR_BLINK)
			blink_start();
		ONE_CHANGED(baddr);
	}
}

/*
 * Change the foreground color for a character in the 3270 buffer.
 */
void
ctlr_add_fg(int baddr, unsigned char color)
{
	if (!appres.m3279)
		return;
	if ((color & 0xf0) != 0xf0)
		color = 0;
	if (h3270.ea_buf[baddr].fg != color)
	{
		h3270.ea_buf[baddr].fg = color;
		ONE_CHANGED(baddr);
	}
}

/*
 * Change the background color for a character in the 3270 buffer.
 */
void
ctlr_add_bg(int baddr, unsigned char color)
{
	if (!appres.m3279)
		return;
	if ((color & 0xf0) != 0xf0)
		color = 0;
	if (h3270.ea_buf[baddr].bg != color)
	{
		h3270.ea_buf[baddr].bg = color;
		ONE_CHANGED(baddr);
	}
}

/*
 * Change the input control bit for a character in the 3270 buffer.
 */
static void
ctlr_add_ic(int baddr, unsigned char ic)
{
	h3270.ea_buf[baddr].ic = ic;
}

/*
 * Wrapping bersion of ctlr_bcopy.
 */
void
ctlr_wrapping_memmove(int baddr_to, int baddr_from, int count)
{
	/*
	 * The 'to' region, the 'from' region, or both can wrap the screen,
	 * and can overlap each other.  memmove() is smart enough to deal with
	 * overlaps, but not across a screen wrap.
	 *
	 * It's faster to figure out if none of this is true, then do a slow
	 * location-at-a-time version only if it happens.
	 */
	if (baddr_from + count <= h3270.rows*h3270.cols &&
	    baddr_to + count <= h3270.rows*h3270.cols) {
		ctlr_bcopy(baddr_from, baddr_to, count, True);
	} else {
		int i, from, to;

		for (i = 0; i < count; i++) {
		    if (baddr_to > baddr_from) {
			/* Shifting right, move left. */
			to = (baddr_to + count - 1 - i) % h3270.rows*h3270.cols;
			from = (baddr_from + count - 1 - i) % h3270.rows*h3270.cols;
		    } else {
			/* Shifting left, move right. */
			to = (baddr_to + i) % h3270.rows*h3270.cols;
			from = (baddr_from + i) % h3270.rows*h3270.cols;
		    }
		    ctlr_bcopy(from, to, 1, True);
		}
	}
}

/*
 * Copy a block of characters in the 3270 buffer, optionally including all of
 * the extended attributes.  (The character set, which is actually kept in the
 * extended attributes, is considered part of the characters here.)
 */
void
ctlr_bcopy(int baddr_from, int baddr_to, int count, int move_ea)
{
	/* Move the characters. */
	if (memcmp((char *) &h3270.ea_buf[baddr_from],
	           (char *) &h3270.ea_buf[baddr_to],
		   count * sizeof(struct ea))) {
		(void) memmove(&h3270.ea_buf[baddr_to], &h3270.ea_buf[baddr_from],
			           count * sizeof(struct ea));
		REGION_CHANGED(baddr_to, baddr_to + count);
	}
	/* XXX: What about move_ea? */
}

#if defined(X3270_ANSI) /*[*/
/*
 * Erase a region of the 3270 buffer, optionally clearing extended attributes
 * as well.
 */
void ctlr_aclear(int baddr, int count, int clear_ea)
{
	if (memcmp((char *) &h3270.ea_buf[baddr], (char *) zero_buf,
		    count * sizeof(struct ea))) {
		(void) memset((char *) &h3270.ea_buf[baddr], 0,
				count * sizeof(struct ea));
		REGION_CHANGED(baddr, baddr + count);
	}
	/* XXX: What about clear_ea? */
}

/*
 * Scroll the screen 1 row.
 *
 * This could be accomplished with ctlr_bcopy() and ctlr_aclear(), but this
 * operation is common enough to warrant a separate path.
 */
void ctlr_scroll(void)
{
	int qty = (h3270.rows - 1) * h3270.cols;

	/* Make sure nothing is selected. (later this can be fixed) */
	// unselect(0, ROWS*COLS);

	/* Synchronize pending changes prior to this. */

	/* Move ea_buf. */
	(void) memmove(&h3270.ea_buf[0], &h3270.ea_buf[h3270.cols],qty * sizeof(struct ea));

	/* Clear the last line. */
	(void) memset((char *) &h3270.ea_buf[qty], 0, h3270.cols * sizeof(struct ea));

	screen_disp(&h3270);

}
#endif /*]*/

/*
 * Note that a particular region of the screen has changed.
 */
void changed(H3270 *session, int bstart, int bend)
{
	/*
	if(session->first_changed < 0)
	{
		session->first_changed = bstart;
		session->last_changed  = bend;
		return;
	}
	if(bstart < session->first_changed)
		session->first_changed = bstart;

	if(bend > session->last_changed)
		session->last_changed = bend;
	*/
}

/*
 * Swap the regular and alternate screen buffers
 */
void ctlr_altbuffer(H3270 *session, int alt)
{
    CHECK_SESSION_HANDLE(session);

	if (alt != session->is_altbuffer)
	{
		struct ea *etmp;

		etmp = session->ea_buf;
		session->ea_buf  = session->aea_buf;
		session->aea_buf = etmp;

		session->is_altbuffer = alt;
		lib3270_unselect(session);

		ALL_CHANGED;

		/*
		 * There may be blinkers on the alternate screen; schedule one
		 * iteration just in case.
		 */
		blink_start();
	}
}


/*
 * Set or clear the MDT on an attribute
 */
void
mdt_set(int baddr)
{
	int faddr;

	faddr = find_field_attribute(&h3270,baddr);
	if (faddr >= 0 && !(h3270.ea_buf[faddr].fa & FA_MODIFY)) {
		h3270.ea_buf[faddr].fa |= FA_MODIFY;
		if (appres.modified_sel)
			ALL_CHANGED;
	}
}

void
mdt_clear(int baddr)
{
	int faddr;

	faddr = find_field_attribute(&h3270,baddr);
	if (faddr >= 0 && (h3270.ea_buf[faddr].fa & FA_MODIFY)) {
		h3270.ea_buf[faddr].fa &= ~FA_MODIFY;
		if (appres.modified_sel)
			ALL_CHANGED;
	}
}


/*
 * Support for screen-size swapping for scrolling
 */
void
ctlr_shrink(void)
{
	int baddr;

	for (baddr = 0; baddr < h3270.rows*h3270.cols; baddr++) {
		if (!h3270.ea_buf[baddr].fa)
			h3270.ea_buf[baddr].cc = visible_control? EBC_space : EBC_null;
	}
	ALL_CHANGED;
	screen_disp(&h3270);
}

#if defined(X3270_DBCS) /*[*/
/*
 * DBCS state query.
 * Returns:
 *  DBCS_NONE:	Buffer position is SBCS.
 *  DBCS_LEFT:	Buffer position is left half of a DBCS character.
 *  DBCS_RIGHT:	Buffer position is right half of a DBCS character.
 *  DBCS_SI:    Buffer position is the SI terminating a DBCS subfield (treated
 *		as DBCS_LEFT for wide cursor tests)
 *  DBCS_SB:	Buffer position is an SBCS character after an SI (treated as
 *		DBCS_RIGHT for wide cursor tests)
 *
 * Takes line-wrapping into account, which probably isn't done all that well.
 */
enum dbcs_state
ctlr_dbcs_state(int baddr)
{
	return dbcs? ea_buf[baddr].db: DBCS_NONE;
}
#endif /*]*/


/*
 * Transaction timing.  The time between sending an interrupt (PF, PA, Enter,
 * Clear) and the host unlocking the keyboard is indicated on the status line
 * to an accuracy of 0.1 seconds.  If we don't repaint the screen before we see
 * the unlock, the time should be fairly accurate.
 */
static struct timeval t_start;
static Boolean ticking = False;
static Boolean mticking = False;
static unsigned long tick_id;
static struct timeval t_want;

/* Return the difference in milliseconds between two timevals. */
static long
delta_msec(struct timeval *t1, struct timeval *t0)
{
	return (t1->tv_sec - t0->tv_sec) * 1000 +
	       (t1->tv_usec - t0->tv_usec + 500) / 1000;
}

static void keep_ticking(H3270 *session)
{
	struct timeval t1;
	long msec;

	do
	{
		(void) gettimeofday(&t1, (struct timezone *) 0);
		t_want.tv_sec++;
		msec = delta_msec(&t_want, &t1);
	} while (msec <= 0);

	tick_id = AddTimeOut(msec, session, keep_ticking);
	status_timing(session,&t_start, &t1);
}

void ticking_start(H3270 *session, Boolean anyway)
{
	CHECK_SESSION_HANDLE(session);

	if(session->set_timer)
	{
		if(toggled(SHOW_TIMING) || anyway)
			session->set_timer(session,1);
	}
	else
	{
		(void) gettimeofday(&t_start, (struct timezone *) 0);

		mticking = True;

		if (!toggled(SHOW_TIMING) && !anyway)
			return;

		status_untiming(session);
		if (ticking)
			RemoveTimeOut(tick_id);
		ticking = True;
		tick_id = AddTimeOut(1000, session, keep_ticking);
		t_want = t_start;
	}

}

static void ticking_stop(H3270 *session)
{
	CHECK_SESSION_HANDLE(session);

	if(session->set_timer)
	{
		session->set_timer(session,0);
	}
	else
	{
		struct timeval t1;

		(void) gettimeofday(&t1, (struct timezone *) 0);
		if (mticking)
			mticking = False;
		else
			return;

		if (!ticking)
			return;
		RemoveTimeOut(tick_id);
		ticking = False;
		status_timing(session,&t_start, &t1);
	}
}

/*
void toggle_showTiming(H3270 *session, struct toggle *t unused, LIB3270_TOGGLE_TYPE tt unused)
{
	if (!toggled(SHOW_TIMING))
		status_untiming(&h3270);
}
*/


/*
 * No-op toggle.
 */
void
toggle_nop(H3270 *session, struct toggle *t unused, LIB3270_TOGGLE_TYPE tt unused)
{
}

/*
int ctlr_get_rows(void)
{
    return h3270.rows;
}

int ctlr_get_cols(void)
{
    return h3270.cols;
}
*/
