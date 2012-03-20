/*
 * Copyright 1995, 1996, 1999, 2000, 2002, 2003, 2005 by Paul Mattes.
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
 *	keypadc.h
 *		Global declarations for keypad.c.
 */

// LIB3270_INTERNAL Boolean keypad_changed;
/*
#if defined(X3270_KEYPAD)

LIB3270_INTERNAL enum kp_placement { kp_right, kp_left, kp_bottom, kp_integral, kp_inside_right } kp_placement;

LIB3270_INTERNAL void keypad_first_up(void);
LIB3270_INTERNAL Widget keypad_init(Widget container, Dimension voffset, Dimension screen_width, Boolean floating, Boolean vert);
LIB3270_INTERNAL void keypad_move(void);
LIB3270_INTERNAL void keypad_placement_init(void);
LIB3270_INTERNAL void keypad_popup_init(void);
LIB3270_INTERNAL Dimension keypad_qheight(void);
LIB3270_INTERNAL void keypad_set_keymap(void);
LIB3270_INTERNAL void keypad_set_temp_keymap(XtTranslations trans);
LIB3270_INTERNAL void keypad_shift(void);
LIB3270_INTERNAL Dimension min_keypad_width(void);

#else

#define keypad_qheight()	0
#define min_keypad_width()	0
#define keypad_first_up()
#define keypad_init(a, b, c, d, e)	0
#define keypad_move()
#define keypad_placement_init()
#define keypad_popup_init()
#define keypad_set_keymap()
#define keypad_set_temp_keymap(n)
#define keypad_shift()

#endif
*/
