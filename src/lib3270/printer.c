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
 * Este programa está nomeado como printer.c e possui 750 linhas de código.
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
 *	printer.c
 *		Printer session support
 */

#include "globals.h"

#if (defined(C3270) || defined(X3270_DISPLAY)) && defined(X3270_PRINTER) /*[*/
#if defined(X3270_DISPLAY) /*[*/
#include <X11/StringDefs.h>
#include <X11/Xaw/Dialog.h>
#endif /*]*/
#if defined(_WIN32) /*[*/
#include "windows.h"
#endif /*]*/
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <fcntl.h>
#include "3270ds.h"
#include "appres.h"
#include "objects.h"
#include "resources.h"
#include "ctlr.h"

#include "charsetc.h"
#include "ctlrc.h"
#include "hostc.h"
// #include "menubarc.h"
#include "popupsc.h"
#include "printerc.h"
#include "printc.h"
#include "savec.h"
#if defined(C3270) /*[*/
#include "screenc.h"
#endif /*]*/
#include "tablesc.h"
#include "telnetc.h"
#include "trace_dsc.h"
#include "utilc.h"
#include "w3miscc.h"

#define PRINTER_BUF	1024

/* Statics */
static int      printer_pid = -1;
#if defined(_WIN32) /*[*/
static HANDLE	printer_handle = NULL;
#endif /*]*/
#if defined(X3270_DISPLAY) /*[*/
static Widget	lu_shell = (Widget)NULL;
#endif /*]*/
static struct pr3o {
	int fd;			/* file descriptor */
	unsigned long input_id;	/* input ID */
	unsigned long timeout_id; /* timeout ID */
	int count;		/* input count */
	char buf[PRINTER_BUF];	/* input buffer */
} printer_stdout = { -1, 0L, 0L, 0 },
  printer_stderr = { -1, 0L, 0L, 0 };
static char charset_file[256];	/* /tmp/cs$PID */
static Boolean need_cs = False;

#if !defined(_WIN32) /*[*/
static void	printer_output(H3270 *session);
static void	printer_error(H3270 *error);
static void	printer_otimeout(H3270 *session);
static void	printer_etimeout(H3270 *session);
static void	printer_dump(struct pr3o *p, Boolean is_err, Boolean is_dead);
#endif /*]*/
static void	printer_host_connect(H3270 *session, int connected unused, void *dunno);
static void	printer_exiting(H3270 *session, int b unused, void *dunno);

/* Globals */

/*
 * Printer initialization function.
 */
void
printer_init(void)
{
	/* Register interest in host connects and mode changes. */
	register_schange(ST_CONNECT, printer_host_connect);
	register_schange(ST_3270_MODE, printer_host_connect);
	register_schange(ST_EXITING, printer_exiting);
}

/*
 * Printer Start-up function
 * If 'lu' is non-NULL, then use the specific-LU form.
 * If not, use the assoc form.
 */
void
printer_start(const char *lu)
{
	const char *cmdlineName;
	const char *cmdline;
#if !defined(_WIN32) /*[*/
	const char *cmd;
#else /*][*/
	const char *printerName;
#endif /*]*/
	int cmd_len = 0;
	const char *s;
	char *cmd_text;
	char c;
	char charset_cmd[256];	/* -charset @/tmp/cs$PID */
	char *proxy_cmd = CN;	/* -proxy <spec> */
#if defined(_WIN32) /*[*/
	char *tmp;
	STARTUPINFO startupinfo;
	PROCESS_INFORMATION process_information;
	char *subcommand;
	char *space;
#else /*][*/
	int stdout_pipe[2];
	int stderr_pipe[2];
#endif /*]*/

#if defined(X3270_DISPLAY) /*[*/
	/* Make sure the popups are initted. */
	printer_popup_init();
#endif /*]*/

	/* Can't start two. */
	if (printer_pid != -1) {
		popup_an_error("printer is already running");
		return;
	}

	/* Gotta be in 3270 mode. */
	if (!IN_3270) {
		popup_an_error("Not in 3270 mode");
		return;
	}

	/* Select the command line to use. */
	if (lu == CN) {
		/* Associate with the current session. */

		/* Gotta be in TN3270E mode. */
		if (!IN_TN3270E) {
			popup_an_error("Not in TN3270E mode");
			return;
		}

		/* Gotta be connected to an LU. */
		if (h3270.connected_lu == CN) {
			popup_an_error("Not connected to a specific LU");
			return;
		}
		lu = h3270.connected_lu;
		cmdlineName = ResAssocCommand;
	} else {
		/* Specific LU passed in. */
		cmdlineName = ResLuCommandLine;
	}

	/* Fetch the command line and command resources. */
	cmdline = get_resource(cmdlineName);
	if (cmdline == CN) {
		popup_an_error("%s resource not defined", cmdlineName);
		return;
	}
#if !defined(_WIN32) /*[*/
	cmd = get_resource(ResPrinterCommand);
	if (cmd == CN) {
		popup_an_error("printer.command resource not defined");
		return;
	}
#else /*][*/
	printerName = get_resource(ResPrinterName);
#endif /*]*/

	/* Construct the charset option. */
#if defined(_WIN32) /*[*/
	if ((tmp = getenv("TMP"))) {
		(void) sprintf(charset_file, "%s\\cs%u", tmp, getpid());
	} else {
		(void) sprintf(charset_file, "cs%u", getpid());
	}
#else /*][*/
	(void) sprintf(charset_file, "/tmp/cs%u", getpid());
#endif /*]*/
	(void) sprintf(charset_cmd, "-charset @%s", charset_file);
	need_cs = False;

	/* Construct proxy option. */
	if (appres.proxy != CN) {
#if !defined(_WIN32) /*[*/
	    	proxy_cmd = xs_buffer("-proxy \"%s\"", appres.proxy);
#else /*][ */
		proxy_cmd = xs_buffer("-proxy %s", appres.proxy);
#endif /*]*/
	}

	/* Construct the command line. */

	/* Figure out how long it will be. */
	cmd_len = strlen(cmdline) + 1;
	s = cmdline;
	while ((s = strstr(s, "%L%")) != CN) {
		cmd_len += strlen(lu) - 3;
		s += 3;
	}
	s = cmdline;
	while ((s = strstr(s, "%H%")) != CN) {
		cmd_len += strlen(h3270.qualified_host) - 3;
		s += 3;
	}
#if !defined(_WIN32) /*[*/
	s = cmdline;
	while ((s = strstr(s, "%C%")) != CN) {
		cmd_len += strlen(cmd) - 3;
		s += 3;
	}
#endif /*]*/
	s = cmdline;
	while ((s = strstr(s, "%R%")) != CN) {
		need_cs = True;
		cmd_len += strlen(charset_cmd) - 3;
		s += 3;
	}
	s = cmdline;
	while ((s = strstr(s, "%P%")) != CN) {
		need_cs = True;
		cmd_len += (proxy_cmd? strlen(proxy_cmd): 0) - 3;
		s += 3;
	}

	/* Allocate a string buffer and substitute into it. */
	cmd_text = Malloc(cmd_len);
	cmd_text[0] = '\0';
	for (s = cmdline; (c = *s) != '\0'; s++) {
		char buf1[2];

		if (c == '%') {
			if (!strncmp(s+1, "L%", 2)) {
				(void) strcat(cmd_text, lu);
				s += 2;
				continue;
			} else if (!strncmp(s+1, "H%", 2)) {
				(void) strcat(cmd_text, h3270.qualified_host);
				s += 2;
				continue;
#if !defined(_WIN32) /*[*/
			} else if (!strncmp(s+1, "C%", 2)) {
				(void) strcat(cmd_text, cmd);
				s += 2;
				continue;
#endif /*]*/
			} else if (!strncmp(s+1, "R%", 2)) {
				(void) strcat(cmd_text, charset_cmd);
				s += 2;
				continue;
			} else if (!strncmp(s+1, "P%", 2)) {
			    	if (proxy_cmd != CN)
					(void) strcat(cmd_text, proxy_cmd);
				s += 2;
				continue;
			}
		}
		buf1[0] = c;
		buf1[1] = '\0';
		(void) strcat(cmd_text, buf1);
	}
	trace_dsn("Printer command line: %s\n", cmd_text);

	/* Create the character set file. */
	if (need_cs) {
		FILE *f = fopen(charset_file, "w");
		int i;

		if (f == NULL) {
		    popup_an_errno(errno, charset_file);
		    Free(cmd_text);
		    if (proxy_cmd != CN)
			    Free(proxy_cmd);
		    return;
		}
		(void) fprintf(f, "# EBCDIC-to-ASCII conversion file "
		    "for pr3287\n");
		(void) fprintf(f, "# Created by %s\n", build);
		(void) fprintf(f, "# Chararter set is '%s'\n",
		    get_charset_name());
		(void) fprintf(f, "cgcsgid=0x%08lx\n", cgcsgid);
#if defined(X3270_DBCS) /*[*/
		if (dbcs) {
			(void) fprintf(f, "cgcsgid_dbcs=0x%08lx\n",
				       cgcsgid_dbcs);
			if (encoding != CN)
				(void) fprintf(f, "encoding=%s\n",
					       encoding);
			(void) fprintf(f, "converters=%s\n",
				       converter_names);
		}
#endif /*]*/
		for (i = 0x40; i <= 0xff; i++) {
		    if (ebc2asc[i] != ebc2asc0[i]) {
			(void) fprintf(f, " %u=%u", i, ebc2asc[i]);
		    }
		}
		(void) fprintf(f, "\n");
		(void) fclose(f);
	}

#if !defined(_WIN32) /*[*/
	/* Make pipes for printer's stdout and stderr. */
	if (pipe(stdout_pipe) < 0) {
		popup_an_errno(errno, "pipe() failed");
		Free(cmd_text);
		if (proxy_cmd != CN)
			Free(proxy_cmd);
		return;
	}
	(void) fcntl(stdout_pipe[0], F_SETFD, 1);
	if (pipe(stderr_pipe) < 0) {
		popup_an_errno(errno, "pipe() failed");
		(void) close(stdout_pipe[0]);
		(void) close(stdout_pipe[1]);
		Free(cmd_text);
		if (proxy_cmd != CN)
			Free(proxy_cmd);
		return;
	}
	(void) fcntl(stderr_pipe[0], F_SETFD, 1);

	/* Fork and exec the printer session. */
	trace_dsn("Printer command line: %s\n", cmd_text);
	switch (printer_pid = fork()) {
	    case 0:	/* child process */
		(void) dup2(stdout_pipe[1], 1);
		(void) close(stdout_pipe[1]);
		(void) dup2(stderr_pipe[1], 2);
		(void) close(stderr_pipe[1]);
		if (setsid() < 0) {
			perror("setsid");
			_exit(1);
		}
		(void) execlp("/bin/sh", "sh", "-c", cmd_text, CN);
		(void) perror("exec(printer)");
		_exit(1);
	    default:	/* parent process */
		(void) close(stdout_pipe[1]);
		printer_stdout.fd = stdout_pipe[0];
		(void) close(stderr_pipe[1]);
		printer_stderr.fd = stderr_pipe[0];
		printer_stdout.input_id = AddInput(printer_stdout.fd,&h3270,printer_output);
		printer_stderr.input_id = AddInput(printer_stderr.fd,&h3270,printer_error);
		++children;
		break;
	    case -1:	/* error */
		popup_an_errno(errno, "fork()");
		(void) close(stdout_pipe[0]);
		(void) close(stdout_pipe[1]);
		(void) close(stderr_pipe[0]);
		(void) close(stderr_pipe[1]);
		break;
	}
#else /*][*/
	/* Pass the command via the environment. */
	if (printerName != NULL) {
		static char pn_buf[1024];

		sprintf(pn_buf, "PRINTER=%s", printerName);
		putenv(pn_buf);
	}

	/* Create the wpr3287 process. */
	(void) memset(&startupinfo, '\0', sizeof(STARTUPINFO));
	startupinfo.cb = sizeof(STARTUPINFO);
	(void) memset(&process_information, '\0', sizeof(PROCESS_INFORMATION));

	subcommand = NewString(cmd_text);
	strcpy(subcommand, cmd_text);
	space = strchr(subcommand, ' ');
	if (space) {
		*space = '\0';
	}

	if (!strcasecmp(subcommand, "wpr3287.exe") ||
	    !strcasecmp(subcommand, "wpr3287")) {
	    	char *pc;

	    	pc = xs_buffer("%s%s", PROGRAM_BIN, subcommand);
		Free(subcommand);
		subcommand = pc;

		if (space)
			pc = xs_buffer("\"%s\" %s", subcommand, space + 1);
		else
			pc = xs_buffer("\"%s%s\"", PROGRAM_BIN, cmd_text);
		Free(cmd_text);
		cmd_text = pc;
	}

	trace_dsn("Printer command line: %s\n", cmd_text);
	if (CreateProcess(
	    subcommand,
	    cmd_text,
	    NULL,
	    NULL,
	    FALSE,
	    0, /* creation flags */
	    NULL,
	    NULL,
	    &startupinfo,
	    &process_information) == 0) {
		popup_an_error("CreateProcess(%s) failed: %s", subcommand,
			win32_strerror(GetLastError()));
	}
	printer_handle = process_information.hProcess;
	CloseHandle(process_information.hThread);
	printer_pid = process_information.dwProcessId;

	Free(subcommand);

#endif /*]*/

	Free(cmd_text);
	if (proxy_cmd != CN)
		Free(proxy_cmd);

	/* Tell everyone else. */
	st_changed(ST_PRINTER, True);
}

#if !defined(_WIN32) /*[*/
/* There's data from the printer session. */
static void
printer_data(struct pr3o *p, Boolean is_err)
{
	int space;
	int nr;
	static char exitmsg[] = "Printer session exited";

	/* Read whatever there is. */
	space = PRINTER_BUF - p->count - 1;
	nr = read(p->fd, p->buf + p->count, space);

	/* Handle read errors and end-of-file. */
	if (nr < 0) {
		popup_an_errno(errno, "printer session pipe input");
		printer_stop();
		return;
	}
	if (nr == 0) {
		if (printer_stderr.timeout_id != 0L) {
			/*
			 * Append a termination error message to whatever the
			 * printer process said, and pop it up.
			 */
			p = &printer_stderr;
			space = PRINTER_BUF - p->count - 1;
			if (p->count && *(p->buf + p->count - 1) != '\n') {
				*(p->buf + p->count) = '\n';
				p->count++;
				space--;
			}
			(void) strncpy(p->buf + p->count, exitmsg, space);
			p->count += strlen(exitmsg);
			if (p->count >= PRINTER_BUF)
				p->count = PRINTER_BUF - 1;
			printer_dump(p, True, True);
		} else {
			popup_an_error(exitmsg);
		}
		printer_stop();
		return;
	}

	/* Add it to the buffer, and add a NULL. */
	p->count += nr;
	p->buf[p->count] = '\0';

	/*
	 * If there's no more room in the buffer, dump it now.  Otherwise,
	 * give it a second to generate more output.
	 */
	if (p->count >= PRINTER_BUF - 1) {
		printer_dump(p, is_err, False);
	} else if (p->timeout_id == 0L) {
		p->timeout_id = AddTimeOut(1000, &h3270, is_err? printer_etimeout: printer_otimeout);
	}
}

/* The printer process has some output for us. */
static void printer_output(H3270 *session)
{
	printer_data(&printer_stdout, False);
}

/* The printer process has some error output for us. */
static void printer_error(H3270 *session)
{
	printer_data(&printer_stderr, True);
}

/* Timeout from printer output or error output. */
static void
printer_timeout(struct pr3o *p, Boolean is_err)
{
	/* Forget the timeout ID. */
	p->timeout_id = 0L;

	/* Dump the output. */
	printer_dump(p, is_err, False);
}

/* Timeout from printer output. */
static void
printer_otimeout(H3270 *session)
{
	printer_timeout(&printer_stdout, False);
}

/* Timeout from printer error output. */
static void
printer_etimeout(H3270 *session)
{
	printer_timeout(&printer_stderr, True);
}

/* Dump pending printer process output. */
static void
printer_dump(struct pr3o *p, Boolean is_err, Boolean is_dead)
{
	if (p->count) {
		/*
		 * Strip any trailing newline, and make sure the buffer is
		 * NULL terminated.
		 */
		if (p->buf[p->count - 1] == '\n')
			p->buf[--(p->count)] = '\0';
		else if (p->buf[p->count])
			p->buf[p->count] = '\0';

		/* Dump it and clear the buffer. */
#if defined(X3270_DISPLAY) /*[*/
		popup_printer_output(is_err, is_dead? NULL: printer_stop,
		    "%s", p->buf);
#else /*][*/
		action_output("%s", p->buf);
#endif
		p->count = 0;
	}
}
#endif /*]*/

#if defined(_WIN32) /*[*/
/* Check for an exited printer session. */
void
printer_check(void)
{
	DWORD exit_code;

	if (printer_pid != -1 &&
	    GetExitCodeProcess(printer_handle, &exit_code) != 0 &&
	    exit_code != STILL_ACTIVE) {

		printer_pid = -1;
		CloseHandle(printer_handle);
		printer_handle = NULL;

		if (need_cs)
		    (void) unlink(charset_file);

		st_changed(ST_PRINTER, False);

		popup_an_error("Printer process exited with status %d",
		    exit_code);
	}
}
#endif /*]*/

/* Close the printer session. */
void
printer_stop(void)
{
	/* Remove inputs. */
	if (printer_stdout.input_id) {
		RemoveInput(printer_stdout.input_id);
		printer_stdout.input_id = 0L;
	}
	if (printer_stderr.input_id) {
		RemoveInput(printer_stderr.input_id);
		printer_stderr.input_id = 0L;
	}

	/* Cancel timeouts. */
	if (printer_stdout.timeout_id) {
		RemoveTimeOut(printer_stdout.timeout_id);
		printer_stdout.timeout_id = 0L;
	}
	if (printer_stderr.timeout_id) {
		RemoveTimeOut(printer_stderr.timeout_id);
		printer_stderr.timeout_id = 0L;
	}

	/* Clear buffers. */
	printer_stdout.count = 0;
	printer_stderr.count = 0;

	/* Kill the process. */
	if (printer_pid != -1) {
#if !defined(_WIN32) /*[*/
		(void) kill(-printer_pid, SIGTERM);
#else /*][*/
		TerminateProcess(printer_handle, 0);
#endif /*]*/
		printer_pid = -1;
#if defined(_WIN32) /*[*/
		printer_handle = NULL;
#endif /*]*/
	}

	/* Delete the character set file. */
	if (need_cs)
	    (void) unlink(charset_file);

	/* Tell everyone else. */
	st_changed(ST_PRINTER, False);
}

/* The emulator is exiting.  Make sure the printer session is cleaned up. */
static void
printer_exiting(H3270 *session, int b unused, void *dunno)
{
	printer_stop();
}

#if defined(X3270_DISPLAY) /*[*/
/* Callback for "OK" button on printer specific-LU popup */
static void
lu_callback(Widget w, XtPointer client_data, XtPointer call_data unused)
{
	char *lu;

	if (w) {
		lu = XawDialogGetValueString((Widget)client_data);
		if (lu == CN || *lu == '\0') {
			popup_an_error("Must supply an LU");
			return;
		} else
			XtPopdown(lu_shell);
	} else
		lu = (char *)client_data;
	printer_start(lu);
}
#endif /*]*/

/* Host connect/disconnect/3270-mode event. */
static void
printer_host_connect(H3270 *session, int connected unused, void *dunno)
{
	if (IN_3270) {
		char *printer_lu = appres.printer_lu;

		if (printer_lu != CN && !printer_running()) {
			if (!strcmp(printer_lu, ".")) {
				if (IN_TN3270E) {
					/* Associate with TN3270E session. */
					trace_dsn("Starting associated printer "
						  "session.\n");
					printer_start(CN);
				}
			} else {
				/* Specific LU. */
				trace_dsn("Starting %s printer session.\n",
				    printer_lu);
				printer_start(printer_lu);
			}
		} else if (!IN_E &&
			   printer_lu != CN &&
			   !strcmp(printer_lu, ".") &&
			   printer_running()) {

			/* Stop an automatic associated printer. */
			trace_dsn("Stopping printer session.\n");
			printer_stop();
		}
	} else if (printer_running()) {
		/*
		 * We're no longer in 3270 mode, then we can no longer have a
		 * printer session.  This may cause some fireworks if there is
		 * a print job pending when we do this, so some sort of awful
		 * timeout may be needed.
		 */
		trace_dsn("Stopping printer session.\n");
		printer_stop();
	}
}

#if defined(X3270_DISPLAY) /*[*/
/* Pop up the LU dialog box. */
void
printer_lu_dialog(void)
{
	if (lu_shell == NULL)
		lu_shell = create_form_popup("printerLu",
		    lu_callback, (XtCallbackProc)NULL, FORM_NO_WHITE);
	popup_popup(lu_shell, XtGrabExclusive);
}
#endif /*]*/

Boolean
printer_running(void)
{
	return printer_pid != -1;
}

#endif /*]*/
