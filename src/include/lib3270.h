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
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como lib3270.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

#ifndef LIB3270_H_INCLUDED

	#define LIB3270_H_INCLUDED 1
	#include <stdarg.h>
	#include <errno.h>

	#ifndef ENOTCONN
		#define ENOTCONN 126
	#endif // !ENOTCONN

	#if defined(__GNUC__)
		#define LIB3270_GNUC_FORMAT(s,f) __attribute__ ((__format__ (__printf__, s, f)))
	#else
		#define LIB3270_GNUC_FORMAT(s, f)
	#endif

	/**
	 * BIND definitions.
	 */
	#define LIB3270_BIND_RU					0x31
	#define LIB3270_BIND_OFF_PLU_NAME_LEN	26
	#define LIB3270_BIND_OFF_PLU_NAME		27
	#define LIB3270_BIND_PLU_NAME_MAX		8

	/**
	 * Character attributes
	 */
	typedef enum _lib3270_attr
	{
		LIB3270_ATTR_COLOR_BACKGROUND		= 0x0000,

		LIB3270_ATTR_COLOR_BLUE				= 0x0001,
		LIB3270_ATTR_COLOR_RED				= 0x0002,
		LIB3270_ATTR_COLOR_PINK				= 0x0003,
		LIB3270_ATTR_COLOR_GREEN			= 0x0004,
		LIB3270_ATTR_COLOR_TURQUOISE		= 0x0005,
		LIB3270_ATTR_COLOR_YELLOW			= 0x0006,
		LIB3270_ATTR_COLOR_WHITE			= 0x0007,
		LIB3270_ATTR_COLOR_BLACK			= 0x0008,
		LIB3270_ATTR_COLOR_DARK_BLUE		= 0x0009,
		LIB3270_ATTR_COLOR_ORANGE			= 0x000A,
		LIB3270_ATTR_COLOR_PURPLE			= 0x000B,
		LIB3270_ATTR_COLOR_DARK_GREEN		= 0x000C,
		LIB3270_ATTR_COLOR_DARK_TURQUOISE	= 0x000D,
		LIB3270_ATTR_COLOR_MUSTARD			= 0x000E,
		LIB3270_ATTR_COLOR_GRAY				= 0x000F,

		LIB3270_ATTR_COLOR					= 0x00FF,

		LIB3270_ATTR_FIELD					= 0x0100,
		LIB3270_ATTR_BLINK					= 0x0200,
		LIB3270_ATTR_UNDERLINE				= 0x0400,
		LIB3270_ATTR_INTENSIFY				= 0x0800,

		LIB3270_ATTR_CG						= 0x1000,
		LIB3270_ATTR_MARKER					= 0x2000,
		LIB3270_ATTR_BACKGROUND_INTENSITY	= 0x4000,
		LIB3270_ATTR_SELECTED				= 0x8000

	} LIB3270_ATTR;

	typedef enum _lib3270_toggle
	{
		LIB3270_TOGGLE_MONOCASE,
		LIB3270_TOGGLE_CURSOR_BLINK,
		LIB3270_TOGGLE_SHOW_TIMING,
		LIB3270_TOGGLE_CURSOR_POS,
		LIB3270_TOGGLE_DS_TRACE,
		LIB3270_TOGGLE_LINE_WRAP,
		LIB3270_TOGGLE_BLANK_FILL,
		LIB3270_TOGGLE_SCREEN_TRACE,
		LIB3270_TOGGLE_EVENT_TRACE,
		LIB3270_TOGGLE_MARGINED_PASTE,
		LIB3270_TOGGLE_RECTANGLE_SELECT,
		LIB3270_TOGGLE_CROSSHAIR,
		LIB3270_TOGGLE_FULL_SCREEN,
		LIB3270_TOGGLE_RECONNECT,
		LIB3270_TOGGLE_INSERT,
		LIB3270_TOGGLE_SMART_PASTE,
		LIB3270_TOGGLE_BOLD,
		LIB3270_TOGGLE_KEEP_SELECTED,
		LIB3270_TOGGLE_UNDERLINE,					/**< Show underline ? */
		LIB3270_TOGGLE_CONNECT_ON_STARTUP,
		LIB3270_TOGGLE_KP_ALTERNATIVE,              /**< Keypad +/- move to next/previous field */
		LIB3270_TOGGLE_BEEP,						/**< Beep on errors */
		LIB3270_TOGGLE_VIEW_FIELD,					/**< View Field attribute */
		LIB3270_TOGGLE_ALTSCREEN,					/**< auto resize on altscreen */
		LIB3270_TOGGLE_KEEP_ALIVE,					/**< Enable network keep-alive with SO_KEEPALIVE */
		LIB3270_TOGGLE_NETWORK_TRACE,				/**< Enable network in/out trace */


//		LIB3270_TOGGLE_ALT_CURSOR,
//		LIB3270_TOGGLE_AID_WAIT,
//		LIB3270_TOGGLE_SCROLL_BAR,
//		LIB3270_TOGGLE_KEYPAD,

		LIB3270_TOGGLE_COUNT

	} LIB3270_TOGGLE;

	typedef enum _lib3270_direction
	{
		LIB3270_DIR_UP,
		LIB3270_DIR_DOWN,
		LIB3270_DIR_LEFT,
		LIB3270_DIR_RIGHT,

		LIB3270_DIR_END,

	} LIB3270_DIRECTION;

	/**
	 * Toggle types.
	 *
	 */
	typedef enum _LIB3270_TOGGLE_TYPE
	{
		LIB3270_TOGGLE_TYPE_INITIAL,
		LIB3270_TOGGLE_TYPE_INTERACTIVE,
		LIB3270_TOGGLE_TYPE_ACTION,
		LIB3270_TOGGLE_TYPE_FINAL,
		LIB3270_TOGGLE_TYPE_UPDATE,

		LIB3270_TOGGLE_TYPE_USER

	} LIB3270_TOGGLE_TYPE;


	/**
	 * OIA Status indicators.
	 *
	 */
	typedef enum _lib3270_flag
	{
		LIB3270_FLAG_BOXSOLID,	/**< System available */
		LIB3270_FLAG_UNDERA,	/**< Control Unit STATUS */
		LIB3270_FLAG_TYPEAHEAD,
		LIB3270_FLAG_PRINTER,	/**< Printer session status */
		LIB3270_FLAG_REVERSE,
		LIB3270_FLAG_SCRIPT,	/**< Script status */

		LIB3270_FLAG_COUNT

	} LIB3270_FLAG;


	/**
	 * 3270 program messages.
	 *
	 */
	typedef enum _LIB3270_MESSAGE
	{
		LIB3270_MESSAGE_NONE,				/**<  0 - No message */
		LIB3270_MESSAGE_SYSWAIT,			/**<  1 - */
		LIB3270_MESSAGE_TWAIT,				/**<  2 - */
		LIB3270_MESSAGE_CONNECTED,			/**<  3 - Connected */
		LIB3270_MESSAGE_DISCONNECTED,		/**<  4 - Disconnected from host */
		LIB3270_MESSAGE_AWAITING_FIRST,		/**<  5 - */
		LIB3270_MESSAGE_MINUS,				/**<  6 - */
		LIB3270_MESSAGE_PROTECTED,			/**<  7 - */
		LIB3270_MESSAGE_NUMERIC,			/**<  8 - */
		LIB3270_MESSAGE_OVERFLOW,			/**<  9 - */
		LIB3270_MESSAGE_INHIBIT,			/**< 10 - */
		LIB3270_MESSAGE_KYBDLOCK,			/**< 11 - Keyboard is locked */

		LIB3270_MESSAGE_X,					/**< 12 - */
		LIB3270_MESSAGE_RESOLVING,			/**< 13 - Resolving hostname (running DNS query) */
		LIB3270_MESSAGE_CONNECTING,			/**< 14 - Connecting to host */

		LIB3270_MESSAGE_USER

	} LIB3270_MESSAGE;


	/**
	 * Connect options.
	 *
	 */
	typedef enum _LIB3270_CONNECT_OPTION
	{
		LIB3270_CONNECT_OPTION_DEFAULTS	= 0x0000,	/**< Default connection options */
		LIB3270_CONNECT_OPTION_SSL		= 0x0001,	/**< Secure connection */
		LIB3270_CONNECT_OPTION_WAIT		= 0x0002,	/**< Wait for screen ready */

	} LIB3270_CONNECT_OPTION;

	/**
	 * Cursor modes.
	 *
	 * Cursor modes set by library; an application can us it
	 * as a hint to change the mouse cursor based on connection status.
	 *
	 */
	typedef enum _LIB3270_CURSOR
	{
		LIB3270_CURSOR_EDITABLE,	/**< Ready for user actions */
		LIB3270_CURSOR_WAITING,		/**< Waiting for host */
		LIB3270_CURSOR_LOCKED,		/**< Locked, can't receive user actions */

		LIB3270_CURSOR_USER
	} LIB3270_CURSOR;


	/**
	 * connection state
	 */
	typedef enum lib3270_cstate
	{
		LIB3270_NOT_CONNECTED,			/**< no socket, disconnected */
		LIB3270_RESOLVING,				/**< resolving hostname */
		LIB3270_PENDING,				/**< connection pending */
		LIB3270_CONNECTED_INITIAL,		/**< connected, no mode yet */
		LIB3270_CONNECTED_ANSI,			/**< connected in NVT ANSI mode */
		LIB3270_CONNECTED_3270,			/**< connected in old-style 3270 mode */
		LIB3270_CONNECTED_INITIAL_E,	/**< connected in TN3270E mode, unnegotiated */
		LIB3270_CONNECTED_NVT,			/**< connected in TN3270E mode, NVT mode */
		LIB3270_CONNECTED_SSCP,			/**< connected in TN3270E mode, SSCP-LU mode */
		LIB3270_CONNECTED_TN3270E		/**< connected in TN3270E mode, 3270 mode */
	} LIB3270_CSTATE;


	/**
	 * Connect options
	 *
	 */
	typedef enum lib3270_option
	{
		LIB3270_OPTION_KYBD_AS400	= 0x0001,	/**< Prefix every PF with PA1 */
		LIB3270_OPTION_TSO			= 0x0002,	/**< Host is TSO? */

	} LIB3270_OPTION;


//		LIB3270_OPTION_COLOR8	 	= 0x0001,	/**< If active, pw3270 will respond to a Query(Color) with a list of 8 supported colors. */

	#define LIB3270_OPTION_DEFAULT	0
	#define LIB3270_OPTION_COUNT	3

	typedef struct _lib3270_option_entry
	{
		LIB3270_OPTION	  value;
		const char		* key;
		const char		* text;
		const char		* tooltip;
	} LIB3270_OPTION_ENTRY;

	/**
	 * SSL state
	 *
	 */
	typedef enum lib3270_ssl_state
	{
		LIB3270_SSL_UNSECURE,			/**< No secure connection */
		LIB3270_SSL_SECURE,				/**< Connection secure with CA check */
		LIB3270_SSL_NEGOTIATED,			/**< Connection secure, no CA or self-signed */
		LIB3270_SSL_NEGOTIATING,		/**< Negotiating SSL */
		LIB3270_SSL_UNDEFINED			/**< Undefined */
	} LIB3270_SSL_STATE;

	#define LIB3270_SSL_FAILED LIB3270_SSL_UNSECURE

#ifdef __cplusplus
	extern "C" {
#endif

	#include <lib3270/config.h>

	#if defined( ANDROID )

		#define LIB3270_EXPORT	__attribute__((visibility("hidden"))) extern

	#elif defined(_WIN32)

		#include <windows.h>
		#define LIB3270_EXPORT	__declspec (dllexport)

	#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x550)

		#define LIB3270_EXPORT

	#elif defined (HAVE_GNUC_VISIBILITY)

		#define LIB3270_EXPORT	__attribute__((visibility("default"))) extern

	#else

		#error Unable to set visibility attribute

	#endif

	/* State change IDs. */
	typedef enum _lib3270_state
	{
		LIB3270_STATE_RESOLVING,
		LIB3270_STATE_HALF_CONNECT,
		LIB3270_STATE_CONNECT,
		LIB3270_STATE_3270_MODE,
		LIB3270_STATE_LINE_MODE,
		LIB3270_STATE_REMODEL,
		LIB3270_STATE_PRINTER,
		LIB3270_STATE_EXITING,
		LIB3270_STATE_CHARSET,

		LIB3270_STATE_USER				// Always the last one
	} LIB3270_STATE;

	typedef struct _h3270 H3270;

	/**
	 * Get current screen size.
	 *
	 * Get the size of the terminal in rows/cols; this value can differ from
	 * the model if there's an active "altscreen" with diferent size.
	 *
	 * @param h	Handle of the desired session.
	 * @param r Pointer to screen rows.
	 * @param c Pointer to screen columns.
	 *
	 */
	LIB3270_EXPORT void lib3270_get_screen_size(H3270 *h, int *r, int *c);


	/**
	 * Get current screen width in columns.
	 *
	 * @param h	Handle of the desired session.
	 *
	 * @return screen width.
	 *
	 */
	LIB3270_EXPORT int lib3270_get_width(H3270 *h);

	LIB3270_EXPORT unsigned int lib3270_get_length(H3270 *h);

	/**
	 * Start a new session (INCOMPLETE).
	 *
	 * Initialize session structure, opens a new session.
	 * WARNING: Multi session ins't yet supported in lib3270, because of this
	 * this call always return the handle of the same session.
	 *
	 * @param model	Terminal model.
	 *
	 * @return lib3270 internal session structure.
	 *
	 */
	LIB3270_EXPORT H3270 * lib3270_session_new(const char *model);

	/**
	 * Destroy session, release memory.
	 *
	 * @param h		Session handle.
	 *
	 */
	LIB3270_EXPORT void lib3270_session_free(H3270 *h);

	/**
	 * Register a state change callback.
	 *
	 * @param h		Session handle.
	 * @param tx	State ID
	 * @param func	Callback
	 * @param data	Data
	 *
	 */
	LIB3270_EXPORT void lib3270_register_schange(H3270 *h,LIB3270_STATE tx, void (*func)(H3270 *, int, void *),void *data);


	/**
	 * Set host id for the connect/reconnect operations.
	 *
	 * @param h		Session handle.
	 * @param url	URL of host to set in the format tn3270://hostname:service ou tn3270s://hostname:service .
	 *
	 * @return Processed host url
	 *
	 */
	 LIB3270_EXPORT const char * lib3270_set_host(H3270 *h, const char *url);

	/**
	 * Get host id for the connect/reconnect operations.
	 *
	 * @param h		Session handle.
	 *
	 * @return Pointer to host id set (internal data, do not change it)
	 *
	 */
	 LIB3270_EXPORT const char * lib3270_get_host(H3270 *h);

	/**
	 * Network connect operation, keep main loop running
	 *
	 * Sets 'reconnect_host', 'current_host' and 'full_current_host' as
	 * side-effects.
	 *
	 * @param h		Session handle.
	 * @param n		Host ID (NULL to use the last one)
	 * @param wait	Non zero to wait for connection to be ok.
	 *
	 * @return 0 for success, EAGAIN if auto-reconnect is in progress, EBUSY if connected, ENOTCONN if connection has failed, -1 on unexpected failure.
	 *
	 */
	LIB3270_EXPORT int lib3270_connect(H3270 *h,const char *n, int wait);

	/**
	 * Connect to defined host, keep main loop running.
	 *
	 * @param hSession	Session handle.
	 * @param hostname	Host name.
	 * @param srvc		Service name (telnet if NULL).
	 * @param opt		Connect options.
	 *
	 * @return 0 for success, EAGAIN if auto-reconnect is in progress, EBUSY if connected, ENOTCONN if connection has failed, -1 on unexpected failure.
	 *
	 */
	LIB3270_EXPORT int lib3270_connect_host(H3270 *hSession, const char *hostname, const char *srvc, LIB3270_CONNECT_OPTION opt);


	/**
	 * Disconnect from host.
	 *
	 * @param h		Session handle.
	 *
	 */
	LIB3270_EXPORT int lib3270_disconnect(H3270 *h);

	/**
	 * Reconnect.
	 *
	 * @param h		Session handle.
	 * @param wait	Non zero to wait for connection to be ok.
	 */
	LIB3270_EXPORT int lib3270_reconnect(H3270 *h,int wait);

	/**
	 * Get connection state.
	 *
	 * @param h		Session handle.
	 *
	 * @return Connection state.
	 *
	 */
	LIB3270_EXPORT LIB3270_CSTATE lib3270_get_connection_state(H3270 *h);

	/**
	 * Pretend that a sequence of keys was entered at the keyboard.
	 *
	 * "Pasting" means that the sequence came from the clipboard.  Returns are
	 * ignored; newlines mean "move to beginning of next line"; tabs and formfeeds
	 * become spaces.  Backslashes are not special, but ASCII ESC characters are
	 * used to signify 3270 Graphic Escapes.
	 *
	 * "Not pasting" means that the sequence is a login string specified in the
	 * hosts file, or a parameter to the String action.  Returns are "move to
	 * beginning of next line"; newlines mean "Enter AID" and the termination of
	 * processing the string.  Backslashes are processed as in C.
	 *
	 * @param s			String to input.
	 * @param len		Size of the string (or -1 to null terminated strings)
	 * @param pasting	Non zero for pasting (See comments).
	 *
	 * @return The number of unprocessed characters or -1 if failed
	 */
	LIB3270_EXPORT int lib3270_emulate_input(H3270 *session, const char *s, int len, int pasting);

	/**
	 * Set string at current cursor position.
	 *
	 * Returns are ignored; newlines mean "move to beginning of next line";
	 * tabs and formfeeds become spaces.  Backslashes are not special
	 *
	 * @param h		Session handle.
	 * @param s		String to input.
	 *
	 * @return Negative if error or number of processed characters.
	 *
	 */
	LIB3270_EXPORT int lib3270_set_string(H3270 *h, const unsigned char *str);

	#define lib3270_set_text_at(h,r,c,t) lib3270_set_string_at(h,r,c,t)
	LIB3270_EXPORT int lib3270_set_string_at(H3270 *h, int row, int col, const unsigned char *str);
	LIB3270_EXPORT int lib3270_input_string(H3270 *hSession, const unsigned char *str);

	/**
	 * Set cursor address.
	 *
	 * @param h		Session handle.
	 * @param baddr	New cursor address.
	 *
	 * @return last cursor address.
	 *
	 */
	LIB3270_EXPORT int lib3270_set_cursor_address(H3270 *h, int baddr);

	/**
	 * Set cursor position.
	 *
	 * @param h		Session handle.
	 * @param row	New cursor row.
	 * @param col	New cursor col.
	 *
	 * @return last cursor address.
	 *
	 */
	LIB3270_EXPORT int lib3270_set_cursor_position(H3270 *h, int row, int col);

	/**
	 * get cursor address.
	 *
	 * @param h		Session handle.
	 *
	 * @return Cursor address.
	 *
	 */
	LIB3270_EXPORT int lib3270_get_cursor_address(H3270 *h);


	/**
	 * Move cursor
	 *
	 * @param h		Session handle.
	 * @param dir	Direction to move
	 * @param sel	Non zero to move and selected to the current cursor position
	 *
	 * @return 0 if the movement can be done, non zero if failed.
	 */
	 LIB3270_EXPORT int lib3270_move_cursor(H3270 *h, LIB3270_DIRECTION dir, unsigned char sel);

	/**
	 * Print page
	 *
	 * @param h		Session Handle.
	 *
	 * @return 0 if ok, error code if not.
	 *
	 */
	LIB3270_EXPORT int lib3270_print(H3270 *h);

	/**
	 * Get buffer contents.
	 *
	 * @param h		Session handle.
	 * @param first	First element to get.
	 * @param last	Last element to get.
	 * @param chr	Pointer to buffer which will receive the read chars.
	 * @param attr	Pointer to buffer which will receive the chars attributes.
	 *
	 */
	LIB3270_EXPORT int lib3270_get_contents(H3270 *h, int first, int last, unsigned char *chr, unsigned short *attr);

	/**
	 * get toggle state.
	 *
	 * @param h		Session handle.
	 * @param ix	Toggle id.
	 *
	 * @return 0 if the toggle is disabled, non zero if enabled.
	 *
	 */
	LIB3270_EXPORT unsigned char lib3270_get_toggle(H3270 *h, LIB3270_TOGGLE ix);

	/**
	 * Set toggle state.
	 *
	 * @param h		Session handle.
	 * @param ix	Toggle id.
	 * @param value	New toggle state (non zero for true).
	 *
	 * @returns 0 if the toggle is already at the state, 1 if the toggle was changed; < 0 on invalid toggle id
	 */
	LIB3270_EXPORT int lib3270_set_toggle(H3270 *h, LIB3270_TOGGLE ix, int value);

	/**
	 * Translate a string toggle name to the corresponding value.
	 *
	 * @param name	Toggle name.
	 *
	 * @return Toggle ID or -1 if it's invalid.
	 *
	 */
	LIB3270_EXPORT LIB3270_TOGGLE lib3270_get_toggle_id(const char *name);

	/**
	 * Get the toggle name as string.
	 *
	 * @param id	Toggle id
	 *
	 * @return Constant string with the toggle name or "" if invalid.
	 *
	 */
	LIB3270_EXPORT const char * lib3270_get_toggle_name(LIB3270_TOGGLE ix);

	/**
	 * Revert toggle status.
	 *
	 * @param h		Session handle.
	 * @param ix	Toggle id.
	 *
	 * @return Toggle status.
	 */
	LIB3270_EXPORT int lib3270_toggle(H3270 *h, LIB3270_TOGGLE ix);

	/**
	 * Check if the active connection is secure.
	 *
	 * @param h		Session handle.
	 *
	 * @return Non 0 if the connection is SSL secured, 0 if not.
	 */
	LIB3270_EXPORT int lib3270_get_ssl_state(H3270 *h);

	/** Callback table
	 *
	 * Structure with GUI unblocking I/O calls, used to replace the lib3270´s internal ones.
	 *
	 */
	struct lib3270_callbacks
	{
		unsigned short sz;

		void	* (*AddTimeOut)(unsigned long interval_ms, H3270 *session, void (*proc)(H3270 *session));
		void	  (*RemoveTimeOut)(void *timer);

#ifdef WIN32
		void	* (*AddInput)(HANDLE source, H3270 *session, void (*fn)(H3270 *session));
		void	* (*AddOutput)(HANDLE source, H3270 *session, void (*fn)(H3270 *session));
#else
		void	* (*AddInput)(int source, H3270 *session, void (*fn)(H3270 *session));
		void	* (*AddOutput)(int source, H3270 *session, void (*fn)(H3270 *session));
#endif // WIN32
		void	  (*RemoveSource)(void *id);

#ifdef WIN32
		void 	* (*AddExcept)(HANDLE source, H3270 *session, void (*fn)(H3270 *session));
#else
		void 	* (*AddExcept)(int source, H3270 *session, void (*fn)(H3270 *session));
#endif

		int 			(*callthread)(int(*callback)(H3270 *, void *), H3270 *session, void *parm);

		int				(*Wait)(H3270 *hSession, int seconds);
		int 			(*event_dispatcher)(H3270 *hSession, int wait);
		void			(*ring_bell)(H3270 *);

	};

	/**
	 * Register application Handlers.
	 *
	 * @param cbk	Structure with the application I/O handles to set.
	 *
	 * @return 0 if ok, error code if not.
	 *
	 */
	int LIB3270_EXPORT lib3270_register_handlers(const struct lib3270_callbacks *cbk);

	/**
	 * Register time handlers.
	 *
	 * @param add	Callback for adding a timeout
	 * @param rm	Callback for removing a timeout
	 *
	 */
	void LIB3270_EXPORT lib3270_register_time_handlers(void * (*add)(unsigned long interval_ms, H3270 *session, void (*proc)(H3270 *session)), void (*rm)(void *timer));

	/**
	 * Get program message.
	 *
	 * @see LIB3270_MESSAGE
	 *
	 * @param h	Session handle.
	 *
	 * @return Latest program message.
	 *
	 */
	LIB3270_EXPORT LIB3270_MESSAGE	  lib3270_get_program_message(H3270 *h);

	/**
	 * Get connected LU name.
	 *
	 * Get the name of the connected LU; the value is internal to lib3270 and
	 * should not be changed ou freed.
	 *
	 * @param h	Session handle.
	 *
	 * @return conected LU name or NULL if not connected.
	 *
	 */
	LIB3270_EXPORT const char		* lib3270_get_luname(H3270 *h);

//	#define lib3270_has_printer_session(h) 	(h->oia_flag[LIB3270_FLAG_PRINTER] != 0)
	#define lib3270_has_active_script(h)	(h->oia_flag[LIB3270_FLAG_SCRIPT] != 0)
	#define lib3270_get_typeahead(h)		(h->oia_flag[LIB3270_FLAG_TYPEAHEAD] != 0)
	#define lib3270_get_undera(h)			(h->oia_flag[LIB3270_FLAG_UNDERA] != 0)
	#define lib3270_get_oia_box_solid(h)	(h->oia_flag[LIB3270_FLAG_BOXSOLID] != 0)

	LIB3270_EXPORT int lib3270_pconnected(H3270 *h);
	LIB3270_EXPORT int lib3270_half_connected(H3270 *h);
	LIB3270_EXPORT int lib3270_connected(H3270 *h);
	LIB3270_EXPORT int lib3270_disconnected(H3270 *h);
	LIB3270_EXPORT int lib3270_in_neither(H3270 *h);
	LIB3270_EXPORT int lib3270_in_ansi(H3270 *h);
	LIB3270_EXPORT int lib3270_in_3270(H3270 *h);
	LIB3270_EXPORT int lib3270_in_sscp(H3270 *h);
	LIB3270_EXPORT int lib3270_in_tn3270e(H3270 *h);
	LIB3270_EXPORT int lib3270_in_e(H3270 *h);

	LIB3270_EXPORT int lib3270_is_ready(H3270 *h);
	LIB3270_EXPORT int lib3270_is_connected(H3270 *h);
	LIB3270_EXPORT int lib3270_is_secure(H3270 *h);

	LIB3270_EXPORT LIB3270_MESSAGE		lib3270_lock_status(H3270 *h);
	LIB3270_EXPORT LIB3270_SSL_STATE	lib3270_get_secure(H3270 *session);
	LIB3270_EXPORT long 				lib3270_get_SSL_verify_result(H3270 *session);


	/**
	 * Call non gui function.
	 *
	 * Call informed function in a separate thread, keep gui main loop running until
	 * the function returns.
	 *
	 * @param callback	Function to call.
	 * @param h			Related session (for timer indicator)
	 * @param parm		Parameter to be passed to the function.
	 *
	 */
	LIB3270_EXPORT int lib3270_call_thread(int(*callback)(H3270 *h, void *), H3270 *h, void *parm);


	/**
	 * Run main iteration.
	 *
	 * Run lib3270 internal iterations, check for network inputs, process signals.
	 *
	 * @param h		Related session.
	 * @param wait	Wait for signal if not available.
	 *
	 */
	LIB3270_EXPORT void lib3270_main_iterate(H3270 *h, int wait);

	/**
	 * Wait for "N" seconds keeping main loop active.
	 *
	 * @param seconds	Number of seconds to wait.
	 *
	 */
	LIB3270_EXPORT int lib3270_wait(H3270 *hSession, int seconds);

	/**
	 * Wait "N" seconds for "ready" state.
	 *
	 * @param seconds	Number of seconds to wait.
	 *
	 * @return 0 if ok, errno code if not.
	 *
	 */
	LIB3270_EXPORT int lib3270_wait_for_ready(H3270 *hSession, int seconds);

	/**
	 * Get the session's widget.
	 *
	 * Get the handle to the GtkWidget who's handling this session.
	 *
	 * @param h		Session handle.
	 *
	 * @return Associated GtkWidget (can be null)
	 *
	 */
	 LIB3270_EXPORT void * lib3270_get_widget(H3270 *h);

	/**
	 * "beep" to notify user.
	 *
	 * If available play a sound signal do alert user.
	 *
	 * @param h		Session handle.
	 *
	 */
	 LIB3270_EXPORT void lib3270_ring_bell(H3270 *session);


	/**
	 * Get lib3270's charset.
	 *
	 * @param h Session handle.
	 *
	 * @return String with current encoding.
	 *
	 */
	 LIB3270_EXPORT const char * lib3270_get_display_charset(H3270 *session);

	 #define lib3270_get_charset(s) lib3270_get_display_charset(s)

	 LIB3270_EXPORT const char * lib3270_get_default_charset(void);

	/**
	 * Get selected area.
	 *
	 * @param h	Session Handle.
	 *
	 * @return selected text if available, or NULL. Release it with free()
	 *
	 */
	LIB3270_EXPORT char * lib3270_get_selected(H3270 *h);

	LIB3270_EXPORT char * lib3270_cut_selected(H3270 *hSession);

	/**
	 * Get all text inside the terminal.
	 *
	 * @param h			Session Handle.
	 * @param offset	Start position.
	 * @param len		Text length or -1 to all text.
	 *
	 * @return Contents at position if available, or NULL. Release it with lib3270_free()
	 *
	 */
	LIB3270_EXPORT char * lib3270_get_text(H3270 *h, int offset, int len);

	/**
	 * Get text at requested position
	 *
	 * @param h			Session Handle.
	 * @param row		Desired row.
	 * @param col		Desired col.
	 * @param length	Text length
	 *
	 * @return Contents at position if available, or NULL. Release it with lib3270_free()
	 *
	 */
	LIB3270_EXPORT char * lib3270_get_text_at(H3270 *h, int row, int col, int len);

	/**
	 * Check for text at requested position
	 *
	 * @param h			Session Handle.
	 * @param row		Desired row.
	 * @param col		Desired col.
	 * @param text		Text to check.
	 *
	 * @return Test result from strcmp
	 *
	 */
	 LIB3270_EXPORT int lib3270_cmp_text_at(H3270 *h, int row, int col, const char *text);


	/**
	 * Get contents of the field at position.
	 *
	 * @param h			Session Handle.
	 * @param baddr		Reference position.
	 *
	 * @return Contents of the entire field, release it with lib3270_free()
	 *
	 */
	LIB3270_EXPORT char * lib3270_get_field_at(H3270 *h, int baddr);

	/**
	 * Find the next unprotected field.
	 *
	 * @param hSession	Session handle.
	 * @param baddr0	Search start addr (-1 to use current cursor position).
	 *
	 * @return address following the unprotected attribute byte, or 0 if no nonzero-width unprotected field can be found.
	 *
	 */
	LIB3270_EXPORT int lib3270_get_next_unprotected(H3270 *hSession, int baddr0);

	/**
	 * Get address of the first blank.
	 *
	 * Get address of the first blank after the last nonblank in the
	 * field, or if the field is full, to the last character in the field.
	 *
	 * @param hSession	Session handle.
	 * @param baddr		Field address.
	 *
	 * @return address of the first blank or -1 if invalid.
	 */
	LIB3270_EXPORT int lib3270_get_field_end(H3270 *hSession, int baddr);

	/**
	 * Find the buffer address of the field attribute for a given buffer address.
	 *
	 * @param h		Session handle.
	 * @param addr	Buffer address of the field.
	 *
	 * @return field address or -1 if the screen isn't formatted.
	 *
	 */
	LIB3270_EXPORT int lib3270_field_addr(H3270 *h, int baddr);

	LIB3270_EXPORT int lib3270_field_attribute(H3270 *hSession, int baddr);

	/**
	 * Get the length of the field at given buffer address.
	 *
	 * @param h		Session handle.
	 * @param addr	Buffer address of the field.
	 *
	 * @return field length.
	 *
	 */
	LIB3270_EXPORT int lib3270_field_length(H3270 *h, int baddr);


	/**
	 * Get a terminal character and attribute.
	 *
	 * @param h		Session Handle.
	 * @param baddr	Element address ((element_row*cols)+element_col)
	 * @param c		Pointer to character.
	 * @param attr	Pointer to attribute.
	 *
	 * @return 0 if ok or error code.
	 *
	 */
	LIB3270_EXPORT int lib3270_get_element(H3270 *h, int baddr, unsigned char *c, unsigned short *attr);

	/**
	 * Get field region
	 *
	 * @param h		Session handle.
	 * @param baddr	Reference position to get the field start/stop offsets.
	 * @param start	return location for start of selection, as a character offset.
	 * @param end	return location for end of selection, as a character offset.
	 *
	 * @return Non 0 if invalid
	 *
	 */
	LIB3270_EXPORT int lib3270_get_field_bounds(H3270 *hSession, int baddr, int *start, int *end);

	LIB3270_EXPORT int lib3270_get_field_start(H3270 *hSession, int baddr);
	LIB3270_EXPORT int lib3270_get_field_len(H3270 *hSession, int baddr);

	LIB3270_EXPORT int lib3270_get_word_bounds(H3270 *hSession, int baddr, int *start, int *end);


	LIB3270_EXPORT int lib3270_set_model(H3270 *session, int model);
	LIB3270_EXPORT int lib3270_set_model_name(H3270 *hSession, const char *name);

	LIB3270_EXPORT int lib3270_get_model(H3270 *session);

	LIB3270_EXPORT int lib3270_is_protected(H3270 *h, unsigned int baddr);

	/**
	 * Alloc/Realloc memory buffer.
	 *
	 * Allocate/reallocate an array.
	 *
	 * @param elsize	Element size.
	 * @param nelem		Number of elements in the array.
	 * @param ptr		Pointer to the actual array.
	 *
	 * @return Clean buffer with size for the number of elements.
	 *
	 */
	LIB3270_EXPORT void * lib3270_calloc(int elsize, int nelem, void *ptr);

	LIB3270_EXPORT void * lib3270_malloc(int len);
	LIB3270_EXPORT void * lib3270_realloc(void *p, int len);
	LIB3270_EXPORT void * lib3270_replace(void **p, void *ptr);
	LIB3270_EXPORT void * lib3270_strdup(const char *str);

	/**
	 * Release allocated memory.
	 *
	 * @param p	Memory block to release (can be NULL)
	 *
	 * @return NULL
	 */
	LIB3270_EXPORT void  * lib3270_free(void *p);


	/**
	 * Get default session handle.
	 *
	 * @return Internal's lib3270 session handle.
	 *
	 */
	LIB3270_EXPORT H3270 * lib3270_get_default_session_handle(void);

	/**
	 * Get resource string.
	 *
	 * @param first_element	First element of resource path
	 * @param ...			Resource path (ends with NULL)
	 *
	 * @return Resource string (Release with lib3270_free())
	 *
	 */
	LIB3270_EXPORT char * lib3270_get_resource_string(H3270 *hSession, const char *first_element, ...);

	/**
	 * Get library version.
	 *
	 * @return Version of active library as string.
	 *
	 */
	LIB3270_EXPORT const char * lib3270_get_version(void);

	/**
	 * Get source code revision.
	 *
	 * @return SVN revision of the current source code.
	 *
	 */
	LIB3270_EXPORT const char * lib3270_get_revision(void);

	LIB3270_EXPORT char * lib3270_vsprintf(const char *fmt, va_list args);
	LIB3270_EXPORT char * lib3270_strdup_printf(const char *fmt, ...);

	LIB3270_EXPORT int lib3270_clear_operator_error(H3270 *hSession);

	LIB3270_EXPORT LIB3270_OPTION lib3270_get_options(H3270 *hSession);
	LIB3270_EXPORT void lib3270_set_options(H3270 *hSession, LIB3270_OPTION opt);
	LIB3270_EXPORT int	lib3270_set_color_type(H3270 *hSession, unsigned short colortype);

	LIB3270_EXPORT const LIB3270_OPTION_ENTRY * lib3270_get_option_list(void);

	/**
	 * The host is TSO?
	 *
	 * @param hSession	Session Handle
	 *
	 * @return Non zero if the host is TSO.
	 *
	 */
	LIB3270_EXPORT int 	 lib3270_is_tso(H3270 *hSession);

#ifdef WIN32
	LIB3270_EXPORT const char	* lib3270_win32_strerror(int e);
	LIB3270_EXPORT const char	* lib3270_win32_local_charset(void);
#endif // WIn32

#ifdef __cplusplus
	}
#endif

#endif // LIB3270_H_INCLUDED
