/*
 * Copyright 2007 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 *
 * x3270, c3270, wc3270, s3270 and tcl3270 are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * file LICENSE for more details.
 */

/*
 *	proxyc.h
 *		Declarations for proxy.c.
 */

LIB3270_INTERNAL int 	  proxy_setup(H3270 *session, char **phost, char **pport);
LIB3270_INTERNAL int	  proxy_negotiate(int type, int fd, char *host, unsigned short port);
LIB3270_INTERNAL char	* proxy_type_name(int type);
