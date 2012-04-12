/*
 * Copyright 2007 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 *
 * x3270, c3270, wc3270, s3270, tcl3270, pr3287 and wpr3287 are distributed in
 * the hope that they will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the file LICENSE for more details.
 */

/*
 *	resolverc.h
 *		Hostname resolution.
 */

LIB3270_INTERNAL int resolve_host_and_port(H3270 *session, const char *host, char *portname, unsigned short *pport,struct sockaddr *sa, socklen_t *sa_len, char *errmsg, int em_size);


