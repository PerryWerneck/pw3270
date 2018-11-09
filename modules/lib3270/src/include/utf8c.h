/*
 * Copyright 2007 by Paul Mattes.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * x3270, c3270, wc3270, s3270, tcl3270, pr3287 and wpr3287 are distributed in
 * the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
 * implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the file LICENSE for more details.
 */

/*
 *	utf8c.h
 *		3270 Terminal Emulator
 *		UTF-8 conversions
 */

enum ulfail {
    	ULFAIL_NOUTF8,		/* not using UTF-8 */
	ULFAIL_INCOMPLETE,	/* incomplete sequence */
	ULFAIL_INVALID		/* invalid sequence */
};

// LIB3270_INTERNAL char *locale_codeset;

// LIB3270_INTERNAL void set_codeset(char *codeset_name);
// LIB3270_INTERNAL Boolean utf8_set_display_charsets(char *cslist, char *csname);
 LIB3270_INTERNAL char *utf8_expand(unsigned char c);
 LIB3270_INTERNAL unsigned char utf8_lookup(char *mbs, enum ulfail *fail, int *consumed);
