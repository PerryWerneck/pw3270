/*
 * Copyright 1995, 1999, 2001, 2002, 2003, 2005, 2006 by Paul Mattes.
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
 *	trace_dsc.h
 *		Global declarations for trace_ds.c.
 */

#if defined(X3270_TRACE) /*[*/

LIB3270_INTERNAL Boolean trace_skipping;

const char *rcba(int baddr);
void toggle_dsTrace(H3270 *h, struct toggle *t, LIB3270_TOGGLE_TYPE tt);
void toggle_eventTrace(H3270 *h, struct toggle *t, LIB3270_TOGGLE_TYPE tt);
void toggle_screenTrace(H3270 *h, struct toggle *t, LIB3270_TOGGLE_TYPE tt);
void trace_ansi_disc(void);
void trace_char(char c);
void trace_ds(const char *fmt, ...) printflike(1, 2);
void trace_ds_nb(const char *fmt, ...) printflike(1, 2);
void trace_dsn(const char *fmt, ...) printflike(1, 2);
void trace_event(const char *fmt, ...) printflike(1, 2);
void trace_screen(void);
void trace_rollover_check(void);

#else /*][*/

#define rcba 0 &&
#if defined(__GNUC__) /*[*/
#define trace_ds(format, args...)
#define trace_dsn(format, args...)
#define trace_ds_nb(format, args...)
#define trace_event(format, args...)
#else /*][*/
#define trace_ds 0 &&
#define trace_ds_nb 0 &&
#define trace_dsn 0 &&
#define trace_event 0 &&
#define rcba 0 &&
#endif /*]*/

#endif /*]*/
