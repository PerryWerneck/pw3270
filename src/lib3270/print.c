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
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como print.c e possui 748 linhas de código.
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
 *	print.c
 *		Screen printing functions.
 */

#include "globals.h"

#include "appres.h"
#include "3270ds.h"
#include "ctlr.h"

#include "ctlrc.h"
#include "tablesc.h"

#include <errno.h>

#if defined(X3270_DISPLAY) /*[*/
#include <X11/StringDefs.h>
#include <X11/Xaw/Dialog.h>
#endif /*]*/

#include "objects.h"
#include "resources.h"

#include "actionsc.h"
#include "popupsc.h"
#include "printc.h"
#include "utf8c.h"
#include "utilc.h"
#if defined(X3270_DBCS) /*[*/
#include "widec.h"
#endif /*]*/
#if defined(_WIN32) /*[*/
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif /*]*/

#include "api.h"

/* Statics */

/*
#if defined(X3270_DISPLAY)
static Widget print_text_shell = (Widget)NULL;
static Widget save_text_shell = (Widget)NULL;
static Widget print_window_shell = (Widget)NULL;
static char *print_window_command = CN;
#endif
*/

/* Print Text popup */

/*
 * Map default 3279 colors.  This code is duplicated three times. ;-(
 */
static int
color_from_fa(unsigned char fa)
{
	static int field_colors[4] = {
		COLOR_GREEN,        /* default */
		COLOR_RED,          /* intensified */
		COLOR_BLUE,         /* protected */
		COLOR_WHITE         /* protected, intensified */
#       define DEFCOLOR_MAP(f) \
		((((f) & FA_PROTECT) >> 4) | (((f) & FA_INT_HIGH_SEL) >> 3))
	};

	if (appres.m3279)
		return field_colors[DEFCOLOR_MAP(fa)];
	else
		return COLOR_GREEN;
}

/*
 * Map 3279 colors onto HTML colors.
 */
static char *
html_color(int color)
{
	static char *html_color_map[] = {
		"black",
		"deepSkyBlue",
		"red",
		"pink",
		"green",
		"turquoise",
		"yellow",
		"white",
		"black",
		"blue3",
		"orange",
		"purple",
		"paleGreen",
		"paleTurquoise2",
		"grey",
		"white"
	};
	if (color >= COLOR_NEUTRAL_BLACK && color <= COLOR_WHITE)
		return html_color_map[color];
	else
		return "black";
}


/*
 * Print the ASCIIfied contents of the screen onto a stream.
 * Returns True if anything printed, False otherwise.
 *
 * If 'use_html' is True, then HTML is generated, which preserves colors, but
 * little else (for now).
 */
Boolean
fprint_screen(FILE *f, Boolean even_if_empty, Boolean use_html)
{
	register int i;
	char c;
	int ns = 0;
	int nr = 0;
	Boolean any = False;
	int fa_addr = find_field_attribute(&h3270,0);
	unsigned char fa = h3270.ea_buf[fa_addr].fa;
	int fa_color, current_color;
	Bool fa_high, current_high;

	if (use_html) {
		even_if_empty = True;
	}

	if (h3270.ea_buf[fa_addr].fg)
		fa_color = h3270.ea_buf[fa_addr].fg & 0x0f;
	else
		fa_color = color_from_fa(fa);
	current_color = fa_color;

	if (h3270.ea_buf[fa_addr].gr & GR_INTENSIFY)
		fa_high = True;
	else
		fa_high = FA_IS_HIGH(fa);
	current_high = fa_high;

	for (i = 0; i < h3270.rows*h3270.cols; i++) {
#if defined(X3270_DBCS) /*[*/
		char mb[16];
		Boolean is_dbcs = False;
#endif /*]*/

		if (i && !(i % h3270.cols)) {
			nr++;
			ns = 0;
		}
		if (h3270.ea_buf[i].fa) {
			c = ' ';
			fa = h3270.ea_buf[i].fa;
			if (h3270.ea_buf[i].fg)
				fa_color = h3270.ea_buf[i].fg & 0x0f;
			else
				fa_color = color_from_fa(fa);
			if (h3270.ea_buf[i].gr & GR_INTENSIFY)
				fa_high = True;
			else
				fa_high = FA_IS_HIGH(fa);
		}
		if (FA_IS_ZERO(fa))
			c = ' ';
#if defined(X3270_DBCS) /*[*/
		else {
			/* XXX: DBCS/html interactions are not done */
			switch (ctlr_dbcs_state(i)) {
			case DBCS_NONE:
			case DBCS_SB:
				c = ebc2asc[ea_buf[i].cc];
				break;
			case DBCS_LEFT:
				dbcs_to_mb(ea_buf[i].cc, ea_buf[i + 1].cc, mb);
				is_dbcs = True;
				c = 'x';
				break;
			default:
				c = ' ';
				break;
			}
		}
#else /*][*/
		else
			c = ebc2asc[h3270.ea_buf[i].cc];
#endif /*]*/
		if (c == ' ')
			ns++;
		else {
			while (nr) {
				(void) fputc('\n', f);
				nr--;
			}
			while (ns) {
				(void) fputc(' ', f);
				ns--;
			}
			if (use_html) {
				int color;
				Bool high;

				if (h3270.ea_buf[i].fg)
					color = h3270.ea_buf[i].fg & 0x0f;
				else
					color = fa_color;
				if (color != current_color) {
					if (any)
						fprintf(f, "</font><font color=%s>",
							html_color(color));
					current_color = color;
				}
				if (h3270.ea_buf[i].gr & GR_INTENSIFY)
					high = True;
				else
					high = fa_high;
				if (high != current_high) {
					if (any) {
						if (high)
							fprintf(f, "<b>");
						else
							fprintf(f, "</b>");
					}
					current_high = high;
				}
				if (!any) {
					fprintf(f, "<html>\n"
						   "<head>\n"
						   " <meta http-equiv=\"Content-Type\" content=\"text/html; charset=%s\">\n"
						   "</head>\n"
						   " <body>\n"
						   "  <table border=0>"
						   "<tr bgcolor=black><td>"
						   "<pre><font color=%s>%s",
						   locale_codeset,
						   html_color(current_color),
						   current_high? "<b>": "");
				}
			}
			any = True;
#if defined(X3270_DBCS) /*[*/
			if (is_dbcs) {
				(void) fputs(mb, f);
				i++;
			}
			else
#endif /*]*/
			{
				if (use_html && c == '<')
					fprintf(f, "&lt;");
				else
					(void) fputs(utf8_expand(c), f);
			}
		}
	}
	nr++;
	if (!any && !even_if_empty)
		return False;
	while (nr) {
		(void) fputc('\n', f);
		nr--;
	}
	if (use_html && any) {
		fprintf(f, "%s</font></pre></td></tr>\n"
		           "  </table>\n"
			   " </body>\n"
			   "</html>\n",
			   current_high? "</b>": "");
	}
	return True;
}

/* Termination code for print text process. */ /*
static void
print_text_done(FILE *f, Boolean do_popdown
#if defined(X3270_DISPLAY)
					    unused
#endif
						  )
{
	int status;

	status = pclose(f);
	if (status) {
		popup_an_error("Print program exited with status %d.",
		    (status & 0xff00) > 8);
	} else {
#if defined(X3270_DISPLAY)
		if (do_popdown)
			XtPopdown(print_text_shell);
		if (appres.do_confirms)
			popup_an_info("Screen image printed.");
#endif
	}

}
*/

#if defined(X3270_DISPLAY)

/* Callback for "OK" button on the print text popup. */ /*
static void
print_text_callback(Widget w unused, XtPointer client_data,
    XtPointer call_data unused)
{
	char *filter;
	FILE *f;

	filter = XawDialogGetValueString((Widget)client_data);
	if (!filter) {
		XtPopdown(print_text_shell);
		return;
	}
	if (!(f = popen(filter, "w"))) {
		popup_an_errno(errno, "popen(%s)", filter);
		return;
	}
	(void) fprint_screen(f, True, False);
	print_text_done(f, True);
}
*/
/* Callback for "Plain Text" button on save text popup. *//*
static void
save_text_plain_callback(Widget w unused, XtPointer client_data,
    XtPointer call_data unused)
{
	char *filename;
	FILE *f;

	filename = XawDialogGetValueString((Widget)client_data);
	if (!filename) {
		XtPopdown(save_text_shell);
		return;
	}
	if (!(f = fopen(filename, "a"))) {
		popup_an_errno(errno, "%s", filename);
		return;
	}
	(void) fprint_screen(f, True, False);
	fclose(f);
	XtPopdown(save_text_shell);
	if (appres.do_confirms)
		popup_an_info("Screen image saved.");
}
*/
/* Callback for "HTML" button on save text popup. */ /*
static void
save_text_html_callback(Widget w unused, XtPointer client_data,
    XtPointer call_data unused)
{
	char *filename;
	FILE *f;

	filename = XawDialogGetValueString((Widget)client_data);
	if (!filename) {
		XtPopdown(save_text_shell);
		return;
	}
	if (!(f = fopen(filename, "a"))) {
		popup_an_errno(errno, "%s", filename);
		return;
	}
	(void) fprint_screen(f, True, True);
	fclose(f);
	XtPopdown(save_text_shell);
	if (appres.do_confirms)
		popup_an_info("Screen image saved.");
}
*/
/* Pop up the Print Text dialog, given a filter. */ /*
static void
popup_print_text(char *filter)
{
	if (print_text_shell == NULL) {
		print_text_shell = create_form_popup("PrintText",
		    print_text_callback, (XtCallbackProc)NULL,
		    FORM_AS_IS);
		XtVaSetValues(XtNameToWidget(print_text_shell, ObjDialog),
		    XtNvalue, filter,
		    NULL);
	}
	popup_popup(print_text_shell, XtGrabExclusive);
}
*/
/* Pop up the Save Text dialog. */ /*
static void
popup_save_text(char *filename)
{
	if (save_text_shell == NULL) {
		save_text_shell = create_form_popup("SaveText",
		    save_text_plain_callback,
		    save_text_html_callback,
		    FORM_AS_IS);
	}
	if (filename != CN)
		XtVaSetValues(XtNameToWidget(save_text_shell, ObjDialog),
		    XtNvalue, filename,
		    NULL);
	popup_popup(save_text_shell, XtGrabExclusive);
}
*/

#endif

/* Print or save the contents of the screen as text. */ /*
void
PrintText_action(Widget w unused, XEvent *event, String *params,
    Cardinal *num_params)
{
	int i;
	char *filter = CN;
	Boolean secure = appres.secure;
	Boolean use_html = False;
	Boolean use_file = False;
	Boolean use_string = False;

	//
	// Pick off optional arguments:
	// file     directs the output to a file instead of a command;
	//			must be the last keyword
	// html     generates HTML output instead of ASCII text (and implies
	//			'file')
	// secure   disables the pop-up dialog, if this action is invoked from
	//			a keymap
	// command  directs the output to a command (this is the default, but
	//			allows the command to be one of the other keywords);
	//			must be the last keyword
	// string   returns the data as a string, allowed only from scripts
	//
	for (i = 0; i < *num_params; i++) {
		if (!strcasecmp(params[i], "file")) {
			use_file = True;
			i++;
			break;
		} else if (!strcasecmp(params[i], "html")) {
			use_html = True;
			use_file = True;
		} else if (!strcasecmp(params[i], "secure")) {
			secure = True;
		} else if (!strcasecmp(params[i], "command")) {
			if (use_html || use_file) {
				popup_an_error("%s: contradictory options",
				    action_name(PrintText_action));
				return;
			}
			i++;
			break;
		} else if (!strcasecmp(params[i], "string")) {
			if (ia_cause != IA_SCRIPT) {
				popup_an_error("%s(string) can only be used "
						"from a script",
				    action_name(PrintText_action));
				return;
			}
			use_string = True;
			use_file = True;
		} else {
			break;
		}
	}
	switch (*num_params - i) {
	case 0:
		// Use the default.
		if (!use_file)
			filter = get_resource(ResPrintTextCommand);
		break;
	case 1:
		if (use_string) {
			popup_an_error("%s: extra arguments or invalid option(s)",
			    action_name(PrintText_action));
			return;
		}
		filter = params[i];
		break;
	default:
		popup_an_error("%s: extra arguments or invalid option(s)",
		    action_name(PrintText_action));
		return;
	}

	if (filter != CN && filter[0] == '@') {
		//
		// Starting the PrintTextCommand resource value with '@'
		// suppresses the pop-up dialog, as does setting the 'secure'
		// resource.
		//
		secure = True;
		filter++;
	}
	if (!use_file && (filter == CN || !*filter))
		filter = "lpr";

#if defined(X3270_DISPLAY)
	if (secure ||
		ia_cause == IA_COMMAND ||
		ia_cause == IA_MACRO ||
		ia_cause == IA_SCRIPT)
#endif
	{
		FILE *f;
		int fd = -1;

		// Invoked non-interactively.
		if (use_file) {
			if (use_string) {
				char temp_name[15];

#if defined(_WIN32)
				strcpy(temp_name, "x3hXXXXXX");
				mktemp(temp_name);
				fd = _open(temp_name, _O_RDWR,
					_S_IREAD | _S_IWRITE);
#else
				strcpy(temp_name, "/tmp/x3hXXXXXX");
				fd = mkstemp(temp_name);
#endif
				if (fd < 0) {
					popup_an_errno(errno, "mkstemp");
					return;
				}
				(void) unlink(temp_name);
				f = fdopen(fd, "w+");
			} else {
				if (filter == CN || !*filter) {
					popup_an_error("%s: missing filename",
						action_name(PrintText_action));
					return;
				}
				f = fopen(filter, "a");
			}
		} else
			f = popen(filter, "w");
		if (f == NULL) {
			popup_an_errno(errno, "%s: %s",
					action_name(PrintText_action),
					filter);
			if (fd >= 0) {
				(void) close(fd);
			}
			return;
		}
		(void) fprint_screen(f, True, use_html);
		if (use_string) {
			char buf[8192];

			rewind(f);
			while (fgets(buf, sizeof(buf), f) != NULL)
				action_output(buf);
		}
		if (use_file)
			fclose(f);
		else
			print_text_done(f, False);
		return;
	}

#if defined(X3270_DISPLAY)
	// Invoked interactively -- pop up the confirmation dialog.
	if (use_file) {
		popup_save_text(filter);
	} else {
		popup_print_text(filter);
	}
#endif
}
*/

#if defined(X3270_DISPLAY) /*[*/
#if defined(X3270_MENUS) /*[*/


/* Callback for Print Text menu option. */ /*
void
print_text_option(Widget w, XtPointer client_data unused,
    XtPointer call_data unused)
{
	char *filter = get_resource(ResPrintTextCommand);
	Boolean secure = appres.secure;
	Boolean use_html = False;

	// Decode the filter.
	if (filter != CN && *filter == '@') {
		secure = True;
		filter++;
	}
	if (filter == CN || !*filter)
		filter = "lpr";

	if (secure) {
		FILE *f;

		// Print the screen without confirming.
		if (!(f = popen(filter, "w"))) {
			popup_an_errno(errno, "popen(%s)", filter);
			return;
		}
		(void) fprint_screen(f, True, use_html);
		print_text_done(f, False);
	} else {
		// Pop up a dialog to confirm or modify their choice.
		popup_print_text(filter);
	}
}
*/

/* Callback for Save Text menu option. */ /*
void
save_text_option(Widget w, XtPointer client_data unused,
    XtPointer call_data unused)
{
	// Pop up a dialog to confirm or modify their choice.
	popup_save_text(CN);
} */
#endif


/* Print Window popup */

/*
 * Printing the window bitmap is a rather convoluted process:
 *    The PrintWindow action calls PrintWindow_action(), or a menu option calls
 *	print_window_option().
 *    print_window_option() pops up the dialog.
 *    The OK button on the dialog triggers print_window_callback.
 *    print_window_callback pops down the dialog, then schedules a timeout
 *     1 second away.
 *    When the timeout expires, it triggers snap_it(), which finally calls
 *     xwd.
 * The timeout indirection is necessary because xwd prints the actual contents
 * of the window, including any pop-up dialog in front of it.  We pop down the
 * dialog, but then it is up to the server and Xt to send us the appropriate
 * expose events to repaint our window.  Hopefully, one second is enough to do
 * that.
 */

/* Termination procedure for window print. */ /*
static void
print_window_done(int status)
{
	if (status)
		popup_an_error("Print program exited with status %d.",
		    (status & 0xff00) >> 8);
	else if (appres.do_confirms)
		popup_an_info("Bitmap printed.");
}
*/
/* Timeout callback for window print. */ /*
static void
snap_it(XtPointer closure unused, XtIntervalId *id unused)
{
	if (!print_window_command)
		return;
	XSync(display, 0);
	print_window_done(system(print_window_command));
	print_window_command = CN;
}
*/
/* Callback for "OK" button on print window popup. */ /*
static void
print_window_callback(Widget w unused, XtPointer client_data,
    XtPointer call_data unused)
{
	print_window_command = XawDialogGetValueString((Widget)client_data);
	XtPopdown(print_window_shell);
	if (print_window_command)
		(void) XtAppAddTimeOut(appcontext, 1000, snap_it, 0);
} */

/* Print the contents of the screen as a bitmap. */ /*
void
PrintWindow_action(Widget w unused, XEvent *event, String *params,
    Cardinal *num_params)
{
	char *filter = get_resource(ResPrintWindowCommand);
	char *fb = XtMalloc(strlen(filter) + 16);
	char *xfb = fb;
	Boolean secure = appres.secure;

	if (*num_params > 0)
		filter = params[0];
	if (*num_params > 1)
		popup_an_error("%s: extra arguments ignored",
		    action_name(PrintWindow_action));
	if (filter == CN) {
		popup_an_error("%s: no %s defined",
		    action_name(PrintWindow_action), ResPrintWindowCommand);
		return;
	}
	(void) sprintf(fb, filter, XtWindow(toplevel));
	if (fb[0] == '@') {
		secure = True;
		xfb = fb + 1;
	}
	if (secure) {
		print_window_done(system(xfb));
		Free(fb);
		return;
	}
	if (print_window_shell == NULL)
		print_window_shell = create_form_popup("printWindow",
		    print_window_callback, (XtCallbackProc)NULL, FORM_AS_IS);
	XtVaSetValues(XtNameToWidget(print_window_shell, ObjDialog),
	    XtNvalue, fb,
	    NULL);
	popup_popup(print_window_shell, XtGrabExclusive);
}
*/

#if defined(X3270_MENUS) /*[*/
/* Callback for menu Print Window option. */ /*
void
print_window_option(Widget w, XtPointer client_data unused,
    XtPointer call_data unused)
{
	Cardinal zero = 0;

	PrintWindow_action(w, (XEvent *)NULL, (String *)NULL, &zero);
} */
#endif /*]*/

#endif /*]*/
