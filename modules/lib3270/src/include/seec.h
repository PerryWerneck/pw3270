/*
 * Copyright 1993, 1994, 1995, 1999, 2000, 2001 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 *
 * x3270, c3270, s3270, tcl3270 and pr3287 are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * file LICENSE for more details.
 */

/*
 *	seec.h
 *		Declarations for see.c
 *
 */

#if defined(X3270_TRACE) /*[*/

LIB3270_INTERNAL const char *see_aid(unsigned char code);
LIB3270_INTERNAL const char *see_attr(unsigned char fa);
LIB3270_INTERNAL const char *see_color(unsigned char setting);
LIB3270_INTERNAL const char *see_ebc(unsigned char ch);
LIB3270_INTERNAL const char *see_efa(unsigned char efa, unsigned char value);
LIB3270_INTERNAL const char *see_efa_only(unsigned char efa);
LIB3270_INTERNAL const char *see_qcode(unsigned char id);
LIB3270_INTERNAL const char *unknown(unsigned char value);

#else /*][*/

#define see_aid 0 &&
#define see_attr 0 &&
#define see_color 0 &&
#define see_ebc 0 &&
#define see_efa 0 &&
#define see_efa_only 0 &&
#define see_qcode 0 &&
#define unknown 0 &&

#endif /*]*/
