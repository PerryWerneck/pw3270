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
 * Este programa está nomeado como ctlr.c e possui - linhas de código.
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
 *	@file
 *		This module handles interpretation of the 3270 data stream and
 *		maintenance of the 3270 device state.  It was split out from
 *		screen.c, which handles X operations.
 *
 */

#include "private.h"
#include <errno.h>
#include <stdlib.h>
#include "3270ds.h"
//#include "appres.h"
// #include "ctlr.h"
#include "screen.h"
#include "resources.h"

#include "ctlrc.h"
#include "ftc.h"
#include "ft_cutc.h"
#include "ft_dftc.h"
#include "hostc.h"
#include "kybdc.h"
#include "popupsc.h"
#include "screenc.h"
// #include "scrollc.h"
#include "seec.h"
#include "sf.h"
#include "statusc.h"
// #include "tablesc.h"
#include "telnetc.h"
#include "trace_dsc.h"
#include "utilc.h"
#include "widec.h"
#include "screenc.h"

// Boolean			dbcs = False;

/* Statics */
static void update_formatted(H3270 *session);
static void set_formatted(H3270 *hSession, int state);
static void ctlr_blanks(H3270 *session);
static void	ctlr_half_connect(H3270 *session, int ignored, void *dunno);
static void	ctlr_connect(H3270 *session, int ignored, void *dunno);
static void ticking_stop(H3270 *session);
static void ctlr_add_ic(H3270 *session, int baddr, unsigned char ic);

/**
 * code_table is used to translate buffer addresses and attributes to the 3270
 * datastream representation
 */
static const unsigned char code_table[64] =
{
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


#define ALL_CHANGED(h)	if(lib3270_in_ansi(h)) (h)->cbk.changed(h,0,(h)->rows*(h)->cols);
#define REGION_CHANGED(h, f, l) if(lib3270_in_ansi(h)) (h)->cbk.changed(h,f,l)
#define ONE_CHANGED(h,n)	if(lib3270_in_ansi(h)) (h)->cbk.changed(h,n,n+1);

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
	lib3270_register_schange(session,LIB3270_STATE_HALF_CONNECT, ctlr_half_connect, 0);
	lib3270_register_schange(session,LIB3270_STATE_CONNECT, ctlr_connect, 0);
	lib3270_register_schange(session,LIB3270_STATE_3270_MODE, ctlr_connect, 0);
}
/*
 * Reinitialize the emulated 3270 hardware.
 */
void ctlr_reinit(H3270 *session, unsigned cmask)
{
	if (cmask & MODEL_CHANGE)
	{
		/* Allocate buffers */
		struct lib3270_ea *tmp;
		size_t sz = (session->maxROWS * session->maxCOLS);

		session->buffer[0] = tmp = lib3270_calloc(sizeof(struct lib3270_ea), sz+1, session->buffer[0]);
		session->ea_buf = tmp + 1;

		session->buffer[1] = tmp = lib3270_calloc(sizeof(struct lib3270_ea),sz+1,session->buffer[1]);
		session->aea_buf = tmp + 1;

		session->text = lib3270_calloc(sizeof(struct lib3270_text),sz,session->text);

		Replace(session->zero_buf, (unsigned char *)Calloc(sizeof(struct lib3270_ea),sz));

		session->cursor_addr = 0;
		session->buffer_addr = 0;
	}
}

 /**
  * Parse the model number.
  *
  * @param session	Session Handle.
  * @param m		Model number.
  *
  * @return -1 (error), 0 (default), or the specified number.
  */
static int parse_model_number(H3270 *session, const char *m)
{
	int sl;
	int n;

	if(!m)
		return 0;

	sl = strlen(m);

	/* An empty model number is no good. */
	if (!sl)
		return 0;

	if (sl > 1) {
		/*
		 * If it's longer than one character, it needs to start with
		 * '327[89]', and it sets the m3279 resource.
		 */
		if (!strncmp(m, "3278", 4))
		{
			session->m3279 = 0;
		}
		else if (!strncmp(m, "3279", 4))
		{
			session->m3279 = 1;
		}
		else
		{
			return -1;
		}
		m += 4;
		sl -= 4;

		/* Check more syntax.  -E is allowed, but ignored. */
		switch (m[0]) {
		case '\0':
			/* Use default model number. */
			return 0;
		case '-':
			/* Model number specified. */
			m++;
			sl--;
			break;
		default:
			return -1;
		}
		switch (sl) {
		case 1: /* n */
			break;
		case 3:	/* n-E */
			if (strcasecmp(m + 1, "-E")) {
				return -1;
			}
			break;
		default:
			return -1;
		}
	}

	/* Check the numeric model number. */
	n = atoi(m);
	if (n >= 2 && n <= 5) {
		return n;
	} else {
		return -1;
	}

}

/**
 * Get current 3270 model.
 *
 * @param hSession selected 3270 session.
 * @return Current model number.
 */
int lib3270_get_model_number(H3270 *hSession)
{
	CHECK_SESSION_HANDLE(hSession);
	return hSession->model_num;
}

const char * lib3270_get_model(H3270 *hSession)
{
	CHECK_SESSION_HANDLE(hSession);
	return hSession->model_name;
}

/**
 * Deal with the relationships between model numbers and rows/cols.
 *
 * @param hSession	Session handle.
 * @param model		New model (updates model name)
 */ /*
int	lib3270_set_model(H3270 *hSession, int model)
{
	if(CONNECTED)
		return EBUSY;

	ctlr_set_rows_cols(hSession,model,hSession->ov_cols,hSession->ov_rows);
	ctlr_reinit(hSession,MODEL_CHANGE);
	screen_update(hSession,0,hSession->rows*hSession->cols);
	return 0;
} */

int lib3270_set_model(H3270 *hSession, const char *model)
{
	int 	ovc, ovr;
	char	junk;
	int		model_number;

	if(hSession->cstate != LIB3270_NOT_CONNECTED)
		return EBUSY;

	strncpy(hSession->full_model_name,"IBM-",LIB3270_FULL_MODEL_NAME_LENGTH);
	hSession->model_name = &hSession->full_model_name[4];

	if(!*model)
		model = "2";	// No model, use the default one

	model_number = parse_model_number(hSession,model);
	if (model_number < 0)
	{
		popup_an_error(hSession,"Invalid model number: %s", model);
		model_number = 0;
	}

	if (!model_number)
	{
#if defined(RESTRICT_3279)
		model_number = 3;
#else
		model_number = 4;
#endif
	}

	if(hSession->mono)
		hSession->m3279 = 0;
	else
		hSession->m3279 = 1;

	if(!hSession->extended)
		hSession->oversize = CN;

#if defined(RESTRICT_3279)
	if (hSession->m3279 && model_number == 4)
		model_number = 3;
#endif

	trace("Model_number: %d",model_number);

	if (!hSession->extended || hSession->oversize == CN || sscanf(hSession->oversize, "%dx%d%c", &ovc, &ovr, &junk) != 2)
	{
		ovc = 0;
		ovr = 0;
	}
	ctlr_set_rows_cols(hSession, model_number, ovc, ovr);

	if (hSession->termname != CN)
		hSession->termtype = hSession->termname;
	else
		hSession->termtype = hSession->full_model_name;

	trace("Termtype: %s",hSession->termtype);

	ctlr_reinit(hSession,MODEL_CHANGE);
	screen_update(hSession,0,hSession->rows*hSession->cols);

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
		popup_an_error(session,"Unknown model: %d - Defaulting to 4 (%dx%d)", mn, sz[idx].cols,sz[idx].rows);
		mn  = 4;
	}

	update_model_info(session,mn,sz[idx].cols,sz[idx].rows);

	// Apply oversize.
	session->ov_cols = 0;
	session->ov_rows = 0;
	if (ovc != 0 || ovr != 0)
	{
		if (ovc <= 0 || ovr <= 0)
			popup_an_error(session,"Invalid %s %dx%d:\nNegative or zero",ResOversize, ovc, ovr);
		else if (ovc * ovr >= 0x4000)
			popup_an_error(session,"Invalid %s %dx%d:\nToo big",ResOversize, ovc, ovr);
		else if (ovc > 0 && ovc < session->maxCOLS)
			popup_an_error(session,"Invalid %s cols (%d):\nLess than model %d cols (%d)",ResOversize, ovc, session->model_num, session->maxCOLS);
		else if (ovr > 0 && ovr < session->maxROWS)
			popup_an_error(session,"Invalid %s rows (%d):\nLess than model %d rows (%d)",ResOversize, ovr, session->model_num, session->maxROWS);
		else
			update_model_info(session,mn,session->ov_cols = ovc,session->ov_rows = ovr);
	}

	set_viewsize(session,sz[idx].rows,sz[idx].cols);

}

static void set_formatted(H3270 *hSession, int state)
{
	hSession->formatted = state;
}

/**
 * Update the formatted screen flag.
 *
 * A formatted screen is a screen that has at least one field somewhere on it.
 *
 * @param hSession	Session Handle
 */
static void update_formatted(H3270 *hSession)
{
	register int baddr;

	CHECK_SESSION_HANDLE(hSession);

	baddr = 0;
	do
	{
		if(hSession->ea_buf[baddr].fa)
		{
			set_formatted(hSession,1);
			return;
		}
		INC_BA(baddr);
	} while (baddr != 0);

	set_formatted(hSession,0);

}

/*
 * Called when a host is half connected.
 */
static void ctlr_half_connect(H3270 *hSession, int ignored unused, void *dunno)
{
	ticking_start(hSession,True);
}

/*
 * Called when a host connects, disconnects, or changes ANSI/3270 modes.
 */
static void ctlr_connect(H3270 *hSession, int ignored unused, void *dunno)
{
	ticking_stop(hSession);
	status_untiming(hSession);

	if (hSession->ever_3270)
		hSession->ea_buf[-1].fa = FA_PRINTABLE | FA_MODIFY;
	else
		hSession->ea_buf[-1].fa = FA_PRINTABLE | FA_PROTECT;

	if (!IN_3270 || (IN_SSCP && (hSession->kybdlock & KL_OIA_TWAIT)))
	{
		lib3270_kybdlock_clear(hSession,KL_OIA_TWAIT);
		status_reset(hSession);
	}

	hSession->default_fg = 0x00;
	hSession->default_bg = 0x00;
	hSession->default_gr = 0x00;
	hSession->default_cs = 0x00;
	hSession->default_ic = 0x00;
	hSession->reply_mode = SF_SRM_FIELD;
	hSession->crm_nattr = 0;
}

LIB3270_EXPORT int lib3270_get_field_start(H3270 *hSession, int baddr)
{
	int sbaddr;

	CHECK_SESSION_HANDLE(hSession);

	if (!hSession->formatted)
		return -1;

    if(baddr < 0)
        baddr = hSession->cursor_addr;

	sbaddr = baddr;
	do
	{
		if(hSession->ea_buf[baddr].fa)
			return baddr;
		DEC_BA(baddr);
	} while (baddr != sbaddr);

	return -1;

}

LIB3270_EXPORT int lib3270_get_field_len(H3270 *hSession, int baddr)
{
	int saddr;
	int addr;
	int width = 0;

	CHECK_SESSION_HANDLE(hSession);

	if (!hSession->formatted)
		return -1;

    if(baddr < 0)
        baddr = hSession->cursor_addr;

	addr = find_field_attribute(hSession,baddr);

	if(addr < 0)
		return -1;

	saddr = addr;
	INC_BA(addr);
	do
	{
		if(hSession->ea_buf[addr].fa)
			return width;
		INC_BA(addr);
		width++;
	} while (addr != saddr);

	return -1;
}

LIB3270_EXPORT int lib3270_field_addr(H3270 *hSession, int baddr)
{
	int sbaddr;

	CHECK_SESSION_HANDLE(hSession);

	if (!hSession->formatted)
		return -1;

	sbaddr = baddr;
	do
	{
		if(hSession->ea_buf[baddr].fa)
			return baddr;
		DEC_BA(baddr);
	} while (baddr != sbaddr);

	return -1;
}

LIB3270_EXPORT int lib3270_field_attribute(H3270 *hSession, int baddr)
{
	int sbaddr;

	CHECK_SESSION_HANDLE(hSession);

	if (!hSession->formatted)
		return -1;

	sbaddr = baddr;
	do
	{
		if(hSession->ea_buf[baddr].fa)
			return hSession->ea_buf[baddr].fa;
		DEC_BA(baddr);
	} while (baddr != sbaddr);

	return -1;
}


/*
 * Get Field width
 */
int lib3270_field_length(H3270 *hSession, int baddr)
{
	int saddr;
	int addr;
	int width = 0;

	CHECK_SESSION_HANDLE(hSession);

	addr = find_field_attribute(hSession,baddr);

	if(addr < 0)
		return -1;

	saddr = addr;
	INC_BA(addr);
	do
	{
		if(hSession->ea_buf[addr].fa)
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
unsigned char get_field_attribute(H3270 *hSession, int baddr)
{
	return hSession->ea_buf[find_field_attribute(hSession,baddr)].fa;
}

LIB3270_EXPORT int lib3270_get_next_unprotected(H3270 *hSession, int baddr0)
{
	register int baddr, nbaddr;

    CHECK_SESSION_HANDLE(hSession);

    if(baddr0 < 0)
        baddr0 = hSession->cursor_addr;

	nbaddr = baddr0;
	do
	{
		baddr = nbaddr;
		INC_BA(nbaddr);
		if(hSession->ea_buf[baddr].fa &&!FA_IS_PROTECTED(hSession->ea_buf[baddr].fa) &&!hSession->ea_buf[nbaddr].fa)
			return nbaddr;
	} while (nbaddr != baddr0);

	return 0;
}

LIB3270_EXPORT int lib3270_get_is_protected_at(H3270 *h, int row, int col) {
	CHECK_SESSION_HANDLE(h);
	return lib3270_get_is_protected(h, ((row-1) * h->cols) + (col-1));
}

LIB3270_EXPORT int lib3270_get_is_protected(H3270 *hSession, int baddr)
{
    CHECK_SESSION_HANDLE(hSession);

    if(baddr < 0)
        baddr = hSession->cursor_addr;

	int faddr = find_field_attribute(hSession,baddr);

	return FA_IS_PROTECTED(hSession->ea_buf[faddr].fa) ? 1 : 0;
}


/**
 * Perform an erase command, which may include changing the (virtual) screen size.
 *
 */
void ctlr_erase(H3270 *session, int alt)
{
	CHECK_SESSION_HANDLE(session);

	kybd_inhibit(session,False);
	ctlr_clear(session,True);
	session->cbk.erase(session);

	if(alt == session->screen_alt)
		return;

	if (alt)
	{
		/* Going from 24x80 to maximum. */
		session->cbk.display(session);

		set_viewsize(session,session->maxROWS,session->maxCOLS);
	}
	else
	{
		/* Going from maximum to 24x80. */
		if (session->maxROWS > 24 || session->maxCOLS > 80)
		{
			if(session->vcontrol)
			{
				ctlr_blanks(session);
				session->cbk.display(session);
			}

			if(lib3270_get_toggle(session,LIB3270_TOGGLE_ALTSCREEN))
				set_viewsize(session,24,80);
			else
				set_viewsize(session,session->maxROWS,80);
		}
	}

	session->screen_alt = alt;

}

/*
 * Interpret an incoming 3270 command.
 */
enum pds process_ds(H3270 *hSession, unsigned char *buf, int buflen)
{
	enum pds rv;

	if (!buflen)
		return PDS_OKAY_NO_OUTPUT;

	trace_ds(hSession,"< ");

	switch (buf[0]) /* 3270 command */
	{
	case CMD_EAU:	/* erase all unprotected */
	case SNA_CMD_EAU:
		trace_ds(hSession, "EraseAllUnprotected\n");
		ctlr_erase_all_unprotected(hSession);
		return PDS_OKAY_NO_OUTPUT;
		break;

	case CMD_EWA:	/* erase/write alternate */
	case SNA_CMD_EWA:
		trace_ds(hSession,"EraseWriteAlternate");
		ctlr_erase(hSession,1);
		if ((rv = ctlr_write(hSession,buf, buflen, True)) < 0)
			return rv;
		return PDS_OKAY_NO_OUTPUT;
		break;

	case CMD_EW:	/* erase/write */
	case SNA_CMD_EW:
		trace_ds(hSession,"EraseWrite");
		ctlr_erase(hSession,0);
		if ((rv = ctlr_write(hSession,buf, buflen, True)) < 0)
			return rv;
		return PDS_OKAY_NO_OUTPUT;
		break;

	case CMD_W:	/* write */
	case SNA_CMD_W:
		trace_ds(hSession,"Write");
		if ((rv = ctlr_write(hSession,buf, buflen, False)) < 0)
			return rv;
		return PDS_OKAY_NO_OUTPUT;
		break;

	case CMD_RB:	/* read buffer */
	case SNA_CMD_RB:
		trace_ds(hSession,"ReadBuffer\n");
		ctlr_read_buffer(hSession,hSession->aid);
		return PDS_OKAY_OUTPUT;
		break;

	case CMD_RM:	/* read modifed */
	case SNA_CMD_RM:
		trace_ds(hSession,"ReadModified\n");
		ctlr_read_modified(hSession, hSession->aid, False);
		return PDS_OKAY_OUTPUT;
		break;

	case CMD_RMA:	/* read modifed all */
	case SNA_CMD_RMA:
		trace_ds(hSession,"ReadModifiedAll\n");
		ctlr_read_modified(hSession, hSession->aid, True);
		return PDS_OKAY_OUTPUT;
		break;

	case CMD_WSF:	/* write structured field */
	case SNA_CMD_WSF:
		trace_ds(hSession,"WriteStructuredField");
		return write_structured_field(hSession,buf, buflen);
		break;

	case CMD_NOP:	/* no-op */
		trace_ds(hSession,"NoOp\n");
		return PDS_OKAY_NO_OUTPUT;
		break;

	default:
		/* unknown 3270 command */
		popup_an_error(hSession,_( "Unknown 3270 Data Stream command: 0x%X" ),buf[0]);
		return PDS_BAD_CMD;
	}
}

/*
 * Functions to insert SA attributes into the inbound data stream.
 */
static void insert_sa1(H3270 *hSession, unsigned char attr, unsigned char value, unsigned char *currentp, Boolean *anyp)
{
	if (value == *currentp)
		return;
	*currentp = value;
	space3270out(hSession,3);
	*hSession->obptr++ = ORDER_SA;
	*hSession->obptr++ = attr;
	*hSession->obptr++ = value;
	if (*anyp)
		trace_ds(hSession,"'");
	trace_ds(hSession, " SetAttribute(%s)", see_efa(attr, value));
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

static void insert_sa(H3270 *hSession, int baddr, unsigned char *current_fgp, unsigned char *current_bgp,unsigned char *current_grp, unsigned char *current_csp, Boolean *anyp)
{
	if (hSession->reply_mode != SF_SRM_CHAR)
		return;

	if (memchr((char *) hSession->crm_attr, XA_FOREGROUND, hSession->crm_nattr))
		insert_sa1(hSession, XA_FOREGROUND, hSession->ea_buf[baddr].fg, current_fgp, anyp);

	if (memchr((char *) hSession->crm_attr, XA_BACKGROUND, hSession->crm_nattr))
		insert_sa1(hSession, XA_BACKGROUND, hSession->ea_buf[baddr].bg, current_bgp, anyp);

	if (memchr((char *) hSession->crm_attr, XA_HIGHLIGHTING, hSession->crm_nattr))
	{
		unsigned char gr;

		gr = hSession->ea_buf[baddr].gr;
		if (gr)
			gr |= 0xf0;
		insert_sa1(hSession, XA_HIGHLIGHTING, gr, current_grp, anyp);
	}

	if (memchr((char *) hSession->crm_attr, XA_CHARSET, hSession->crm_nattr))
	{
		insert_sa1(hSession, XA_CHARSET, host_cs(hSession->ea_buf[baddr].cs), current_csp,anyp);
	}
}


/**
 * Process a 3270 Read-Modified command and transmit the data back to the host.
 */
void ctlr_read_modified(H3270 *hSession, unsigned char aid_byte, Boolean all)
{
	register int	baddr, sbaddr;
	Boolean			send_data = True;
	Boolean			short_read = False;
	unsigned char	current_fg = 0x00;
	unsigned char	current_bg = 0x00;
	unsigned char	current_gr = 0x00;
	unsigned char	current_cs = 0x00;

	if (IN_SSCP && aid_byte != AID_ENTER)
		return;

#if defined(X3270_FT) /*[*/
	if (aid_byte == AID_SF)
	{
		dft_read_modified(hSession);
		return;
	}
#endif /*]*/

	trace_ds(hSession,"> ");
	hSession->obptr = hSession->obuf;

	switch (aid_byte)
	{
	case AID_SYSREQ:				/* test request */
		space3270out(hSession,4);
		*hSession->obptr++ = 0x01;	/* soh */
		*hSession->obptr++ = 0x5b;	/*  %  */
		*hSession->obptr++ = 0x61;	/*  /  */
		*hSession->obptr++ = 0x02;	/* stx */
		trace_ds(hSession,"SYSREQ");
		break;

	case AID_PA1:					/* short-read AIDs */
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
		if (!IN_SSCP)
		{
			space3270out(hSession,3);
			*hSession->obptr++ = aid_byte;
			trace_ds(hSession,"%s",see_aid(aid_byte));

			if (short_read)
				goto rm_done;

			ENCODE_BADDR(hSession->obptr, hSession->cursor_addr);
			trace_ds(hSession,"%s",rcba(hSession,hSession->cursor_addr));
		}
		else
		{
			space3270out(hSession,1);	/* just in case */
		}
		break;
	}

	baddr = 0;
	if (hSession->formatted)
	{
		/* find first field attribute */
		do
		{
			if (hSession->ea_buf[baddr].fa)
				break;
			INC_BA(baddr);
		} while (baddr != 0);

		sbaddr = baddr;
		do
		{
			if (FA_IS_MODIFIED(hSession->ea_buf[baddr].fa))
			{
				Boolean	any = False;

				INC_BA(baddr);
				space3270out(hSession,3);
				*hSession->obptr++ = ORDER_SBA;
				ENCODE_BADDR(hSession->obptr, baddr);
				trace_ds(hSession," SetBufferAddress%s (Cols: %d Rows: %d)", rcba(hSession,baddr), hSession->cols, hSession->rows);
				while (!hSession->ea_buf[baddr].fa)
				{

					if (send_data && hSession->ea_buf[baddr].cc)
					{
						insert_sa(hSession,baddr,&current_fg,&current_bg,&current_gr,&current_cs,&any);
						if (hSession->ea_buf[baddr].cs & CS_GE)
						{
							space3270out(hSession,1);
							*hSession->obptr++ = ORDER_GE;
							if (any)
								trace_ds(hSession,"'");
							trace_ds(hSession," GraphicEscape");
							any = False;
						}
						space3270out(hSession,1);
						*hSession->obptr++ = hSession->ea_buf[baddr].cc;
						if (!any)
							trace_ds(hSession," '");

						trace_ds(hSession,"%s",see_ebc(hSession->ea_buf[baddr].cc));
						any = True;
					}
					INC_BA(baddr);
				}
				if (any)
					trace_ds(hSession,"'");
			}
			else
			{	/* not modified - skip */
				do
				{
					INC_BA(baddr);
				} while (!hSession->ea_buf[baddr].fa);
			}
		} while (baddr != sbaddr);

	}
	else
	{
		Boolean	any = False;
		int nbytes = 0;

		/*
		 * If we're in SSCP-LU mode, the starting point is where the
		 * host left the cursor.
		 */
		if (IN_SSCP)
			baddr = hSession->sscp_start;

		do
		{
			if (hSession->ea_buf[baddr].cc)
			{
				insert_sa(hSession,baddr,&current_fg,&current_bg,&current_gr,&current_cs,&any);
				if (hSession->ea_buf[baddr].cs & CS_GE)
				{
					space3270out(hSession,1);
					*hSession->obptr++ = ORDER_GE;
					if (any)
						trace_ds(hSession,"' ");
					trace_ds(hSession," GraphicEscape ");
					any = False;
				}

				space3270out(hSession,1);
				*hSession->obptr++ = hSession->ea_buf[baddr].cc;
				if (!any)
					trace_ds(hSession,"%s","'");
				trace_ds(hSession,"%s",see_ebc(hSession->ea_buf[baddr].cc));
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
			trace_ds(hSession,"'");
	}

    rm_done:
	trace_ds(hSession,"\n");
	net_output(hSession);
}

/*
 * Process a 3270 Read-Buffer command and transmit the data back to the
 * host.
 */
void ctlr_read_buffer(H3270 *hSession, unsigned char aid_byte)
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
	if (aid_byte == AID_SF)
	{
		dft_read_modified(hSession);
		return;
	}
#endif /*]*/

	trace_ds(hSession,"> ");
	hSession->obptr = hSession->obuf;

	space3270out(hSession,3);
	*hSession->obptr++ = aid_byte;
	ENCODE_BADDR(hSession->obptr, hSession->cursor_addr);
	trace_ds(hSession,"%s%s", see_aid(aid_byte), rcba(hSession,hSession->cursor_addr));

	baddr = 0;
	do {
		if (hSession->ea_buf[baddr].fa)
		{
			if (hSession->reply_mode == SF_SRM_FIELD)
			{
				space3270out(hSession,2);
				*hSession->obptr++ = ORDER_SF;
			}
			else
			{
				space3270out(hSession,4);
				*hSession->obptr++ = ORDER_SFE;
				attr_count = hSession->obptr - hSession->obuf;
				*hSession->obptr++ = 1; /* for now */
				*hSession->obptr++ = XA_3270;
			}
			fa = hSession->ea_buf[baddr].fa & ~FA_PRINTABLE;
			*hSession->obptr++ = code_table[fa];

			if (any)
				trace_ds(hSession,"'");
			trace_ds(hSession," StartField%s%s%s",
			    (hSession->reply_mode == SF_SRM_FIELD) ? "" : "Extended",
			    rcba(hSession,baddr), see_attr(fa));

			if (hSession->reply_mode != SF_SRM_FIELD)
			{
				if (hSession->ea_buf[baddr].fg) {
					space3270out(hSession,2);
					*hSession->obptr++ = XA_FOREGROUND;
					*hSession->obptr++ = hSession->ea_buf[baddr].fg;
					trace_ds(hSession,"%s", see_efa(XA_FOREGROUND, hSession->ea_buf[baddr].fg));
					(*(hSession->obuf + attr_count))++;
				}
				if (hSession->ea_buf[baddr].bg) {
					space3270out(hSession,2);
					*hSession->obptr++ = XA_BACKGROUND;
					*hSession->obptr++ = hSession->ea_buf[baddr].bg;
					trace_ds(hSession,"%s", see_efa(XA_BACKGROUND, hSession->ea_buf[baddr].bg));
					(*(hSession->obuf + attr_count))++;
				}
				if (hSession->ea_buf[baddr].gr) {
					space3270out(hSession,2);
					*hSession->obptr++ = XA_HIGHLIGHTING;
					*hSession->obptr++ = hSession->ea_buf[baddr].gr | 0xf0;
					trace_ds(hSession,"%s", see_efa(XA_HIGHLIGHTING,
					    hSession->ea_buf[baddr].gr | 0xf0));
					(*(hSession->obuf + attr_count))++;
				}
				if (hSession->ea_buf[baddr].cs & CS_MASK) {
					space3270out(hSession,2);
					*hSession->obptr++ = XA_CHARSET;
					*hSession->obptr++ = host_cs(hSession->ea_buf[baddr].cs);
					trace_ds(hSession,"%s", see_efa(XA_CHARSET,host_cs(hSession->ea_buf[baddr].cs)));
					(*(hSession->obuf + attr_count))++;
				}
			}
			any = False;
		} else {
			insert_sa(hSession,baddr,&current_fg,&current_bg,&current_gr,&current_cs,&any);
			if (hSession->ea_buf[baddr].cs & CS_GE) {
				space3270out(hSession,1);
				*hSession->obptr++ = ORDER_GE;
				if (any)
					trace_ds(hSession,"'");
				trace_ds(hSession," GraphicEscape");
				any = False;
			}
			space3270out(hSession,1);
			*hSession->obptr++ = hSession->ea_buf[baddr].cc;
			if (hSession->ea_buf[baddr].cc <= 0x3f ||
			    hSession->ea_buf[baddr].cc == 0xff) {
				if (any)
					trace_ds(hSession,"'");

				trace_ds(hSession," %s", see_ebc(hSession->ea_buf[baddr].cc));
				any = False;
			} else {
				if (!any)
					trace_ds(hSession," '");
				trace_ds(hSession,"%s", see_ebc(hSession->ea_buf[baddr].cc));
				any = True;
			}
		}
		INC_BA(baddr);
	} while (baddr != 0);
	if (any)
		trace_ds(hSession,"'");

	trace_ds(hSession,"\n");
	net_output(hSession);
}

/*
 * Process a 3270 Erase All Unprotected command.
 */
void ctlr_erase_all_unprotected(H3270 *hSession)
{
	register int	baddr, sbaddr;
	unsigned char	fa;
	Boolean		f;

	kybd_inhibit(hSession,False);

	if (hSession->formatted)
	{
		/* find first field attribute */
		baddr = 0;
		do {
			if (hSession->ea_buf[baddr].fa)
				break;
			INC_BA(baddr);
		} while (baddr != 0);
		sbaddr = baddr;
		f = False;
		do {
			fa = hSession->ea_buf[baddr].fa;
			if (!FA_IS_PROTECTED(fa)) {
				mdt_clear(hSession,baddr);
				do {
					INC_BA(baddr);
					if (!f) {
						cursor_move(hSession,baddr);
						f = True;
					}
					if (!hSession->ea_buf[baddr].fa) {
						ctlr_add(hSession,baddr, EBC_null, 0);
					}
				} while (!hSession->ea_buf[baddr].fa);
			}
			else {
				do {
					INC_BA(baddr);
				} while (!hSession->ea_buf[baddr].fa);
			}
		} while (baddr != sbaddr);
		if (!f)
			cursor_move(hSession,0);
	} else {
		ctlr_clear(hSession,True);
	}
	hSession->aid = AID_NO;
	do_reset(hSession,False);
	ALL_CHANGED(hSession);
}



/*
 * Process a 3270 Write command.
 */
enum pds ctlr_write(H3270 *hSession, unsigned char buf[], int buflen, Boolean erase)
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

#define END_TEXT0		{ if (previous == TEXT) trace_ds(hSession,"'"); }
#define END_TEXT(cmd)	{ END_TEXT0; trace_ds(hSession," %s", cmd); }

/* XXX: Should there be a ctlr_add_cs call here? */
#define START_FIELD(fa) { \
			current_fa = fa; \
			ctlr_add_fa(hSession,hSession->buffer_addr, fa, 0); \
			ctlr_add_cs(hSession,hSession->buffer_addr, 0); \
			ctlr_add_fg(hSession,hSession->buffer_addr, 0); \
			ctlr_add_bg(hSession,hSession->buffer_addr, 0); \
			ctlr_add_gr(hSession,hSession->buffer_addr, 0); \
			ctlr_add_ic(hSession,hSession->buffer_addr, 0); \
			trace_ds(hSession,"%s",see_attr(fa)); \
			set_formatted(hSession,1); \
		}

	kybd_inhibit(hSession,False);

	if (buflen < 2)
		return PDS_BAD_CMD;

	hSession->default_fg = 0;
	hSession->default_bg = 0;
	hSession->default_gr = 0;
	hSession->default_cs = 0;
	hSession->default_ic = 0;

	hSession->trace_primed = 1;
	hSession->buffer_addr = hSession->cursor_addr;
	if (WCC_RESET(buf[1]))
	{
		if (erase)
			hSession->reply_mode = SF_SRM_FIELD;
		trace_ds(hSession,"%sreset", paren);
		paren = ",";
	}
	wcc_sound_alarm = WCC_SOUND_ALARM(buf[1]);
	if (wcc_sound_alarm)
	{
		trace_ds(hSession,"%salarm", paren);
		paren = ",";
	}
	wcc_keyboard_restore = WCC_KEYBOARD_RESTORE(buf[1]);
	if (wcc_keyboard_restore)
		ticking_stop(hSession);

	if (wcc_keyboard_restore)
	{
		trace_ds(hSession,"%srestore", paren);
		paren = ",";
	}

	if (WCC_RESET_MDT(buf[1]))
	{
		trace_ds(hSession,"%sresetMDT", paren);
		paren = ",";
		baddr = 0;
		if (hSession->modified_sel)
			ALL_CHANGED(hSession);
		do
		{
			if (hSession->ea_buf[baddr].fa)
			{
				mdt_clear(hSession,baddr);
			}
			INC_BA(baddr);
		} while (baddr != 0);
	}
	if (strcmp(paren, "("))
		trace_ds(hSession,")");

	last_cmd = True;
	last_zpt = False;
	current_fa = get_field_attribute(hSession,hSession->buffer_addr);

#define ABORT_WRITEx { \
	rv = PDS_BAD_ADDR; \
	aborted = True; \
	break; \
}
#define ABORT_WRITE(s) { \
	trace_ds(hSession," [" s "; write aborted]\n"); \
	ABORT_WRITEx; \
} \

	for (cp = &buf[2]; !aborted && cp < (buf + buflen); cp++)
	{
		switch (*cp)
		{
		case ORDER_SF:	/* start field */
			END_TEXT("StartField");
			if (previous != SBA)
				trace_ds(hSession,"%s",rcba(hSession,hSession->buffer_addr));
			previous = ORDER;
			cp++;		/* skip field attribute */
			START_FIELD(*cp);
			ctlr_add_fg(hSession,hSession->buffer_addr, 0);
			ctlr_add_bg(hSession,hSession->buffer_addr, 0);
			INC_BA(hSession->buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;

		case ORDER_SBA:	/* set buffer address */
			cp += 2;	/* skip buffer address */
			hSession->buffer_addr = DECODE_BADDR(*(cp-1), *cp);
			END_TEXT("SetBufferAddress");
			previous = SBA;
			trace_ds(hSession,"%s",rcba(hSession,hSession->buffer_addr));
			if(hSession->buffer_addr >= hSession->cols * hSession->rows)
			{
				ABORT_WRITE("invalid SBA address");
			}
			current_fa = get_field_attribute(hSession,hSession->buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;

		case ORDER_IC:	/* insert cursor */
			END_TEXT("InsertCursor");
			if (previous != SBA)
				trace_ds(hSession,"%s",rcba(hSession,hSession->buffer_addr));
			previous = ORDER;
			cursor_move(hSession,hSession->buffer_addr);
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
			if (hSession->ea_buf[hSession->buffer_addr].fa && !FA_IS_PROTECTED(hSession->ea_buf[hSession->buffer_addr].fa))
			{
				INC_BA(hSession->buffer_addr);
				last_zpt = False;
				last_cmd = True;
				break;
			}
			/*
			 * Otherwise, advance to the first position of the
			 * next unprotected field.
			 */
			baddr = next_unprotected(hSession,hSession->buffer_addr);
			if (baddr < hSession->buffer_addr)
				baddr = 0;
			/*
			 * Null out the remainder of the current field -- even
			 * if protected -- if the PT doesn't follow a command
			 * or order, or (honestly) if the last order we saw was
			 * a null-filling PT that left the buffer address at 0.
			 * XXX: There's some funky DBCS rule here.
			 */
			if (!last_cmd || last_zpt)
			{
				trace_ds(hSession,"(nulling)");

				while((hSession->buffer_addr != baddr) && (!hSession->ea_buf[hSession->buffer_addr].fa))
				{
					ctlr_add(hSession,hSession->buffer_addr, EBC_null, 0);
					ctlr_add_cs(hSession,hSession->buffer_addr, 0);
					ctlr_add_fg(hSession,hSession->buffer_addr, 0);
					ctlr_add_bg(hSession,hSession->buffer_addr, 0);
					ctlr_add_gr(hSession,hSession->buffer_addr, 0);
					ctlr_add_ic(hSession,hSession->buffer_addr, 0);
					INC_BA(hSession->buffer_addr);
				}
				if (baddr == 0)
					last_zpt = True;
			}
			else
				last_zpt = False;

			hSession->buffer_addr = baddr;
			last_cmd = True;
			break;

		case ORDER_RA:	/* repeat to address */
			END_TEXT("RepeatToAddress");
			cp += 2;	/* skip buffer address */
			baddr = DECODE_BADDR(*(cp-1), *cp);
			trace_ds(hSession,"%s",rcba(hSession,baddr));
			cp++;		/* skip char to repeat */
			add_dbcs = False;
			ra_ge = False;
			previous = ORDER;
#if defined(X3270_DBCS) /*[*/
			if (dbcs)
			{
				d = ctlr_lookleft_state(buffer_addr, &why);
				if (d == DBCS_RIGHT)
				{
					ABORT_WRITE("RA over right half of DBCS character");
				}
				if (default_cs == CS_DBCS || d == DBCS_LEFT)
				{
					add_dbcs = True;
				}
			}
			if (add_dbcs)
			{
				if ((baddr - buffer_addr) % 2)
				{
					ABORT_WRITE("DBCS RA with odd length");
				}
				add_c1 = *cp;
				cp++;
				if (cp >= buf + buflen)
				{
					ABORT_WRITE("missing second half of DBCS character");
				}
				add_c2 = *cp;
				if (add_c1 == EBC_null)
				{
					switch (add_c2)
					{
					case EBC_null:
					case EBC_nl:
					case EBC_em:
					case EBC_ff:
					case EBC_cr:
					case EBC_dup:
					case EBC_fm:
						break;

					default:
						trace_ds(hSession," [invalid DBCS RA control character X'%02x%02x'; write aborted]",add_c1, add_c2);
						ABORT_WRITEx;
					}
				}
				else if (add_c1 < 0x40 || add_c1 > 0xfe || add_c2 < 0x40 || add_c2 > 0xfe)
				{
					trace_ds(hSession," [invalid DBCS RA character X'%02x%02x'; write aborted]",add_c1, add_c2);
					ABORT_WRITEx;
				}
				dbcs_to_mb(add_c1, add_c2, mb);
				trace_ds_nb(hSession,"'%s'", mb);
			}
			else
#endif /*]*/
			{
				if (*cp == ORDER_GE)
				{
					ra_ge = True;
					trace_ds(hSession,"GraphicEscape");
					cp++;
				}
				add_c1 = *cp;
				if (add_c1)
					trace_ds(hSession,"'");

				trace_ds(hSession,"%s", see_ebc(add_c1));
				if (add_c1)
					trace_ds(hSession,"'");

			}
			if (baddr >= hSession->cols * hSession->rows)
			{
				ABORT_WRITE("invalid RA address");
			}
			do
			{
				if (add_dbcs)
				{
					ctlr_add(hSession,hSession->buffer_addr, add_c1,hSession->default_cs);
				}
				else
				{
					if (ra_ge)
						ctlr_add(hSession,hSession->buffer_addr, add_c1,CS_GE);
					else if (hSession->default_cs)
						ctlr_add(hSession,hSession->buffer_addr, add_c1,hSession->default_cs);
					else
						ctlr_add(hSession,hSession->buffer_addr, add_c1,0);
				}
				ctlr_add_fg(hSession,hSession->buffer_addr, hSession->default_fg);
				ctlr_add_gr(hSession,hSession->buffer_addr, hSession->default_gr);
				ctlr_add_ic(hSession,hSession->buffer_addr, hSession->default_ic);

				INC_BA(hSession->buffer_addr);
				if (add_dbcs)
				{
					ctlr_add(hSession,hSession->buffer_addr, add_c2,hSession->default_cs);
					ctlr_add_fg(hSession,hSession->buffer_addr, hSession->default_fg);
					ctlr_add_bg(hSession,hSession->buffer_addr, hSession->default_bg);
					ctlr_add_gr(hSession,hSession->buffer_addr, hSession->default_gr);
					ctlr_add_ic(hSession,hSession->buffer_addr, hSession->default_ic);
					INC_BA(hSession->buffer_addr);
				}
			} while (hSession->buffer_addr != baddr);

			current_fa = get_field_attribute(hSession,hSession->buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;

		case ORDER_EUA:	/* erase unprotected to address */
			cp += 2;	/* skip buffer address */
			baddr = DECODE_BADDR(*(cp-1), *cp);
			END_TEXT("EraseUnprotectedAll");
			if (previous != SBA)
				trace_ds(hSession,"%s",rcba(hSession,baddr));

			previous = ORDER;
			if (baddr >= hSession->cols * hSession->rows)
			{
				ABORT_WRITE("invalid EUA address");
			}
			d = ctlr_lookleft_state(buffer_addr, &why);
			if (d == DBCS_RIGHT)
			{
				ABORT_WRITE("EUA overwriting right half of DBCS character");
			}
			d = ctlr_lookleft_state(baddr, &why);
			if (d == DBCS_LEFT)
			{
				ABORT_WRITE("EUA overwriting left half of DBCS character");
			}
			do
			{
				if (hSession->ea_buf[hSession->buffer_addr].fa)
					current_fa = hSession->ea_buf[hSession->buffer_addr].fa;
				else if (!FA_IS_PROTECTED(current_fa))
				{
					ctlr_add(hSession,hSession->buffer_addr, EBC_null,CS_BASE);
				}
				INC_BA(hSession->buffer_addr);
			} while (hSession->buffer_addr != baddr);
			current_fa = get_field_attribute(hSession,hSession->buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;

		case ORDER_GE:	/* graphic escape */
			/* XXX: DBCS? */
			END_TEXT("GraphicEscape ");
			cp++;		/* skip char */
			previous = ORDER;
			if (*cp)
				trace_ds(hSession,"'");
			trace_ds(hSession,"%s", see_ebc(*cp));
			if (*cp)
				trace_ds(hSession,"'");

			ctlr_add(hSession,hSession->buffer_addr, *cp, CS_GE);
			ctlr_add_fg(hSession,hSession->buffer_addr, hSession->default_fg);
			ctlr_add_bg(hSession,hSession->buffer_addr, hSession->default_bg);
			ctlr_add_gr(hSession,hSession->buffer_addr, hSession->default_gr);
			ctlr_add_ic(hSession,hSession->buffer_addr, hSession->default_ic);
			INC_BA(hSession->buffer_addr);

			current_fa = get_field_attribute(hSession,hSession->buffer_addr);
			last_cmd = False;
			last_zpt = False;
			break;

		case ORDER_MF:	/* modify field */
			END_TEXT("ModifyField");
			if (previous != SBA)
				trace_ds(hSession,"%s",rcba(hSession,hSession->buffer_addr));
			previous = ORDER;
			cp++;
			na = *cp;
			if (hSession->ea_buf[hSession->buffer_addr].fa)
			{
				for (i = 0; i < (int)na; i++)
				{
					cp++;
					if (*cp == XA_3270)
					{
						trace_ds(hSession," 3270");
						cp++;
						ctlr_add_fa(hSession,hSession->buffer_addr, *cp,hSession->ea_buf[hSession->buffer_addr].cs);
						trace_ds(hSession,"%s",see_attr(*cp));
					}
					else if (*cp == XA_FOREGROUND)
					{
						trace_ds(hSession,"%s",see_efa(*cp,*(cp + 1)));
						cp++;
						if (hSession->m3279)
							ctlr_add_fg(hSession,hSession->buffer_addr, *cp);
					}
					else if (*cp == XA_BACKGROUND)
					{
						trace_ds(hSession,"%s",see_efa(*cp,*(cp + 1)));
						cp++;
						if (hSession->m3279)
							ctlr_add_bg(hSession,hSession->buffer_addr, *cp);
					}
					else if (*cp == XA_HIGHLIGHTING)
					{
						trace_ds(hSession,"%s",see_efa(*cp,*(cp + 1)));
						cp++;
						ctlr_add_gr(hSession,hSession->buffer_addr, *cp & 0x0f);
					}
					else if (*cp == XA_CHARSET)
					{
						int cs = 0;

						trace_ds(hSession,"%s",see_efa(*cp,*(cp + 1)));
						cp++;
						if (*cp == 0xf1)
							cs = CS_APL;
						else if (*cp == 0xf8)
							cs = CS_DBCS;
						ctlr_add_cs(hSession,hSession->buffer_addr, cs);
					}
					else if (*cp == XA_ALL)
					{
						trace_ds(hSession,"%s",see_efa(*cp,*(cp + 1)));
						cp++;
					}
					else if (*cp == XA_INPUT_CONTROL)
					{
						trace_ds(hSession,"%s",see_efa(*cp,*(cp + 1)));
						ctlr_add_ic(hSession,hSession->buffer_addr,(*(cp + 1) == 1));
						cp++;
					}
					else
					{
						trace_ds(hSession,"%s[unsupported]", see_efa(*cp, *(cp + 1)));
						cp++;
					}
				}
				INC_BA(hSession->buffer_addr);
			}
			else
				cp += na * 2;

			last_cmd = True;
			last_zpt = False;
			break;

		case ORDER_SFE:	/* start field extended */
			END_TEXT("StartFieldExtended");
			if (previous != SBA)
				trace_ds(hSession,"%s",rcba(hSession,hSession->buffer_addr));
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
					trace_ds(hSession," 3270");
					cp++;
					START_FIELD(*cp);
					any_fa++;
				} else if (*cp == XA_FOREGROUND) {
					trace_ds(hSession,"%s", see_efa(*cp, *(cp + 1)));
					cp++;
					if (hSession->m3279)
						efa_fg = *cp;
				} else if (*cp == XA_BACKGROUND) {
					trace_ds(hSession,"%s", see_efa(*cp, *(cp + 1)));
					cp++;
					if (hSession->m3279)
						efa_bg = *cp;
				} else if (*cp == XA_HIGHLIGHTING) {
					trace_ds(hSession,"%s", see_efa(*cp, *(cp + 1)));
					cp++;
					efa_gr = *cp & 0x07;
				} else if (*cp == XA_CHARSET) {
					trace_ds(hSession,"%s", see_efa(*cp, *(cp + 1)));
					cp++;
					if (*cp == 0xf1)
						efa_cs = CS_APL;
					else if (hSession->dbcs && (*cp == 0xf8))
						efa_cs = CS_DBCS;
					else
						efa_cs = CS_BASE;
				} else if (*cp == XA_ALL) {
					trace_ds(hSession,"%s", see_efa(*cp, *(cp + 1)));
					cp++;
				} else if (*cp == XA_INPUT_CONTROL) {
					trace_ds(hSession,"%s", see_efa(*cp, *(cp + 1)));
					if (hSession->dbcs)
					    efa_ic = (*(cp + 1) == 1);
					cp++;
				} else {
					trace_ds(hSession,"%s[unsupported]", see_efa(*cp, *(cp + 1)));
					cp++;
				}
			}
			if (!any_fa)
				START_FIELD(0);
			ctlr_add_cs(hSession,hSession->buffer_addr, efa_cs);
			ctlr_add_fg(hSession,hSession->buffer_addr, efa_fg);
			ctlr_add_bg(hSession,hSession->buffer_addr, efa_bg);
			ctlr_add_gr(hSession,hSession->buffer_addr, efa_gr);
			ctlr_add_ic(hSession,hSession->buffer_addr, efa_ic);
			INC_BA(hSession->buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;
		case ORDER_SA:	/* set attribute */
			END_TEXT("SetAttribute");
			previous = ORDER;
			cp++;
			if (*cp == XA_FOREGROUND)  {
				trace_ds(hSession,"%s", see_efa(*cp, *(cp + 1)));
				if (hSession->m3279)
					hSession->default_fg = *(cp + 1);
			} else if (*cp == XA_BACKGROUND)  {
				trace_ds(hSession,"%s", see_efa(*cp, *(cp + 1)));
				if (hSession->m3279)
					hSession->default_bg = *(cp + 1);
			} else if (*cp == XA_HIGHLIGHTING)  {
				trace_ds(hSession,"%s", see_efa(*cp, *(cp + 1)));
				hSession->default_gr = *(cp + 1) & 0x0f;
			} else if (*cp == XA_ALL)  {
				trace_ds(hSession,"%s", see_efa(*cp, *(cp + 1)));
				hSession->default_fg = 0;
				hSession->default_bg = 0;
				hSession->default_gr = 0;
				hSession->default_cs = 0;
				hSession->default_ic = 0;
			} else if (*cp == XA_CHARSET) {
				trace_ds(hSession,"%s", see_efa(*cp, *(cp + 1)));
				switch (*(cp + 1)) {
				case 0xf1:
				    hSession->default_cs = CS_APL;
				    break;
				case 0xf8:
				    hSession->default_cs = CS_DBCS;
				    break;
				default:
				    hSession->default_cs = CS_BASE;
				    break;
				}
			} else if (*cp == XA_INPUT_CONTROL) {
				trace_ds(hSession,"%s", see_efa(*cp, *(cp + 1)));
				if (*(cp + 1) == 1)
					hSession->default_ic = 1;
				else
					hSession->default_ic = 0;
			} else
				trace_ds(hSession,"%s[unsupported]",see_efa(*cp, *(cp + 1)));
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
			if (hSession->default_cs == CS_DBCS || d != DBCS_NONE) {
				ABORT_WRITE("invalid format control order in DBCS field");
			}
			ctlr_add(hSession,hSession->buffer_addr, *cp, hSession->default_cs);
			ctlr_add_fg(hSession,hSession->buffer_addr, hSession->default_fg);
			ctlr_add_bg(hSession,hSession->buffer_addr, hSession->default_bg);
			ctlr_add_gr(hSession,hSession->buffer_addr, hSession->default_gr);
			ctlr_add_ic(hSession,hSession->buffer_addr, hSession->default_ic);
			INC_BA(hSession->buffer_addr);
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
			ctlr_add(hSession,hSession->buffer_addr, *cp, hSession->default_cs);
			ctlr_add_fg(hSession,hSession->buffer_addr, hSession->default_fg);
			ctlr_add_bg(hSession,hSession->buffer_addr, hSession->default_bg);
			ctlr_add_gr(hSession,hSession->buffer_addr, hSession->default_gr);
			ctlr_add_ic(hSession,hSession->buffer_addr, hSession->default_ic);
			INC_BA(hSession->buffer_addr);
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
			fa_addr = find_field_attribute(hSession,hSession->buffer_addr);
			baddr = hSession->buffer_addr;
			DEC_BA(baddr);
			while (!aborted &&
			       ((fa_addr >= 0 && baddr != fa_addr) ||
			        (fa_addr < 0 && baddr != hSession->rows*hSession->cols - 1))) {
				if (hSession->ea_buf[baddr].cc == FCORDER_SI) {
					ABORT_WRITE("double SI");
				}
				if (hSession->ea_buf[baddr].cc == FCORDER_SO)
					break;
				DEC_BA(baddr);
			}
			if (aborted)
				break;
			if (hSession->ea_buf[baddr].cc != FCORDER_SO) {
				ABORT_WRITE("SI without SO");
			}
			/* All is well. */
			previous = ORDER;
			ctlr_add(hSession,hSession->buffer_addr, *cp, hSession->default_cs);
			ctlr_add_fg(hSession,hSession->buffer_addr, hSession->default_fg);
			ctlr_add_bg(hSession,hSession->buffer_addr, hSession->default_bg);
			ctlr_add_gr(hSession,hSession->buffer_addr, hSession->default_gr);
			ctlr_add_ic(hSession,hSession->buffer_addr, hSession->default_ic);
			INC_BA(hSession->buffer_addr);
			last_cmd = True;
			last_zpt = False;
			break;
		case FCORDER_NULL:	/* NULL or DBCS control char */
			previous = NULLCH;
			add_dbcs = False;
			d = ctlr_lookleft_state(hSession->buffer_addr, &why);
			if (d == DBCS_RIGHT) {
				ABORT_WRITE("NULL overwriting right half of DBCS character");
			}
			if (d != DBCS_NONE || hSession->default_cs == CS_DBCS) {
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
					trace_ds(hSession," [invalid DBCS control character X'%02x%02x'; write aborted]",add_c1, add_c2);
					ABORT_WRITEx;
					break;
				}
				if (aborted)
					break;
			} else {
				END_TEXT("NULL");
				add_c1 = *cp;
			}
			ctlr_add(hSession,hSession->buffer_addr, add_c1, hSession->default_cs);
			ctlr_add_fg(hSession,hSession->buffer_addr, hSession->default_fg);
			ctlr_add_bg(hSession,hSession->buffer_addr, hSession->default_bg);
			ctlr_add_gr(hSession,hSession->buffer_addr, hSession->default_gr);
			ctlr_add_ic(hSession,hSession->buffer_addr, hSession->default_ic);
			INC_BA(hSession->buffer_addr);
			if (add_dbcs)
			{
				ctlr_add(hSession,hSession->buffer_addr, add_c2, hSession->default_cs);
				ctlr_add_fg(hSession,hSession->buffer_addr, hSession->default_fg);
				ctlr_add_bg(hSession,hSession->buffer_addr, hSession->default_bg);
				ctlr_add_gr(hSession,hSession->buffer_addr, hSession->default_gr);
				ctlr_add_ic(hSession,hSession->buffer_addr, hSession->default_ic);
				INC_BA(hSession->buffer_addr);
			}
			last_cmd = False;
			last_zpt = False;
			break;
		default:	/* enter character */
			if (*cp <= 0x3F) {
				END_TEXT("UnsupportedOrder");
				trace_ds(hSession,"(%02X)", *cp);
				previous = ORDER;
				last_cmd = True;
				last_zpt = False;
				break;
			}
			if (previous != TEXT)
				trace_ds(hSession," '");
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
					trace_ds(hSession," [invalid DBCS character X'%02x%02x'; write aborted]",add_c1, add_c2);
					ABORT_WRITEx;
			       }
			       add_dbcs = True;
			       dbcs_to_mb(add_c1, add_c2, mb);
			       trace_ds_nb(hSession,"%s", mb);
			} else {
#endif /*]*/
				add_c1 = *cp;
				trace_ds(hSession,"%s", see_ebc(*cp));
#if defined(X3270_DBCS) /*[*/
			}
#endif /*]*/
			ctlr_add(hSession,hSession->buffer_addr, add_c1, hSession->default_cs);
			ctlr_add_fg(hSession,hSession->buffer_addr, hSession->default_fg);
			ctlr_add_bg(hSession,hSession->buffer_addr, hSession->default_bg);
			ctlr_add_gr(hSession,hSession->buffer_addr, hSession->default_gr);
			ctlr_add_ic(hSession,hSession->buffer_addr, hSession->default_ic);
			INC_BA(hSession->buffer_addr);
#if defined(X3270_DBCS) /*[*/
			if (add_dbcs) {
				ctlr_add(hSession->buffer_addr, add_c2, hSession->default_cs);
				ctlr_add_fg(hSession,hSession->buffer_addr, hSession->default_fg);
				ctlr_add_bg(hSession->buffer_addr, hSession->default_bg);
				ctlr_add_gr(hSession->buffer_addr, hSession->default_gr);
				ctlr_add_ic(hSession,hSession->buffer_addr, hSession->default_ic);
				INC_BA(hSession->buffer_addr);
			}
#endif /*]*/
			last_cmd = False;
			last_zpt = False;
			break;
		}
	}
	update_formatted(hSession);
	END_TEXT0;
	trace_ds(hSession,"\n");
	if (wcc_keyboard_restore) {
		hSession->aid = AID_NO;
		do_reset(hSession,False);
	} else if (hSession->kybdlock & KL_OIA_TWAIT) {
		lib3270_kybdlock_clear(hSession,KL_OIA_TWAIT);
		status_changed(hSession,LIB3270_MESSAGE_SYSWAIT);
	}
	if (wcc_sound_alarm)
		lib3270_ring_bell(hSession);

	/* Set up the DBCS state. */
	if (ctlr_dbcs_postprocess(hSession) < 0 && rv == PDS_OKAY_NO_OUTPUT)
		rv = PDS_BAD_ADDR;

	hSession->trace_primed = 0;

	ps_process(hSession);

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
void ctlr_write_sscp_lu(H3270 *hSession, unsigned char buf[], int buflen)
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

	trace_ds(hSession,"SSCP-LU data\n");
	for (i = 0; i < buflen; cp++, i++) {
		switch (*cp) {
		case FCORDER_NL:
			/*
			 * Insert NULLs to the end of the line and advance to
			 * the beginning of the next line.
			 */
			s_row = hSession->buffer_addr / hSession->cols;
			while ((hSession->buffer_addr / hSession->cols) == s_row)
			{
				ctlr_add(hSession,hSession->buffer_addr, EBC_null, hSession->default_cs);
				ctlr_add_fg(hSession,hSession->buffer_addr, hSession->default_fg);
				ctlr_add_bg(hSession,hSession->buffer_addr, hSession->default_bg);
				ctlr_add_gr(hSession,hSession->buffer_addr, hSession->default_gr);
				ctlr_add_ic(hSession,hSession->buffer_addr, hSession->default_ic);
				INC_BA(hSession->buffer_addr);
			}
			break;

		case ORDER_SF:
			/* Some hosts forget they're talking SSCP-LU. */
			cp++;
			i++;
			trace_ds(hSession," StartField%s %s [translated to space]\n",rcba(hSession,hSession->buffer_addr), see_attr(*cp));
			ctlr_add(hSession,hSession->buffer_addr, EBC_space, hSession->default_cs);
			ctlr_add_fg(hSession,hSession->buffer_addr, hSession->default_fg);
			ctlr_add_bg(hSession,hSession->buffer_addr, hSession->default_bg);
			ctlr_add_gr(hSession,hSession->buffer_addr, hSession->default_gr);
			ctlr_add_ic(hSession,hSession->buffer_addr, hSession->default_ic);
			INC_BA(hSession->buffer_addr);
			break;

		case ORDER_IC:
			trace_ds(hSession," InsertCursor%s [ignored]\n",rcba(hSession,hSession->buffer_addr));
			break;

		case ORDER_SBA:
//			baddr = DECODE_BADDR(*(cp+1), *(cp+2));
			trace_ds(hSession," SetBufferAddress%s [ignored]\n", rcba(hSession,DECODE_BADDR(*(cp+1), *(cp+2))));
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

			ctlr_add(hSession,hSession->buffer_addr, c, CS_GE);
			ctlr_add_fg(hSession,hSession->buffer_addr, hSession->default_fg);
			ctlr_add_bg(hSession,hSession->buffer_addr, hSession->default_bg);
			ctlr_add_gr(hSession,hSession->buffer_addr, hSession->default_gr);
			ctlr_add_ic(hSession,hSession->buffer_addr, hSession->default_ic);
			INC_BA(hSession->buffer_addr);
			break;

		default:
			ctlr_add(hSession,hSession->buffer_addr, *cp, hSession->default_cs);
			ctlr_add_fg(hSession,hSession->buffer_addr, hSession->default_fg);
			ctlr_add_bg(hSession,hSession->buffer_addr, hSession->default_bg);
			ctlr_add_gr(hSession,hSession->buffer_addr, hSession->default_gr);
			ctlr_add_ic(hSession,hSession->buffer_addr, hSession->default_ic);
			INC_BA(hSession->buffer_addr);
			break;
		}
	}
	cursor_move(hSession,hSession->buffer_addr);
	hSession->sscp_start = hSession->buffer_addr;

	/* Unlock the keyboard. */
	hSession->aid = AID_NO;
	do_reset(hSession,False);

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
int ctlr_dbcs_postprocess(H3270 *hSession)
{
	int baddr;		/* current buffer address */
	int faddr0;		/* address of first field attribute */
	int faddr;		/* address of current field attribute */
	int last_baddr;		/* last buffer address to search */
	int pbaddr = -1;	/* previous buffer address */
	int dbaddr = -1;	/* first data position of current DBCS (sub-) field */
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
				    trace_ds(hSession,"DBCS postprocess: invalid SO found at %s\n", rcba(baddr));
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
				    trace_ds(hSession,"Postprocess: Invalid SO found at %s\n", rcba(baddr));
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
				    trace_ds(hSession,"DBCS postprocess: invalid character set found at %s\n",rcba(baddr));
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
				trace_ds(hSession,"DBCS postprocess: dead position at %s\n", rcba(pbaddr));
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

/**
 * Process pending input.
 *
 * @param hSession	Session handle.
 */
void ps_process(H3270 *hSession)
{
	while(run_ta(hSession))
		;

//	sms_continue();

#if defined(X3270_FT)
	/* Process file transfers. */
	if (lib3270_get_ft_state(hSession) != LIB3270_FT_STATE_NONE &&		/* transfer in progress */
	    hSession->formatted &&          								/* screen is formatted */
	    !hSession->screen_alt &&        								/* 24x80 screen */
	    !hSession->kybdlock &&                							/* keyboard not locked */
	    /* magic field */
	    hSession->ea_buf[1919].fa && FA_IS_SKIP(hSession->ea_buf[1919].fa))
	{
		ft_cut_data(hSession);
	}
#endif
}

/*
 * Tell me if there is any data on the screen.
 */
int ctlr_any_data(H3270 *session)
{
	register int i;

	for(i = 0; i < session->rows*session->cols; i++)
	{
		if (!IsBlank(session->ea_buf[i].cc))
			return 1;
	}
	return 0;
}

/*
 * Clear the text (non-status) portion of the display.  Also resets the cursor
 * and buffer addresses and extended attributes.
 */
void ctlr_clear(H3270 *session, Boolean can_snap)
{
	/* Snap any data that is about to be lost into the trace file. */
#if defined(X3270_TRACE)

	if (ctlr_any_data(session))
	{
		if (can_snap && !session->trace_skipping && lib3270_get_toggle(session,LIB3270_TOGGLE_SCREEN_TRACE))
			trace_screen(session);
	}

	session->trace_skipping = 0;

#endif

	/* Clear the screen. */
	(void) memset((char *)session->ea_buf, 0, session->rows*session->cols*sizeof(struct lib3270_ea));
	cursor_move(session,0);
	session->buffer_addr = 0;
	lib3270_unselect(session);
	set_formatted(session,0);
	session->default_fg = 0;
	session->default_bg = 0;
	session->default_gr = 0;
	session->default_ic = 0;

	session->sscp_start = 0;

//	ALL_CHANGED;
	session->cbk.erase(session);
}

/**
 * Fill the screen buffer with blanks.
 *
 * @param session	Session handle
 */
static void ctlr_blanks(H3270 *session)
{
	int baddr;

	for (baddr = 0; baddr < session->rows*session->cols; baddr++)
	{
		if (!session->ea_buf[baddr].fa)
			session->ea_buf[baddr].cc = EBC_space;
	}
	cursor_move(session,0);
	session->buffer_addr = 0;
	lib3270_unselect(session);
	set_formatted(session,0);
	ALL_CHANGED(session);
}


/**
 * Change a character in the 3270 buffer, removes any field attribute defined at that location.
 *
 *
 */
void ctlr_add(H3270 *hSession, int baddr, unsigned char c, unsigned char cs)
{
	unsigned char oc = 0;

	if(hSession->ea_buf[baddr].fa || ((oc = hSession->ea_buf[baddr].cc) != c || hSession->ea_buf[baddr].cs != cs))
	{
		if (hSession->trace_primed && !IsBlank(oc))
		{
#if defined(X3270_TRACE) /*[*/
			if (lib3270_get_toggle(hSession,LIB3270_TOGGLE_SCREEN_TRACE))
				trace_screen(hSession);
#endif /*]*/
			hSession->trace_primed = 0;
		}

		hSession->ea_buf[baddr].cc = c;
		hSession->ea_buf[baddr].cs = cs;
		hSession->ea_buf[baddr].fa = 0;
		ONE_CHANGED(hSession,baddr);
	}
}

/*
 * Set a field attribute in the 3270 buffer.
 */
void ctlr_add_fa(H3270 *hSession, int baddr, unsigned char fa, unsigned char cs)
{
	/* Put a null in the display buffer. */
	ctlr_add(hSession, baddr, EBC_null, cs);

	/*
	 * Store the new attribute, setting the 'printable' bits so that the
	 * value will be non-zero.
	 */
	hSession->ea_buf[baddr].fa = FA_PRINTABLE | (fa & FA_MASK);
}

/*
 * Change the character set for a field in the 3270 buffer.
 */
void
ctlr_add_cs(H3270 *hSession, int baddr, unsigned char cs)
{
	if (hSession->ea_buf[baddr].cs != cs)
	{
		/*
		if (SELECTED(baddr))
			unselect(baddr, 1);
		*/
		hSession->ea_buf[baddr].cs = cs;
		ONE_CHANGED(hSession,baddr);
	}
}

/*
 * Change the graphic rendition of a character in the 3270 buffer.
 */
void ctlr_add_gr(H3270 *hSession, int baddr, unsigned char gr)
{
	if (hSession->ea_buf[baddr].gr != gr)
	{
		hSession->ea_buf[baddr].gr = gr;
//		if (gr & GR_BLINK)
//			blink_start();
		ONE_CHANGED(hSession,baddr);
	}
}

/*
 * Change the foreground color for a character in the 3270 buffer.
 */
void ctlr_add_fg(H3270 *hSession, int baddr, unsigned char color)
{
	if (!hSession->m3279)
		return;

	if ((color & 0xf0) != 0xf0)
		color = 0;

	if (hSession->ea_buf[baddr].fg != color)
	{
		hSession->ea_buf[baddr].fg = color;
		ONE_CHANGED(hSession,baddr);
	}
}

/*
 * Change the background color for a character in the 3270 buffer.
 */
void ctlr_add_bg(H3270 *hSession, int baddr, unsigned char color)
{
	if (!hSession->m3279)
		return;

	if ((color & 0xf0) != 0xf0)
		color = 0;

	if (hSession->ea_buf[baddr].bg != color)
	{
		hSession->ea_buf[baddr].bg = color;
		ONE_CHANGED(hSession,baddr);
	}
}

/*
 * Change the input control bit for a character in the 3270 buffer.
 */
static void ctlr_add_ic(H3270 *hSession, int baddr, unsigned char ic)
{
	hSession->ea_buf[baddr].ic = ic;
}

/*
 * Wrapping bersion of ctlr_bcopy.
 */
void ctlr_wrapping_memmove(H3270 *hSession, int baddr_to, int baddr_from, int count)
{
	/*
	 * The 'to' region, the 'from' region, or both can wrap the screen,
	 * and can overlap each other.  memmove() is smart enough to deal with
	 * overlaps, but not across a screen wrap.
	 *
	 * It's faster to figure out if none of this is true, then do a slow
	 * location-at-a-time version only if it happens.
	 */
	if (baddr_from + count <= hSession->rows*hSession->cols &&
	    baddr_to + count <= hSession->rows*hSession->cols) {
		ctlr_bcopy(hSession,baddr_from, baddr_to, count, True);
	} else {
		int i, from, to;

		for (i = 0; i < count; i++) {
		    if (baddr_to > baddr_from) {
			/* Shifting right, move left. */
			to = (baddr_to + count - 1 - i) % hSession->rows*hSession->cols;
			from = (baddr_from + count - 1 - i) % hSession->rows*hSession->cols;
		    } else {
			/* Shifting left, move right. */
			to = (baddr_to + i) % hSession->rows*hSession->cols;
			from = (baddr_from + i) % hSession->rows*hSession->cols;
		    }
		    ctlr_bcopy(hSession,from, to, 1, True);
		}
	}
}

/**
 * Copy a block of characters in the 3270 buffer.
 *
 * Copy a block of characters in the 3270 buffer, optionally including all of
 * the extended attributes.  (The character set, which is actually kept in the
 * extended attributes, is considered part of the characters here.)
 *
 * @param hSession	Session handle
 */
void ctlr_bcopy(H3270 *hSession, int baddr_from, int baddr_to, int count, int move_ea)
{
	/* Move the characters. */
	if (memcmp((char *) &hSession->ea_buf[baddr_from],(char *) &hSession->ea_buf[baddr_to],count * sizeof(struct lib3270_ea)))
	{
		(void) memmove(&hSession->ea_buf[baddr_to], &hSession->ea_buf[baddr_from],count * sizeof(struct lib3270_ea));
		REGION_CHANGED(hSession,baddr_to, baddr_to + count);
	}
	/* XXX: What about move_ea? */
}

#if defined(X3270_ANSI) /*[*/
/**
 * Erase a region of the 3270 buffer, optionally clearing extended attributes as well.
 *
 * @param hSession	Session handle
 *
 */
void ctlr_aclear(H3270 *hSession, int baddr, int count, int clear_ea)
{
	if (memcmp((char *) &hSession->ea_buf[baddr], (char *) hSession->zero_buf,
		    count * sizeof(struct lib3270_ea))) {
		(void) memset((char *) &hSession->ea_buf[baddr], 0,
				count * sizeof(struct lib3270_ea));
		REGION_CHANGED(hSession,baddr, baddr + count);
	}
	/* XXX: What about clear_ea? */
}

/*
 * Scroll the screen 1 row.
 *
 * This could be accomplished with ctlr_bcopy() and ctlr_aclear(), but this
 * operation is common enough to warrant a separate path.
 */
void ctlr_scroll(H3270 *hSession)
{
	int qty = (hSession->rows - 1) * hSession->cols;

	/* Make sure nothing is selected. (later this can be fixed) */
	// unselect(0, ROWS*COLS);

	/* Synchronize pending changes prior to this. */

	/* Move ea_buf. */
	(void) memmove(&hSession->ea_buf[0], &hSession->ea_buf[hSession->cols],qty * sizeof(struct lib3270_ea));

	/* Clear the last line. */
	(void) memset((char *) &hSession->ea_buf[qty], 0, hSession->cols * sizeof(struct lib3270_ea));

	hSession->cbk.display(hSession);

}
#endif /*]*/

/*
 * Swap the regular and alternate screen buffers
 */
void ctlr_altbuffer(H3270 *session, int alt)
{
    CHECK_SESSION_HANDLE(session);

	if (alt != session->is_altbuffer)
	{
		struct lib3270_ea *etmp;

		etmp = session->ea_buf;
		session->ea_buf  = session->aea_buf;
		session->aea_buf = etmp;

		session->is_altbuffer = alt;
		lib3270_unselect(session);

		ALL_CHANGED(session);

		/*
		 * There may be blinkers on the alternate screen; schedule one
		 * iteration just in case.
		 */
//		blink_start();
	}
}


/*
 * Set or clear the MDT on an attribute
 */
void mdt_set(H3270 *hSession, int baddr)
{
	int faddr;

	faddr = find_field_attribute(hSession,baddr);
	if (faddr >= 0 && !(hSession->ea_buf[faddr].fa & FA_MODIFY))
	{
		hSession->ea_buf[faddr].fa |= FA_MODIFY;
		if (hSession->modified_sel)
			ALL_CHANGED(hSession);
	}
}

void mdt_clear(H3270 *hSession, int baddr)
{
	int faddr = find_field_attribute(hSession,baddr);

	if (faddr >= 0 && (hSession->ea_buf[faddr].fa & FA_MODIFY))
	{
		hSession->ea_buf[faddr].fa &= ~FA_MODIFY;
		if (hSession->modified_sel)
			ALL_CHANGED(hSession);
	}
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
//static struct timeval t_start;
// static Boolean ticking = False;
//static Boolean mticking = False;
//static void * tick_id;
//static struct timeval t_want;

/* Return the difference in milliseconds between two timevals. */
static long
delta_msec(struct timeval *t1, struct timeval *t0)
{
	return (t1->tv_sec - t0->tv_sec) * 1000 +
	       (t1->tv_usec - t0->tv_usec + 500) / 1000;
}

static void keep_ticking(H3270 *hSession)
{
	struct timeval t1;
	long msec;

	do
	{
		(void) gettimeofday(&t1, (struct timezone *) 0);
		hSession->t_want.tv_sec++;
		msec = delta_msec(&hSession->t_want, &t1);
	} while (msec <= 0);

	hSession->tick_id = AddTimeOut(msec, hSession, keep_ticking);
	status_timing(hSession,&hSession->t_start, &t1);
}

void ticking_start(H3270 *hSession, Boolean anyway)
{
	CHECK_SESSION_HANDLE(hSession);

	if(hSession->cbk.set_timer)
	{
		if(lib3270_get_toggle(hSession,LIB3270_TOGGLE_SHOW_TIMING) || anyway)
			hSession->cbk.set_timer(hSession,1);
	}
	else
	{
		(void) gettimeofday(&hSession->t_start, (struct timezone *) 0);

		hSession->mticking = 1;

		if (!lib3270_get_toggle(hSession,LIB3270_TOGGLE_SHOW_TIMING) && !anyway)
			return;

		status_untiming(hSession);

		if (hSession->ticking)
			RemoveTimeOut(hSession, hSession->tick_id);

		hSession->ticking = 1;
		hSession->tick_id = AddTimeOut(1000, hSession, keep_ticking);
		hSession->t_want = hSession->t_start;
	}

}

static void ticking_stop(H3270 *hSession)
{
	CHECK_SESSION_HANDLE(hSession);

	if(hSession->cbk.set_timer)
	{
		hSession->cbk.set_timer(hSession,0);
	}
	else
	{
		struct timeval t1;

		(void) gettimeofday(&t1, (struct timezone *) 0);
		if (hSession->mticking)
			hSession->mticking = 0;
		else
			return;

		if (!hSession->ticking)
			return;
		RemoveTimeOut(hSession, hSession->tick_id);
		hSession->ticking = 0;
		status_timing(hSession,&hSession->t_start, &t1);
	}
}
