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
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como private.h e possui - linhas de código.
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

#define action_name(x)  #x

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

/* types of internal actions */
enum iaction {
	IA_STRING, IA_PASTE, IA_REDRAW,
	IA_KEYPAD, IA_DEFAULT, IA_KEY,
	IA_MACRO, IA_SCRIPT, IA_PEEK,
	IA_TYPEAHEAD, IA_FT, IA_COMMAND, IA_KEYMAP,
	IA_IDLE
};

// Version strings
LIB3270_INTERNAL const char * build;
LIB3270_INTERNAL const char * app_defaults_version;
LIB3270_INTERNAL const char * sccsid;
LIB3270_INTERNAL const char * build_rpq_timestamp;
LIB3270_INTERNAL const char * build_rpq_version;
LIB3270_INTERNAL const char * build_rpq_revision;

#if defined(X3270_DBCS) /*[*/
	LIB3270_INTERNAL Boolean		dbcs;
#endif /*]*/


/*   toggle names */
struct toggle_name {
	const char *name;
	int index;
};


/*   input key type */

/* Naming convention for private actions. */
#define PA_PFX	"PA-"

/* Shorthand macros */

#define CN	((char *) NULL)
#define PN	((XtPointer) NULL)
#define Replace(var, value) { lib3270_free(var); var = (value); };

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

LIB3270_INTERNAL struct _ansictl
{
	char     vintr;
	char     vquit;
	char     verase;
	char     vkill;
	char     veof;
	char     vwerase;
	char     vrprnt;
	char     vlnext;
} ansictl;

/**  extended attributes */
struct lib3270_ea
{
	unsigned char cc;		///< @brief EBCDIC or ASCII character code
	unsigned char fa;		///< @brief field attribute, it nonzero
	unsigned char fg;		///< @brief foreground color (0x00 or 0xf<n>)
	unsigned char bg;		///< @brief background color (0x00 or 0xf<n>)
	unsigned char gr;		///< @brief ANSI graphics rendition bits
	unsigned char cs;		///< @brief character set (GE flag, or 0..2)
	unsigned char ic;		///< @brief input control (DBCS)
	unsigned char db;		///< @brief DBCS state
};

struct lib3270_text
{
	unsigned char  chr;		///< @brief ASCII character code
	unsigned short attr;	///< @brief Converted character attribute (color & etc)
};




/* default charset translation tables */
// LIB3270_INTERNAL const unsigned short ebc2asc0[256];
// LIB3270_INTERNAL const unsigned short asc2ft0[256];


/* Library internal calls */
LIB3270_INTERNAL void key_ACharacter(H3270 *hSession, unsigned char c, enum keytype keytype, enum iaction cause,Boolean *skipped);
LIB3270_INTERNAL void lib3270_initialize(void);
LIB3270_INTERNAL int  cursor_move(H3270 *session, int baddr);

LIB3270_INTERNAL void toggle_rectselect(H3270 *session, struct lib3270_toggle *t, LIB3270_TOGGLE_TYPE tt);
LIB3270_INTERNAL void remove_input_calls(H3270 *session);

LIB3270_INTERNAL int	lib3270_sock_send(H3270 *hSession, unsigned const char *buf, int len);
LIB3270_INTERNAL void	lib3270_sock_disconnect(H3270 *hSession);

LIB3270_INTERNAL int	lib3270_default_event_dispatcher(H3270 *hSession, int block);

#if defined(DEBUG)
	#define CHECK_SESSION_HANDLE(x) check_session_handle(&x,__FUNCTION__);
	LIB3270_INTERNAL void check_session_handle(H3270 **hSession, const char *fname);
#else
	#define CHECK_SESSION_HANDLE(x) check_session_handle(&x);
	LIB3270_INTERNAL void check_session_handle(H3270 **hSession);
#endif // DEBUG

LIB3270_INTERNAL int non_blocking(H3270 *session, Boolean on);

#if defined(HAVE_LIBSSL) /*[*/

	LIB3270_INTERNAL int	ssl_init(H3270 *session);
	LIB3270_INTERNAL int	ssl_negotiate(H3270 *hSession);
	LIB3270_INTERNAL void	set_ssl_state(H3270 *session, LIB3270_SSL_STATE state);


	#if OPENSSL_VERSION_NUMBER >= 0x00907000L /*[*/
		#define INFO_CONST const
	#else /*][*/
		#define INFO_CONST
	#endif /*]*/

	LIB3270_INTERNAL void ssl_info_callback(INFO_CONST SSL *s, int where, int ret);

#endif /*]*/

