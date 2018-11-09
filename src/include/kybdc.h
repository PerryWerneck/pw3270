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
	typedef enum lib3270_kl_state
	{
		LIB3270_KL_OERR_MASK		= 0x000f,
		LIB3270_KL_OERR_PROTECTED	= 0x0001,
		LIB3270_KL_OERR_NUMERIC		= 0x0002,
		LIB3270_KL_OERR_OVERFLOW	= 0x0003,
		LIB3270_KL_OERR_DBCS		= 0x0004,
		LIB3270_KL_NOT_CONNECTED	= 0x0010,
		LIB3270_KL_AWAITING_FIRST	= 0x0020,
		LIB3270_KL_OIA_TWAIT		= 0x0040,
		LIB3270_KL_OIA_LOCKED		= 0x0080,
		LIB3270_KL_DEFERRED_UNLOCK	= 0x0100,
		LIB3270_KL_ENTER_INHIBIT	= 0x0200,
		LIB3270_KL_SCROLLED			= 0x0400,
		LIB3270_KL_OIA_MINUS		= 0x0800

	} LIB3270_KL_STATE;

	#define KL_OERR_MASK		LIB3270_KL_OERR_MASK
	#define KL_OERR_PROTECTED	LIB3270_KL_OERR_PROTECTED
	#define KL_OERR_NUMERIC		LIB3270_KL_OERR_NUMERIC
	#define KL_OERR_OVERFLOW	LIB3270_KL_OERR_OVERFLOW
	#define KL_OERR_DBCS		LIB3270_KL_OERR_DBCS
	#define	KL_NOT_CONNECTED	LIB3270_KL_NOT_CONNECTED
	#define	KL_AWAITING_FIRST	LIB3270_KL_AWAITING_FIRST
	#define	KL_OIA_TWAIT		LIB3270_KL_OIA_TWAIT
	#define	KL_OIA_LOCKED		LIB3270_KL_OIA_LOCKED
	#define	KL_DEFERRED_UNLOCK	LIB3270_KL_DEFERRED_UNLOCK
	#define KL_ENTER_INHIBIT	LIB3270_KL_ENTER_INHIBIT
	#define KL_SCROLLED			LIB3270_KL_SCROLLED
	#define KL_OIA_MINUS		LIB3270_KL_OIA_MINUS



	/* other functions */
	LIB3270_INTERNAL void add_xk(KeySym key, KeySym assoc);
	LIB3270_INTERNAL void clear_xks(void);
	LIB3270_INTERNAL void do_reset(H3270 *session, Boolean explicit);
//	LIB3270_INTERNAL void hex_input(char *s);

//	#define kybdlock_clr(session, bits, cause) lib3270_kybdlock_clear(session, bits)
	LIB3270_INTERNAL void lib3270_kybdlock_clear(H3270 *hSession, LIB3270_KL_STATE bits);


	LIB3270_INTERNAL void kybd_inhibit(H3270 *session, Boolean inhibit);
//	LIB3270_INTERNAL void kybd_init(void);
	LIB3270_INTERNAL int kybd_prime(H3270 *hSession);
	LIB3270_INTERNAL void kybd_scroll_lock(Boolean lock);
	LIB3270_INTERNAL int	run_ta(H3270 *hSession);
//	LIB3270_INTERNAL int state_from_keymap(char keymap[32]);
	LIB3270_INTERNAL void kybd_connect(H3270 *session, int connected, void *dunno);
	LIB3270_INTERNAL void kybd_in3270(H3270 *session, int in3270 unused, void *dunno);


#endif /* KYBDC_H_INCLUDED */
