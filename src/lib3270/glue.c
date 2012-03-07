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
 * Este programa está nomeado como glue.c e possui 1103 linhas de código.
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
 *	glue.c
 *		A displayless 3270 Terminal Emulator
 *		Glue for missing parts.
 */



#include "globals.h"

#if !defined(_WIN32) /*[*/
	#include <sys/wait.h>
#else
	#include <windows.h>
#endif /*]*/

#include <signal.h>
#include <errno.h>

#include "appres.h"
#include "3270ds.h"
#include "resources.h"

#include "actionsc.h"
#include "ansic.h"
#include "charsetc.h"
#include "ctlrc.h"
#include "gluec.h"
#include "hostc.h"
// #include "keymapc.h"
#include "kybdc.h"
//#include "macrosc.h"
// #include "menubarc.h"
#include "popupsc.h"
#include "screenc.h"
// #include "selectc.h"
#include "tablesc.h"
#include "telnetc.h"
#include "togglesc.h"
#include "trace_dsc.h"
#include "utilc.h"
// #include "idlec.h"
#include "printerc.h"

#if defined(X3270_FT)
	#include "ftc.h"
#endif

#if defined(_WIN32) /*[*/
#include "winversc.h"
#endif /*]*/

// #include "session.h"

#if defined WIN32
	BOOL WINAPI DllMain(HANDLE hinst, DWORD dwcallpurpose, LPVOID lpvResvd);
#else
	int lib3270_loaded(void) __attribute__((constructor));
	int lib3270_unloaded(void) __attribute__((destructor));
#endif

#ifdef DEBUG
	static int init_calls = 0;
#endif

 static void lib3270_session_init(H3270 *hSession, const char *model);

 #define LAST_ARG	"--"

/*---[ Statics ]--------------------------------------------------------------------------------------------------------------*/

 static int parse_model_number(const char *m);

/*---[ Globals ]--------------------------------------------------------------------------------------------------------------*/
 H3270				  h3270;
 const char		* programname;
 AppRes				  appres;
 int				  children = 0;
 Boolean			  exiting = False;
// char				* command_string = CN;
 static Boolean	  sfont = False;
 Boolean			* standard_font = &sfont;

#if defined(WC3270) || defined(LIB3270)/*[*/
char			*profile_name = CN;
#endif /*]*/

const char *toggle_names[N_TOGGLES] =
{
	"Monocase",
	"AltCursor",
	"CursorBlink",
	"ShowTiming",
	"CursorPos",
	"DSTrace",
	"ScrollBar",
	"LineWrap",
	"BlankFill",
	"ScreenTrace",
	"EventTrace",
	"MarginedPaste",
	"RectSelect",
	"CrossHair",
	"VisibleControl",
	"AidWait",
	"FullScreen",
	"Reconnect",
	"Insert",
	"Keypad",
	"SmartPaste"
};

void lib3270_session_free(H3270 *h)
{
	int f;

	// Terminate session
	if(lib3270_connected(h))
		lib3270_disconnect(h);

	shutdown_toggles(h,appres.toggle);

	// Release state change callbacks
	for(f=0;f<N_ST;f++)
	{
		while(h->st_callbacks[f])
		{
			struct lib3270_state_callback *next = h->st_callbacks[f]->next;
			Free(h->st_callbacks[f]);
			h->st_callbacks[f] = next;
		}
	}

}

static void lib3270_session_init(H3270 *hSession, const char *model)
{
	int 	ovc, ovr;
	char	junk;
	int		model_number;

	memset(hSession,0,sizeof(H3270));
	hSession->sz = sizeof(H3270);
	hSession->sock = -1;
	hSession->model_num = -1;
//	hSession->first_changed = -1;
//	hSession->last_changed = -1;
	hSession->cstate = NOT_CONNECTED;
	hSession->oia_status = -1;

	strncpy(hSession->full_model_name,"IBM-",LIB3270_FULL_MODEL_NAME_LENGTH);
	hSession->model_name = &hSession->full_model_name[4];

	/*
	 * Sort out model and color modes, based on the model number resource.
	 */ /*
	if(appres.model && *appres.model)
		model = appres.model;
	*/

	if(!*model)
		model = "2";	// No model, use the default one

//	Trace("Parsing model: %s",appres.model);
	model_number = parse_model_number(model);
	if (model_number < 0)
	{
		popup_an_error("Invalid model number: %s", model);
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

	if(appres.mono)
		appres.m3279 = False;

	if(!appres.extended)
		appres.oversize = CN;

#if defined(RESTRICT_3279)
	if (appres.m3279 && model_number == 4)
		model_number = 3;
#endif

	Trace("Model_number: %d",model_number);

	if (!appres.extended || appres.oversize == CN || sscanf(appres.oversize, "%dx%d%c", &ovc, &ovr, &junk) != 2)
	{
		ovc = 0;
		ovr = 0;
	}
	ctlr_set_rows_cols(hSession, model_number, ovc, ovr);

	if (appres.termname != CN)
		hSession->termtype = appres.termname;
	else
		hSession->termtype = hSession->full_model_name;

	Trace("Termtype: %s",hSession->termtype);

	if (appres.apl_mode)
		appres.charset = Apl;

}

H3270 * lib3270_session_new(const char *model)
{
	static int configured = 0;

	H3270		*hSession = &h3270;

	Trace("%s - configured=%d",__FUNCTION__,configured);

	if(configured)
	{
		// TODO (perry#5#): Allocate a new structure.
		errno = EBUSY;
		return hSession;
	}

	configured = 1;

	lib3270_session_init(hSession, model);

	if(screen_init(hSession))
		return NULL;

	Trace("Charset: %s",appres.charset);
	if (charset_init(appres.charset) != CS_OKAY)
	{
		Warning( _( "Cannot find charset \"%s\", using defaults" ), appres.charset);
		(void) charset_init(CN);
	}

	kybd_init();
//	hostfile_init();
//	hostfile_init();
	ansi_init();

#if defined(X3270_FT)
	ft_init();
#endif

#if defined(X3270_PRINTER)
	printer_init();
#endif

	Trace("%s finished",__FUNCTION__);

	errno = 0;
	return hSession;
}

/*
 * Set default options
 */
static void initialize(void)
{
	memset(&appres,0,sizeof(appres));

#ifdef DEBUG
	init_calls++;
	Trace("Initializing library (calls: %d)",init_calls);
#endif

	initialize_toggles(&h3270,appres.toggle);

#if defined(_WIN32)
	(void) get_version_info();
#endif

	Trace("%s (init_calls: %d)",__FUNCTION__,init_calls);

	/* Set the defaults. */
	appres.mono = False;
	appres.extended = True;

#if defined(C3270) /*[*/
	appres.m3279 = True;
#else /*][*/
	appres.m3279 = False;
#endif /*]*/

	appres.modified_sel = False;
	appres.apl_mode = False;

#if defined(C3270) || defined(TCL3270) /*[*/
//	appres.scripted = False;
#else /*][*/
	appres.scripted = True;
#endif /*]*/

	appres.numeric_lock = False;
//	appres.secure = False;

#if defined(C3270) /*[*/
	appres.oerr_lock = True;
#else /*][*/
	appres.oerr_lock = False;
#endif /*]*/

	appres.typeahead = True;
	appres.debug_tracing = True;

#if defined(C3270) /*[*/
	appres.compose_map = "latin1";
#endif /*]*/

	appres.model = "";
	appres.hostsfile = CN;
	appres.port = "telnet";

#if !defined(_WIN32) /*[*/
	appres.charset = "bracket";
#else /*][*/

	if (is_nt)
		appres.charset = "bracket";
	else
		appres.charset = "bracket437";
#endif /*]*/

	appres.termname = CN;
	appres.macros = CN;

#if defined(X3270_TRACE) /*[*/

#if !defined(_WIN32) /*[*/
	appres.trace_dir = "/tmp";
#endif /*]*/

#if defined(X3270_DISPLAY) || defined(WC3270) /*[*/
	appres.trace_monitor = True;
#endif /*]*/

#endif /*]*/

	appres.oversize = CN;

#if defined(C3270) /*[*/
	appres.meta_escape = "auto";
//	appres.curses_keypad = True;
//	appres.cbreak_mode = False;
#endif /*]*/

#if defined(X3270_ANSI) /*[*/
	appres.icrnl = True;
	appres.inlcr = False;
	appres.onlcr = True;
	appres.erase = "^H";
	appres.kill = "^U";
	appres.werase = "^W";
	appres.rprnt = "^R";
	appres.lnext = "^V";
	appres.intr = "^C";
	appres.quit = "^\\";
	appres.eof = "^D";
#endif /*]*/

	appres.unlock_delay = True;

#if defined(X3270_FT) /*[*/
	appres.dft_buffer_size = DFT_BUF;
#endif /*]*/

#if defined(C3270) && !defined(LIB3270) /*[*/
	appres.toggle[CURSOR_POS].value = True;
#endif /*]*/

#if defined(X3270_SCRIPT) || defined(TCL3270) /*[*/
	appres.toggle[AID_WAIT].value = True;
#endif /*]*/

#if defined(C3270) && defined(X3270_SCRIPT) /*[*/
	appres.plugin_command = "x3270hist.pl";
#endif /*]*/

#if defined(C3270) && defined(_WIN32) /*[*/
	appres.highlight_underline = True;
#endif /*]*/

/*
#if defined(C3270) && !defined(_WIN32) && !defined(LIB3270)
	// Merge in the profile.
	merge_profile();
#endif
*/

}

#if defined WIN32

BOOL WINAPI DllMain(HANDLE hinst, DWORD dwcallpurpose, LPVOID lpvResvd)
{
//	Trace("%s - Library %s",__FUNCTION__,(dwcallpurpose == DLL_PROCESS_ATTACH) ? "Loaded" : "Unloaded");

    if(dwcallpurpose == DLL_PROCESS_ATTACH)
		initialize();

    return TRUE;
}

#else

int lib3270_loaded(void)
{
//	Trace("%s - Library loaded",__FUNCTION__);
	initialize();
    return 0;
}

int lib3270_unloaded(void)
{
    return 0;
}

#endif


#define offset(n) (void *) &appres.n
#define toggle_offset(index) offset(toggle[index].value)

static const struct lib3270_option options[] =
{
	// TODO (perry#5#): Add option descriptions.
//#if defined(C3270) /*[*/
//    { OptAllBold,  OPT_BOOLEAN, True,  ResAllBold,   offset(all_bold_on), NULL },
//    { OptAltScreen,OPT_STRING,  False, ResAltScreen, offset(altscreen), NULL },
//#endif /*]*/
    { OptAplMode,  OPT_BOOLEAN, True,  ResAplMode,   offset(apl_mode), NULL },
#if defined(C3270) /*[*/
//    { OptCbreak,   OPT_BOOLEAN, True,  ResCbreak,    offset(cbreak_mode), NULL },
#endif /*]*/
#if defined(HAVE_LIBSSL) /*[*/
    { OptCertFile, OPT_STRING,  False, ResCertFile,  offset(cert_file), NULL },
#endif /*]*/
    { OptCharset,  OPT_STRING,  False, ResCharset,   offset(charset), NULL },
    { OptClear,    OPT_SKIP2,   False, NULL,         NULL, NULL },
//    { OptDefScreen,OPT_STRING,  False, ResDefScreen, offset(defscreen), NULL },
#if defined(X3270_TRACE) /*[*/
    { OptDsTrace,  OPT_BOOLEAN, True,  ResDsTrace,   toggle_offset(DS_TRACE), NULL },
#endif /*]*/
    { OptHostsFile,OPT_STRING,  False, ResHostsFile, offset(hostsfile), NULL },
//#if defined(C3270)
//    { OptKeymap,   OPT_STRING,  False, ResKeymap,    offset(key_map), N_( "Specifies a keymap name and optional modifiers." ) },
// #endif

// #if defined(X3270_DBCS) /*[*/
//    { OptLocalEncoding,OPT_STRING,False,ResLocalEncoding,offset(local_encoding), NULL },
//#endif /*]*/
    { OptModel,    OPT_STRING,  False, ResKeymap,    offset(model), N_( "Set terminal model (screen size)" ) },
#if !defined(_WIN32) /*[*/
    { OptMono,     OPT_BOOLEAN, True,  ResMono,      offset(mono), N_( "Forces monochrome display" ) },
#endif /*]*/
//    { OptOnce,     OPT_BOOLEAN, True,  ResOnce,      offset(once), NULL },
    { OptOversize, OPT_STRING,  False, ResOversize,  offset(oversize), N_( "Sets the screen dimensions to be larger than the default for the chosen model (COLSxROWS)." ) },
    { OptPort,     OPT_STRING,  False, ResPort,      offset(port), N_( "The name of the default TCP port to connect" ) },
#if defined(C3270) && !defined(LIB3270) /*[*/
    { OptPrinterLu,OPT_STRING,  False, ResPrinterLu, offset(printer_lu), NULL },
#endif /*]*/
    { OptProxy,	   OPT_STRING,  False, ResProxy,     offset(proxy), N_( "Proxy server (type:host[:port])" ) },
#if defined(S3270) /*[*/
    { OptScripted, OPT_NOP,     False, ResScripted,  NULL, NULL },
#endif /*]*/
//#if defined(C3270) /*[*/
//    { OptSecure,   OPT_BOOLEAN, True,  ResSecure,    offset(secure), NULL },
//#endif /*]*/
    { OptSet,      OPT_SKIP2,   False, NULL,         NULL, NULL },
#if defined(X3270_SCRIPT) /*[*/
    { OptSocket,   OPT_BOOLEAN, True,  ResSocket,    offset(socket), NULL },
#endif /*]*/

    { OptTermName, OPT_STRING,  False, ResTermName,  offset(termname), N_( "Specifies the terminal name to be transmitted over the telnet connection." ) },

#if defined(WC3270) /*[*/
    { OptTitle,    OPT_STRING,  False, ResTitle,     offset(title), NULL },
#endif /*]*/
#if defined(X3270_TRACE) /*[*/
    { OptTraceFile,OPT_STRING,  False, ResTraceFile, offset(trace_file), NULL },
    { OptTraceFileSize,OPT_STRING,False,ResTraceFileSize,offset(trace_file_size), NULL },
#endif /*]*/
    { "-xrm",      OPT_XRM,     False, NULL,         NULL, NULL },
    { LAST_ARG,    OPT_DONE,    False, NULL,         NULL, NULL },
    { CN,          OPT_SKIP2,   False, NULL,         NULL, NULL }
};

/*
 * Get library option table
 */
const struct lib3270_option * get_3270_option_table(int sz)
{
	if(sz == sizeof(struct lib3270_option))
		return options;
	return NULL;
}

/*
 * Parse the model number.
 * Returns -1 (error), 0 (default), or the specified number.
 */
static int parse_model_number(const char *m)
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
		if (!strncmp(m, "3278", 4)) {
			appres.m3279 = False;
		} else if (!strncmp(m, "3279", 4)) {
			appres.m3279 = True;
		} else {
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

/*
 * Parse '-xrm' options.
 * Understands only:
 *   {c,s,tcl}3270.<resourcename>: value
 * Asterisks and class names need not apply.
 */

static struct {
	const char *name;
	void *address;
	enum resource_type { XRM_STRING, XRM_BOOLEAN, XRM_INT } type;
} resources[] = {
	{ ResAllBold,	offset(all_bold),	XRM_STRING },
//	{ ResAltScreen,	offset(altscreen),	XRM_STRING },
	{ ResBsdTm,	offset(bsd_tm),		XRM_BOOLEAN },
#if defined(HAVE_LIBSSL) /*[*/
	{ ResCertFile,	offset(cert_file),	XRM_STRING },
#endif /*]*/
	{ ResCharset,	offset(charset),	XRM_STRING },
	{ ResColor8,	offset(color8),		XRM_BOOLEAN },
	{ ResConfDir,	offset(conf_dir),	XRM_STRING },
//	{ ResDefScreen,	offset(defscreen),	XRM_STRING },
#if defined(X3270_ANSI) /*[*/
	{ ResEof,	offset(eof),		XRM_STRING },
	{ ResErase,	offset(erase),		XRM_STRING },
#endif /*]*/
	{ ResExtended,	offset(extended),	XRM_BOOLEAN },
#if defined(X3270_FT) /*[*/
	{ ResFtCommand,	offset(ft_command),	XRM_STRING },
	{ ResDftBufferSize,offset(dft_buffer_size),XRM_INT },
#endif /*]*/
#if defined(WC3270) /*[*/
	{ "hostname",	offset(hostname),	XRM_STRING },
#endif /*]*/
	{ ResHostsFile,	offset(hostsfile),	XRM_STRING },
#if defined(X3270_ANSI) /*[*/
	{ ResIcrnl,	offset(icrnl),		XRM_BOOLEAN },
	{ ResInlcr,	offset(inlcr),		XRM_BOOLEAN },
	{ ResOnlcr,	offset(onlcr),		XRM_BOOLEAN },
	{ ResIntr,	offset(intr),		XRM_STRING },
#endif /*]*/
#if defined(X3270_SCRIPT) /*[*/
	{ ResPluginCommand, offset(plugin_command), XRM_STRING },
#endif /*]*/
#if defined(C3270) && defined(_WIN32) /*[*/
	{ ResHighlightUnderline, offset(highlight_underline), XRM_BOOLEAN },
#endif /*]*/
#if defined(C3270) && defined(X3270_SCRIPT) /*[*/
	{ ResIdleCommand,offset(idle_command),	XRM_STRING },
	{ ResIdleCommandEnabled,offset(idle_command_enabled),	XRM_BOOLEAN },
	{ ResIdleTimeout,offset(idle_timeout),	XRM_STRING },
#endif /*]*/
#if defined(C3270) /*[*/
//	{ ResKeymap,	offset(key_map),	XRM_STRING },
	{ ResMetaEscape,offset(meta_escape),	XRM_STRING },
//	{ ResCursesKeypad,offset(curses_keypad),XRM_BOOLEAN },
//	{ ResCbreak,	offset(cbreak_mode),	XRM_BOOLEAN },
#endif /*]*/
#if defined(X3270_ANSI) /*[*/
	{ ResKill,	offset(kill),		XRM_STRING },
	{ ResLnext,	offset(lnext),		XRM_STRING },
#endif /*]*/
	{ ResLoginMacro,offset(login_macro),	XRM_STRING },
	{ ResM3279,	offset(m3279),		XRM_BOOLEAN },
//	{ ResModel,	offset(model),		XRM_STRING },
	{ ResModifiedSel, offset(modified_sel),	XRM_BOOLEAN },
#if defined(C3270) && !defined(_WIN32) /*[*/
	{ ResMono,	offset(mono),		XRM_BOOLEAN },
#endif /*]*/
	{ ResNumericLock, offset(numeric_lock),	XRM_BOOLEAN },
	{ ResOerrLock,	offset(oerr_lock),	XRM_BOOLEAN },
	{ ResOversize,	offset(oversize),	XRM_STRING },
	{ ResPort,	offset(port),		XRM_STRING },
#if defined(C3270) /*[*/
	{ ResPrinterLu,	offset(printer_lu),	XRM_STRING },
	{ ResPrintTextCommand,	NULL,		XRM_STRING },
#endif /*]*/
	{ ResProxy,	offset(proxy),		XRM_STRING },
#if defined(X3270_ANSI) /*[*/
	{ ResQuit,	offset(quit),		XRM_STRING },
	{ ResRprnt,	offset(rprnt),		XRM_STRING },
#endif /*]*/
//	{ ResSecure,	offset(secure),		XRM_BOOLEAN },
	{ ResTermName,	offset(termname),	XRM_STRING },
#if defined(WC3270) /*[*/
	{ ResTitle,	offset(title),		XRM_STRING },
#endif /*]*/
#if defined(X3270_TRACE) /*[*/
#if !defined(_WIN32) /*[*/
	{ ResTraceDir,	offset(trace_dir),	XRM_STRING },
#endif /*]*/
	{ ResTraceFile,	offset(trace_file),	XRM_STRING },
	{ ResTraceFileSize,offset(trace_file_size),XRM_STRING },
#if defined(WC3270) /*[*/
	{ ResTraceMonitor,offset(trace_monitor),XRM_BOOLEAN },
#endif /*]*/
#endif /*]*/
	{ ResTypeahead,	offset(typeahead),	XRM_BOOLEAN },
	{ ResUnlockDelay,offset(unlock_delay),	XRM_BOOLEAN },
#if defined(X3270_ANSI) /*[*/
	{ ResWerase,	offset(werase),		XRM_STRING },
#endif /*]*/

	{ CN,		0,			XRM_STRING }
};

/*
 * Compare two strings, allowing the second to differ by uppercasing the
 * first character of the second.
 */
static int
strncapcmp(const char *known, const char *unknown, unsigned unk_len)
{
	if (unk_len != strlen(known))
		return -1;
	if (!strncmp(known, unknown, unk_len))
		return 0;
	if (unk_len > 1 &&
	    unknown[0] == toupper(known[0]) &&
	    !strncmp(known + 1, unknown + 1, unk_len - 1))
		return 0;
	return -1;
}


#if !defined(ME) /*[*/
#if defined(C3270) /*[*/
#if defined(WC3270) /*[*/
#define ME	"wc3270"
#else /*][*/
#define ME	"c3270"
#endif /*]*/
#elif defined(TCL3270) /*][*/
#define ME	"tcl3270"
#else /*][*/
#define ME	"s3270"
#endif /*]*/
#endif /*]*/

void
parse_xrm(const char *arg, const char *where)
{
	static char me_dot[] = ME ".";
	static char me_star[] = ME "*";
	unsigned match_len;
	const char *s;
	unsigned rnlen;
	int i;
	char *t;
	void *address = NULL;
	enum resource_type type = XRM_STRING;
#if defined(C3270) /*[*/
	char *add_buf = CN;
	char *hide;
	Boolean arbitrary = False;
#endif /*]*/

	/* Enforce "-3270." or "-3270*" or "*". */
	if (!strncmp(arg, me_dot, sizeof(me_dot)-1))
		match_len = sizeof(me_dot)-1;
	else if (!strncmp(arg, me_star, sizeof(me_star)-1))
		match_len = sizeof(me_star)-1;
	else if (arg[0] == '*')
		match_len = 1;
	else {
		xs_warning("%s: Invalid resource syntax '%.*s', name must "
		    "begin with '%s'",
		    where, (int) sizeof(me_dot)-1, arg, me_dot);
		return;
	}

	/* Separate the parts. */
	s = arg + match_len;
	while (*s && *s != ':' && !isspace(*s))
		s++;
	rnlen = s - (arg + match_len);
	if (!rnlen) {
		xs_warning("%s: Invalid resource syntax, missing resource "
		    "name", where);
		return;
	}
	while (isspace(*s))
		s++;
	if (*s != ':') {
		xs_warning("%s: Invalid resource syntax, missing ':'", where);
		return;
	}
	s++;
	while (isspace(*s))
		s++;

	/* Look up the name. */
	for (i = 0; resources[i].name != CN; i++) {
		if (!strncapcmp(resources[i].name, arg + match_len, rnlen)) {
			address = resources[i].address;
			type = resources[i].type;
#if defined(C3270) /*[*/
			if (address == NULL) {
				add_buf = Malloc(strlen(s) + 1);
				address = add_buf;
			}
#endif /*]*/
			break;
		}
	}

#if defined(C3270) /*[*/
	if (address == NULL) {
		if (!strncasecmp(ResKeymap ".", arg + match_len,
		                 strlen(ResKeymap ".")) ||
		    !strncasecmp(ResCharset ".", arg + match_len,
		                 strlen(ResCharset ".")) ||
		    !strncasecmp(ResDisplayCharset ".", arg + match_len,
		                 strlen(ResDisplayCharset ".")) ||
		    !strncasecmp(ResCodepage ".", arg + match_len,
		                 strlen(ResCodepage ".")) ||
		    !strncasecmp("host.", arg + match_len, 5) ||
		    !strncasecmp("printer.", arg + match_len, 8) ||
#if defined(_WIN32) /*[*/
		    !strncasecmp(ResHostColorFor, arg + match_len,
			    strlen(ResHostColorFor)) ||
		    !strncasecmp(ResConsoleColorForHostColor, arg + match_len,
			    strlen(ResConsoleColorForHostColor))
#else /*][*/
		    !strncasecmp(ResCursesColorFor, arg + match_len,
			    strlen(ResCursesColorFor))
#endif /*]*/
		    ) {
			address = &hide;
			type = XRM_STRING;
			arbitrary = True;
		}
	}
#endif /*]*/
	if (address == NULL) {
		xs_warning("%s: Unknown resource name: %.*s",
		    where, (int)rnlen, arg + match_len);
		return;
	}
	switch (type) {
	case XRM_BOOLEAN:
		if (!strcasecmp(s, "true") ||
		    !strcasecmp(s, "t") ||
		    !strcmp(s, "1")) {
			*(Boolean *)address = True;
		} else if (!strcasecmp(s, "false") ||
		    !strcasecmp(s, "f") ||
		    !strcmp(s, "0")) {
			*(Boolean *)address = False;
		} else {
			xs_warning("%s: Invalid Boolean value: %s", where, s);
		}
		break;
	case XRM_STRING:
		t = Malloc(strlen(s) + 1);
		*(char **)address = t;
		if (*s == '"') {
			Boolean quoted = False;
			char c;

			s++;
			while ((c = *s++) != '\0') {
				if (quoted) {
					switch (c) {
					case 'n':
						*t++ = '\n';
						break;
					case 'r':
						*t++ = '\r';
						break;
					case 'b':
						*t++ = '\b';
						break;
					default:
						*t++ = c;
						break;
					}
					quoted = False;
				} else if (c == '\\') {
					quoted = True;
				} else if (c == '"') {
					break;
				} else {
					*t++ = c;
				}
			}
			*t = '\0';
		} else {
			(void) strcpy(t, s);
		}
		break;
	case XRM_INT: {
		long n;
		char *ptr;

		n = strtol(s, &ptr, 0);
		if (*ptr != '\0') {
			xs_warning("%s: Invalid Integer value: %s", where, s);
		} else {
			*(int *)address = (int)n;
		}
		break;
		}
	}

#if defined(C3270) /*[*/
	/* Add a new, arbitrarily-named resource. */
	if (arbitrary) {
		char *rsname;

		rsname = Malloc(rnlen + 1);
		(void) strncpy(rsname, arg + match_len, rnlen);
		rsname[rnlen] = '\0';
		add_resource(rsname, hide);
	}
#endif /*]*/
}

/* Read resources from a file. */
int
read_resource_file(const char *filename, Boolean fatal)
{
	FILE *f;
	int ilen;
	char buf[4096];
	char *where;
	int lno = 0;

	f = fopen(filename, "r");
	if (f == NULL) {
		if (fatal)
			xs_warning("Cannot open '%s': %s", filename,
			    strerror(errno));
		return -1;
	}

	/* Merge in what's in the file into the resource database. */
	where = Malloc(strlen(filename) + 64);

	ilen = 0;
	while (fgets(buf + ilen, sizeof(buf) - ilen, f) != CN || ilen) {
		char *s, *t;
		unsigned sl;
		Boolean bsl;

		lno++;

		/* Stip any trailing newline. */
		sl = strlen(buf + ilen);
		if (sl && (buf + ilen)[sl-1] == '\n')
			(buf + ilen)[--sl] = '\0';

		/*
		 * Translate backslash-n to real newline characters, and
		 * remember if the last character is a backslash.
		 */
		for (bsl = False, s = buf + ilen, t = s; *s; s++) {
			if (bsl) {
				if (*s == 'n')
					*t++ = '\n';
				else
					*t++ = *s;
				bsl = False;
			} else if (*s == '\\')
				bsl = True;
			else {
				*t++ = *s;
				bsl = False;
			}
		}
		*t = '\0';

		/* Skip leading whitespace. */
		s = buf;
		while (isspace(*s))
			s++;

		/* Skip comments _before_ checking for line continuation. */
		if (*s == '!') {
		    ilen = 0;
		    continue;
		}
		if (*s == '#') {
			(void) sprintf(where, "%s:%d: Invalid profile "
			    "syntax ('#' ignored)", filename, lno);
			Warning(NULL,where);
			ilen = 0;
			continue;
		}

		/* If this line is a continuation, try again. */
		if (bsl) {
			ilen += strlen(buf + ilen);
			if (ilen >= sizeof(buf) - 1) {
				(void) sprintf(where, "%s:%d: Line too long\n",
				    filename, lno);
				Warning(NULL,where);
				break;
			}
			continue;
		}

		/* Strip trailing whitespace and check for empty lines. */
		sl = strlen(s);
		while (sl && isspace(s[sl-1]))
			s[--sl] = '\0';
		if (!sl) {
			ilen = 0;
			continue;
		}

		/* Digest it. */
		(void) sprintf(where, "%s:%d", filename, lno);
		parse_xrm(s, where);

		/* Get ready for the next iteration. */
		ilen = 0;
	}
	Free(where);
	return 0;
}

/* Screen globals. */

static int cw = 7;
int *char_width = &cw;

static int ch = 7;
int *char_height = &ch;

Boolean visible_control = False;

// Boolean flipped = False;

/* Replacements for functions in popups.c. */

#include <stdarg.h>

Boolean error_popup_visible = False;


/* Pop up an error dialog, based on an error number. */
void popup_an_errno(int errn, const char *fmt, ...)
{
	char 	vmsgbuf[4096];
	va_list	args;

	va_start(args, fmt);
	(void) vsprintf(vmsgbuf, fmt, args);
	va_end(args);

	WriteLog("3270", "Error Popup:\n%s\nrc=%d (%s)",vmsgbuf,errn,strerror(errn));

	Error(NULL,vmsgbuf);
}

#ifdef DEBUG
extern void lib3270_initialize(void)
{
	initialize();
}
#endif

void
action_output(const char *fmt, ...)
{
// TODO (perry#1#): Implement a callback to browse the text string.
/*
	va_list args;

	va_start(args, fmt);
	(void) vsprintf(vmsgbuf, fmt, args);
	va_end(args);
	if (sms_redirect()) {
		sms_info("%s", vmsgbuf);
		return;
	} else {
		FILE *aout;

#if defined(C3270) || defined(WC3270)
		screen_suspend();
//		aout = start_pager();
//		any_error_output = True;
#else
		aout = stdout;
#endif
		(void) fprintf(aout, "%s\n", vmsgbuf);
		macro_output = True;
	}
*/
}


#if defined(_WIN32) /*[*/

/* Missing parts for wc3270. */
#include <windows.h>
#define SECS_BETWEEN_EPOCHS	11644473600ULL
#define SECS_TO_100NS		10000000ULL /* 10^7 */

int
gettimeofday(struct timeval *tv, void *ignored)
{
	FILETIME t;
	ULARGE_INTEGER u;

	GetSystemTimeAsFileTime(&t);
	memcpy(&u, &t, sizeof(ULARGE_INTEGER));

	/* Isolate seconds and move epochs. */
	tv->tv_sec = (DWORD)((u.QuadPart / SECS_TO_100NS) -
			       	SECS_BETWEEN_EPOCHS);
	tv->tv_usec = (u.QuadPart % SECS_TO_100NS) / 10ULL;
	return 0;
}

#endif /*]*/

