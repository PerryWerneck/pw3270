/*
 * Copyright 1995, 1999, 2000, 2001, 2002, 2003, 2005 by Paul Mattes.
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
 *	actionsc.h
 *		Global declarations for actions.c.
 */

// extern enum iaction ia_cause;

extern int              actioncount;
// extern XtActionsRec     *actions;

// extern const char       *ia_name[];

// #if defined(X3270_TRACE) /*[*/
// extern void action_debug(XtActionProc action, XEvent *event, String *params, Cardinal *num_params);
// #else /*][*/
// #define action_debug(a, e, p, n)
// #endif /*]*/

extern void action_init(void);

// extern void action_internal(XtActionProc action, enum iaction cause, const char *parm1, const char *parm2);

#define action_name(x)  #x
// extern const char *action_name(XtActionProc action);
// extern int check_usage(XtActionProc action, Cardinal nargs, Cardinal nargs_min, Cardinal nargs_max);

extern Boolean event_is_meta(int state);


