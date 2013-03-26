/*
 * Copyright 1999, 2000, 2002 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 *
 * c3270 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the file LICENSE for more details.
 */

/* c3270 verson of statusc.h */

#include "api.h"

LIB3270_INTERNAL void 		status_compose(int on, unsigned char c, enum keytype keytype);
LIB3270_INTERNAL void 		status_ctlr_done(H3270 *session);

LIB3270_INTERNAL void 		status_timing(H3270 *session, struct timeval *t0, struct timeval *t1);
LIB3270_INTERNAL void 		status_untiming(H3270 *session);

LIB3270_INTERNAL void 		status_lu(H3270 *session, const char *);
LIB3270_INTERNAL void 		status_oerr(H3270 *session, int error_type);
LIB3270_INTERNAL void 		status_reset(H3270 *session);
LIB3270_INTERNAL void 		status_twait(H3270 *session);




LIB3270_INTERNAL void 		status_changed(H3270 *session, LIB3270_STATUS id);

LIB3270_INTERNAL void 		set_status(H3270 *session, OIA_FLAG id, Boolean on);


#define status_typeahead(h,on)	set_status(h,OIA_FLAG_TYPEAHEAD,on)

