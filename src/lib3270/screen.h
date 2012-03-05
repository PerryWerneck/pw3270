/*
 * Copyright 1999, 2001 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 *
 * c3270, s3270 and tcl3270 are distributed in the hope that they will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the file LICENSE
 * for more details.
 */

/* Non-display version of screen.h */

#define SELECTED(baddr)	False
LIB3270_INTERNAL int *char_width, *char_height;
LIB3270_INTERNAL Boolean screen_has_changes;

// LIB3270_INTERNAL void screen_update(H3270 *session, int bstart, int bend);

LIB3270_INTERNAL void status_connecting(H3270 *session, Boolean on);
LIB3270_INTERNAL void status_resolving(H3270 *session, Boolean on);

