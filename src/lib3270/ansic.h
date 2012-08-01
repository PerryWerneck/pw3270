/*
 * Copyright 1995, 1999, 2000 by Paul Mattes.
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
 *	ansic.h
 *		Global declarations for ansi.c.
 */

#if defined(X3270_ANSI) /*[*/

LIB3270_INTERNAL void ansi_process(H3270 *hSession, unsigned int c);
LIB3270_INTERNAL void ansi_send_clear(H3270 *hSession);
LIB3270_INTERNAL void ansi_send_down(H3270 *hSession);
LIB3270_INTERNAL void ansi_send_home(H3270 *hSession);
LIB3270_INTERNAL void ansi_send_left(H3270 *hSession);
LIB3270_INTERNAL void ansi_send_pa(H3270 *hSession, int nn);
LIB3270_INTERNAL void ansi_send_pf(H3270 *hSession, int nn);
LIB3270_INTERNAL void ansi_send_right(H3270 *hSession);
LIB3270_INTERNAL void ansi_send_up(H3270 *hSession);
LIB3270_INTERNAL void ansi_in3270(H3270 *session, int in3270, void *dunno);

LIB3270_INTERNAL void toggle_lineWrap(H3270 *session, struct lib3270_toggle *t, LIB3270_TOGGLE_TYPE type);

#else /*][*/

#define ansi_process(n)
#define ansi_send_clear()
#define ansi_send_down()
#define ansi_send_home()
#define ansi_send_left()
#define ansi_send_pa(n)
#define ansi_send_pf(n)
#define ansi_send_right()
#define ansi_send_up()

#endif /*]*/
