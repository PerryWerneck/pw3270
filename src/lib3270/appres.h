/*
 * Modifications Copyright 1993, 1994, 1995, 1996, 1999, 2000, 2001, 2002,
 *  2003, 2004, 2005, 2007 by Paul Mattes.
 * Copyright 1990 by Jeff Sparkes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 *
 * x3270, c3270, s3270 and tcl3270 are distributed in the hope that they will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the file LICENSE
 * for more details.
 */

/*
 *	appres.h
 *		Application resource definitions for x3270, c3270, s3270 and
 *		tcl3270.
 */

#include "toggle.h"

/* Toggles */

/*
struct toggle {
	char	value;		// toggle value
//	char	changed;	// has the value changed since init
//	Widget	w[2];		// the menu item widgets
//	const char *label[2];	// labels
	void (*upcall)(H3270 *, struct toggle *, LIB3270_TOGGLE_TYPE); // change value
//	void (*callback)(H3270 *, int, LIB3270_TOGGLE_TYPE);

};

// #define toggle_toggle(t) { (t)->value = !(t)->value; }
*/

#define toggled(ix)		lib3270_get_toggle(NULL,ix)

/* Application resources */

typedef struct {
	/* Basic colors */
// #if defined(X3270_DISPLAY) /*[*/
//	Pixel	foreground;
//	Pixel	background;
// #endif /*]*/

	/* Options (not toggles) */
//	char mono;
//	char extended;
//	char m3279;
//	char modified_sel;
//	char once;
//#if defined(X3270_DISPLAY) /*[*/
//	char visual_bell;
//	char menubar;
//	char active_icon;
//	char label_icon;
//	char invert_kpshift;
//	char use_cursor_color;
//	char allow_resize;
//	char no_other;
//	char do_confirms;
// #if !defined(G3270)
//	char reconnect;
// #endif
//	char visual_select;
//	char suppress_host;
//	char suppress_font_menu;
//# if defined(X3270_KEYPAD) /*[*/
//	char keypad_on;
//# endif /*]*/
//#endif /*]*/
//#if defined(C3270) /*[*/
//	char all_bold_on;
//	char curses_keypad;
//	char cbreak_mode;
//#endif /*]*/
//	char apl_mode;
//	char scripted;
//	char numeric_lock;
//	char secure;
//	char oerr_lock;
//	char typeahead;
	char debug_tracing;
	char disconnect_clear;
	char highlight_bold;
//	char color8;
//	char bsd_tm;
//	char unlock_delay;
// #if defined(X3270_SCRIPT) /*[*/
// 	char socket;
// #endif /*]*/
// #if defined(C3270) && defined(_WIN32) /*[*/
//	char highlight_underline;
//#endif /*]*/

	/* Named resources */
/*
#if defined(X3270_KEYPAD)
	char	*keypad;
#endif
*/
#if defined(X3270_DISPLAY) || defined(C3270)
//	char	*key_map;
	char	*compose_map;
//	char	*printer_lu;
#endif
/*
#if defined(X3270_DISPLAY)
	char	*efontname;
	char	*fixed_size;
	char	*debug_font;
	char	*icon_font;
	char	*icon_label_font;
	int		save_lines;
	char	*normal_name;
	char	*select_name;
	char	*bold_name;
	char	*colorbg_name;
	char	*keypadbg_name;
	char	*selbg_name;
	char	*cursor_color_name;
	char	*color_scheme;
	int		bell_volume;
	char	*char_class;
	int		modified_sel_color;
	int		visual_select_color;
#if defined(X3270_DBCS)
	char	*input_method;
	char	*preedit_type;
#endif
#endif
*/
#if defined(X3270_DBCS) /*[*/
	char	*local_encoding;
#endif /*]*/
#if defined(C3270) /*[*/
	char	*meta_escape;
	char	*all_bold;
//	char	*altscreen;
//	char	*defscreen;
#endif /*]*/
	char	*conf_dir;
	char	*model;
//	char	*hostsfile;
//	char	*port;
	char	*charset;
//	char	*termname;
//	char	*login_macro;
	char	*macros;
#if defined(X3270_TRACE) /*[*/
#if !defined(_WIN32) /*[*/
	char	*trace_dir;
#endif /*]*/
	char	*trace_file;
	char	*screentrace_file;
	char	*trace_file_size;
/*
#if defined(X3270_DISPLAY) || defined(WC3270)
	char	trace_monitor;
#endif
*/

#endif /*]*/
//	char	*oversize;
#if defined(X3270_FT) /*[*/
	char	*ft_command;
	int	dft_buffer_size;
#endif /*]*/
	char	*connectfile_name;
	char	*idle_command;
	char	idle_command_enabled;
	char	*idle_timeout;

/*
#if defined(X3270_SCRIPT)
	char	*plugin_command;
#endif
*/

#if defined(HAVE_LIBSSL) /*[*/
	char	*cert_file;
#endif /*]*/
//	char	*proxy;

	/* Toggles */
//	struct toggle toggle[N_TOGGLES];
/*
#if defined(X3270_DISPLAY)
	// Simple widget resources
	Cursor	normal_mcursor;
	Cursor	wait_mcursor;
	Cursor	locked_mcursor;
#endif
*/

/*
#if defined(X3270_ANSI)
	// Line-mode TTY parameters
	char	icrnl;
	char	inlcr;
	char	onlcr;
	char	*erase;
	char	*kill;
	char	*werase;
	char	*rprnt;
	char	*lnext;
	char	*intr;
	char	*quit;
	char	*eof;
#endif
*/

// #if defined(WC3270) /*[*/
//	char	*hostname;
// #endif

/*
#if defined(WC3270)
	char	*title;
#endif
*/

/*
#if defined(USE_APP_DEFAULTS)
	// App-defaults version
	char	*ad_version;
#endif
*/

} AppRes, *AppResptr;

extern AppRes appres;

void toggle_rectselect(H3270 *session, struct lib3270_toggle *t, LIB3270_TOGGLE_TYPE tt);
