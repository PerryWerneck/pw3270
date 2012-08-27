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
 * Este programa está nomeado como trace_ds.c e possui - linhas de código.
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
 *	trace_ds.c
 *		3270 data stream tracing.
 *
 */

#include "globals.h"

#if defined(X3270_TRACE) /*[*/

#if defined(X3270_DISPLAY) /*[*/
#include <X11/StringDefs.h>
#include <X11/Xaw/Dialog.h>
#endif /*]*/
#if defined(_WIN32) /*[*/
#include <windows.h>
#endif /*]*/
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <fcntl.h>
#include "3270ds.h"
//#include "appres.h"
// #include "objects.h"
#include "resources.h"
// #include "ctlr.h"

#include "charsetc.h"
// #include "childc.h"
#include "ctlrc.h"
#include "popupsc.h"
// #include "printc.h"
// #include "savec.h"
#include "tablesc.h"
#include "telnetc.h"
#include "trace_dsc.h"
#include "utilc.h"
// #include "w3miscc.h"
#include "toggle.h"

/* Maximum size of a tracefile header. */
#define MAX_HEADER_SIZE		(10*1024)

/* Minimum size of a trace file. */
// #define MIN_TRACEFILE_SIZE	(64*1024)
// #define MIN_TRACEFILE_SIZE_NAME	"64K"

/* System calls which may not be there. */
// #if !defined(HAVE_FSEEKO)
// #define fseeko(s, o, w)	fseek(s, (long)o, w)
// #define ftello(s)	(off_t)ftell(s)
// #endif

// #include <lib3270/api.h>

/* Statics */
// static int      dscnt = 0;

/*
#if defined (LIB3270)
	HCONSOLE		tracewindow_handle = 0;
#elif defined(_WIN32)
	static HANDLE	tracewindow_handle = NULL;
#else
	static int		tracewindow_pid = -1;
#endif
*/

// static FILE    *tracef = NULL;
// static FILE    *tracef_pipe = NULL;
// static char    *tracef_bufptr = CN;
// static off_t	tracef_size = 0;
// static off_t	tracef_max = 0;
// static char    *tracef_midpoint_header = CN;
// static off_t	tracef_midpoint = 0;

static void __vwtrace(H3270 *session, const char *fmt, va_list args);
static void	wtrace(H3270 *session, const char *fmt, ...);
// static char    *create_tracefile_header(const char *mode);
// static void	stop_tracing(void);

/* Globals */
// struct timeval  ds_ts;

static void (*vwtrace)(H3270 *session, const char *fmt, va_list args) = __vwtrace;
// Boolean         trace_skipping = False;


LIB3270_EXPORT void lib3270_set_trace_handler( void (*handler)(H3270 *session, const char *fmt, va_list args) )
{
	vwtrace = handler ? handler : __vwtrace;
}

/* display a (row,col) */
const char * rcba(H3270 *hSession, int baddr)
{
	static char buf[16];
	(void) sprintf(buf, "(%d,%d)", baddr/hSession->cols + 1, baddr%hSession->cols + 1);
	return buf;
}

/* Data Stream trace print, handles line wraps */

// static char *tdsbuf = CN;
// #define TDS_LEN	75

static void trace_ds_s(H3270 *hSession, char *s, Boolean can_break)
{
	static int      dscnt = 0;
	int len = strlen(s);
	Boolean nl = False;

	if (!lib3270_get_toggle(hSession,DS_TRACE) || !len)
		return;

	if (s && s[len-1] == '\n')
	{
		len--;
		nl = True;
	}

	if (!can_break && dscnt + len >= 75)
	{
		wtrace(hSession,"...\n... ");
		dscnt = 0;
	}

	while (dscnt + len >= 75)
	{
		int plen = 75-dscnt;

		wtrace(hSession,"%.*s ...\n... ", plen, s);
		dscnt = 4;
		s += plen;
		len -= plen;
	}

	if (len)
	{
		wtrace(hSession,"%.*s", len, s);
		dscnt += len;
	}

	if (nl)
	{
		wtrace(hSession,"\n");
		dscnt = 0;
	}
}

void trace_ds(H3270 *hSession, const char *fmt, ...)
{
	char	* text;
	va_list   args;

	if (!lib3270_get_toggle(hSession,DS_TRACE))
		return;

	va_start(args, fmt);

	/* print out remainder of message */
	text = lib3270_vsprintf(fmt,args);
	trace_ds_s(hSession,text, True);
	va_end(args);
	lib3270_free(text);
}

void trace_ds_nb(H3270 *hSession, const char *fmt, ...)
{
	char *text;
	va_list args;

	if (!lib3270_get_toggle(hSession,DS_TRACE))
		return;

	va_start(args, fmt);

	/* print out remainder of message */
	text = lib3270_vsprintf(fmt,args);
	trace_ds_s(hSession, text, False);
	lib3270_free(text);
}

/* Conditional data stream trace, without line splitting. */
void trace_dsn(H3270 *hSession, const char *fmt, ...)
{
	va_list args;

	if (!lib3270_get_toggle(hSession,DS_TRACE))
		return;

	/* print out message */
	va_start(args, fmt);
	vwtrace(hSession,fmt, args);
	va_end(args);
}

/*
 * Write to the trace file, varargs style.
 * This is the only function that actually does output to the trace file --
 * all others are wrappers around this function.
 */
static void __vwtrace(H3270 *session, const char *fmt, va_list args)
{
	vfprintf(stdout,fmt,args);
	fflush(stdout);
}

/* Write to the trace file. */
static void wtrace(H3270 *hSession, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vwtrace(hSession,fmt, args);
	va_end(args);
}

LIB3270_EXPORT void lib3270_write_dstrace(H3270 *session, const char *fmt, ...)
{
	va_list args;

	if(!lib3270_get_toggle(session,LIB3270_TOGGLE_DS_TRACE))
		return;

	va_start(args, fmt);
	vwtrace(session,fmt, args);
	va_end(args);
}

LIB3270_EXPORT void lib3270_trace_event(H3270 *session, const char *fmt, ...)
{
	va_list args;

	if(!lib3270_get_toggle(session,LIB3270_TOGGLE_EVENT_TRACE))
		return;

	va_start(args, fmt);
	vwtrace(session,fmt, args);
	va_end(args);
}

/**
 * Screen trace function, called when the host clears the screen.
 *
 * @param session	Session Handle
 */
void trace_screen(H3270 *session)
{
	session->trace_skipping = 0;

	if (lib3270_get_toggle(session,LIB3270_TOGGLE_SCREEN_TRACE))
	{
		int row, baddr;

		for(row=baddr=0;row < session->rows;row++)
		{
			int col;
			wtrace(session,"%02d ",row+1);

			for(col = 0; col < session->cols;col++)
			{
				if(session->text[baddr].attr & LIB3270_ATTR_CG)
					wtrace(session,"%c",'.');
				else if(session->text[baddr].chr)
					wtrace(session,"%c",session->text[baddr].chr);
				else
					wtrace(session,"%c",'.');
				baddr++;
			}
			wtrace(session,"%s\n","");
		}
	}
}

/* Called from ANSI emulation code to log a single character. */
void trace_char(H3270 *hSession, char c)
{
	if (lib3270_get_toggle(hSession,LIB3270_TOGGLE_SCREEN_TRACE))
		wtrace(hSession,"%c",c);
	return;
}

/**
 * Called when disconnecting in ANSI modeto finish off the trace file.
 *
 * Called when disconnecting in ANSI mode to finish off the trace file
 * and keep the next screen clear from re-recording the screen image.
 * (In a gross violation of data hiding and modularity, trace_skipping is
 * manipulated directly in ctlr_clear()).
 *
 *
 */
void trace_ansi_disc(H3270 *hSession)
{
	int i;

	wtrace(hSession,"%c",'\n');
	for (i = 0; i < hSession->cols; i++)
		wtrace(hSession,"%c",'=');
	wtrace(hSession,"%c",'\n');

	hSession->trace_skipping = 1;
}


#endif
