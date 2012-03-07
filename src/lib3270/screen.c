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
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */

/*
 *	screen.c
 *		A callback based 3270 Terminal Emulator
 *		Screen drawing
 */

#include "globals.h"
#include <signal.h>
#include "appres.h"
#include "3270ds.h"
#include "resources.h"
#include "ctlr.h"

#include "actionsc.h"
#include "ctlrc.h"
#include "hostc.h"
// #include "keymapc.h"
#include "kybdc.h"
// #include "macrosc.h"
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

#if defined(WC3270) /*[*/
extern char *profile_name;
#endif

static const struct lib3270_screen_callbacks *callbacks = NULL;
// static SCRIPT_STATE script_state = SCRIPT_STATE_NONE;


int lib3270_event_counter[COUNTER_ID_USER] = { 0, 0 };

enum ts { TS_AUTO, TS_ON, TS_OFF };

static void screen_update(H3270 *session, int bstart, int bend);
static void status_connect(H3270 *session, int ignored, void *dunno);
static void status_3270_mode(H3270 *session, int ignored, void *dunno);
static void status_printer(H3270 *session, int on, void *dunno);
static unsigned short color_from_fa(unsigned char fa);
static void relabel(H3270 *session, int ignored, void *dunno);

void set_display_charset(char *dcs)
{
	if(callbacks && callbacks->charset)
		callbacks->charset(dcs);
}

static void addch(H3270 *session, int baddr, unsigned char c, unsigned short attr)
{
	if(ea_buf[baddr].chr == c && ea_buf[baddr].attr == attr)
			return;

	/* Converted char has changed, update it */
	ea_buf[baddr].chr  = c;
	ea_buf[baddr].attr = attr;

	if(callbacks && callbacks->addch)
		callbacks->addch(baddr/session->cols, baddr%session->cols, c, attr);

	if(session->update)
		session->update(session,baddr,c,attr,baddr == session->cursor_addr);
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

	/* Initialize the console. */
	if(callbacks)
	{
		/* Init default callbacks */
		if(callbacks->move_cursor)
			session->update_cursor = callbacks->move_cursor;

		if(callbacks->set_oia)
			session->update_oia = callbacks->set_oia;

		if(callbacks->set_viewsize)
			session->configure = callbacks->set_viewsize;

		if(callbacks->lu)
			session->update_luname = callbacks->lu;

		if(callbacks->status)
			session->update_status = callbacks->status;

		if(callbacks->erase)
			session->erase = callbacks->erase;

		if(callbacks->cursor)
			session->cursor = callbacks->cursor;

		if(callbacks->toggle_changed)
			session->update_toggle = callbacks->toggle_changed;

		if(callbacks->model_changed)
			session->update_model = callbacks->model_changed;

		if(callbacks->init())
		{
			popup_an_error(session,"Can't initialize terminal.");
			return -1;
		}
	}

	/* Set up callbacks for state changes. */
	lib3270_register_schange(session,ST_CONNECT, status_connect,0);
	lib3270_register_schange(session,ST_3270_MODE, status_3270_mode,0);
	lib3270_register_schange(session,ST_PRINTER, status_printer,0);

	lib3270_register_schange(session,ST_HALF_CONNECT, relabel,0);
	lib3270_register_schange(session,ST_CONNECT, relabel,0);
	lib3270_register_schange(session,ST_3270_MODE, relabel,0);

	/* See about all-bold behavior. */
//	if (appres.all_bold_on)
//		ab_mode = TS_ON;
//	else if (!ts_value(appres.all_bold, &ab_mode))
//		(void) fprintf(stderr, "invalid %s value: '%s', assuming 'auto'\n", ResAllBold, appres.all_bold);
//	if (ab_mode == TS_AUTO)
//		ab_mode = appres.m3279? TS_ON: TS_OFF;

	/* Set up the controller. */
	ctlr_init(session,-1);
	ctlr_reinit(session,-1);

	/* Set the window label. */
#if defined(WC3270) /*[*/

	if (appres.title != CN)
		screen_title(appres.title);
	else if (profile_name != CN)
		screen_title(profile_name);
	else
		screen_title(NULL);
#endif

	/* Finish screen initialization. */
	screen_suspend(session);

	return 0;
}

/* Map a field attribute to its default colors. */
static unsigned short color_from_fa(unsigned char fa)
{
	if (appres.m3279)
		return get_color_pair(DEFCOLOR_MAP(fa),0) | COLOR_ATTR_FIELD;

	// Green on black
	return get_color_pair(0,0) | COLOR_ATTR_FIELD | ((FA_IS_HIGH(fa)) ? COLOR_ATTR_INTENSIFY : 0);
}

/*
 * Find the display attributes for a baddr, fa_addr and fa.
 */
static unsigned short calc_attrs(int baddr, int fa_addr, int fa)
{
	unsigned short fg=0, bg=0, a;
	int gr;

	/* Compute the color. */

	/* Monochrome is easy, and so is color if nothing is specified. */
	if (!appres.m3279 ||
		(!ea_buf[baddr].fg &&
		 !ea_buf[fa_addr].fg &&
		 !ea_buf[baddr].bg &&
		 !ea_buf[fa_addr].bg))
	{
		a = color_from_fa(fa);
	}
	else
	{

		/* The current location or the fa specifies the fg or bg. */
		if (ea_buf[baddr].fg)
		{
			fg = ea_buf[baddr].fg & 0x0f;
		}
		else if (ea_buf[fa_addr].fg)
		{
			fg = ea_buf[fa_addr].fg & 0x0f;
		}
		else
		{
			fg = DEFCOLOR_MAP(fa);
		}

		if (ea_buf[baddr].bg)
			bg = ea_buf[baddr].bg & 0x0f;
		else if (ea_buf[fa_addr].bg)
			bg = ea_buf[fa_addr].bg & 0x0f;
		else
			bg = 0;

		a = get_color_pair(fg, bg);
	}

	/* Compute the display attributes. */

	if (ea_buf[baddr].gr)
		gr = ea_buf[baddr].gr;
	else if (ea_buf[fa_addr].gr)
		gr = ea_buf[fa_addr].gr;
	else
		gr = 0;

	if(!(gr & GR_REVERSE) && !bg)
	{
		if(gr & GR_BLINK)
			a |= LIB3270_ATTR_BLINK;

		if(gr & GR_UNDERLINE)
			a |= LIB3270_ATTR_UNDERLINE;
	}

	if(appres.m3279 && (gr & (GR_BLINK | GR_UNDERLINE)) && !(gr & GR_REVERSE) && !bg)
    	a |= LIB3270_ATTR_BACKGROUND_INTENSITY;

	if(!appres.m3279 &&	((gr & GR_INTENSIFY) || FA_IS_HIGH(fa)))
		a |= LIB3270_ATTR_INTENSIFY;

	if (gr & GR_REVERSE)
		a = get_color_pair(((a & 0xF0) >> 4),(a & 0x0F)) | (a&0xFF00); // a = reverse_colors(a);

	return a;
}

/* Erase screen */
void screen_erase(H3270 *session)
{
	/* If the application supplies a callback use it!, if not just redraw with blanks */
	if(session->erase)
		session->erase(session);
	else
		screen_update(session,0,session->rows * session->cols);
}

LIB3270_EXPORT void lib3270_get_screen_size(H3270 *h, int *r, int *c)
{
	*r = h->rows;
	*c = h->cols;
}

void update_model_info(H3270 *session, int model, int cols, int rows)
{
	if(model == session->model_num && session->maxROWS == rows && session->maxCOLS == cols)
		return;

	session->maxCOLS   	= cols;
	session->maxROWS   	= rows;
	session->model_num	= model;

	/* Update the model name. */
	(void) sprintf(session->model_name, "327%c-%d%s",appres.m3279 ? '9' : '8',session->model_num,appres.extended ? "-E" : "");

	if(session->update_model)
		session->update_model(session, session->model_name,session->model_num,rows,cols);
	else if(callbacks && callbacks->model_changed)
		callbacks->model_changed(session, session->model_name,session->model_num,rows,cols);
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
		*(chr++)  = ea_buf[baddr].chr ? ea_buf[baddr].chr : ' ';
		*(attr++) = ea_buf[baddr].attr;
	}

	return 0;
}

/* Get screen contents */
int screen_read(char *dest, int baddr, int count)
{
	unsigned char fa	= get_field_attribute(&h3270,baddr);
	int 			max = (h3270.maxROWS * h3270.maxCOLS);

	*dest = 0;

	while(count-- > 0)
	{
		if(baddr > max)
		{
			*dest = 0;
			return EFAULT;
		}

		if (ea_buf[baddr].fa)
			fa = ea_buf[baddr].fa;

		if(FA_IS_ZERO(fa))
			*dest = ' ';
		else if(ea_buf[baddr].cc)
			*dest = ebc2asc[ea_buf[baddr].cc];
		else
			*dest = ' ';

		dest++;
		baddr++;
	}
	*dest = 0;

	return 0;
}

/* Display what's in the buffer. */
static void screen_update(H3270 *session, int bstart, int bend)
{
	int baddr;
	unsigned short a;
	int attr = COLOR_GREEN;
	unsigned char fa;
	int fa_addr;

	fa		= get_field_attribute(session,bstart);
	a  		= color_from_fa(fa);
	fa_addr = find_field_attribute(session,bstart); // may be -1, that's okay

	Trace("%s ea_buf=%p",__FUNCTION__,ea_buf);

	for(baddr = bstart; baddr < bend; baddr++)
	{
		if(ea_buf[baddr].fa)
		{
			// Field attribute.
			fa_addr = baddr;
			fa = ea_buf[baddr].fa;
			a = calc_attrs(baddr, baddr, fa);
			addch(session,baddr,' ',(attr = COLOR_GREEN)|CHAR_ATTR_MARKER);
		}
		else if (FA_IS_ZERO(fa))
		{
			// Blank.
			addch(session,baddr,' ',attr=a);
		}
		else
		{
			// Normal text.
			if (!(ea_buf[baddr].gr || ea_buf[baddr].fg || ea_buf[baddr].bg))
			{
				attr = a;
			}
			else
			{
//				unsigned short b;
				//
				// Override some of the field
				// attributes.
				//
//				attr = b = calc_attrs(baddr, fa_addr, fa);

				attr = calc_attrs(baddr, fa_addr, fa);
			}

			if (ea_buf[baddr].cs == CS_LINEDRAW)
			{
				addch(session,baddr,ea_buf[baddr].cc,attr);
			}
			else if (ea_buf[baddr].cs == CS_APL || (ea_buf[baddr].cs & CS_GE))
			{
				addch(session,baddr,ea_buf[baddr].cc,attr|CHAR_ATTR_CG);
			}
			else
			{
				if (toggled(MONOCASE))
					addch(session,baddr,asc2uc[ebc2asc[ea_buf[baddr].cc]],attr);
				else
					addch(session,baddr,ebc2asc[ea_buf[baddr].cc],attr);
			}
		}

	}

	if(session->changed)
		session->changed(session,bstart,bend);

}

void screen_disp(H3270 *session)
{
//	session->first_changed = -1;
//	session->last_changed = -1;

	screen_update(session,0,session->rows*session->cols);

	if(callbacks && callbacks->display)
		callbacks->display(session);
}

void screen_suspend(H3270 *session)
{
	if(callbacks && callbacks->set_suspended)
		callbacks->set_suspended(1);
}

void screen_resume(H3270 *session)
{
	screen_disp(session);

	if(callbacks && callbacks->set_suspended)
		callbacks->set_suspended(0);
}

LIB3270_EXPORT int lib3270_get_cursor_address(H3270 *h)
{
    CHECK_SESSION_HANDLE(h);
    return h->cursor_addr;
}

LIB3270_EXPORT int lib3270_set_cursor_address(H3270 *h, int baddr)
{
    int ret;

    CHECK_SESSION_HANDLE(h);

    ret = h->cursor_addr;

	if(ret == baddr)
		return ret;

	h->cursor_addr = baddr;

	if(h->update_cursor)
		h->update_cursor(h,(unsigned short) (baddr/h->cols),(unsigned short) (baddr%h->cols),ea_buf[baddr].chr,ea_buf[baddr].attr);

    return ret;
}

/* Status line stuff. */

void set_status(H3270 *session, LIB3270_FLAG id, Boolean on)
{
	CHECK_SESSION_HANDLE(session);

	if(id < LIB3270_FLAG_COUNT)
	{
		session->oia_flag[id] = (on != 0);

		if(session->update_oia)
			session->update_oia(session,id,session->oia_flag[id]);
	}

}

void status_ctlr_done(H3270 *session)
{
	CHECK_SESSION_HANDLE(session);
	lib3270_event_counter[COUNTER_ID_CTLR_DONE]++;
	set_status(session,OIA_FLAG_UNDERA,True);
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

void status_resolving(H3270 *session, Boolean on)
{
	if(session->cursor)
			session->cursor(session, on ? CURSOR_MODE_LOCKED : CURSOR_MODE_NORMAL);

	status_changed(session, on ? LIB3270_STATUS_RESOLVING : LIB3270_STATUS_BLANK);
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

	screen_disp(session);

	if(callbacks && callbacks->reset)
	{
		Trace("%s calling reset",__FUNCTION__);
		callbacks->reset(kybdlock);
	}

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

#if defined(HAVE_LIBSSL) /*[*/
		set_status(session,OIA_FLAG_SECURE,session->secure_connection);
#endif /*]*/

	}
	else
	{
		set_status(session,OIA_FLAG_BOXSOLID,False);
		set_status(session,OIA_FLAG_SECURE,False);

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

static void status_printer(H3270 *session, int on, void *dunno)
{
	set_status(session,OIA_FLAG_PRINTER,on);
}

/*
SCRIPT_STATE status_script(SCRIPT_STATE state)
{
	if(state != script_state && callbacks && callbacks->set_script)
		callbacks->set_script(state);
	return script_state = state;
}
*/

void status_timing(H3270 *session, struct timeval *t0, struct timeval *t1)
{
	CHECK_SESSION_HANDLE(session);

	if(callbacks && callbacks->show_timer)
		callbacks->show_timer(t1->tv_sec - t0->tv_sec);
}

void status_untiming(H3270 *session)
{
	CHECK_SESSION_HANDLE(session);

	if(callbacks && callbacks->show_timer)
		callbacks->show_timer(-1);

	if(session->set_timer)
		session->set_timer(session,0);
}

void ring_bell(void)
{
	if(callbacks && callbacks->ring_bell)
		callbacks->ring_bell();
}

/* Set the window title. */
void
screen_title(char *text)
{
	if(callbacks && callbacks->title)
		callbacks->title(text);
}

static void
relabel(H3270 *session, int ignored unused, void *dunno)
{
#if defined(WC3270) /*[*/
	if (appres.title != CN)
	    	return;
#endif

	if (PCONNECTED)
	{
#if defined(WC3270) /*[*/
		if (profile_name != CN)
			screen_title(profile_name);
		else
#endif
			screen_title(session->reconnect_host);

	}
	else
	{
	    	screen_title(0);
	}
}

int query_counter(COUNTER_ID id)
{
	return lib3270_event_counter[id];
}

int Register3270ScreenCallbacks(const struct lib3270_screen_callbacks *cbk)
{
	if(!cbk)
		return EINVAL;

	if(cbk->sz != sizeof(struct lib3270_screen_callbacks))
		return -EINVAL;

	callbacks = cbk;

	return 0;
}

void show_3270_popup_dialog(H3270 *session, LIB3270_NOTIFY type, const char *title, const char *msg, const char *fmt, ...)
{
	CHECK_SESSION_HANDLE(session);

	if(!fmt)
		fmt = "";

	va_list arg_ptr;
	va_start(arg_ptr, fmt);

	if(callbacks && callbacks->notify)
		callbacks->notify(session,type,title,msg,fmt,arg_ptr);
	else
		lib3270_write_va_log(session,"lib3270",fmt,arg_ptr);

	va_end(arg_ptr);

}


void Error(H3270 *session, const char *fmt, ...)
{
	va_list arg_ptr;

	CHECK_SESSION_HANDLE(session);

	va_start(arg_ptr, fmt);

	if(callbacks && callbacks->notify)
		callbacks->notify(session,LIB3270_NOTIFY_ERROR, _( "3270 Error" ),NULL,fmt,arg_ptr);
	else
		lib3270_write_va_log(&h3270,"lib3270",fmt,arg_ptr);

	va_end(arg_ptr);

}

void Warning(H3270 *session, const char *fmt, ...)
{
	va_list arg_ptr;

	CHECK_SESSION_HANDLE(session);

	va_start(arg_ptr, fmt);

	if(callbacks && callbacks->notify)
		callbacks->notify(session,LIB3270_NOTIFY_WARNING, _( "3270 Warning" ),NULL,fmt,arg_ptr);
	else
		lib3270_write_va_log(&h3270,"lib3270",fmt,arg_ptr);

	va_end(arg_ptr);

}

/* Pop up an error dialog. */
extern void popup_an_error(H3270 *session, const char *fmt, ...)
{
	va_list	args;

	CHECK_SESSION_HANDLE(session);

	va_start(args, fmt);

	if(callbacks && callbacks->notify)
		callbacks->notify(session,LIB3270_NOTIFY_ERROR,_( "3270 Error" ),NULL,fmt,args);
	else
		lib3270_write_va_log(&h3270,"lib3270",fmt,args);

	va_end(args);

}

void popup_system_error(H3270 *session, const char *title, const char *message, const char *fmt, ...)
{
	va_list	args;

	CHECK_SESSION_HANDLE(session);

	va_start(args, fmt);

	if(callbacks && callbacks->notify)
		callbacks->notify(session,LIB3270_NOTIFY_ERROR,title ? title : _( "3270 Error" ), message,fmt,args);
	else
		lib3270_write_va_log(&h3270,"lib3270",fmt,args);

	va_end(args);
}



LIB3270_EXPORT void update_toggle_actions(void)
{
	int f;

	if(callbacks && callbacks->toggle_changed)
	{
		for(f=0;f< N_TOGGLES;f++)
			callbacks->toggle_changed(&h3270,f,appres.toggle[f].value,TT_UPDATE,toggle_names[f]);
	}
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
	int max = (h3270.maxROWS * h3270.maxCOLS);
	int pos = 0;
	int f;
	int fg = COLOR_BLUE;

	Trace("%s begins",__FUNCTION__);
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
		ea_buf[f].fg = fg;
		ea_buf[f].bg = (fg == COLOR_BLACK) ? COLOR_WHITE : COLOR_BLACK;
		ea_buf[f].cs = pat[row].cs;
		ea_buf[f].cc = pat[row].cc[pos++];
	}

	Trace("%s display",__FUNCTION__);

	screen_disp(&h3270);

	Trace("%s ends",__FUNCTION__);
	return 0;
}

/*
LIB3270_EXPORT struct ea * copy_device_buffer(int *el)
{
	int			sz		=  sizeof(struct ea) * (h3270.maxROWS * h3270.maxCOLS);
	struct ea	*ret	=  malloc(sz);
	memcpy(ret,ea_buf,sz);
	if(el)
		*el = (h3270.maxROWS * h3270.maxCOLS);
	return ret;
}
*/

/*
LIB3270_EXPORT HCONSOLE console_window_new(const char *title, const char *label)
{
	if(callbacks && callbacks->console_new )
		return callbacks->console_new(title,label);

	return NULL;
}

LIB3270_EXPORT void console_window_delete(HCONSOLE hwnd)
{
	if(callbacks && callbacks->console_delete )
		callbacks->console_delete(hwnd);
}

LIB3270_EXPORT int console_window_append(HCONSOLE hwnd, const char *fmt, ...)
{
	va_list args;

	if(callbacks && callbacks->console_append)
	{
		va_start(args, fmt);
		callbacks->console_append(hwnd, fmt, args);
		va_end(args);
	}

	return 0;
}

LIB3270_EXPORT char * console_window_wait_for_user_entry(HCONSOLE hwnd)
{
	if(callbacks && callbacks->console_entry )
		return callbacks->console_entry(hwnd);
	return NULL;
}
*/

