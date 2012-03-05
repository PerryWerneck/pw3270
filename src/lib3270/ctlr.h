/*
 * Copyright 1995, 1999, 2000, 2002, 2005 by Paul Mattes.
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
 *	ctlr.h
 *		External declarations for ctlr.c data structures.
 */

LIB3270_INTERNAL int			buffer_addr;	/**< buffer address */
// LIB3270_INTERNAL int			cursor_addr;	/**< cursor address */
LIB3270_INTERNAL struct ea	*ea_buf;			/**< 3270 device buffer */
//LIB3270_INTERNAL Boolean		formatted;		/**< contains at least one field? */
//LIB3270_INTERNAL Boolean		is_altbuffer;	/**< in alternate-buffer mode? */
