/*
 * Modifications Copyright 1993, 1994, 1995, 1999, 2000, 2002, 2003, 2004,
 *   2005 by Paul Mattes.
 * RPQNAMES modifications Copyright 2004 by Don Russell.
 * Original X11 Port Copyright 1990 by Jeff Sparkes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 *
 * Copyright 1989 by Georgia Tech Research Corporation, Atlanta, GA 30332.
 *  All Rights Reserved.  GTRC hereby grants public use of this software.
 *  Derivative works based on this software must incorporate this copyright
 *  notice.
 *
 * x3270, c3270, s3270, tcl3270 and pr3287 are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * file LICENSE for more details.
 */

/*
 *	3270ds.h
 *
 *		Header file for the 3270 Data Stream Protocol.
 */

/* 3270 commands */
#define CMD_W		0x01	/* write */
#define CMD_RB		0x02	/* read buffer */
#define CMD_NOP		0x03	/* no-op */
#define CMD_EW		0x05	/* erase/write */
#define CMD_RM		0x06	/* read modified */
#define CMD_EWA		0x0d	/* erase/write alternate */
#define CMD_RMA		0x0e	/* read modified all */
#define CMD_EAU		0x0f	/* erase all unprotected */
#define CMD_WSF		0x11	/* write structured field */

/* SNA 3270 commands */
#define SNA_CMD_RMA	0x6e	/* read modified all */
#define SNA_CMD_EAU	0x6f	/* erase all unprotected */
#define SNA_CMD_EWA	0x7e	/* erase/write alternate */
#define SNA_CMD_W	0xf1	/* write */
#define SNA_CMD_RB	0xf2	/* read buffer */
#define SNA_CMD_WSF	0xf3	/* write structured field */
#define SNA_CMD_EW	0xf5	/* erase/write */
#define SNA_CMD_RM	0xf6	/* read modified */

/* 3270 orders */
#define ORDER_PT	0x05	/* program tab */
#define ORDER_GE	0x08	/* graphic escape */
#define ORDER_SBA	0x11	/* set buffer address */
#define ORDER_EUA	0x12	/* erase unprotected to address */
#define ORDER_IC	0x13	/* insert cursor */
#define ORDER_SF	0x1d	/* start field */
#define ORDER_SA	0x28	/* set attribute */
#define ORDER_SFE	0x29	/* start field extended */
#define ORDER_YALE	0x2b	/* Yale sub command */
#define ORDER_MF	0x2c	/* modify field */
#define ORDER_RA	0x3c	/* repeat to address */

#define FCORDER_NULL	0x00	/* format control: null */
#define FCORDER_FF	0x0c	/*		   form feed */
#define FCORDER_CR	0x0d	/*		   carriage return */
#define FCORDER_SO	0x0e	/*                 shift out (DBCS subfield) */
#define FCORDER_SI	0x0f	/*                 shift in (DBCS end) */
#define FCORDER_NL	0x15	/*		   new line */
#define FCORDER_EM	0x19	/*		   end of medium */
#define FCORDER_DUP	0x1c	/*		   duplicate */
#define FCORDER_FM	0x1e	/*		   field mark */
#define FCORDER_SUB	0x3f	/*		   substitute */
#define FCORDER_EO	0xff	/*		   eight ones */

/* SCS control code, some overlap orders */
#define SCS_BS      	0x16	/* Back Space  */
#define SCS_BEL		0x2f	/* Bell Function */
#define SCS_CR      	0x0d	/* Carriage Return */
#define SCS_ENP		0x14	/* Enable Presentation */
#define SCS_FF		0x0c	/* Forms Feed */
#define SCS_GE		0x08	/* Graphic Escape */
#define SCS_HT		0x05	/* Horizontal Tab */
#define SCS_INP		0x24	/* Inhibit Presentation */
#define SCS_IRS		0x1e	/* Interchange-Record Separator */
#define SCS_LF		0x25	/* Line Feed */
#define SCS_NL		0x15	/* New Line */
#define SCS_SA		0x28	/* Set Attribute: */
#define  SCS_SA_RESET	0x00	/*  Reset all */
#define  SCS_SA_HIGHLIGHT 0x41	/*  Highlighting */
#define  SCS_SA_CS	0x42	/*  Character set */
#define  SCS_SA_GRID	0xc2	/*  Grid */
#define SCS_SET		0x2b	/* Set: */
#define  SCS_SHF	0xc1	/*  Horizontal format */
#define  SCS_SLD	0xc6	/*  Line Density */
#define  SCS_SVF	0xc2	/*  Vertical Format */
#define SCS_SO		0x0e	/* Shift out (DBCS subfield start) */
#define SCS_SI		0x0f	/* Shift in (DBCS subfield end) */
#define SCS_TRN		0x35	/* Transparent */
#define SCS_VCS		0x04	/* Vertical Channel Select */
#define SCS_VT		0x0b	/* Vertical Tab */

/* Structured fields */
#define SF_READ_PART	0x01	/* read partition */
#define  SF_RP_QUERY	0x02	/*  query */
#define  SF_RP_QLIST	0x03	/*  query list */
#define   SF_RPQ_LIST	0x00	/*   QCODE list */
#define   SF_RPQ_EQUIV	0x40	/*   equivalent+ QCODE list */
#define   SF_RPQ_ALL	0x80	/*   all */
#define SF_ERASE_RESET	0x03	/* erase/reset */
#define  SF_ER_DEFAULT	0x00	/*  default */
#define  SF_ER_ALT	0x80	/*  alternate */
#define SF_SET_REPLY_MODE 0x09	/* set reply mode */
#define  SF_SRM_FIELD	0x00	/*  field */
#define  SF_SRM_XFIELD	0x01	/*  extended field */
#define  SF_SRM_CHAR	0x02	/*  character */
#define SF_CREATE_PART	0x0c	/* create partition */
#define  CPFLAG_PROT	0x40	/*  protected flag */
#define  CPFLAG_COPY_PS	0x20	/*  local copy to presentation space */
#define  CPFLAG_BASE	0x07	/*  base character set index */
#define SF_OUTBOUND_DS	0x40	/* outbound 3270 DS */
#define SF_TRANSFER_DATA 0xd0   /* file transfer open request */

/* Query replies */
#define QR_SUMMARY	0x80	/* summary */
#define QR_USABLE_AREA	0x81	/* usable area */
#define QR_ALPHA_PART	0x84	/* alphanumeric partitions */
#define QR_CHARSETS	0x85	/* character sets */
#define QR_COLOR	0x86	/* color */
#define QR_HIGHLIGHTING	0x87	/* highlighting */
#define QR_REPLY_MODES	0x88	/* reply modes */
#define QR_DBCS_ASIA	0x91	/* DBCS-Asia */
#define QR_PC3270	0x93    /* PC3270 */
#define QR_DDM    	0x95    /* distributed data management */
#define QR_RPQNAMES	0xa1	/* RPQ names */
#define QR_IMP_PART	0xa6	/* implicit partition */
#define QR_NULL		0xff	/* null */

#define BA_TO_ROW(ba)		((ba) / h3270.cols)
#define BA_TO_COL(ba)		((ba) % h3270.cols)
#define ROWCOL_TO_BA(r,c)	(((r) * h3270.cols) + c)
#define INC_BA(ba)		{ (ba) = ((ba) + 1) % (h3270.cols * h3270.rows); }
#define DEC_BA(ba)		{ (ba) = (ba) ? (ba - 1) : ((h3270.cols*h3270.rows) - 1); }

/* Field attributes. */
#define FA_PRINTABLE	0xc0	/* these make the character "printable" */
#define FA_PROTECT	0x20	/* unprotected (0) / protected (1) */
#define FA_NUMERIC	0x10	/* alphanumeric (0) /numeric (1) */
#define FA_INTENSITY	0x0c	/* display/selector pen detectable: */
#define FA_INT_NORM_NSEL 0x00	/*  00 normal, non-detect */
#define FA_INT_NORM_SEL	 0x04	/*  01 normal, detectable */
#define FA_INT_HIGH_SEL	 0x08	/*  10 intensified, detectable */
#define FA_INT_ZERO_NSEL 0x0c	/*  11 nondisplay, non-detect */
#define FA_RESERVED	0x02	/* must be 0 */
#define FA_MODIFY	0x01	/* modified (1) */

/* Bits in the field attribute that are stored. */
#define FA_MASK		(FA_PROTECT | FA_NUMERIC | FA_INTENSITY | FA_MODIFY)

/* Tests for various attribute properties. */
#define FA_IS_MODIFIED(c)	((c) & FA_MODIFY)
#define FA_IS_NUMERIC(c)	((c) & FA_NUMERIC)
#define FA_IS_PROTECTED(c)	((c) & FA_PROTECT)
#define FA_IS_SKIP(c)		(((c) & FA_PROTECT) && ((c) & FA_NUMERIC))

#define FA_IS_ZERO(c)					\
	(((c) & FA_INTENSITY) == FA_INT_ZERO_NSEL)
#define FA_IS_HIGH(c)					\
	(((c) & FA_INTENSITY) == FA_INT_HIGH_SEL)
#define FA_IS_NORMAL(c)					\
    (							\
	((c) & FA_INTENSITY) == FA_INT_NORM_NSEL	\
	||						\
	((c) & FA_INTENSITY) == FA_INT_NORM_SEL		\
    )
#define FA_IS_SELECTABLE(c)				\
    (							\
	((c) & FA_INTENSITY) == FA_INT_NORM_SEL		\
	||						\
	((c) & FA_INTENSITY) == FA_INT_HIGH_SEL		\
    )
#define FA_IS_INTENSE(c)				\
	((c & FA_INT_HIGH_SEL) == FA_INT_HIGH_SEL)

/* Extended attributes */
#define XA_ALL		0x00
#define XA_3270		0xc0
#define XA_VALIDATION	0xc1
#define  XAV_FILL	0x04
#define  XAV_ENTRY	0x02
#define  XAV_TRIGGER	0x01
#define XA_OUTLINING	0xc2
#define  XAO_UNDERLINE	0x01
#define  XAO_RIGHT	0x02
#define  XAO_OVERLINE	0x04
#define  XAO_LEFT	0x08
#define XA_HIGHLIGHTING	0x41
#define  XAH_DEFAULT	0x00
#define  XAH_NORMAL	0xf0
#define  XAH_BLINK	0xf1
#define  XAH_REVERSE	0xf2
#define  XAH_UNDERSCORE	0xf4
#define  XAH_INTENSIFY	0xf8
#define XA_FOREGROUND	0x42
#define  XAC_DEFAULT	0x00
#define XA_CHARSET	0x43
#define XA_BACKGROUND	0x45
#define XA_TRANSPARENCY	0x46
#define  XAT_DEFAULT	0x00
#define  XAT_OR		0xf0
#define  XAT_XOR	0xf1
#define  XAT_OPAQUE	0xff
#define XA_INPUT_CONTROL 0xfe
#define  XAI_DISABLED	0x00
#define  XAI_ENABLED	0x01

/* WCC definitions */
#define WCC_RESET(c)		((c) & 0x40)
#define WCC_START_PRINTER(c)	((c) & 0x08)
#define WCC_SOUND_ALARM(c)	((c) & 0x04)
#define WCC_KEYBOARD_RESTORE(c)	((c) & 0x02)
#define WCC_RESET_MDT(c)	((c) & 0x01)

/* AIDs */
#define AID_NO		0x60	/* no AID generated */
#define AID_QREPLY	0x61
#define AID_ENTER	0x7d
#define AID_PF1		0xf1
#define AID_PF2		0xf2
#define AID_PF3		0xf3
#define AID_PF4		0xf4
#define AID_PF5		0xf5
#define AID_PF6		0xf6
#define AID_PF7		0xf7
#define AID_PF8		0xf8
#define AID_PF9		0xf9
#define AID_PF10	0x7a
#define AID_PF11	0x7b
#define AID_PF12	0x7c
#define AID_PF13	0xc1
#define AID_PF14	0xc2
#define AID_PF15	0xc3
#define AID_PF16	0xc4
#define AID_PF17	0xc5
#define AID_PF18	0xc6
#define AID_PF19	0xc7
#define AID_PF20	0xc8
#define AID_PF21	0xc9
#define AID_PF22	0x4a
#define AID_PF23	0x4b
#define AID_PF24	0x4c
#define AID_OICR	0xe6
#define AID_MSR_MHS	0xe7
#define AID_SELECT	0x7e
#define AID_PA1		0x6c
#define AID_PA2		0x6e
#define AID_PA3		0x6b
#define AID_CLEAR	0x6d
#define AID_SYSREQ	0xf0

#define AID_SF		0x88
#define SFID_QREPLY	0x81

/* Colors */
#define COLOR_NEUTRAL_BLACK	0
#define COLOR_BLUE		1
#define COLOR_RED		2
#define COLOR_PINK		3
#define COLOR_GREEN		4
#define COLOR_TURQUOISE		5
#define COLOR_YELLOW		6
#define COLOR_NEUTRAL_WHITE	7
#define COLOR_BLACK		8
#define COLOR_DEEP_BLUE		9
#define COLOR_ORANGE		10
#define COLOR_PURPLE		11
#define COLOR_PALE_GREEN	12
#define COLOR_PALE_TURQUOISE	13
#define COLOR_GREY		14
#define COLOR_WHITE		15

/* Data stream manipulation macros. */
#define MASK32	0xff000000U
#define MASK24	0x00ff0000U
#define MASK16	0x0000ff00U
#define MASK08	0x000000ffU
#define MINUS1	0xffffffffU

#define SET16(ptr, val) { \
	*((ptr)++) = ((val) & MASK16) >> 8; \
	*((ptr)++) = ((val) & MASK08); \
}
#define GET16(val, ptr) { \
	(val) = *((ptr)+1); \
	(val) += *(ptr) << 8; \
}
#define SET32(ptr, val) { \
	*((ptr)++) = ((val) & MASK32) >> 24; \
	*((ptr)++) = ((val) & MASK24) >> 16; \
	*((ptr)++) = ((val) & MASK16) >> 8; \
	*((ptr)++) = ((val) & MASK08); \
}
#define HIGH8(s)        (((s) >> 8) & 0xff)
#define LOW8(s)         ((s) & 0xff)

/* Other EBCDIC control codes. */
#define EBC_null	0x00
#define EBC_ff		0x0c
#define EBC_cr		0x0d
#define EBC_so		0x0e
#define EBC_si		0x0f
#define EBC_nl		0x15
#define EBC_em		0x19
#define EBC_dup		0x1c
#define EBC_fm		0x1e
#define EBC_space	0x40
#define EBC_nobreakspace 0x41
#define EBC_period	0x4b
#define EBC_ampersand	0x50
#define EBC_underscore	0x6d
#define EBC_greater	0x6e
#define EBC_question	0x6f
#define EBC_Yacute	0xad
#define EBC_diaeresis	0xbd
#define EBC_minus	0xca
#define EBC_0		0xf0
#define EBC_9		0xf9
#define EBC_eo		0xff

/* BIND definitions. */
#define BIND_RU					LIB3270_BIND_RU
#define BIND_OFF_PLU_NAME_LEN	LIB3270_BIND_OFF_PLU_NAME_LEN
#define BIND_OFF_PLU_NAME		LIB3270_BIND_OFF_PLU_NAME
#define BIND_PLU_NAME_MAX		LIB3270_BIND_PLU_NAME_MAX
