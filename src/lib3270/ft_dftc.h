/*
 * Modifications Copyright 1996, 1999, 2000, 2004 by Paul Mattes.
 * Copyright October 1995 by Dick Altenbern.
 * Based in part on code Copyright 1993, 1994, 1995 by Paul Mattes.
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

LIB3270_INTERNAL void ft_dft_data(H3270 *hSession, unsigned char *data, int length);
LIB3270_INTERNAL void dft_read_modified(H3270 *hSession);
LIB3270_INTERNAL void set_dft_buffersize(H3270 *hSession);
