/*
 * Copyright 2002, 2003, 2004, 2005 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 *
 * x3270 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the file LICENSE for more details.
 */

/*
 *	widec.h
 *		Global declarations for wide.c.
 */

#if defined(X3270_DBCS) /*[*/
#include <unicode/ucnv.h>

LIB3270_INTERNAL int wide_init(char *dbcs_converters, char *local_name);
LIB3270_INTERNAL void dbcs_to_display(unsigned char ebc1, unsigned char ebc2,
    unsigned char c[]);
LIB3270_INTERNAL void dbcs_to_unicode16(unsigned char ebc1, unsigned char ebc2,
    unsigned char c[]);

LIB3270_INTERNAL int dbcs_to_mb(unsigned char ebc1, unsigned char ebc2, char *mb);
LIB3270_INTERNAL int sbcs_to_mb(unsigned char ebc, char *mb);
LIB3270_INTERNAL int mb_to_unicode(char *mb, int mblen, UChar *u, int ulen,
    UErrorCode *err);
LIB3270_INTERNAL int dbcs_map8(UChar, unsigned char *);
LIB3270_INTERNAL int dbcs_map16(UChar, unsigned char *);
#endif /*]*/
