/*
 * "Software G3270, desenvolvido com base nos códigos fontes do WC3270  e  X3270
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
 * Este programa está nomeado como globals.h e possui 315 linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas de Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */

/* Autoconf settings. */
#include <lib3270/config.h>		/* autoconf settings */
#include <lib3270.h>			/* lib3270 API calls and defs */
#include "api.h"

/* From glibconfig.h */
#if defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)
	#define LIB3270_INTERNAL __hidden extern
#elif defined (__GNUC__) && defined (HAVE_GNUC_VISIBILITY)
	#define LIB3270_INTERNAL __attribute__((visibility("hidden"))) extern
#else
	#define LIB3270_INTERNAL
#endif

#if defined(X3270_TN3270E) && !defined(X3270_ANSI) /*[*/
		#define X3270_ANSI	1	/* RFC2355 requires NVT mode */
#endif /*]*/

#if defined(HAVE_VASPRINTF) && !defined(_GNU_SOURCE) /*[*/
	#define _GNU_SOURCE		/* vasprintf isn't POSIX */
#endif /*]*/

/*
 * gettext stuff
 */
#ifdef ANDROID
	#undef HAVE_LIBINTL
	#undef HAVE_LIBSSL
#endif

#ifdef HAVE_LIBINTL
	#include <libintl.h>
	#define _( x ) 			gettext(x)
	#define N_( x ) 		x
#else
	#define _( x ) 			x
	#define N_( x ) 		x
#endif // HAVE_LIBINTL


/*
 * OS-specific #defines.  Except for the blocking-connect workarounds, these
 * should be replaced with autoconf probes as soon as possible.
 */

/*
 * BLOCKING_CONNECT_ONLY
 *   Use only blocking sockets.
 */
#if defined(sco) /*[*/
	#define BLOCKING_CONNECT_ONLY	1
#endif /*]*/

#if defined(apollo) /*[*/
	#define BLOCKING_CONNECT_ONLY	1
#endif /*]*/

/*
 * Compiler-specific #defines.
 */

/* 'unused' explicitly flags an unused parameter */
#if defined(__GNUC__) /*[*/
	#define unused __attribute__((__unused__))
	#define printflike(s,f) __attribute__ ((__format__ (__printf__, s, f)))
#else /*][*/
	#define unused /* nothing */
	#define printflike(s, f) /* nothing */
#endif /*]*/



/*
 * Prerequisite #includes.
 */
#include <stdio.h>				/* Unix standard I/O library */
// #include <stdlib.h>				/* Other Unix library functions */
#include <unistd.h>				/* Unix system calls */
#include <ctype.h>				/* Character classes */
#include <string.h>				/* String manipulations */
#include <sys/types.h>			/* Basic system data types */
#include <sys/time.h>			/* System time-related data types */
#include <time.h>				/* C library time functions */
#include "localdefs.h"			/* {s,tcl,c}3270-specific defines */

/*
 * Cancel out contradictory parts.
 */
#if !defined(X3270_DISPLAY) /*[*/
	#undef X3270_KEYPAD
	#undef X3270_MENUS
#endif /*]*/

/* Local process (-e) header files. */ /*
#if defined(X3270_LOCAL_PROCESS) && defined(HAVE_LIBUTIL)
	#define LOCAL_PROCESS	1
	#include <termios.h>

	#if defined(HAVE_PTY_H)
		#include <pty.h>
	#endif

	#if defined(HAVE_LIBUTIL_H)
		#include <libutil.h>
	#endif

	#if defined(HAVE_UTIL_H)
		#include <util.h>
	#endif
#endif
*/

/* Functions we may need to supply. */
#if defined(NEED_STRTOK_R) /*[*/
	extern char *strtok_r(char *str, const char *sep, char **last);
#endif /*]*/

/* Stop conflicting with curses' COLS, even if we don't link with it. */
// #define COLS cCOLS

#define CHECK_SESSION_HANDLE(x) if(!x) x = &h3270;


/* types of internal actions */
enum iaction {
	IA_STRING, IA_PASTE, IA_REDRAW,
	IA_KEYPAD, IA_DEFAULT, IA_KEY,
	IA_MACRO, IA_SCRIPT, IA_PEEK,
	IA_TYPEAHEAD, IA_FT, IA_COMMAND, IA_KEYMAP,
	IA_IDLE
};

LIB3270_INTERNAL int		COLS;
LIB3270_INTERNAL int		ROWS;
extern H3270		h3270;

#if defined(X3270_DISPLAY) /*[*/
	LIB3270_INTERNAL Atom		a_3270, a_registry, a_encoding;
	LIB3270_INTERNAL XtAppContext	appcontext;
#endif /*]*/


// Version strings
LIB3270_INTERNAL const char * build;
LIB3270_INTERNAL const char * app_defaults_version;
LIB3270_INTERNAL const char * sccsid;
LIB3270_INTERNAL const char * build_rpq_timestamp;
LIB3270_INTERNAL const char * build_rpq_version;
LIB3270_INTERNAL const char * build_rpq_revision;

LIB3270_INTERNAL int			  children;

#if defined(X3270_DBCS) /*[*/
	LIB3270_INTERNAL Boolean		dbcs;
#endif /*]*/

#if defined(X3270_FT) /*[*/
	LIB3270_INTERNAL int		dft_buffersize;
#endif /*]*/

// LIB3270_INTERNAL char			*efontname;
LIB3270_INTERNAL Boolean		ever_3270;
LIB3270_INTERNAL Boolean		exiting;

/*
#if defined(X3270_DISPLAY)
	LIB3270_INTERNAL Boolean		*extended_3270font;
	LIB3270_INTERNAL Font			*fid;
	LIB3270_INTERNAL Boolean		*font_8bit;
#endif
*/

// LIB3270_INTERNAL Boolean	flipped;
// LIB3270_INTERNAL char		*full_current_host;
// LIB3270_INTERNAL char		*full_efontname;

#if defined(X3270_DBCS) /*[*/
	LIB3270_INTERNAL char	*full_efontname_dbcs;
#endif /*]*/

//LIB3270_INTERNAL char		*funky_font;
//LIB3270_INTERNAL char		*hostname;

#if defined(X3270_DBCS) /*[*/
	LIB3270_INTERNAL char	*local_encoding;

	#if defined(X3270_DISPLAY) /*[*/
		LIB3270_INTERNAL char	*locale_name;
	#endif /*]*/

#endif /*]*/

/*
#if defined(LOCAL_PROCESS)
	LIB3270_INTERNAL Boolean	local_process;
#endif
*/

// LIB3270_INTERNAL int			maxCOLS;
// LIB3270_INTERNAL int			maxROWS;
// LIB3270_INTERNAL char			*model_name;
// LIB3270_INTERNAL int			model_num;
LIB3270_INTERNAL Boolean		no_login_host;
LIB3270_INTERNAL Boolean		non_tn3270e_host;
// LIB3270_INTERNAL int			ov_cols, ov_rows;
 LIB3270_INTERNAL Boolean		passthru_host;
extern const char	*programname;
LIB3270_INTERNAL char			*qualified_host;
LIB3270_INTERNAL char			*reconnect_host;
LIB3270_INTERNAL int			screen_depth;
LIB3270_INTERNAL Boolean		scroll_initted;

//#if defined(HAVE_LIBSSL) /*[*/
//	LIB3270_INTERNAL Boolean	secure_connection;
//#endif /*]*/

LIB3270_INTERNAL Boolean		shifted;
LIB3270_INTERNAL Boolean		ssl_host;
LIB3270_INTERNAL Boolean		*standard_font;
LIB3270_INTERNAL Boolean		std_ds_host;
LIB3270_INTERNAL char			*termtype;
LIB3270_INTERNAL Widget			toplevel;
// LIB3270_INTERNAL Boolean		visible_control;
LIB3270_INTERNAL int			*xtra_width;

/*
#if defined(X3270_DISPLAY)
	LIB3270_INTERNAL Atom				a_delete_me;
	LIB3270_INTERNAL Atom				a_save_yourself;
	LIB3270_INTERNAL Atom				a_state;
	LIB3270_INTERNAL Display			*display;
	LIB3270_INTERNAL Pixmap				gray;
	LIB3270_INTERNAL Pixel				keypadbg_pixel;
	LIB3270_INTERNAL XrmDatabase		rdb;
	LIB3270_INTERNAL Window				root_window;
	LIB3270_INTERNAL char				*user_title;
	LIB3270_INTERNAL unsigned char	xk_selector;
#endif
*/

/* Connection state */
// LIB3270_INTERNAL enum ft_state ft_state;

/*
LIB3270_INTERNAL enum cstate cstate;
#define PCONNECTED	((int)h3270.cstate >= (int)RESOLVING)
#define HALF_CONNECTED	(h3270.cstate == RESOLVING || h3270.cstate == PENDING)
#define CONNECTED	((int)h3270.cstate >= (int)CONNECTED_INITIAL)
#define IN_NEITHER	(h3270.cstate == CONNECTED_INITIAL)
#define IN_ANSI		(h3270.cstate == CONNECTED_ANSI || h3270.cstate == CONNECTED_NVT)
#define IN_3270		(h3270.cstate == CONNECTED_3270 || h3270.cstate == CONNECTED_TN3270E || h3270.cstate == CONNECTED_SSCP)
#define IN_SSCP		(h3270.cstate == CONNECTED_SSCP)
#define IN_TN3270E	(h3270.cstate == CONNECTED_TN3270E)
#define IN_E		(h3270.cstate >= CONNECTED_INITIAL_E)
*/

/*   keyboard modifer bitmap */
#define ShiftKeyDown	0x01
#define MetaKeyDown	0x02
#define AltKeyDown	0x04

/*   toggle names */
struct toggle_name {
	const char *name;
	int index;
};


/*   translation lists */
struct trans_list {
	char			*name;
	char			*pathname;
	Boolean			is_temp;
	Boolean			from_server;
	struct trans_list	*next;
};
LIB3270_INTERNAL struct trans_list *trans_list;

/*   input key type */
// enum keytype { KT_STD, KT_GE };

/* Naming convention for private actions. */
#define PA_PFX	"PA-"

/* Shorthand macros */

#define CN	((char *) NULL)
#define PN	((XtPointer) NULL)
#define Replace(var, value) { Free(var); var = (value); }

/* Configuration change masks. */
#define NO_CHANGE	0x0000	/* no change */
#define MODEL_CHANGE	0x0001	/* screen dimensions changed */
#define FONT_CHANGE	0x0002	/* emulator font changed */
#define COLOR_CHANGE	0x0004	/* color scheme or 3278/9 mode changed */
#define SCROLL_CHANGE	0x0008	/* scrollbar snapped on or off */
#define CHARSET_CHANGE	0x0010	/* character set changed */
#define ALL_CHANGE	0xffff	/* everything changed */

/* Portability macros */

/*   Equivalent of setlinebuf */

#if defined(_IOLBF) /*[*/
	#define SETLINEBUF(s)	setvbuf(s, (char *)NULL, _IOLBF, BUFSIZ)
#else /*][*/
	#define SETLINEBUF(s)	setlinebuf(s)
#endif /*]*/

/*   Motorola version of gettimeofday */

#if defined(MOTOROLA)
	#define gettimeofday(tp,tz)	gettimeofday(tp)
#endif

/* Default DFT file transfer buffer size. */
#if defined(X3270_FT) && !defined(DFT_BUF) /*[*/
	#define DFT_BUF		(4 * 1024)
#endif /*]*/

/* DBCS Preedit Types */
#if defined(X3270_DBCS) /*[*/
	#define PT_ROOT			"Root"
	#define PT_OVER_THE_SPOT	"OverTheSpot"
	#define PT_OFF_THE_SPOT		"OffTheSpot"
	#define PT_ON_THE_SPOT		"OnTheSpot"
#endif /*]*/

/** input key type */
enum keytype
{
	KT_STD,
	KT_GE
};


/* Library internal calls */
LIB3270_INTERNAL void key_ACharacter(unsigned char c, enum keytype keytype, enum iaction cause,Boolean *skipped);
LIB3270_INTERNAL void lib3270_initialize(void);
LIB3270_INTERNAL int  cursor_move(H3270 *session, int baddr);

