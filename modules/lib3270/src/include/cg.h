/*
 * Copyright 1993, 1994, 1995, 1999, 2000 by Paul Mattes.
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
 *	cg.h
 *
 *		Character encoding for the 3270 character generator font,
 *		using the same suffixes as Latin-1 XK_xxx keysyms.
 *
 *		Charaters that represent unique EBCDIC or status line codes
 *		are noted with comments.
 */

#define CG_null		0x00	/* EBCDIC 00 */
#define CG_nobreakspace	0x01
#define CG_ff		0x02	/* EBCDIC 0C */
#define CG_cr		0x03	/* EBCDIC 0D */
#define CG_nl		0x04	/* EBCDIC 15 */
#define CG_em		0x05	/* EBCDIC 19 */
#define CG_eightones	0x06	/* EBCDIC FF */
#define CG_hyphen	0x07
#define CG_greater	0x08
#define CG_less		0x09
#define CG_bracketleft	0x0a
#define CG_bracketright	0x0b
#define CG_parenleft	0x0c
#define CG_parenright	0x0d
#define CG_braceleft	0x0e
#define CG_braceright	0x0f
#define CG_space	0x10
#define CG_equal	0x11
#define CG_apostrophe	0x12
#define CG_quotedbl	0x13
#define CG_slash	0x14
#define CG_backslash	0x15
#define CG_bar		0x16
#define CG_brokenbar	0x17
#define CG_question	0x18
#define CG_exclam	0x19
#define CG_dollar	0x1a
#define CG_cent		0x1b
#define CG_sterling	0x1c
#define CG_yen		0x1d
#define CG_paragraph	0x1e
#define CG_currency	0x1f
#define CG_0		0x20
#define CG_1		0x21
#define CG_2		0x22
#define CG_3		0x23
#define CG_4		0x24
#define CG_5		0x25
#define CG_6		0x26
#define CG_7		0x27
#define CG_8		0x28
#define CG_9		0x29
#define CG_ssharp	0x2a
#define CG_section	0x2b
#define CG_numbersign	0x2c
#define CG_at		0x2d
#define CG_percent	0x2e
#define CG_underscore	0x2f
#define CG_ampersand	0x30
#define CG_minus	0x31
#define CG_period	0x32
#define CG_comma	0x33
#define CG_colon	0x34
#define CG_plus		0x35
#define CG_notsign	0x36
#define CG_macron	0x37
#define CG_degree	0x38
#define CG_periodcentered	0x39
#define CG_asciicircum	0x3a
#define CG_asciitilde	0x3b
#define CG_diaeresis	0x3c
#define CG_grave	0x3d
#define CG_acute	0x3e
#define CG_cedilla	0x3f
#define CG_agrave	0x40
#define CG_egrave	0x41
#define CG_igrave	0x42
#define CG_ograve	0x43
#define CG_ugrave	0x44
#define CG_atilde	0x45
#define CG_otilde	0x46
#define CG_ydiaeresis	0x47
#define CG_Yacute	0x48
#define CG_yacute	0x49
#define CG_eacute	0x4a
#define CG_onequarter	0x4b
#define CG_onehalf	0x4c
#define CG_threequarters	0x4d
#define CG_udiaeresis	0x4e
#define CG_udiaeresis	0x4e
#define CG_ccedilla	0x4f
#define CG_adiaeresis	0x50
#define CG_ediaeresis	0x51
#define CG_idiaeresis	0x52
#define CG_odiaeresis	0x53
#define CG_mu		0x54
#define CG_acircumflex	0x55
#define CG_ecircumflex	0x56
#define CG_icircumflex	0x57
#define CG_ocircumflex	0x58
#define CG_ucircumflex	0x59
#define CG_aacute	0x5a
#define CG_multiply	0x5b
#define CG_iacute	0x5c
#define CG_oacute	0x5d
#define CG_uacute	0x5e
#define CG_ntilde	0x5f
#define CG_Agrave	0x60
#define CG_Egrave	0x61
#define CG_Igrave	0x62
#define CG_Ograve	0x63
#define CG_Ugrave	0x64
#define CG_Atilde	0x65
#define CG_Otilde	0x66
#define CG_onesuperior	0x67
#define CG_twosuperior	0x68
#define CG_threesuperior	0x69
#define CG_ordfeminine	0x6a
#define CG_masculine	0x6b
#define CG_guillemotleft	0x6c
#define CG_guillemotright	0x6d
#define CG_exclamdown	0x6e
#define CG_questiondown	0x6f
#define CG_Adiaeresis	0x70
#define CG_Ediaeresis	0x71
#define CG_Idiaeresis	0x72
#define CG_Odiaeresis	0x73
#define CG_Udiaeresis	0x74
#define CG_Acircumflex	0x75
#define CG_Ecircumflex	0x76
#define CG_Icircumflex	0x77
#define CG_Ocircumflex	0x78
#define CG_Ucircumflex	0x79
#define CG_Aacute	0x7a
#define CG_Eacute	0x7b
#define CG_Iacute	0x7c
#define CG_Oacute	0x7d
#define CG_Uacute	0x7e
#define CG_Ntilde	0x7f
#define CG_a		0x80
#define CG_b		0x81
#define CG_c		0x82
#define CG_d		0x83
#define CG_e		0x84
#define CG_f		0x85
#define CG_g		0x86
#define CG_h		0x87
#define CG_i		0x88
#define CG_j		0x89
#define CG_k		0x8a
#define CG_l		0x8b
#define CG_m		0x8c
#define CG_n		0x8d
#define CG_o		0x8e
#define CG_p		0x8f
#define CG_q		0x90
#define CG_r		0x91
#define CG_s		0x92
#define CG_t		0x93
#define CG_u		0x94
#define CG_v		0x95
#define CG_w		0x96
#define CG_x		0x97
#define CG_y		0x98
#define CG_z		0x99
#define CG_ae		0x9a
#define CG_oslash	0x9b
#define CG_aring	0x9c
#define CG_division	0x9d
#define CG_fm		0x9e	/* EBCDIC 1E */
#define CG_dup		0x9f	/* EBCDIC 1C */
#define CG_A		0xa0
#define CG_B		0xa1
#define CG_C		0xa2
#define CG_D		0xa3
#define CG_E		0xa4
#define CG_F		0xa5
#define CG_G		0xa6
#define CG_H		0xa7
#define CG_I		0xa8
#define CG_J		0xa9
#define CG_K		0xaa
#define CG_L		0xab
#define CG_M		0xac
#define CG_N		0xad
#define CG_O		0xae
#define CG_P		0xaf
#define CG_Q		0xb0
#define CG_R		0xb1
#define CG_S		0xb2
#define CG_T		0xb3
#define CG_U		0xb4
#define CG_V		0xb5
#define CG_W		0xb6
#define CG_X		0xb7
#define CG_Y		0xb8
#define CG_Z		0xb9
#define CG_AE		0xba
#define CG_Ooblique	0xbb
#define CG_Aring	0xbc
#define CG_Ccedilla	0xbd
#define CG_semicolon	0xbe
#define CG_asterisk	0xbf

    /* codes 0xc0 through 0xcf are for field attributes */

#define CG_copyright	0xd0
#define CG_registered	0xd1
#define CG_boxA		0xd2	/* status boxed A */
#define CG_insert	0xd3	/* status insert mode indicator */
#define CG_boxB		0xd4	/* status boxed B */
#define CG_box6		0xd5	/* status boxed 6 */
#define CG_plusminus	0xd6
#define CG_ETH		0xd7
#define CG_rightarrow	0xd8
#define CG_THORN	0xd9
#define CG_upshift	0xda	/* status upshift indicator */
#define CG_human	0xdb	/* status illegal position indicator */
#define CG_underB	0xdc	/* status underlined B */
#define CG_downshift	0xdd	/* status downshift indicator */
#define CG_boxquestion	0xde	/* status boxed question mark */
#define CG_boxsolid	0xdf	/* status solid block */

    /* codes 0xe0 through 0xef are for field attributes */

#define CG_badcommhi	0xf0	/* status bad communication indicator */
#define CG_commhi	0xf1	/* status communication indicator */
#define CG_commjag	0xf2	/* status communication indicator */
#define CG_commlo	0xf3	/* status communication indicator */
#define CG_clockleft	0xf4	/* status wait symbol */
#define CG_clockright	0xf5	/* status wait symbol */
#define CG_lock		0xf6	/* status keyboard lock X symbol */
#define CG_eth		0xf7
#define CG_leftarrow	0xf8
#define CG_thorn	0xf9
#define CG_keyleft	0xfa	/* status key lock indicator */
#define CG_keyright	0xfb	/* status key lock indicator */
#define CG_box4		0xfc	/* status boxed 4 */
#define CG_underA	0xfd	/* status underlined A */
#define CG_magcard	0xfe	/* status magnetic card indicator */
#define CG_boxhuman	0xff	/* status boxed position indicator */
