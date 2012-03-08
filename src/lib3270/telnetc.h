/*
 * Copyright 1995, 1999, 2000, 2001, 2002, 2003, 2004, 2005,
 *   2007 by Paul Mattes.
 * RPQNAMES modifications Copyright 2004 by Don Russell.
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
 *	telnetc.h
 *		Global declarations for telnet.c.
 */

/* Output buffer. */
extern unsigned char *obuf;
extern unsigned char *obptr;

/* Spelled-out tty control character. */
struct ctl_char {
	const char *name;
	char value[3];
};

LIB3270_INTERNAL void net_abort(void);
LIB3270_INTERNAL Boolean net_add_dummy_tn3270e(void);
LIB3270_INTERNAL void net_add_eor(unsigned char *buf, int len);
LIB3270_INTERNAL void net_break(void);
LIB3270_INTERNAL void net_charmode(void);
LIB3270_INTERNAL int net_connect(const char *, char *, Boolean, Boolean *, Boolean *);
LIB3270_INTERNAL void net_disconnect(void);
LIB3270_INTERNAL void net_exception(H3270 *session);
LIB3270_INTERNAL void net_hexansi_out(unsigned char *buf, int len);
LIB3270_INTERNAL void net_input(H3270 *session);
LIB3270_INTERNAL void net_interrupt(void);
LIB3270_INTERNAL void net_linemode(void);
LIB3270_INTERNAL struct ctl_char *net_linemode_chars(void);
LIB3270_INTERNAL void net_output(void);
LIB3270_INTERNAL const char *net_query_bind_plu_name(void);
LIB3270_INTERNAL const char *net_query_connection_state(void);
LIB3270_INTERNAL const char *net_query_host(void);
LIB3270_INTERNAL const char *net_query_lu_name(void);
LIB3270_INTERNAL void net_sendc(char c);
LIB3270_INTERNAL void net_sends(const char *s);
LIB3270_INTERNAL void net_send_erase(void);
LIB3270_INTERNAL void net_send_kill(void);
LIB3270_INTERNAL void net_send_werase(void);
LIB3270_INTERNAL Boolean net_snap_options(void);
LIB3270_INTERNAL void space3270out(int n);
LIB3270_INTERNAL const char *tn3270e_current_opts(void);
LIB3270_INTERNAL void trace_netdata(char direction, unsigned const char *buf, int len);
LIB3270_INTERNAL char *net_proxy_type(void);
LIB3270_INTERNAL char *net_proxy_host(void);
LIB3270_INTERNAL char *net_proxy_port(void);

LIB3270_INTERNAL int net_getsockname(const H3270 *h3270, void *buf, int *len);
