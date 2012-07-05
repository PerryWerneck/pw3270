/*
 * Copyright 2007 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 *
 * wc3270 and wpr3287 are distributed in the hope that they will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the file LICENSE for more details.
 */

/*
 *	w3misc.c
 *		Miscellaneous Win32 functions.
 */

#if defined(_WIN32)
	LIB3270_INTERNAL const char *inet_ntop(int af, const void *src, char *dst,socklen_t cnt);
	LIB3270_INTERNAL const char *win32_strerror(int e);
#endif
