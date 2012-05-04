/*
 * Modifications Copyright 1996, 1999, 2000, 2001, 2002, 2003 by Paul Mattes.
 * Copyright October 1995 by Dick Altenbern
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
 *	ftc.h
 *		Global declarations for ft.c.
 */

#if defined(X3270_FT) /*[*/

#include <lib3270/filetransfer.h>

LIB3270_INTERNAL Boolean ascii_flag;
LIB3270_INTERNAL Boolean cr_flag;
LIB3270_INTERNAL unsigned long ft_length;

LIB3270_INTERNAL H3270FT * ftsession;

// LIB3270_INTERNAL FILE *ft_local_file;
// extern char *ft_local_filename;

LIB3270_INTERNAL Boolean ft_last_cr;
LIB3270_INTERNAL Boolean remap_flag;

LIB3270_INTERNAL void ft_init(H3270FT *h);
LIB3270_INTERNAL void ft_aborting(H3270FT *h);
LIB3270_INTERNAL void ft_complete(H3270FT *h, const char *errmsg);
LIB3270_INTERNAL void ft_running(H3270FT *h, Boolean is_cut);
LIB3270_INTERNAL void ft_update_length(H3270FT *h);

#endif /*]*/
