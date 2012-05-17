/*
 * Copyright 1999, 2000, 2002 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 *
 * c3270 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the file LICENSE for more details.
 */

#ifndef SCREENC_H_INCLUDED

#define SCREENC_H_INCLUDED 1
/* c3270 version of screenc.h */

#define blink_start()
#define display_heightMM()	100
#define display_height()	1
#define display_widthMM()	100
#define display_width()		1
// #define screen_obscured()	False
#define screen_scroll()		screen_disp()
#define screen_132()	/* */
#define screen_80()		/* */

#define screen_erase(x) x->erase(x)
// LIB3270_INTERNAL void screen_erase(H3270 *session);

LIB3270_INTERNAL void screen_changed(H3270 *session, int bstart, int bend);
LIB3270_INTERNAL int screen_init(H3270 *session);
// LIB3270_INTERNAL void screen_flip(void);
LIB3270_INTERNAL FILE *start_pager(void);
LIB3270_INTERNAL Boolean screen_new_display_charsets(char *cslist, char *csname);

LIB3270_INTERNAL void mcursor_set(H3270 *session,LIB3270_CURSOR m);

#define mcursor_locked(x) mcursor_set(x,CURSOR_MODE_LOCKED)
#define mcursor_normal(x) mcursor_set(x,CURSOR_MODE_NORMAL)
#define mcursor_waiting(x) mcursor_set(x,CURSOR_MODE_WAITING)


//LIB3270_INTERNAL void mcursor_locked(H3270 *session);
//LIB3270_INTERNAL void mcursor_normal(H3270 *session);
//LIB3270_INTERNAL void mcursor_waiting(H3270 *session);

LIB3270_INTERNAL void notify_toggle_changed(H3270 *session, LIB3270_TOGGLE ix, unsigned char value, LIB3270_TOGGLE_TYPE reason);
LIB3270_INTERNAL void set_viewsize(H3270 *session, int rows, int cols);

LIB3270_INTERNAL Boolean escaped;


LIB3270_INTERNAL void screen_title(char *text);

#endif // SCREENC_H_INCLUDED
