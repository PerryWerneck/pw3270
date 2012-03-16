/*
 * Copyright 2000, 2002 by Paul Mattes.
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

/*
 *	localdefs.h
 *		Local definitions for c3270.
 *
 *		This file contains definitions for environment-specific
 *		facilities, such as memory allocation, I/O registration,
 *		and timers.
 */

/* Identify ourselves. */
#define C3270	1

/* Conditional 80/132 mode switch support. */
#if defined(BROKEN_NEWTERM) /*[*/
#undef C3270_80_132
#else /*][*/
#define C3270_80_132 1
#endif /*]*/

/* These first definitions were cribbed from X11 -- but no X code is used. */
#define False 0
#define True 1
typedef void *XtPointer;
typedef void *Widget;
typedef void *XEvent;
typedef char Boolean;
typedef char *String;
typedef unsigned int Cardinal;
typedef unsigned long KeySym;
#define Bool int
typedef void (*XtActionProc)(
    Widget 		/* widget */,
    XEvent*		/* event */,
    String*		/* params */,
    Cardinal*		/* num_params */
);
typedef struct _XtActionsRec{
    String	 string;
    XtActionProc proc;
} XtActionsRec;
#define XtNumber(n)	(sizeof(n)/sizeof((n)[0]))
#define NoSymbol		0L

/* These are local functions with similar semantics to X functions. */

void * Malloc(size_t);
void   Free(void *);
void * Calloc(size_t, size_t);
void * Realloc(void *, size_t);

/**
 * Alloc/Realloc memory buffer.
 *
 * Allocate/reallocate an array.
 *
 * @param elsize	Element size.
 * @param nelem		Number of elements in the array.
 * @param ptr		Pointer to the actual array.
 *
 * @return ptr allocated with the new array size.
 *
 */
void * lib3270_calloc(size_t elsize, size_t nelem, void *ptr);

#define NewString(x) strdup(x)
//extern char *NewString(const char *);

