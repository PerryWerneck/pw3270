/*
 * Copyright 1995, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006 by Paul
 *    Mattes.
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
 *	utilc.h
 *		Global declarations for util.c.
 */

LIB3270_INTERNAL void add_resource(const char *name, const char *value);
LIB3270_INTERNAL char *ctl_see(int c);
LIB3270_INTERNAL char *do_subst(const char *s, Boolean do_vars, Boolean do_tilde);
LIB3270_INTERNAL void fcatv(FILE *f, char *s);
LIB3270_INTERNAL const char *get_message(const char *key);
LIB3270_INTERNAL const char *get_fresource(const char *fmt, ...) printflike(1, 2);
LIB3270_INTERNAL const char *get_resource(const char *name);
LIB3270_INTERNAL char *scatv(const char *s, char *buf, size_t len);
LIB3270_INTERNAL int split_dbcs_resource(const char *value, char sep, char **part1,
    char **part2);
LIB3270_INTERNAL int split_dresource(char **st, char **left, char **right);
LIB3270_INTERNAL int split_lresource(char **st, char **value);
LIB3270_INTERNAL char *strip_whitespace(const char *s);

LIB3270_INTERNAL char *xs_buffer(const char *fmt, ...) printflike(1, 2);
LIB3270_INTERNAL void xs_error(const char *fmt, ...) printflike(1, 2);
LIB3270_INTERNAL void xs_warning(const char *fmt, ...) printflike(1, 2);

#ifdef WIN32
	LIB3270_INTERNAL void * AddInput(HANDLE, H3270 *session, void (*fn)(H3270 *session));
	LIB3270_INTERNAL void * AddExcept(HANDLE, H3270 *session, void (*fn)(H3270 *session));
#else
	LIB3270_INTERNAL void * AddInput(int, H3270 *session, void (*fn)(H3270 *session));
	LIB3270_INTERNAL void * AddExcept(int, H3270 *session, void (*fn)(H3270 *session));
#endif // WIN32
LIB3270_INTERNAL void   RemoveInput(void *);
LIB3270_INTERNAL void * AddTimeOut(unsigned long msec, H3270 *session, void (*fn)(H3270 *session));
LIB3270_INTERNAL void RemoveTimeOut(void *cookie);

LIB3270_INTERNAL const char * KeysymToString(KeySym k);

// LIB3270_INTERNAL int read_resource_file(const char *filename, Boolean fatal);
LIB3270_INTERNAL Boolean split_hier(char *label, char **base, char ***parents);

typedef struct {
	char *buf;
	int alloc_len;
	int cur_len;
} rpf_t;

LIB3270_INTERNAL void rpf_init(rpf_t *r);
LIB3270_INTERNAL void rpf_reset(rpf_t *r);
LIB3270_INTERNAL void rpf(rpf_t *r, char *fmt, ...) printflike(2, 3);
LIB3270_INTERNAL void rpf_free(rpf_t *r);
