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
// extern unsigned char *obuf;
// extern unsigned char *obptr;

/* Spelled-out tty control character. */
struct ctl_char {
	const char *name;
	char value[3];
};

LIB3270_INTERNAL void net_abort(H3270 *hSession);
LIB3270_INTERNAL Boolean net_add_dummy_tn3270e(void);
LIB3270_INTERNAL void net_add_eor(unsigned char *buf, int len);
LIB3270_INTERNAL void net_break(H3270 *hSession);
LIB3270_INTERNAL void net_charmode(H3270 *hSession);
LIB3270_INTERNAL int net_connect(H3270 *session, const char *, char *, Boolean, Boolean *, Boolean *);
LIB3270_INTERNAL void net_disconnect(H3270 *session);
LIB3270_INTERNAL void net_exception(H3270 *session);
// LIB3270_INTERNAL void net_hexansi_out(unsigned char *buf, int len);
LIB3270_INTERNAL void net_input(H3270 *session);
LIB3270_INTERNAL void net_interrupt(H3270 *hSession);
// LIB3270_INTERNAL void net_linemode(void);
// LIB3270_INTERNAL struct ctl_char *net_linemode_chars(void);
LIB3270_INTERNAL void net_output(H3270 *hSession);
LIB3270_INTERNAL const char *net_query_bind_plu_name(void);
LIB3270_INTERNAL const char *net_query_connection_state(void);
LIB3270_INTERNAL const char *net_query_host(void);
LIB3270_INTERNAL const char *net_query_lu_name(void);
LIB3270_INTERNAL void net_sendc(char c);
LIB3270_INTERNAL void net_sends(const char *s);
LIB3270_INTERNAL void net_send_erase(void);
LIB3270_INTERNAL void net_send_kill(void);
LIB3270_INTERNAL void net_send_werase(void);
// LIB3270_INTERNAL Boolean net_snap_options(void);
LIB3270_INTERNAL void space3270out(H3270 *hSession, int n);
// LIB3270_INTERNAL const char *tn3270e_current_opts(void);
// LIB3270_INTERNAL char *net_proxy_type(void);
//LIB3270_INTERNAL char *net_proxy_host(void);
// LIB3270_INTERNAL char *net_proxy_port(void);

#if defined(X3270_TRACE)
	LIB3270_INTERNAL void trace_netdata(H3270 *hSession, char direction, unsigned const char *buf, int len);
#else
	#define trace_netdata(direction, buf, len) /* */
#endif // X3270_TRACE


LIB3270_INTERNAL int net_getsockname(const H3270 *h3270, void *buf, int *len);
