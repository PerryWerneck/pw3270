/*
 * Modifications Copyright 1993, 1994, 1995, 1996, 1999, 2000, 2001, 2002,
 *  2004, 2005 by Paul Mattes.
 * Original X11 Port Copyright 1990 by Jeff Sparkes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 *
 * Copyright 1989 by Georgia Tech Research Corporation, Atlanta, GA 30332.
 *  All Rights Reserved.  GTRC hereby grants public use of this software.
 *  Derivative works based on this software must incorporate this copyright
 *  notice.
 *
 * x3270, c3270, s3270 and tcl3270 are distributed in the hope that they will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the file LICENSE
 * for more details.
 */

/*
 *	charsetc.h
 *		Global declarations for charset.c
 */

LIB3270_INTERNAL Boolean charset_changed;
LIB3270_INTERNAL unsigned long cgcsgid;
#if defined(X3270_DBCS) /*[*/
LIB3270_INTERNAL unsigned long cgcsgid_dbcs;
LIB3270_INTERNAL char *converter_names;
LIB3270_INTERNAL char *encoding;
#endif /*]*/
LIB3270_INTERNAL char *default_display_charset;

enum cs_result { CS_OKAY, CS_NOTFOUND, CS_BAD, CS_PREREQ, CS_ILLEGAL };

LIB3270_INTERNAL enum cs_result charset_init(char *csname);
LIB3270_INTERNAL char *get_charset_name(void);
LIB3270_INTERNAL void set_display_charset(char *dcs);

