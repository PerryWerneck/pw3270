/*
 * Copyright 1995, 1999, 2000, 2002, 2001, 2002, 2005, 2006 by Paul Mattes.
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
 *	tablesc.h
 *		Global declarations for tables.c.
 */
LIB3270_INTERNAL void initialize_tables(H3270 *hSession);
LIB3270_INTERNAL unsigned short ebc2cg[256];
LIB3270_INTERNAL unsigned short cg2ebc[256];
// LIB3270_INTERNAL unsigned short ebc2asc[256];
LIB3270_INTERNAL unsigned short asc2ebc[256];
LIB3270_INTERNAL unsigned short ft2asc[256];
LIB3270_INTERNAL unsigned short asc2ft[256];

#ifdef EXTENDED_TABLES
LIB3270_INTERNAL unsigned short ebc2asc7[256];
LIB3270_INTERNAL const unsigned short ebc2asc70[256];
LIB3270_INTERNAL const unsigned short ge2asc[256];
#endif // EXTENDED_TABLES

LIB3270_INTERNAL const unsigned short asc2cg[256];
LIB3270_INTERNAL const unsigned short cg2asc[256];
LIB3270_INTERNAL const unsigned short ebc2cg0[256];
LIB3270_INTERNAL const unsigned short cg2ebc0[256];
LIB3270_INTERNAL const unsigned short ebc2asc0[256];
LIB3270_INTERNAL const unsigned short asc2ebc0[256];
LIB3270_INTERNAL const unsigned short asc2uc[256];
LIB3270_INTERNAL const unsigned short ebc2uc[256];
LIB3270_INTERNAL const unsigned short ft2asc0[256];
LIB3270_INTERNAL const unsigned short asc2ft0[256];
