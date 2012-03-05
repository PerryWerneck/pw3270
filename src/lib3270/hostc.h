/*
 * Copyright 1995, 1996, 1999, 2000, 2001, 2002, 2003, 2005 by Paul Mattes.
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
 *	hostc.h
 *		Global declarations for host.c.
 */

#include <lib3270/api.h>

/*
	struct host {
		char *name;
		char **parents;
		char *hostname;
		enum { PRIMARY, ALIAS, RECENT } entry_type;
		char *loginstring;
		time_t connect_time;
		struct host *prev, *next;
	};
	extern struct host *hosts;
*/

	#define st_changed(tx,mode) lib3270_st_changed(NULL,tx,mode)

	LIB3270_INTERNAL void lib3270_st_changed(H3270 *h, int tx, int mode);
//	LIB3270_INTERNAL void hostfile_init(void);
	LIB3270_INTERNAL void host_connected(H3270 *session);
	LIB3270_INTERNAL void host_disconnected(H3270 *session);
	LIB3270_INTERNAL void host_in3270(H3270 *session, LIB3270_CSTATE);
	LIB3270_INTERNAL void host_disconnect(H3270 *h, int disable);

