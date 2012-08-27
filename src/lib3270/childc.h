/*
 * Copyright 2001 by Paul Mattes.
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
 *	childc.h
 *		Global declarations for child.c.
 */

/*
#if defined(X3270_DISPLAY) || defined(C3270)
LIB3270_INTERNAL int fork_child(void);
LIB3270_INTERNAL void child_ignore_output(void);
#else
#define fork_child()	fork()
#define child_ignore_output()
#endif
*/

#error Deprecated
