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
 * Este programa está nomeado como trace_ds.c e possui 1089 linhas de código.
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
#include "appres.h"
#include "objects.h"
#include "resources.h"
// #include "ctlr.h"

#include "charsetc.h"
#include "childc.h"
#include "ctlrc.h"
#include "popupsc.h"
#include "printc.h"
#include "savec.h"
#include "tablesc.h"
#include "telnetc.h"
#include "trace_dsc.h"
#include "utilc.h"
#include "w3miscc.h"

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
static void	wtrace(const char *fmt, ...);
// static char    *create_tracefile_header(const char *mode);
static void	stop_tracing(void);

/* Globals */
struct timeval  ds_ts;
static void (*vwtrace)(H3270 *session, const char *fmt, va_list args) = __vwtrace;
Boolean         trace_skipping = False;


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

static void
trace_ds_s(char *s, Boolean can_break)
{
	static int      dscnt = 0;
	int len = strlen(s);
	Boolean nl = False;

	if (!toggled(DS_TRACE) || !len)
		return;

	if (s && s[len-1] == '\n') {
		len--;
		nl = True;
	}
	if (!can_break && dscnt + len >= 75) {
		wtrace("...\n... ");
		dscnt = 0;
	}
	while (dscnt + len >= 75) {
		int plen = 75-dscnt;

		wtrace("%.*s ...\n... ", plen, s);
		dscnt = 4;
		s += plen;
		len -= plen;
	}
	if (len) {
		wtrace("%.*s", len, s);
		dscnt += len;
	}
	if (nl) {
		wtrace("\n");
		dscnt = 0;
	}
}

void
trace_ds(const char *fmt, ...)
{
	char tdsbuf[4096];
	va_list args;

	if (!toggled(DS_TRACE))
		return;

	va_start(args, fmt);

	/* print out remainder of message */
	(void) vsprintf(tdsbuf, fmt, args);
	trace_ds_s(tdsbuf, True);
	va_end(args);
}

void
trace_ds_nb(const char *fmt, ...)
{
	char tdsbuf[4096];
	va_list args;

	if (!toggled(DS_TRACE))
		return;

	va_start(args, fmt);

	/* print out remainder of message */
	(void) vsprintf(tdsbuf, fmt, args);
	trace_ds_s(tdsbuf, False);
	va_end(args);
}

/* Conditional event trace. */
void
trace_event(const char *fmt, ...)
{
	va_list args;

	if (!toggled(EVENT_TRACE))
		return;

	/* print out message */
	va_start(args, fmt);
	vwtrace(&h3270,fmt, args);
	va_end(args);
}

/* Conditional data stream trace, without line splitting. */
void
trace_dsn(const char *fmt, ...)
{
	va_list args;

	if (!toggled(DS_TRACE))
		return;

	/* print out message */
	va_start(args, fmt);
	vwtrace(&h3270,fmt, args);
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

/*
static void vwtrace(const char *fmt, va_list args)
{
	char buf[16384];

	vsnprintf(buf,16384,fmt,args);

	if(!tracewindow_handle)
		tracewindow_handle = console_window_new( _( "Trace Window" ), NULL );

	if(tracewindow_handle)
		console_window_append(tracewindow_handle,"%s",buf);

	if(tracef != NULL)
	{
		if(fwrite(buf,strlen(buf),1,tracef) != 1)
			popup_an_errno(errno,_( "Write to trace file failed\n%s" ),strerror(errno));
	}
}
*/

/* Write to the trace file. */
static void wtrace(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vwtrace(&h3270,fmt, args);
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



/*
static void stop_tracing(void)
{
	if (tracef != NULL && tracef != stdout)
		(void) fclose(tracef);
	tracef = NULL;
	if (tracef_pipe != NULL) {
		(void) fclose(tracef_pipe);
		tracef_pipe = NULL;
	}

	lib3270_set_toggle(&h3270,DS_TRACE,0);
	lib3270_set_toggle(&h3270,EVENT_TRACE,0);

}
*/

/* Check for a trace file rollover event.
void
trace_rollover_check(void)
{
	if (tracef == NULL || tracef_max == 0)
		return;

	// See if we've reached the midpoint.
	if (!tracef_midpoint) {
		if (tracef_size >= tracef_max / 2) {
			tracef_midpoint = ftello(tracef);
#if defined(ROLLOVER_DEBUG)
			printf("midpoint is %lld\n", tracef_midpoint);
#endif
			tracef_midpoint_header =
			    create_tracefile_header("rolled over");
		}
		return;
	}

	// See if we've reached a rollover point.
	if (tracef_size >= tracef_max) {
		char buf[8*1024];
		int nr;
		off_t rpos = tracef_midpoint, wpos = 0;

		if (!tracef_midpoint)
			Error("Tracefile rollover logic error");
#if defined(ROLLOVER_DEBUG)
		printf("rolling over at %lld\n", tracef_size);
#endif
		//
		// Overwrite the file with the midpoint header, and the data
		// which follows the midpoint.
		//
		if (fseeko(tracef, 0, SEEK_SET) < 0) {
			popup_an_errno(errno, "trace file fseeko(0) failed");
			stop_tracing();
			return;
		}
		wtrace("%s", tracef_midpoint_header);
		wpos = ftello(tracef);
		if (wpos < 0) {
			popup_an_errno(errno, "trace file ftello() failed");
			stop_tracing();
			return;
		}
		if (fseeko(tracef, rpos, SEEK_SET) < 0) {
			popup_an_errno(errno, "trace file fseeko(%ld) failed",
			    (long)rpos);
			stop_tracing();
			return;
		}
#if defined(ROLLOVER_DEBUG)
		printf("rpos = %lld, wpos = %lld\n", rpos, wpos);
#endif
		while ((nr = fread(buf, 1, sizeof(buf), tracef)) > 0) {
			rpos = ftello(tracef);
			if (fseeko(tracef, wpos, SEEK_SET) < 0) {
				popup_an_errno(errno, "trace file fseeko(%ld) "
				    "failed", (long)wpos);
				stop_tracing();
				return;
			}
			if (fwrite(buf, nr, 1, tracef) < 1)
				break;
			wpos = ftello(tracef);
			if (wpos < 0) {
				popup_an_errno(errno, "trace file ftello() "
				    "failed");
				stop_tracing();
				return;
			}
			if (fseeko(tracef, rpos, SEEK_SET) < 0) {
				popup_an_errno(errno, "trace file fseeko(%ld)"
				    "failed", (long)rpos);
				stop_tracing();
				return;
			}
		}
		if (ferror(tracef)) {
			popup_an_errno(errno, "trace file rollover copy "
			    "failed");
			stop_tracing();
			return;
		}
#if defined(ROLLOVER_DEBUG)
		printf("final wpos = %lld\n", wpos);
#endif
		if (ftruncate(fileno(tracef), wpos) < 0) {
			popup_an_errno(errno, "trace file ftruncate(%ld) "
			    "failed", (long)wpos);
			stop_tracing();
			return;
		}
		if (fseeko(tracef, wpos, SEEK_SET) < 0) {
			popup_an_errno(errno, "trace file fseeko(%ld) failed",
			    (long)wpos);
			stop_tracing();
			return;
		}
		tracef_size = wpos;
		tracef_midpoint = wpos;
		Replace(tracef_midpoint_header,
		    create_tracefile_header("rolled over"));
	}
}
*/

/*
#if defined(X3270_DISPLAY)
static Widget trace_shell = (Widget)NULL;
#endif
static int trace_reason;
*/

/* Create a trace file header. */ /*
static char * create_tracefile_header(const char *mode)
{
	char *buf;
	time_t clk;

	// Create a buffer and redirect output.
	buf = Malloc(MAX_HEADER_SIZE);
	tracef_bufptr = buf;

	// Display current status
	clk = time((time_t *)0);
	wtrace("Trace %s %s", mode, ctime(&clk));
	wtrace(" Version: %s\n", build);
	wtrace(" Model %s", h3270.model_name);
	wtrace(", %s display", appres.mono ? "monochrome" : "color");
	if (appres.extended)
		wtrace(", extended data stream");
	wtrace(", %s emulation", appres.m3279 ? "color" : "monochrome");
	wtrace(", %s charset", lib3270_get_charset(&h3270));
	if (appres.apl_mode)
		wtrace(", APL mode");
	wtrace("\n");
	if (CONNECTED)
		wtrace(" Connected to %s, port %u\n",h3270.current_host, h3270.current_port);

	// Snap the current TELNET options.
	if (net_snap_options()) {
		wtrace(" TELNET state:\n");
		trace_netdata('<', obuf, obptr - obuf);
	}

	// Dump the screen contents and modes into the trace file.
	if (CONNECTED) {
		//
		// Note that if the screen is not formatted, we do not
		// attempt to save what's on it.  However, if we're in
		// 3270 SSCP-LU or NVT mode, we'll do a dummy, empty
		// write to ensure that the display is in the right
		// mode.
		//
		if (h3270.formatted) {
			wtrace(" Screen contents:\n");
			obptr = obuf;
#if defined(X3270_TN3270E)
			(void) net_add_dummy_tn3270e();
#endif
			ctlr_snap_buffer();
			space3270out(2);
			net_add_eor(obuf, obptr - obuf);
			obptr += 2;
			trace_netdata('<', obuf, obptr - obuf);

			obptr = obuf;
#if defined(X3270_TN3270E)
			(void) net_add_dummy_tn3270e();
#endif
			if (ctlr_snap_modes()) {
				wtrace(" 3270 modes:\n");
				space3270out(2);
				net_add_eor(obuf, obptr - obuf);
				obptr += 2;
				trace_netdata('<', obuf, obptr - obuf);
			}
		}
#if defined(X3270_TN3270E)
		else if (IN_E) {
			obptr = obuf;
			if (net_add_dummy_tn3270e()) {
				wtrace(" Screen contents:\n");
				space3270out(2);
				net_add_eor(obuf, obptr - obuf);
				obptr += 2;
				trace_netdata('<', obuf, obptr - obuf);
			}
		}
#endif
	}

	wtrace(" Data stream:\n");

	// Return the buffer.
	tracef_bufptr = CN;
	return buf;
}
*/

/* Calculate the tracefile maximum size. */ /*
static void
get_tracef_max(void)
{
	static Boolean calculated = False;
	char *ptr;
	Boolean bad = False;

	if (calculated)
		return;

	calculated = True;

	if (appres.trace_file_size == CN ||
	    !strcmp(appres.trace_file_size, "0") ||
	    !strncasecmp(appres.trace_file_size, "none",
			 strlen(appres.trace_file_size))) {
		tracef_max = 0;
		return;
	}

	tracef_max = strtoul(appres.trace_file_size, &ptr, 0);
	if (tracef_max == 0 || ptr == appres.trace_file_size || *(ptr + 1)) {
		bad = True;
	} else switch (*ptr) {
	case 'k':
	case 'K':
		tracef_max *= 1024;
		break;
	case 'm':
	case 'M':
		tracef_max *= 1024 * 1024;
		break;
	case '\0':
		break;
	default:
		bad = True;
		break;
	}

	if (bad) {
		tracef_max = MIN_TRACEFILE_SIZE;
#if defined(X3270_DISPLAY)
		popup_an_info("Invalid %s '%s', assuming "
		    MIN_TRACEFILE_SIZE_NAME,
		    ResTraceFileSize,
		    appres.trace_file_size);
#endif
	} else if (tracef_max < MIN_TRACEFILE_SIZE) {
		tracef_max = MIN_TRACEFILE_SIZE;
	}
}
*/

/* Parse the name '/dev/fd<n>', so we can simulate it. */ /*
static int
get_devfd(const char *pathname)
{
	unsigned long fd;
	char *ptr;

	if (strncmp(pathname, "/dev/fd/", 8))
		return -1;
	fd = strtoul(pathname + 8, &ptr, 10);
	if (ptr == pathname + 8 || *ptr != '\0' || fd < 0)
		return -1;
	return fd;
}
*/

/* Callback for "OK" button on trace popup */ /*
static void tracefile_callback(Widget w, XtPointer client_data, XtPointer call_data unused)
{
	char *tfn = CN;
	int devfd = -1;
	char *buf;

	tfn = (char *)client_data;
	tfn = do_subst(tfn, True, True);
	if (strchr(tfn, '\'') || ((int)strlen(tfn) > 0 && tfn[strlen(tfn)-1] == '\\'))
	{
		popup_an_error("Illegal file name: %s", tfn);
		Free(tfn);
		return;
	}

	tracef_max = 0;
	tracef_midpoint = 0;
	Replace(tracef_midpoint_header, CN);

	if (!strcmp(tfn, "stdout"))
	{
		tracef = stdout;
	}
	else
	{
		// Get the trace file maximum.
		get_tracef_max();

		// If there's a limit, the file can't exist.
		if (tracef_max && !access(tfn, R_OK))
		{
			popup_an_error("Trace file '%s' already exists",tfn);
			Free(tfn);
			return;
		}

		// Open and configure the file.
		if ((devfd = get_devfd(tfn)) >= 0)
			tracef = fdopen(dup(devfd), "a");
		else
			tracef = fopen(tfn, tracef_max? "w+": "a");
		if (tracef == (FILE *)NULL)
		{
			popup_an_errno(errno, tfn);
			Free(tfn);
			return;
		}
		(void) SETLINEBUF(tracef);
#if !defined(_WIN32)
		(void) fcntl(fileno(tracef), F_SETFD, 1);
#endif
	}

	// Open pw3270's console window
	if(!tracewindow_handle)
		tracewindow_handle = console_window_new( tfn, NULL );

	Free(tfn);

	// We're really tracing, turn the flag on.
	appres.toggle[trace_reason].value = True;
//	appres.toggle[trace_reason].changed = True;
//	menubar_retoggle(&appres.toggle[trace_reason]);

	// Display current status
	buf = create_tracefile_header("started");
	wtrace("%s", buf);
	Free(buf);

}

#if defined(X3270_DISPLAY)
// Callback for "No File" button on trace popup
static void
no_tracefile_callback(Widget w, XtPointer client_data,
	XtPointer call_data unused)
{
	tracefile_callback((Widget)NULL, "", PN);
	XtPopdown(trace_shell);
}
#endif
*/

/* Open the trace file.
static void
tracefile_on(int reason, LIB3270_TOGGLE_TYPE tt)
{
	char *tracefile_buf = NULL;
	char *tracefile;

	if (tracef != (FILE *)NULL)
		return;

	trace_reason = reason;
	if (appres.secure && tt != TT_INITIAL) {
		tracefile_callback((Widget)NULL, "none", PN);
		return;
	}
	if (appres.trace_file)
	{
		tracefile = appres.trace_file;
	}
	else
	{
#if defined(_WIN32)
		tracefile_buf = xs_buffer("%sx3trc.%u.txt", PROGRAM_DATA,getpid());
#else

        if(appres.trace_dir)
            tracefile_buf = xs_buffer("%s/x3trc.%u", appres.trace_dir,getpid());
        else
            tracefile_buf = xs_buffer("%s/x3trc.%u", ".",getpid());

#endif
		tracefile = tracefile_buf;
	}


#if defined(X3270_DISPLAY)
	if (tt == TT_INITIAL || tt == TT_ACTION)
#endif
	{
		tracefile_callback((Widget)NULL, tracefile, PN);
		if (tracefile_buf != NULL)
		    	Free(tracefile_buf);
		return;
	}
#if defined(X3270_DISPLAY)
	if (trace_shell == NULL) {
		trace_shell = create_form_popup("trace",
		    tracefile_callback,
		    appres.trace_monitor? no_tracefile_callback: NULL,
		    FORM_NO_WHITE);
		XtVaSetValues(XtNameToWidget(trace_shell, ObjDialog),
		    XtNvalue, tracefile,
		    NULL);
	}

	// Turn the toggle _off_ until the popup succeeds.
	appres.toggle[reason].value = False;
	appres.toggle[reason].changed = True;

	popup_popup(trace_shell, XtGrabExclusive);
#endif

	if (tracefile_buf != NULL)
		Free(tracefile_buf);
}

// Close the trace file.
static void tracefile_off(void)
{
	time_t clk;

	clk = time((time_t *)0);
	wtrace("Trace stopped %s", ctime(&clk));

#if defined (LIB3270)

	if(tracewindow_handle != NULL)
	{
		console_window_delete(tracewindow_handle);
		tracewindow_handle = NULL;
	}

#elif !defined(_WIN32)

	if (tracewindow_pid != -1)
		(void) kill(tracewindow_pid, SIGKILL);
	tracewindow_pid = -1;

#else

	if (tracewindow_handle != NULL)
	{
	   	TerminateProcess(tracewindow_handle, 0);
		CloseHandle(tracewindow_handle);
		tracewindow_handle = NULL;
	}

#endif

	stop_tracing();
}


void toggle_dsTrace(H3270 *session, struct toggle *t unused, LIB3270_TOGGLE_TYPE tt)
{
	if (toggled(DS_TRACE) && tracef == NULL)
		tracefile_on(DS_TRACE, tt);

	// If turning off trace and not still tracing events, close the trace file.
	else if (!toggled(DS_TRACE) && !toggled(EVENT_TRACE))
		tracefile_off();

	if (toggled(DS_TRACE))
		(void) gettimeofday(&ds_ts, (struct timezone *)NULL);
}
*/

/*
void toggle_eventTrace(H3270 *session, struct toggle *t unused, LIB3270_TOGGLE_TYPE tt)
{
	// If turning on event debug, and no trace file, open one.

	if (toggled(EVENT_TRACE) && tracef == NULL)
		tracefile_on(EVENT_TRACE, tt);

	// If turning off event debug, and not tracing the data stream, close the trace file.
	else if (!toggled(EVENT_TRACE) && !toggled(DS_TRACE))
		tracefile_off();
}
*/

/* Screen trace file support. */

/*
#if defined(X3270_DISPLAY)
static Widget screentrace_shell = (Widget)NULL;
#endif
static FILE *screentracef = (FILE *)0;
*/
/*
 * Screen trace function, called when the host clears the screen.
 */
static void do_screentrace(void)
{
	wtrace("\n%s - Not implemented\n",__FUNCTION__);
/*
	register int i;

	if (fprint_screen(screentracef, False, False)) {
		for (i = 0; i < h3270.cols; i++)
			(void) fputc('=', screentracef);
		(void) fputc('\n', screentracef);
	}
*/
}

void trace_screen(void)
{
	trace_skipping = False;

	if (!toggled(SCREEN_TRACE))
		do_screentrace();
}

/* Called from ANSI emulation code to log a single character. */
void trace_char(char c)
{
	if (toggled(SCREEN_TRACE))
		wtrace("%c",c);
	return;
}

/*
 * Called when disconnecting in ANSI mode, to finish off the trace file
 * and keep the next screen clear from re-recording the screen image.
 * (In a gross violation of data hiding and modularity, trace_skipping is
 * manipulated directly in ctlr_clear()).
 */
void trace_ansi_disc(void)
{
	int i;

	wtrace("%c",'\n');
	for (i = 0; i < h3270.cols; i++)
		wtrace("%c",'=');
	wtrace("%c",'\n');

	trace_skipping = True;
}

/*
 * Screen tracing callback.
 * Returns True for success, False for failure.
 */ /*
static Boolean
screentrace_cb(char *tfn)
{
	tfn = do_subst(tfn, True, True);
	screentracef = fopen(tfn, "a");
	if (screentracef == (FILE *)NULL) {
		popup_an_errno(errno, tfn);
		Free(tfn);
		return False;
	}
	Free(tfn);
	(void) SETLINEBUF(screentracef);
#if !defined(_WIN32)
	(void) fcntl(fileno(screentracef), F_SETFD, 1);
#endif

	// We're really tracing, turn the flag on.
	appres.toggle[SCREEN_TRACE].value = True;
//	appres.toggle[SCREEN_TRACE].changed = True;
//	menubar_retoggle(&appres.toggle[SCREEN_TRACE]);
	return True;
}
*/

/*
#if defined(X3270_DISPLAY)
// Callback for "OK" button on screentrace popup
static void
screentrace_callback(Widget w unused, XtPointer client_data,
    XtPointer call_data unused)
{
	if (screentrace_cb(XawDialogGetValueString((Widget)client_data)))
		XtPopdown(screentrace_shell);
}

// Callback for second "OK" button on screentrace popup
static void
onescreen_callback(Widget w, XtPointer client_data, XtPointer call_data unused)
{
	char *tfn;

	if (w)
		tfn = XawDialogGetValueString((Widget)client_data);
	else
		tfn = (char *)client_data;
	tfn = do_subst(tfn, True, True);
	screentracef = fopen(tfn, "a");
	if (screentracef == (FILE *)NULL) {
		popup_an_errno(errno, tfn);
		XtFree(tfn);
		return;
	}
	(void) fcntl(fileno(screentracef), F_SETFD, 1);
	XtFree(tfn);

	// Save the current image, once.
	do_screentrace();

	// Close the file, we're done.
	(void) fclose(screentracef);
	screentracef = (FILE *)NULL;

	if (w)
		XtPopdown(screentrace_shell);
}
#endif */

/*
void toggle_screenTrace(H3270 *session, struct toggle *t unused, LIB3270_TOGGLE_TYPE tt)
{
	wtrace("Screen trace is %s\n",toggled(SCREEN_TRACE),"Enabled" : "Disabled");

	char *tracefile_buf = NULL;
	char *tracefile;

	if (toggled(SCREEN_TRACE)) {
		if (appres.screentrace_file)
			tracefile = appres.screentrace_file;
		else {
#if defined(_WIN32)
			tracefile_buf = xs_buffer("%sx3scr.%u.txt",PROGRAM_DATA, getpid());
#else
            if(appres.trace_dir)
                tracefile_buf = xs_buffer("%s/x3scr.%u",appres.trace_dir, getpid());
            else
                tracefile_buf = xs_buffer("%s/x3scr.%u",".", getpid());
#endif
			tracefile = tracefile_buf;
		}
		if (tt == TT_INITIAL || tt == TT_ACTION) {
			(void) screentrace_cb(NewString(tracefile));
			if (tracefile_buf != NULL)
				Free(tracefile_buf);
			return;
		}
#if defined(X3270_DISPLAY)
		if (screentrace_shell == NULL) {
			screentrace_shell = create_form_popup("screentrace",
			    screentrace_callback, onescreen_callback,
			    FORM_NO_WHITE);
			XtVaSetValues(XtNameToWidget(screentrace_shell,
					ObjDialog),
			    XtNvalue, tracefile,
			    NULL);
		}
		appres.toggle[SCREEN_TRACE].value = False;
		appres.toggle[SCREEN_TRACE].changed = True;
		popup_popup(screentrace_shell, XtGrabExclusive);
#endif
	} else {
		if (ctlr_any_data() && !trace_skipping)
			do_screentrace();
		(void) fclose(screentracef);
	}

	if (tracefile_buf != NULL)
		Free(tracefile_buf);
}
*/

#endif /*]*/
