/*
 * Copyright 1995, 1999, 2000, 2001, 2002, 2004, 2005 by Paul Mattes.
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
 *	kybdc.h
 *		Global declarations for kybd.c.
 */

#ifndef KYBDC_H_INCLUDED

#define KYBDC_H_INCLUDED

/* keyboard lock states */
// LIB3270_INTERNAL unsigned int kybdlock;
#define KL_OERR_MASK		0x000f
#define  KL_OERR_PROTECTED	1
#define  KL_OERR_NUMERIC	2
#define  KL_OERR_OVERFLOW	3
#define  KL_OERR_DBCS		4
#define	KL_NOT_CONNECTED	0x0010
#define	KL_AWAITING_FIRST	0x0020
#define	KL_OIA_TWAIT		0x0040
#define	KL_OIA_LOCKED		0x0080
#define	KL_DEFERRED_UNLOCK	0x0100
#define KL_ENTER_INHIBIT	0x0200
#define KL_SCROLLED		0x0400
#define KL_OIA_MINUS		0x0800


/* other functions */
LIB3270_INTERNAL void add_xk(KeySym key, KeySym assoc);
LIB3270_INTERNAL void clear_xks(void);
LIB3270_INTERNAL void do_reset(H3270 *session, Boolean explicit);
LIB3270_INTERNAL void hex_input(char *s);
LIB3270_INTERNAL void kybdlock_clr(H3270 *session, unsigned int bits, const char *cause);
LIB3270_INTERNAL void kybd_inhibit(H3270 *session, Boolean inhibit);
LIB3270_INTERNAL void kybd_init(void);
LIB3270_INTERNAL int kybd_prime(void);
LIB3270_INTERNAL void kybd_scroll_lock(Boolean lock);
LIB3270_INTERNAL Boolean run_ta(void);
LIB3270_INTERNAL int state_from_keymap(char keymap[32]);
LIB3270_INTERNAL void kybd_connect(H3270 *session, int connected, void *dunno);
LIB3270_INTERNAL void kybd_in3270(H3270 *session, int in3270 unused, void *dunno);


#endif /* KYBDC_H_INCLUDED */
