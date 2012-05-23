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
 * Este programa está nomeado como screen.c e possui 894 linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

/*
 *	screen.c
 *		A callback based 3270 Terminal Emulator
 *		Screen drawing
 */

#include "globals.h"
#include <signal.h>
//#include "appres.h"
#include "3270ds.h"
#include "resources.h"
// #include "ctlr.h"

#include "actionsc.h"
#include "ctlrc.h"
#include "hostc.h"
#include "kybdc.h"
#include "screenc.h"
#include "tablesc.h"
#include "trace_dsc.h"
#include "utilc.h"
#include "w3miscc.h"
#include "widec.h"
#include "xioc.h"
#include "screen.h"
#include "errno.h"
#include "statusc.h"
#include "togglesc.h"
#include "api.h"
#include "charsetc.h"

#if defined(_WIN32)
	#include <windows.h>
	#include <wincon.h>
	#include "winversc.h"
#else
	#include <stdarg.h>
#endif


#define get_color_pair(fg,bg) (((bg&0x0F) << 4) | (fg&0x0F))
#define DEFCOLOR_MAP(f) ((((f) & FA_PROTECT) >> 4) | (((f) & FA_INT_HIGH_SEL) >> 3))

/*--[ Implement ]------------------------------------------------------------------------------------*/

static int logpopup(H3270 *session, void *widget, LIB3270_NOTIFY type, const char *title, const char *msg, const char *fmt, va_list arg);

static int (*popup_handler)(H3270 *, void *, LIB3270_NOTIFY, const char *, const char *, const char *, va_list) = logpopup;

enum ts { TS_AUTO, TS_ON, TS_OFF };

static void status_connect(H3270 *session, int ignored, void *dunno);
static void status_3270_mode(H3270 *session, int ignored, void *dunno);
// static void status_printer(H3270 *session, int on, void *dunno);
static unsigned short color_from_fa(unsigned char fa);

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void addch(H3270 *session, int baddr, unsigned char c, unsigned short attr, int *first, int *last)
{
	// If set to keep selection adjust corresponding flag based on the current state
	if(lib3270_get_toggle(session,LIB3270_TOGGLE_KEEP_SELECTED))
		attr |= (session->text[baddr].attr & LIB3270_ATTR_SELECTED);

	if(session->text[baddr].chr == c && session->text[baddr].attr == attr)
		return;

	if(*first < 0)
		*first = baddr;
	*last = baddr;

	/* Converted char has changed, update it */
	session->text[baddr].chr  = c;
	session->text[baddr].attr = attr;

	session->update(session,baddr,c,attr,baddr == session->cursor_addr);

}

LIB3270_EXPORT int lib3270_get_element(H3270 *h, int baddr, unsigned char *c, unsigned short *attr)
{
	CHECK_SESSION_HANDLE(h);

	if(!h->text || baddr < 0 || baddr > (h->rows*h->cols))
		return EINVAL;

	*c		= h->text[baddr].chr;
	*attr	= h->text[baddr].attr;

	return 0;
}

/**
 * Initialize the screen.
 *
 * @return 0 if ok, non zero if not
 *
 */
int screen_init(H3270 *session)
{
	CHECK_SESSION_HANDLE(session);

	/* Set up callbacks for state changes. */
	lib3270_register_schange(session,LIB3270_STATE_CONNECT, status_connect,0);
	lib3270_register_schange(session,LIB3270_STATE_3270_MODE, status_3270_mode,0);
//	lib3270_register_schange(session,LIB3270_STATE_PRINTER, status_printer,0);

	/* Set up the controller. */
	ctlr_init(session,-1);
	ctlr_reinit(session,-1);

	/* Finish screen initialization. */
	session->suspend(session);

	return 0;
}

/* Map a field attribute to its default colors. */
static unsigned short color_from_fa(unsigned char fa)
{
	if (h3270.m3279)
		return get_color_pair(DEFCOLOR_MAP(fa),0) | COLOR_ATTR_FIELD;

	// Green on black
	return get_color_pair(0,0) | COLOR_ATTR_FIELD | ((FA_IS_HIGH(fa)) ? COLOR_ATTR_INTENSIFY : 0);
}

/*
 * Find the display attributes for a baddr, fa_addr and fa.
 */
static unsigned short calc_attrs(H3270 *session, int baddr, int fa_addr, int fa)
{
	unsigned short fg=0, bg=0, a;
	int gr;

	/* Compute the color. */

	/* Monochrome is easy, and so is color if nothing is specified. */
	if (!h3270.m3279 ||
		(!session->ea_buf[baddr].fg &&
		 !session->ea_buf[fa_addr].fg &&
		 !session->ea_buf[baddr].bg &&
		 !session->ea_buf[fa_addr].bg))
	{
		a = color_from_fa(fa);
	}
	else
	{

		/* The current location or the fa specifies the fg or bg. */
		if (session->ea_buf[baddr].fg)
		{
			fg = session->ea_buf[baddr].fg & 0x0f;
		}
		else if (session->ea_buf[fa_addr].fg)
		{
			fg = session->ea_buf[fa_addr].fg & 0x0f;
		}
		else
		{
			fg = DEFCOLOR_MAP(fa);
		}

		if (session->ea_buf[baddr].bg)
			bg = session->ea_buf[baddr].bg & 0x0f;
		else if (session->ea_buf[fa_addr].bg)
			bg = session->ea_buf[fa_addr].bg & 0x0f;
		else
			bg = 0;

		a = get_color_pair(fg, bg);
	}

	/* Compute the display attributes. */

	if (session->ea_buf[baddr].gr)
		gr = session->ea_buf[baddr].gr;
	else if (session->ea_buf[fa_addr].gr)
		gr = session->ea_buf[fa_addr].gr;
	else
		gr = 0;

	if(!(gr & GR_REVERSE) && !bg)
	{
		if(gr & GR_BLINK)
			a |= LIB3270_ATTR_BLINK;

		if(gr & GR_UNDERLINE)
			a |= LIB3270_ATTR_UNDERLINE;
	}

	if(h3270.m3279 && (gr & (GR_BLINK | GR_UNDERLINE)) && !(gr & GR_REVERSE) && !bg)
    	a |= LIB3270_ATTR_BACKGROUND_INTENSITY;

	if(!h3270.m3279 &&	((gr & GR_INTENSIFY) || FA_IS_HIGH(fa)))
		a |= LIB3270_ATTR_INTENSIFY;

	if (gr & GR_REVERSE)
		a = get_color_pair(((a & 0xF0) >> 4),(a & 0x0F)) | (a&0xFF00); // a = reverse_colors(a);

	return a;
}

LIB3270_EXPORT unsigned int lib3270_get_length(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return h->rows * h->cols;
}

LIB3270_EXPORT void lib3270_get_screen_size(H3270 *h, int *r, int *c)
{
	CHECK_SESSION_HANDLE(h);
	*r = h->rows;
	*c = h->cols;
}

LIB3270_EXPORT int lib3270_get_width(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return h->cols;
}

void update_model_info(H3270 *session, int model, int cols, int rows)
{
	if(model == session->model_num && session->maxROWS == rows && session->maxCOLS == cols)
		return;

	session->maxCOLS   	= cols;
	session->maxROWS   	= rows;
	session->model_num	= model;

	/* Update the model name. */
	(void) sprintf(session->model_name, "327%c-%d%s",session->m3279 ? '9' : '8',session->model_num,session->extended ? "-E" : "");

	trace("%s: %p",__FUNCTION__,session->update_model);
	session->update_model(session, session->model_name,session->model_num,rows,cols);
}

LIB3270_EXPORT int lib3270_get_contents(H3270 *h, int first, int last, unsigned char *chr, unsigned short *attr)
{
	int baddr;
	int len;

    CHECK_SESSION_HANDLE(h);

	len = h->rows * h->cols;

	if(first > len || last > len || first < 0 || last < 0)
		return EFAULT;

	for(baddr = first; baddr <= last;baddr++)
	{
		*(chr++)  = h->text[baddr].chr ? h->text[baddr].chr : ' ';
		*(attr++) = h->text[baddr].attr;
	}

	return 0;
}

/* Display what's in the buffer. */
void screen_update(H3270 *session, int bstart, int bend)
{
	int				baddr;
	unsigned short	a;
	int				attr = COLOR_GREEN;
	unsigned char	fa;
	int				fa_addr;
	int				first	= -1;
	int				last	= -1;

	fa		= get_field_attribute(session,bstart);
	a  		= color_from_fa(fa);
	fa_addr = find_field_attribute(session,bstart); // may be -1, that's okay

	trace("%s start=%d end=%d",__FUNCTION__,bstart,bend);

	for(baddr = bstart; baddr < bend; baddr++)
	{
		if(session->ea_buf[baddr].fa)
		{
			// Field attribute.
			fa_addr = baddr;
			fa = session->ea_buf[baddr].fa;
			a = calc_attrs(session, baddr, baddr, fa);
			addch(session,baddr,' ',(attr = COLOR_GREEN)|CHAR_ATTR_MARKER,&first,&last);
		}
		else if (FA_IS_ZERO(fa))
		{
			// Blank.
			addch(session,baddr,' ',attr=a,&first,&last);
		}
		else
		{
			// Normal text.
			if (!(session->ea_buf[baddr].gr || session->ea_buf[baddr].fg || session->ea_buf[baddr].bg))
			{
				attr = a;
			}
			else
			{
				attr = calc_attrs(session, baddr, fa_addr, fa);
			}

			if (session->ea_buf[baddr].cs == CS_LINEDRAW)
			{
				addch(session,baddr,session->ea_buf[baddr].cc,attr,&first,&last);
			}
			else if (session->ea_buf[baddr].cs == CS_APL || (session->ea_buf[baddr].cs & CS_GE))
			{
				addch(session,baddr,session->ea_buf[baddr].cc,attr|LIB3270_ATTR_CG,&first,&last);
			}
			else
			{
				if(lib3270_get_toggle(session,LIB3270_TOGGLE_MONOCASE))
					addch(session,baddr,asc2uc[ebc2asc[session->ea_buf[baddr].cc]],attr,&first,&last);
				else
					addch(session,baddr,ebc2asc[session->ea_buf[baddr].cc],attr,&first,&last);
			}
		}
	}

	if(first >= 0)
	{
		int len = (last - first)+1;
		int f;

		for(f=first;f<last;f++)
		{
			if(f%session->cols == 0)
				len++;
		}

		session->changed(session,first,len);
	}

	trace("%s ends",__FUNCTION__);
}

LIB3270_EXPORT int lib3270_get_cursor_address(H3270 *h)
{
    CHECK_SESSION_HANDLE(h);
    return h->cursor_addr;
}

LIB3270_EXPORT int lib3270_set_cursor_address(H3270 *h, int baddr)
{
    CHECK_SESSION_HANDLE(h);

	if(h->selected && !lib3270_get_toggle(h,LIB3270_TOGGLE_KEEP_SELECTED))
		lib3270_unselect(h);

	return cursor_move(h,baddr);
}

int cursor_move(H3270 *h, int baddr)
{
    int ret;

    ret = h->cursor_addr;

	if(ret == baddr)
		return ret;

	h->cursor_addr = baddr;
	h->update_cursor(h,(unsigned short) (baddr/h->cols),(unsigned short) (baddr%h->cols),h->text[baddr].chr,h->text[baddr].attr);

    return ret;
}

/* Status line stuff. */

void set_status(H3270 *session, LIB3270_FLAG id, Boolean on)
{
	CHECK_SESSION_HANDLE(session);

	session->oia_flag[id] = (on != 0);
	session->update_oia(session,id,session->oia_flag[id]);

}

void status_ctlr_done(H3270 *session)
{
	CHECK_SESSION_HANDLE(session);
	set_status(session,OIA_FLAG_UNDERA,True);
	session->ctlr_done(session);
}

void status_oerr(H3270 *session, int error_type)
{
	LIB3270_STATUS sts = LIB3270_STATUS_USER;

	CHECK_SESSION_HANDLE(session);

	switch (error_type)
	{
	case KL_OERR_PROTECTED:
		sts = LIB3270_STATUS_PROTECTED;
		break;
	case KL_OERR_NUMERIC:
		sts = LIB3270_STATUS_NUMERIC;
		break;
	case KL_OERR_OVERFLOW:
		sts = LIB3270_STATUS_OVERFLOW;
		break;

	default:
		return;
	}

	status_changed(session,sts);

}

void status_connecting(H3270 *session, Boolean on)
{
	if(session->cursor)
			session->cursor(session,on ? CURSOR_MODE_LOCKED : CURSOR_MODE_NORMAL);

	status_changed(session, on ? LIB3270_STATUS_CONNECTING : LIB3270_STATUS_BLANK);
}

void status_reset(H3270 *session)
{
	CHECK_SESSION_HANDLE(session);

	if (kybdlock & KL_ENTER_INHIBIT)
	{
		status_changed(session,LIB3270_STATUS_INHIBIT);
	}
	else if (kybdlock & KL_DEFERRED_UNLOCK)
	{
		status_changed(session,LIB3270_STATUS_X);
	}
	else
	{
		if(session->cursor)
			session->cursor(session,CURSOR_MODE_NORMAL);
		status_changed(session,LIB3270_STATUS_BLANK);
	}

	session->display(session);

}

/**
 * Query the updated terminal status.
 *
 * @return status-code.
 *
 * @see LIB3270_STATUS
 */
LIB3270_EXPORT LIB3270_STATUS lib3270_get_program_message(H3270 *session)
{
	CHECK_SESSION_HANDLE(session);
	return session->oia_status;
}

void status_changed(H3270 *session, LIB3270_STATUS id)
{
	CHECK_SESSION_HANDLE(session);

	if(id == session->oia_status)
		return;

	session->oia_status = id;

	if(session->update_status)
		session->update_status(session,id);
}

void status_twait(H3270 *session)
{
	CHECK_SESSION_HANDLE(session);
	set_status(session,OIA_FLAG_UNDERA,False);
	status_changed(session,LIB3270_STATUS_TWAIT);
}

void set_viewsize(H3270 *session, int rows, int cols)
{
	CHECK_SESSION_HANDLE(session);

	if(rows == session->rows && session->cols == cols)
		return;

	session->rows = rows;
	session->cols = cols;

	if(session->configure)
		session->configure(session,session->rows,session->cols);

}

void status_lu(H3270 *session, const char *lu)
{
	CHECK_SESSION_HANDLE(session);

	if(session->update_luname)
		session->update_luname(session,lu);

}

static void status_connect(H3270 *session, int connected, void *dunno)
{
	LIB3270_STATUS id = LIB3270_STATUS_USER;

	ctlr_erase(session,1);

	if (connected)
	{
		set_status(session,OIA_FLAG_BOXSOLID,IN_3270 && !IN_SSCP);

		if (kybdlock & KL_AWAITING_FIRST)
			id = LIB3270_STATUS_AWAITING_FIRST;
		else
			id = LIB3270_STATUS_CONNECTED;

/*
#if defined(HAVE_LIBSSL)
		set_status(session,OIA_FLAG_SECURE,session->secure_connection);
#endif
*/

	}
	else
	{
		set_status(session,OIA_FLAG_BOXSOLID,False);
/*
		set_status(session,OIA_FLAG_SECURE,False);
*/

		id = LIB3270_STATUS_DISCONNECTED;
	}

	status_changed(session,id);

}

static void status_3270_mode(H3270 *session, int ignored unused, void *dunno)
{
	Boolean oia_boxsolid = (IN_3270 && !IN_SSCP);

	CHECK_SESSION_HANDLE(session);

	if(oia_boxsolid)
		set_status(session,OIA_FLAG_UNDERA,True);
	set_status(session,OIA_FLAG_BOXSOLID,oia_boxsolid);

}

/*
static void status_printer(H3270 *session, int on, void *dunno)
{
	set_status(session,OIA_FLAG_PRINTER,on);
}
*/

void status_timing(H3270 *session, struct timeval *t0, struct timeval *t1)
{
}

void status_untiming(H3270 *session)
{
	CHECK_SESSION_HANDLE(session);

	if(session->set_timer)
		session->set_timer(session,0);
}

void show_3270_popup_dialog(H3270 *session, LIB3270_NOTIFY type, const char *title, const char *msg, const char *fmt, ...)
{
	CHECK_SESSION_HANDLE(session);

	trace("%s: title=%s msg=%s",__FUNCTION__,title,msg);

	if(!fmt)
		fmt = "";

	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	popup_handler(session,session->widget,type,title,msg,fmt,arg_ptr);
	va_end(arg_ptr);

}

static int logpopup(H3270 *session, void *widget, LIB3270_NOTIFY type, const char *title, const char *msg, const char *fmt, va_list arg)
{
#ifdef ANDROID

	char   len	= strlen(fmt);
	char * mask	= malloc(len+5);
	strncpy(mask,fmt,len);
	mask[len]	= '\n';
	mask[len+1]	= 0;
	__android_log_vprint(ANDROID_LOG_VERBOSE, PACKAGE_NAME, mask, arg);

#else

	lib3270_write_va_log(session,"lib3270",fmt,arg);

#endif // ANDROID

	return 0;
}

void Error(H3270 *session, const char *fmt, ...)
{
	va_list arg_ptr;

	CHECK_SESSION_HANDLE(session);

	trace("%s: title=%s fmt=%s",__FUNCTION__,"3270 Error",fmt);

	va_start(arg_ptr, fmt);

	if(session && session->sz == sizeof(H3270))
		popup_handler(session,session->widget,LIB3270_NOTIFY_ERROR, _( "3270 Error" ),NULL,fmt,arg_ptr);
	else
		popup_handler(NULL,NULL,LIB3270_NOTIFY_ERROR, _( "3270 Error" ),NULL,fmt,arg_ptr);

	va_end(arg_ptr);

}

void Warning(H3270 *session, const char *fmt, ...)
{
	va_list arg_ptr;

	CHECK_SESSION_HANDLE(session);

	trace("%s: title=%s fmt=%s",__FUNCTION__,"3270 Warning",fmt);

	va_start(arg_ptr, fmt);
	popup_handler(session,session->widget,LIB3270_NOTIFY_WARNING, _( "3270 Warning" ),NULL,fmt,arg_ptr);
	va_end(arg_ptr);

}

/* Pop up an error dialog. */
void popup_an_error(H3270 *session, const char *fmt, ...)
{
	va_list	args;

	CHECK_SESSION_HANDLE(session);

	trace("%s: title=%s fmt=%s",__FUNCTION__,"3270 Error",fmt);

	va_start(args, fmt);
	popup_handler(session,session->widget,LIB3270_NOTIFY_ERROR,_( "3270 Error" ),NULL,fmt,args);
	va_end(args);

}

void popup_system_error(H3270 *session, const char *title, const char *message, const char *fmt, ...)
{
	va_list	args;

	CHECK_SESSION_HANDLE(session);

	trace("%s: title=%s msg=%s",__FUNCTION__,"3270 Error",message);

	va_start(args, fmt);
	popup_handler(session,session->widget,LIB3270_NOTIFY_ERROR,title ? title : _( "3270 Error" ), message,fmt,args);
	va_end(args);
}

void mcursor_set(H3270 *session,LIB3270_CURSOR m)
{
	CHECK_SESSION_HANDLE(session);

	if(session->cursor)
		session->cursor(session,m);
}

LIB3270_ACTION( testpattern )
{
	static const unsigned char text_pat[] =
	{
		// Latin-1 chars
		0x48, 0x68, 0x45, 0x51, 0x55, 0xce, 0xde, 0x46, 0xcf, 0x65, 0x71, 0x75, 0xee, 0xfe, 0x66, 0xef,
		0x90, 0x9a, 0x9b, 0xb5, 0x42, 0x52, 0x56, 0xcb, 0xdb, 0x62, 0x72, 0x76, 0xeb, 0xfb,

		// Regular ASCII chars
		0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86,
		0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0xa2, 0xa3, 0xa4, 0xa5,
		0xa6, 0xa7, 0xa8, 0xa9, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xd1, 0xd2, 0xd3,
		0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0x7d, 0x5a,
		0x7c, 0x7b, 0x5b, 0x6c, 0x50, 0x5c, 0x4d, 0x5d, 0x60, 0x6d, 0x4e, 0x7e, 0x7d, 0x79, 0xad, 0xc0,
		0xa1, 0xb0, 0xd0, 0xbd, 0x6b, 0x4c, 0x4b, 0x6e, 0x5e, 0x7a, 0x61, 0x6f, 0x7f,

		// End marker
		0x00
	};

	static const unsigned char cg_pat[] =
	{
		0x8c, // CG 0xf7, less or equal "≤"
		0xae, // CG 0xd9, greater or equal "≥"
		0xbe, // CG 0x3e, not equal "≠"
		0xad, // "["
		0xbd, // "]"
		0xc6, // CG 0xa5, left tee
		0xd3, // CG 0xab, plus
		0xa2, // CG 0x92, horizontal line
		0xc7, // CG 0xa6, bottom tee
		0xd7, // CG 0xaf, top tee
		0xd6, // CG 0xae, right tee
		0xc5, // CG 0xa4, UL corner
		0xd5, // CG 0xad, UR corner
		0x85, // CG 0x184, vertical line
		0xc4, // CG 0xa3, LL corner
		0xd4, // CG 0xac, LR corner

		0x00

	};

	static const struct _pat
	{
		unsigned char cs;
		const unsigned char *cc;
	} pat[] =
	{
		{ CS_APL,	cg_pat		},
		{ 0,		text_pat	},
	};

	int row = 0;
	int max;
	int pos = 0;
	int f;
	int fg = COLOR_BLUE;

	CHECK_SESSION_HANDLE(hSession);

	max = (hSession->maxROWS * hSession->maxCOLS);
	for(f=0;f<max;f++)
	{
		if(!pat[row].cc[pos])
		{
			if(++row >= (sizeof(pat)/sizeof(struct _pat)) )
			{
				row = 0;
				if(++fg > COLOR_WHITE)
					fg = COLOR_BLUE;
			}
			pos = 0;
		}
		hSession->ea_buf[f].fg = fg;
		hSession->ea_buf[f].bg = (fg == COLOR_BLACK) ? COLOR_WHITE : COLOR_BLACK;
		hSession->ea_buf[f].cs = pat[row].cs;
		hSession->ea_buf[f].cc = pat[row].cc[pos++];
	}

	hSession->display(hSession);

	return 0;
}

LIB3270_EXPORT void lib3270_set_popup_handler(int (*handler)(H3270 *, void *, LIB3270_NOTIFY, const char *, const char *, const char *, va_list))
{
	popup_handler = handler ? handler : logpopup;
}

LIB3270_EXPORT void lib3270_popup_dialog(H3270 *session, LIB3270_NOTIFY id , const char *title, const char *message, const char *fmt, ...)
{
	va_list	args;

	CHECK_SESSION_HANDLE(session);

	trace("%s: title=%s msg=%s",__FUNCTION__,"3270 Error",message);

	va_start(args, fmt);
	popup_handler(session,session->widget,id,title ? title : _( "3270 Error" ), message,fmt,args);
	va_end(args);
}

LIB3270_EXPORT int lib3270_is_protected(H3270 *h, unsigned int baddr)
{
	unsigned char fa;

	CHECK_SESSION_HANDLE(h);

	if(baddr > (h->rows * h->cols))
		return -1;

	fa = get_field_attribute(h,baddr);

	return FA_IS_PROTECTED(fa);
}

