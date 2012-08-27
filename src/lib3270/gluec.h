/*
 * Copyright 2000 by Paul Mattes.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * c3270, s3270 and tcl3270 are distributed in the hope that they will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the file LICENSE
 * for more details.
 */

/*
 *	gluec.h
 *		Declarations for glue.c and XtGlue.c
 */

/* glue.c */
// extern int parse_command_line(int argc, const char **argv, const char **cl_hostname);
// extern int parse_program_parameters(int argc, const char **argv);
// LIB3270_INTERNAL void parse_xrm(const char *arg, const char *where);
// LIB3270_INTERNAL void notify_ssl_error(H3270 *session, const char *title,  const char *msg, const char *state, const char *alert);

/* XtGlue.c */
// LIB3270_INTERNAL void (*Warning_redirect)(const char *);

// #if !defined(_WIN32)
// LIB3270_INTERNAL int select_setup(int *nfds, fd_set *readfds, fd_set *writefds,fd_set *exceptfds, struct timeval **timeout, struct timeval *timebuf);
// #endif

#error Deprecated
