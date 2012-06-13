/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob o nome G3270.
 *
 * Copyright (C) <2008> <Banco do Brasil S.A.>
 *
 * Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
 * os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
 * Free Software Foundation.
 *
 * Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
 * GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
 * A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
 * obter mais detalhes.
 *
 * Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como ansi.c e possui 1902 linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */

/*
 *	ansi.c
 *		ANSI terminal emulation.
 */

#include "globals.h"

#if defined(X3270_ANSI) /*[*/

#if defined(X3270_DISPLAY) /*[*/
#include <X11/Shell.h>
#endif /*]*/

//#include "appres.h"
// #include "ctlr.h"
#if defined(X3270_DBCS) /*[*/
#include "3270ds.h"
#endif /*]*/

#include "ansic.h"
#include "ctlrc.h"
#include "hostc.h"
#include "screenc.h"
#include "scrollc.h"
#include "tablesc.h"
#include "telnetc.h"
#include "trace_dsc.h"
#include "utf8c.h"
#if defined(X3270_DBCS) /*[*/
#include "widec.h"
#endif /*]*/

#define MB_MAX	16

#define	SC	1	/* save cursor position */
#define RC	2	/* restore cursor position */
#define NL	3	/* new line */
#define UP	4	/* cursor up */
#define	E2	5	/* second level of ESC processing */
#define rS	6	/* reset */
#define IC	7	/* insert chars */
#define DN	8	/* cursor down */
#define RT	9	/* cursor right */
#define LT	10	/* cursor left */
#define CM	11	/* cursor motion */
#define ED	12	/* erase in display */
#define EL	13	/* erase in line */
#define IL	14	/* insert lines */
#define DL	15	/* delete lines */
#define DC	16	/* delete characters */
#define	SG	17	/* set graphic rendition */
#define BL	18	/* ring bell */
#define NP	19	/* new page */
#define BS	20	/* backspace */
#define CR	21	/* carriage return */
#define LF	22	/* line feed */
#define HT	23	/* horizontal tab */
#define E1	24	/* first level of ESC processing */
#define Xx	25	/* undefined control character (nop) */
#define Pc	26	/* printing character */
#define Sc	27	/* semicolon (after ESC [) */
#define Dg	28	/* digit (after ESC [ or ESC [ ?) */
#define RI	29	/* reverse index */
#define DA	30	/* send device attributes */
#define SM	31	/* set mode */
#define RM	32	/* reset mode */
#define DO	33	/* return terminal ID (obsolete) */
#define SR	34	/* device status report */
#define CS	35	/* character set designate */
#define E3	36	/* third level of ESC processing */
#define DS	37	/* DEC private set */
#define DR	38	/* DEC private reset */
#define DV	39	/* DEC private save */
#define DT	40	/* DEC private restore */
#define SS	41	/* set scrolling region */
#define TM	42	/* text mode (ESC ]) */
#define T2	43	/* semicolon (after ESC ]) */
#define TX	44	/* text parameter (after ESC ] n ;) */
#define TB	45	/* text parameter done (ESC ] n ; xxx BEL) */
#define TS	46	/* tab set */
#define TC	47	/* tab clear */
#define C2	48	/* character set designate (finish) */
#define G0	49	/* select G0 character set */
#define G1	50	/* select G1 character set */
#define G2	51	/* select G2 character set */
#define G3	52	/* select G3 character set */
#define S2	53	/* select G2 for next character */
#define S3	54	/* select G3 for next character */
#define MB	55	/* process multi-byte character */

static enum state {
    DATA = 0, ESC = 1, CSDES = 2,
    N1 = 3, DECP = 4, TEXT = 5, TEXT2 = 6,
    MBPEND = 7
} state = DATA;

static enum state ansi_data_mode(H3270 *, int, int);
static enum state dec_save_cursor(H3270 *, int, int);
static enum state dec_restore_cursor(H3270 *, int, int);
static enum state ansi_newline(H3270 *, int, int);
static enum state ansi_cursor_up(H3270 *, int, int);
static enum state ansi_esc2(H3270 *, int, int);
static enum state ansi_reset(H3270 *, int, int);
static enum state ansi_insert_chars(H3270 *, int, int);
static enum state ansi_cursor_down(H3270 *, int, int);
static enum state ansi_cursor_right(H3270 *, int, int);
static enum state ansi_cursor_left(H3270 *, int, int);
static enum state ansi_cursor_motion(H3270 *, int, int);
static enum state ansi_erase_in_display(H3270 *, int, int);
static enum state ansi_erase_in_line(H3270 *, int, int);
static enum state ansi_insert_lines(H3270 *, int, int);
static enum state ansi_delete_lines(H3270 *, int, int);
static enum state ansi_delete_chars(H3270 *, int, int);
static enum state ansi_sgr(H3270 *, int, int);
static enum state ansi_bell(H3270 *, int, int);
static enum state ansi_newpage(H3270 *, int, int);
static enum state ansi_backspace(H3270 *, int, int);
static enum state ansi_cr(H3270 *, int, int);
static enum state ansi_lf(H3270 *, int, int);
static enum state ansi_htab(H3270 *, int, int);
static enum state ansi_escape(H3270 *, int, int);
static enum state ansi_nop(H3270 *, int, int);
static enum state ansi_printing(H3270 *, int, int);
static enum state ansi_semicolon(H3270 *, int, int);
static enum state ansi_digit(H3270 *, int, int);
static enum state ansi_reverse_index(H3270 *, int, int);
static enum state ansi_send_attributes(H3270 *, int, int);
static enum state ansi_set_mode(H3270 *, int, int);
static enum state ansi_reset_mode(H3270 *, int, int);
static enum state dec_return_terminal_id(H3270 *, int, int);
static enum state ansi_status_report(H3270 *, int, int);
static enum state ansi_cs_designate(H3270 *, int, int);
static enum state ansi_esc3(H3270 *, int, int);
static enum state dec_set(H3270 *, int, int);
static enum state dec_reset(H3270 *, int, int);
static enum state dec_save(H3270 *, int, int);
static enum state dec_restore(H3270 *, int, int);
static enum state dec_scrolling_region(H3270 *, int, int);
static enum state xterm_text_mode(H3270 *, int, int);
static enum state xterm_text_semicolon(H3270 *, int, int);
static enum state xterm_text(H3270 *, int, int);
static enum state xterm_text_do(H3270 *, int, int);
static enum state ansi_htab_set(H3270 *, int, int);
static enum state ansi_htab_clear(H3270 *, int, int);
static enum state ansi_cs_designate2(H3270 *, int, int);
static enum state ansi_select_g0(H3270 *, int, int);
static enum state ansi_select_g1(H3270 *, int, int);
static enum state ansi_select_g2(H3270 *, int, int);
static enum state ansi_select_g3(H3270 *, int, int);
static enum state ansi_one_g2(H3270 *, int, int);
static enum state ansi_one_g3(H3270 *, int, int);
static enum state ansi_multibyte(H3270 *, int, int);

typedef enum state (*afn_t)(H3270 *, int, int);
static afn_t ansi_fn[] = {
/* 0 */		&ansi_data_mode,
/* 1 */		&dec_save_cursor,
/* 2 */		&dec_restore_cursor,
/* 3 */		&ansi_newline,
/* 4 */		&ansi_cursor_up,
/* 5 */		&ansi_esc2,
/* 6 */		&ansi_reset,
/* 7 */		&ansi_insert_chars,
/* 8 */		&ansi_cursor_down,
/* 9 */		&ansi_cursor_right,
/* 10 */	&ansi_cursor_left,
/* 11 */	&ansi_cursor_motion,
/* 12 */	&ansi_erase_in_display,
/* 13 */	&ansi_erase_in_line,
/* 14 */	&ansi_insert_lines,
/* 15 */	&ansi_delete_lines,
/* 16 */	&ansi_delete_chars,
/* 17 */	&ansi_sgr,
/* 18 */	&ansi_bell,
/* 19 */	&ansi_newpage,
/* 20 */	&ansi_backspace,
/* 21 */	&ansi_cr,
/* 22 */	&ansi_lf,
/* 23 */	&ansi_htab,
/* 24 */	&ansi_escape,
/* 25 */	&ansi_nop,
/* 26 */	&ansi_printing,
/* 27 */	&ansi_semicolon,
/* 28 */	&ansi_digit,
/* 29 */	&ansi_reverse_index,
/* 30 */	&ansi_send_attributes,
/* 31 */	&ansi_set_mode,
/* 32 */	&ansi_reset_mode,
/* 33 */	&dec_return_terminal_id,
/* 34 */	&ansi_status_report,
/* 35 */	&ansi_cs_designate,
/* 36 */	&ansi_esc3,
/* 37 */	&dec_set,
/* 38 */	&dec_reset,
/* 39 */	&dec_save,
/* 40 */	&dec_restore,
/* 41 */	&dec_scrolling_region,
/* 42 */	&xterm_text_mode,
/* 43 */	&xterm_text_semicolon,
/* 44 */	&xterm_text,
/* 45 */	&xterm_text_do,
/* 46 */	&ansi_htab_set,
/* 47 */	&ansi_htab_clear,
/* 48 */	&ansi_cs_designate2,
/* 49 */	&ansi_select_g0,
/* 50 */	&ansi_select_g1,
/* 51 */	&ansi_select_g2,
/* 52 */	&ansi_select_g3,
/* 53 */	&ansi_one_g2,
/* 54 */	&ansi_one_g3,
/* 55 */	&ansi_multibyte,
};

static unsigned char st[8][256] = {
/*
 * State table for base processing (state == DATA)
 */
{
	     /* 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f  */
/* 00 */       Xx,Xx,Xx,Xx,Xx,Xx,Xx,BL,BS,HT,LF,LF,NP,CR,G1,G0,
/* 10 */       Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,E1,Xx,Xx,Xx,Xx,
/* 20 */       Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,
/* 30 */       Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,
/* 40 */       Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,
/* 50 */       Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,
/* 60 */       Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,
/* 70 */       Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Xx,
/* 80 */       Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,
/* 90 */       Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,Xx,
/* a0 */       Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,
/* b0 */       Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,
/* c0 */       Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,
/* d0 */       Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,
/* e0 */       Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,
/* f0 */       Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc,Pc
},

/*
 * State table for ESC processing (state == ESC)
 */
{
	     /* 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f  */
/* 00 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 10 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 20 */	0, 0, 0, 0, 0, 0, 0, 0,CS,CS,CS,CS, 0, 0, 0, 0,
/* 30 */	0, 0, 0, 0, 0, 0, 0,SC,RC, 0, 0, 0, 0, 0, 0, 0,
/* 40 */	0, 0, 0, 0, 0,NL, 0, 0,TS, 0, 0, 0, 0,RI,S2,S3,
/* 50 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,E2, 0,TM, 0, 0,
/* 60 */	0, 0, 0,rS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,G2,G3,
/* 70 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 80 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 90 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* a0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* b0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* c0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* d0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* e0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* f0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
},

/*
 * State table for ESC ()*+ C processing (state == CSDES)
 */
{
	     /* 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f  */
/* 00 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 10 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 20 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 30 */       C2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 40 */	0,C2,C2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 50 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 60 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 70 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 80 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 90 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* a0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* b0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* c0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* d0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* e0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* f0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
},

/*
 * State table for ESC [ processing (state == N1)
 */
{
	     /* 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f  */
/* 00 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 10 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 20 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 30 */       Dg,Dg,Dg,Dg,Dg,Dg,Dg,Dg,Dg,Dg, 0,Sc, 0, 0, 0,E3,
/* 40 */       IC,UP,DN,RT,LT, 0, 0, 0,CM, 0,ED,EL,IL,DL, 0, 0,
/* 50 */       DC, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 60 */	0, 0, 0,DA, 0, 0,CM,TC,SM, 0, 0, 0,RM,SG,SR, 0,
/* 70 */	0, 0,SS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 80 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 90 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* a0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* b0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* c0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* d0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* e0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* f0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
},

/*
 * State table for ESC [ ? processing (state == DECP)
 */
{
	     /* 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f  */
/* 00 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 10 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 20 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 30 */       Dg,Dg,Dg,Dg,Dg,Dg,Dg,Dg,Dg,Dg, 0, 0, 0, 0, 0, 0,
/* 40 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 50 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 60 */	0, 0, 0, 0, 0, 0, 0, 0,DS, 0, 0, 0,DR, 0, 0, 0,
/* 70 */	0, 0,DT,DV, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 80 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 90 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* a0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* b0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* c0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* d0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* e0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* f0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
},

/*
 * State table for ESC ] processing (state == TEXT)
 */
{
	     /* 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f  */
/* 00 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 10 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 20 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 30 */       Dg,Dg,Dg,Dg,Dg,Dg,Dg,Dg,Dg,Dg, 0,T2, 0, 0, 0, 0,
/* 40 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 50 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 60 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 70 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 80 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 90 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* a0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* b0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* c0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* d0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* e0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* f0 */	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
},

/*
 * State table for ESC ] n ; processing (state == TEXT2)
 */
{
	     /* 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f  */
/* 00 */        0, 0, 0, 0, 0, 0, 0,TB, 0, 0, 0, 0, 0, 0, 0, 0,
/* 10 */        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
/* 20 */       TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,
/* 30 */       TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,
/* 40 */       TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,
/* 50 */       TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,
/* 60 */       TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,
/* 70 */       TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,Xx,
/* 80 */       TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,
/* 90 */       TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,
/* a0 */       TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,
/* b0 */       TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,
/* c0 */       TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,
/* d0 */       TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,
/* e0 */       TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,
/* f0 */       TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX,TX
},
/*
 * State table for multi-byte characters (state == MBPEND)
 */
{
	     /* 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f  */
/* 00 */       MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,
/* 10 */       MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,
/* 20 */       MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,
/* 30 */       MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,
/* 40 */       MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,
/* 50 */       MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,
/* 60 */       MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,
/* 70 */       MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,
/* 80 */       MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,
/* 90 */       MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,
/* a0 */       MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,
/* b0 */       MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,
/* c0 */       MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,
/* d0 */       MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,
/* e0 */       MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,
/* f0 */       MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB,MB
},
};

/* Character sets. */
#define CS_G0		0
#define CS_G1		1
#define CS_G2		2
#define CS_G3		3

/* Character set designations. */
#define CSD_LD		0
#define CSD_UK		1
#define CSD_US		2

static int      saved_cursor = 0;
#define NN	20
static int      n[NN], nx = 0;
#define NT	256
static char     text[NT + 1];
static int      tx = 0;
static int      ansi_ch;
static unsigned char gr = 0;
static unsigned char saved_gr = 0;
static unsigned char fg = 0;
static unsigned char saved_fg = 0;
static unsigned char bg = 0;
static unsigned char saved_bg = 0;
static int	cset = CS_G0;
static int	saved_cset = CS_G0;
static int	csd[4] = { CSD_US, CSD_US, CSD_US, CSD_US };
static int	saved_csd[4] = { CSD_US, CSD_US, CSD_US, CSD_US };
static int	once_cset = -1;
static int      insert_mode = 0;
static int      auto_newline_mode = 0;
static int      appl_cursor = 0;
static int      saved_appl_cursor = 0;
static int      wraparound_mode = 1;
static int      saved_wraparound_mode = 1;
static int      rev_wraparound_mode = 0;
static int      saved_rev_wraparound_mode = 0;
static int	allow_wide_mode = 0;
static int	saved_allow_wide_mode = 0;
static int	wide_mode = 0;
static int	saved_wide_mode = 0;
static Boolean  saved_altbuffer = False;
static int      scroll_top = -1;
static int      scroll_bottom = -1;
static unsigned char *tabs = (unsigned char *) NULL;
static char	gnnames[] = "()*+";
static char	csnames[] = "0AB";
static int	cs_to_change;
#if defined(X3270_DBCS) /*[*/
static unsigned char mb_pending = 0;
static char	mb_buffer[MB_MAX];
static int	dbcs_process(int ch, unsigned char ebc[]);
#endif /*]*/
static int	pmi = 0;
static char	pending_mbs[MB_MAX];

static Boolean  held_wrap = False;

static void	ansi_scroll(void);

static enum state
ansi_data_mode(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	return DATA;
}

static enum state
dec_save_cursor(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	int i;

	saved_cursor = h3270.cursor_addr;
	saved_cset = cset;
	for (i = 0; i < 4; i++)
		saved_csd[i] = csd[i];
	saved_fg = fg;
	saved_bg = bg;
	saved_gr = gr;
	return DATA;
}

static enum state
dec_restore_cursor(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	int i;

	cset = saved_cset;
	for (i = 0; i < 4; i++)
		csd[i] = saved_csd[i];
	fg = saved_fg;
	bg = saved_bg;
	gr = saved_gr;
	cursor_move(&h3270,saved_cursor);
	held_wrap = False;
	return DATA;
}

static enum state
ansi_newline(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	int nc;

	cursor_move(&h3270,h3270.cursor_addr - (h3270.cursor_addr % h3270.cols));
	nc = h3270.cursor_addr + h3270.cols;
	if (nc < scroll_bottom * h3270.cols)
		cursor_move(&h3270,nc);
	else
		ansi_scroll();
	held_wrap = False;
	return DATA;
}

static enum state
ansi_cursor_up(H3270 *hSession, int nn, int ig2 unused)
{
	int rr;

	if (nn < 1)
		nn = 1;
	rr = h3270.cursor_addr / h3270.cols;
	if (rr - nn < 0)
		cursor_move(&h3270, h3270.cursor_addr % h3270.cols);
	else
		cursor_move(&h3270, h3270.cursor_addr - (nn * h3270.cols));
	held_wrap = False;
	return DATA;
}

static enum state
ansi_esc2(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	register int	i;

	for (i = 0; i < NN; i++)
		n[i] = 0;
	nx = 0;
	return N1;
}

static enum state
ansi_reset(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	int i;
	static Boolean first = True;

	gr = 0;
	saved_gr = 0;
	fg = 0;
	saved_fg = 0;
	bg = 0;
	saved_bg = 0;
	cset = CS_G0;
	saved_cset = CS_G0;
	csd[0] = csd[1] = csd[2] = csd[3] = CSD_US;
	saved_csd[0] = saved_csd[1] = saved_csd[2] = saved_csd[3] = CSD_US;
	once_cset = -1;
	saved_cursor = 0;
	insert_mode = 0;
	auto_newline_mode = 0;
	appl_cursor = 0;
	saved_appl_cursor = 0;
	wraparound_mode = 1;
	saved_wraparound_mode = 1;
	rev_wraparound_mode = 0;
	saved_rev_wraparound_mode = 0;
	allow_wide_mode = 0;
	saved_allow_wide_mode = 0;
	wide_mode = 0;
	allow_wide_mode = 0;
	saved_altbuffer = False;
	scroll_top = 1;
	scroll_bottom = h3270.rows;
	Replace(tabs, (unsigned char *)lib3270_malloc((h3270.cols+7)/8));
	for (i = 0; i < (h3270.cols+7)/8; i++)
		tabs[i] = 0x01;
	held_wrap = False;
	if (!first) {
		ctlr_altbuffer(&h3270,True);
		ctlr_aclear(&h3270, 0, h3270.rows * h3270.cols, 1);
		ctlr_altbuffer(&h3270,False);
		ctlr_clear(&h3270,False);
		screen_80();
	}
	first = False;
	pmi = 0;
	return DATA;
}

static enum state
ansi_insert_chars(H3270 *hSession, int nn, int ig2 unused)
{
	int cc = h3270.cursor_addr % h3270.cols;	/* current col */
	int mc = h3270.cols - cc;		/* max chars that can be inserted */
	int ns;				/* chars that are shifting */

	if (nn < 1)
		nn = 1;
	if (nn > mc)
		nn = mc;

	/* Move the surviving chars right */
	ns = mc - nn;
	if (ns)
		ctlr_bcopy(h3270.cursor_addr, h3270.cursor_addr + nn, ns, 1);

	/* Clear the middle of the line */
	ctlr_aclear(&h3270, h3270.cursor_addr, nn, 1);
	return DATA;
}

static enum state
ansi_cursor_down(H3270 *hSession, int nn, int ig2 unused)
{
	int rr;

	if (nn < 1)
		nn = 1;
	rr = h3270.cursor_addr / h3270.cols;
	if (rr + nn >= h3270.cols)
		cursor_move(&h3270,(h3270.cols-1)*h3270.cols + (h3270.cursor_addr%h3270.cols));
	else
		cursor_move(&h3270,h3270.cursor_addr + (nn * h3270.cols));
	held_wrap = False;
	return DATA;
}

static enum state
ansi_cursor_right(H3270 *hSession, int nn, int ig2 unused)
{
	int cc;

	if (nn < 1)
		nn = 1;
	cc = h3270.cursor_addr % h3270.cols;
	if (cc == h3270.cols-1)
		return DATA;
	if (cc + nn >= h3270.cols)
		nn = h3270.cols - 1 - cc;
	cursor_move(&h3270,h3270.cursor_addr + nn);
	held_wrap = False;
	return DATA;
}

static enum state
ansi_cursor_left(H3270 *hSession, int nn, int ig2 unused)
{
	int cc;

	if (held_wrap) {
		held_wrap = False;
		return DATA;
	}
	if (nn < 1)
		nn = 1;
	cc = h3270.cursor_addr % h3270.cols;
	if (!cc)
		return DATA;
	if (nn > cc)
		nn = cc;
	cursor_move(&h3270,h3270.cursor_addr - nn);
	return DATA;
}

static enum state
ansi_cursor_motion(H3270 *hSession, int n1, int n2)
{
	if (n1 < 1) n1 = 1;
	if (n1 > h3270.rows) n1 = h3270.rows;
	if (n2 < 1) n2 = 1;
	if (n2 > h3270.cols) n2 = h3270.cols;
	cursor_move(&h3270,(n1 - 1) * h3270.cols + (n2 - 1));
	held_wrap = False;
	return DATA;
}

static enum state
ansi_erase_in_display(H3270 *hSession, int nn, int ig2 unused)
{
	switch (nn) {
	    case 0:	/* below */
		ctlr_aclear(&h3270, h3270.cursor_addr, (h3270.rows * h3270.cols) - h3270.cursor_addr, 1);
		break;
	    case 1:	/* above */
		ctlr_aclear(&h3270, 0, h3270.cursor_addr + 1, 1);
		break;
	    case 2:	/* all (without moving cursor) */
//		if (h3270.cursor_addr == 0 && !h3270.is_altbuffer) scroll_save(h3270.rows, True);
		ctlr_aclear(&h3270, 0, h3270.rows * h3270.cols, 1);
		break;
	}
	return DATA;
}

static enum state
ansi_erase_in_line(H3270 *hSession, int nn, int ig2 unused)
{
	int nc = h3270.cursor_addr % h3270.cols;

	switch (nn) {
	    case 0:	/* to right */
		ctlr_aclear(&h3270, h3270.cursor_addr, h3270.cols - nc, 1);
		break;
	    case 1:	/* to left */
		ctlr_aclear(&h3270, h3270.cursor_addr - nc, nc+1, 1);
		break;
	    case 2:	/* all */
		ctlr_aclear(&h3270, h3270.cursor_addr - nc, h3270.cols, 1);
		break;
	}
	return DATA;
}

static enum state
ansi_insert_lines(H3270 *hSession, int nn, int ig2 unused)
{
	int rr = h3270.cursor_addr / h3270.cols;	/* current row */
	int mr = scroll_bottom - rr;	/* rows left at and below this one */
	int ns;				/* rows that are shifting */

	/* If outside of the scrolling region, do nothing */
	if (rr < scroll_top - 1 || rr >= scroll_bottom)
		return DATA;

	if (nn < 1)
		nn = 1;
	if (nn > mr)
		nn = mr;

	/* Move the victims down */
	ns = mr - nn;
	if (ns)
		ctlr_bcopy(rr * h3270.cols, (rr + nn) * h3270.cols, ns * h3270.cols, 1);

	/* Clear the middle of the screen */
	ctlr_aclear(&h3270, rr * h3270.cols, nn * h3270.cols, 1);
	return DATA;
}

static enum state
ansi_delete_lines(H3270 *hSession, int nn, int ig2 unused)
{
	int rr = h3270.cursor_addr / h3270.cols;	/* current row */
	int mr = scroll_bottom - rr;	/* max rows that can be deleted */
	int ns;				/* rows that are shifting */

	/* If outside of the scrolling region, do nothing */
	if (rr < scroll_top - 1 || rr >= scroll_bottom)
		return DATA;

	if (nn < 1)
		nn = 1;
	if (nn > mr)
		nn = mr;

	/* Move the surviving rows up */
	ns = mr - nn;
	if (ns)
		ctlr_bcopy((rr + nn) * h3270.cols, rr * h3270.cols, ns * h3270.cols, 1);

	/* Clear the rest of the screen */
	ctlr_aclear(&h3270, (rr + ns) * h3270.cols, nn * h3270.cols, 1);
	return DATA;
}

static enum state
ansi_delete_chars(H3270 *hSession, int nn, int ig2 unused)
{
	int cc = h3270.cursor_addr % h3270.cols;	/* current col */
	int mc = h3270.cols - cc;		/* max chars that can be deleted */
	int ns;				/* chars that are shifting */

	if (nn < 1)
		nn = 1;
	if (nn > mc)
		nn = mc;

	/* Move the surviving chars left */
	ns = mc - nn;
	if (ns)
		ctlr_bcopy(h3270.cursor_addr + nn, h3270.cursor_addr, ns, 1);

	/* Clear the end of the line */
	ctlr_aclear(&h3270, h3270.cursor_addr + ns, nn, 1);
	return DATA;
}

static enum state
ansi_sgr(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	int i;

	for (i = 0; i <= nx && i < NN; i++)
	    switch (n[i]) {
		case 0:
		    gr = 0;
		    fg = 0;
		    bg = 0;
		    break;
		case 1:
		    gr |= GR_INTENSIFY;
		    break;
		case 4:
		    gr |= GR_UNDERLINE;
		    break;
		case 5:
		    gr |= GR_BLINK;
		    break;
		case 7:
		    gr |= GR_REVERSE;
		    break;
		case 30:
		    fg = 0xf0;	/* black */
		    break;
		case 31:
		    fg = 0xf2;	/* red */
		    break;
		case 32:
		    fg = 0xf4;	/* green */
		    break;
		case 33:
		    fg = 0xf6;	/* yellow */
		    break;
		case 34:
		    fg = 0xf1;	/* blue */
		    break;
		case 35:
		    fg = 0xf3;	/* magenta */
		    break;
		case 36:
#if defined(WC3270) /*[*/
		    fg = 0xf6;	/* turquoise */
#else /*][*/
		    fg = 0xfd;	/* cyan */
#endif /*]*/
		    break;
		case 37:
#if defined(WC3270) /*[*/
		    fg = 0xf7;	/* white */
#else /*][*/
		    fg = 0xff;	/* white */
#endif /*]*/
		    break;
		case 39:
		    fg = 0;	/* default */
		    break;
		case 40:
		    bg = 0xf0;	/* black */
		    break;
		case 41:
		    bg = 0xf2;	/* red */
		    break;
		case 42:
		    bg = 0xf4;	/* green */
		    break;
		case 43:
		    bg = 0xf6;	/* yellow */
		    break;
		case 44:
		    bg = 0xf1;	/* blue */
		    break;
		case 45:
		    bg = 0xf3;	/* magenta */
		    break;
		case 46:
#if defined(WC3270) /*[*/
		    bg = 0xf6;	/* turquoise */
#else /*][*/
		    bg = 0xfd;	/* cyan */
#endif /*]*/
		    break;
		case 47:
#if defined(WC3270) /*[*/
		    bg = 0xf7;	/* white */
#else /*][*/
		    bg = 0xff;	/* white */
#endif /*]*/
		    break;
		case 49:
		    bg = 0;	/* default */
		    break;
	    }

	return DATA;
}

static enum state
ansi_bell(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	lib3270_ring_bell(hSession);
	return DATA;
}

static enum state
ansi_newpage(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	ctlr_clear(&h3270,False);
	return DATA;
}

static enum state
ansi_backspace(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	if (held_wrap) {
		held_wrap = False;
		return DATA;
	}
	if (rev_wraparound_mode) {
		if (h3270.cursor_addr > (scroll_top - 1) * h3270.cols)
			cursor_move(&h3270,h3270.cursor_addr - 1);
	} else {
		if (h3270.cursor_addr % h3270.cols)
			cursor_move(&h3270,h3270.cursor_addr - 1);
	}
	return DATA;
}

static enum state
ansi_cr(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	if (h3270.cursor_addr % h3270.cols)
		cursor_move(&h3270,h3270.cursor_addr - (h3270.cursor_addr % h3270.cols));
	if (auto_newline_mode)
		(void) ansi_lf(hSession, 0, 0);
	held_wrap = False;
	return DATA;
}

static enum state
ansi_lf(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	int nc = h3270.cursor_addr + h3270.cols;

	held_wrap = False;

	/* If we're below the scrolling region, don't scroll. */
	if ((h3270.cursor_addr / h3270.cols) >= scroll_bottom) {
		if (nc < h3270.rows * h3270.cols)
			cursor_move(&h3270,nc);
		return DATA;
	}

	if (nc < scroll_bottom * h3270.cols)
		cursor_move(&h3270,nc);
	else
		ansi_scroll();
	return DATA;
}

static enum state
ansi_htab(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	int col = h3270.cursor_addr % h3270.cols;
	int i;

	held_wrap = False;
	if (col == h3270.cols-1)
		return DATA;
	for (i = col+1; i < h3270.cols-1; i++)
		if (tabs[i/8] & 1<<(i%8))
			break;
	cursor_move(&h3270,h3270.cursor_addr - col + i);
	return DATA;
}

static enum state
ansi_escape(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	return ESC;
}

static enum state
ansi_nop(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	return DATA;
}

#define PWRAP { \
    nc = h3270.cursor_addr + 1; \
    if (nc < scroll_bottom * h3270.cols) \
	    cursor_move(&h3270,nc); \
    else { \
	    if (h3270.cursor_addr / h3270.cols >= scroll_bottom) \
		    cursor_move(&h3270,h3270.cursor_addr / h3270.cols * h3270.cols); \
	    else { \
		    ansi_scroll(); \
		    cursor_move(&h3270,nc - h3270.cols); \
	    } \
    } \
}

static enum state
ansi_printing(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	int nc;
	unsigned char ebc_ch;
	int default_cs = CS_BASE;
#if defined(X3270_DBCS) /*[*/
	enum dbcs_state d;
	Boolean preserve_right = False;
#endif /*]*/

	if ((pmi == 0) && (ansi_ch & 0x80)) {
	    	char mbs[2];
		enum ulfail fail;
		unsigned char ch;

		mbs[0] = (char)ansi_ch;
		mbs[1] = '\0';
		ch = utf8_lookup(mbs, &fail, NULL);
		if (ch == 0) {
			switch (fail) {
			case ULFAIL_NOUTF8:
			    	/* Leave it alone. */
				break;
			case ULFAIL_INCOMPLETE:
				/* Start munching multi-byte. */
				pmi = 0;
				pending_mbs[pmi++] = (char)ansi_ch;
				return MBPEND;
			case ULFAIL_INVALID:
				/* Invalid multi-byte -> '?' */
				ansi_ch = '?';
				/* XXX: If DBCS, we should let
				 * ICU have a crack at it
				 */
				break;
			}
		}
	}
	pmi = 0;

	if (held_wrap) {
		PWRAP;
		held_wrap = False;
	}

	if (insert_mode)
		(void) ansi_insert_chars(hSession,1, 0);
	switch (csd[(once_cset != -1) ? once_cset : cset]) {
	    case CSD_LD:	/* line drawing "0" */
		if (ansi_ch >= 0x5f && ansi_ch <= 0x7e)
			ctlr_add(&h3270,h3270.cursor_addr, (unsigned char)(ansi_ch - 0x5f),CS_LINEDRAW);
		else
			ctlr_add(&h3270,h3270.cursor_addr, asc2ebc[ansi_ch], CS_BASE);
		break;
	    case CSD_UK:	/* UK "A" */
		if (ansi_ch == '#')
			ctlr_add(&h3270,h3270.cursor_addr, 0x1e, CS_LINEDRAW);
		else
			ctlr_add(&h3270,h3270.cursor_addr, asc2ebc[ansi_ch], CS_BASE);
		break;
	    case CSD_US:	/* US "B" */
		ebc_ch = asc2ebc[ansi_ch];
#if defined(X3270_DBCS) /*[*/
		d = ctlr_dbcs_state(cursor_addr);
		if (dbcs) {
			if (mb_pending || (ansi_ch & 0x80)) {
				int len;
				unsigned char ebc[2];

				len = dbcs_process(ansi_ch, ebc);
				switch (len) {
				    default:
				    case 0:
					/* Translation failed. */
					return DATA;
				    case 1:
					/* It was really SBCS. */
					ebc_ch = ebc[0];
					break;
				    case 2:
					/* DBCS. */
					if ((cursor_addr % COLS) == (COLS-1)) {
						ebc_ch = EBC_space;
						break;
					}
					ctlr_add(cursor_addr, ebc[0], CS_DBCS);
					ctlr_add_gr(cursor_addr, gr);
					ctlr_add_fg(cursor_addr, fg);
					ctlr_add_bg(cursor_addr, bg);
					if (wraparound_mode) {
						if (!((cursor_addr + 1) % COLS)) {
							held_wrap = True;
						} else {
							PWRAP;
						}
					} else {
						if ((cursor_addr % COLS) != (COLS - 1))
							cursor_move(cursor_addr + 1);
					}

					/*
					 * Set up the right-hand side to be
					 * stored below.
					 */
					ebc_ch = ebc[1];
					default_cs = CS_DBCS;
					preserve_right = True;
					break;
				}
			} else if (ansi_ch & 0x80) {
				(void) dbcs_process(ansi_ch, NULL);
				ebc_ch = EBC_space;
			}
		}

		/* Handle conflicts with existing DBCS characters. */
		if (!preserve_right &&
		    (d == DBCS_RIGHT || d == DBCS_RIGHT_WRAP)) {
			int xaddr;

			xaddr = cursor_addr;
			DEC_BA(xaddr);
			ctlr_add(xaddr, EBC_space, CS_BASE);
			ea_buf[xaddr].db = DBCS_NONE;
			ea_buf[cursor_addr].db = DBCS_NONE;
		}
		if (d == DBCS_LEFT || d == DBCS_LEFT_WRAP) {
			int xaddr;

			xaddr = cursor_addr;
			INC_BA(xaddr);
			ctlr_add(xaddr, EBC_space, CS_BASE);
			ea_buf[xaddr].db = DBCS_NONE;
			ea_buf[cursor_addr].db = DBCS_NONE;
		}
#endif /*]*/
		ctlr_add(&h3270,h3270.cursor_addr, ebc_ch, default_cs);
#if defined(X3270_DBCS) /*[*/
		if (default_cs == CS_DBCS)
			(void) ctlr_dbcs_postprocess();
#endif /*]*/
		break;
	}
	once_cset = -1;
	ctlr_add_gr(&h3270,h3270.cursor_addr, gr);
	ctlr_add_fg(&h3270,h3270.cursor_addr, fg);
	ctlr_add_bg(&h3270,h3270.cursor_addr, bg);
	if (wraparound_mode) {
		/*
		 * There is a fascinating behavior of xterm which we will
		 * attempt to emulate here.  When a character is printed in the
		 * last column, the cursor sticks there, rather than wrapping
		 * to the next line.  Another printing character will put the
		 * cursor in column 2 of the next line.  One cursor-left
		 * sequence won't budge it; two will.  Saving and restoring
		 * the cursor won't move the cursor, but will cancel all of
		 * the above behaviors...
		 *
		 * In my opinion, very strange, but among other things, 'vi'
		 * depends on it!
		 */
		if (!((h3270.cursor_addr + 1) % h3270.cols)) {
			held_wrap = True;
		} else {
			PWRAP;
		}
	} else {
		if ((h3270.cursor_addr % h3270.cols) != (h3270.cols - 1))
			cursor_move(&h3270,h3270.cursor_addr + 1);
	}
	return DATA;
}

static enum state
ansi_multibyte(H3270 *hSession, int ig1, int ig2)
{
    	char mbs[MB_MAX];
	unsigned char ch;
	enum ulfail fail;
	afn_t fn;

	if (pmi >= MB_MAX - 2) {
	    	/* String too long. */
		pmi = 0;
	    	ansi_ch = '?';
		return ansi_printing(hSession,ig1, ig2);
	}

	strncpy(mbs, pending_mbs, pmi);
	mbs[pmi] = (char)ansi_ch;
	mbs[pmi + 1] = '\0';

	ch = utf8_lookup(mbs, &fail, NULL);
	if (ch != 0) {
	    	// Success!
	    	ansi_ch = ch;
		return ansi_printing(hSession, ig1, ig2);
	}
	if (fail == ULFAIL_INCOMPLETE) {
	    	// Go get more.
	    	pending_mbs[pmi++] = (char)ansi_ch;
		return MBPEND;
	}

	/* Failure. */

	/* Replace the sequence with '?'. */
	ch = ansi_ch; /* save for later */
	pmi = 0;
	ansi_ch = '?';
	(void) ansi_printing(hSession, ig1, ig2);

	/* Reprocess whatever we choked on. */
	ansi_ch = ch;
	state = DATA;
	fn = ansi_fn[st[(int)DATA][ansi_ch]];
	return (*fn)(hSession,n[0], n[1]);
}

static enum state
ansi_semicolon(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	if (nx >= NN)
		return DATA;
	nx++;
	return state;
}

static enum state
ansi_digit(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	n[nx] = (n[nx] * 10) + (ansi_ch - '0');
	return state;
}

static enum state
ansi_reverse_index(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	int rr = h3270.cursor_addr / h3270.cols;	/* current row */
	int np = (scroll_top - 1) - rr;	/* number of rows in the scrolling
					   region, above this line */
	int ns;				/* number of rows to scroll */
	int nn = 1;			/* number of rows to index */

	held_wrap = False;

	/* If the cursor is above the scrolling region, do a simple margined
	   cursor up.  */
	if (np < 0) {
		(void) ansi_cursor_up(hSession, nn, 0);
		return DATA;
	}

	/* Split the number of lines to scroll into ns */
	if (nn > np) {
		ns = nn - np;
		nn = np;
	} else
		ns = 0;

	/* Move the cursor up without scrolling */
	if (nn)
		(void) ansi_cursor_up(hSession,nn, 0);

	/* Insert lines at the top for backward scroll */
	if (ns)
		(void) ansi_insert_lines(hSession, ns, 0);

	return DATA;
}

static enum state
ansi_send_attributes(H3270 *hSession, int nn, int ig2 unused)
{
	if (!nn)
		net_sends("\033[?1;2c");
	return DATA;
}

static enum state
dec_return_terminal_id(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	return ansi_send_attributes(hSession, 0, 0);
}

static enum state
ansi_set_mode(H3270 *hSession, int nn, int ig2 unused)
{
	switch (nn) {
	    case 4:
		insert_mode = 1;
		break;
	    case 20:
		auto_newline_mode = 1;
		break;
	}
	return DATA;
}

static enum state
ansi_reset_mode(H3270 *hSession, int nn, int ig2 unused)
{
	switch (nn) {
	    case 4:
		insert_mode = 0;
		break;
	    case 20:
		auto_newline_mode = 0;
		break;
	}
	return DATA;
}

static enum state
ansi_status_report(H3270 *hSession, int nn, int ig2 unused)
{
	static char cpr[11];

	switch (nn) {
	    case 5:
		net_sends("\033[0n");
		break;
	    case 6:
		(void) sprintf(cpr, "\033[%d;%dR",
		    (h3270.cursor_addr/h3270.cols) + 1, (h3270.cursor_addr%h3270.cols) + 1);
		net_sends(cpr);
		break;
	}
	return DATA;
}

static enum state
ansi_cs_designate(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	cs_to_change = strchr(gnnames, ansi_ch) - gnnames;
	return CSDES;
}

static enum state
ansi_cs_designate2(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	csd[cs_to_change] = strchr(csnames, ansi_ch) - csnames;
	return DATA;
}

static enum state
ansi_select_g0(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	cset = CS_G0;
	return DATA;
}

static enum state
ansi_select_g1(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	cset = CS_G1;
	return DATA;
}

static enum state
ansi_select_g2(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	cset = CS_G2;
	return DATA;
}

static enum state
ansi_select_g3(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	cset = CS_G3;
	return DATA;
}

static enum state
ansi_one_g2(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	once_cset = CS_G2;
	return DATA;
}

static enum state
ansi_one_g3(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	once_cset = CS_G3;
	return DATA;
}

static enum state
ansi_esc3(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	return DECP;
}

static enum state
dec_set(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	int i;

	for (i = 0; i <= nx && i < NN; i++)
		switch (n[i]) {
		    case 1:	/* application cursor keys */
			appl_cursor = 1;
			break;
		    case 2:	/* set G0-G3 */
			csd[0] = csd[1] = csd[2] = csd[3] = CSD_US;
			break;
		    case 3:	/* 132-column mode */
			if (allow_wide_mode) {
				wide_mode = 1;
				screen_132();
			}
			break;
		    case 7:	/* wraparound mode */
			wraparound_mode = 1;
			break;
		    case 40:	/* allow 80/132 switching */
			allow_wide_mode = 1;
			break;
		    case 45:	/* reverse-wraparound mode */
			rev_wraparound_mode = 1;
			break;
		    case 47:	/* alt buffer */
			ctlr_altbuffer(&h3270,True);
			break;
		}
	return DATA;
}

static enum state
dec_reset(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	int i;

	for (i = 0; i <= nx && i < NN; i++)
		switch (n[i]) {
		    case 1:	/* normal cursor keys */
			appl_cursor = 0;
			break;
		    case 3:	/* 132-column mode */
			if (allow_wide_mode) {
				wide_mode = 0;
				screen_80();
			}
			break;
		    case 7:	/* no wraparound mode */
			wraparound_mode = 0;
			break;
		    case 40:	/* allow 80/132 switching */
			allow_wide_mode = 0;
			break;
		    case 45:	/* no reverse-wraparound mode */
			rev_wraparound_mode = 0;
			break;
		    case 47:	/* alt buffer */
			ctlr_altbuffer(&h3270,False);
			break;
		}
	return DATA;
}

static enum state
dec_save(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	int i;

	for (i = 0; i <= nx && i < NN; i++)
		switch (n[i]) {
		    case 1:	/* application cursor keys */
			saved_appl_cursor = appl_cursor;
			break;
		    case 3:	/* 132-column mode */
			saved_wide_mode = wide_mode;
			break;
		    case 7:	/* wraparound mode */
			saved_wraparound_mode = wraparound_mode;
			break;
		    case 40:	/* allow 80/132 switching */
			saved_allow_wide_mode = allow_wide_mode;
			break;
		    case 45:	/* reverse-wraparound mode */
			saved_rev_wraparound_mode = rev_wraparound_mode;
			break;
		    case 47:	/* alt buffer */
			saved_altbuffer = h3270.is_altbuffer;
			break;
		}
	return DATA;
}

static enum state
dec_restore(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	int i;

	for (i = 0; i <= nx && i < NN; i++)
		switch (n[i]) {
		    case 1:	/* application cursor keys */
			appl_cursor = saved_appl_cursor;
			break;
		    case 3:	/* 132-column mode */
			if (allow_wide_mode) {
				wide_mode = saved_wide_mode;
				if (wide_mode)
					screen_132();
				else
					screen_80();
			}
			break;
		    case 7:	/* wraparound mode */
			wraparound_mode = saved_wraparound_mode;
			break;
		    case 40:	/* allow 80/132 switching */
			allow_wide_mode = saved_allow_wide_mode;
			break;
		    case 45:	/* reverse-wraparound mode */
			rev_wraparound_mode = saved_rev_wraparound_mode;
			break;
		    case 47:	/* alt buffer */
			ctlr_altbuffer(&h3270,saved_altbuffer);
			break;
		}
	return DATA;
}

static enum state
dec_scrolling_region(H3270 *hSession, int top, int bottom)
{
	if (top < 1)
		top = 1;
	if (bottom > h3270.rows)
		bottom = h3270.rows;
	if (top <= bottom && (top > 1 || bottom < h3270.rows)) {
		scroll_top = top;
		scroll_bottom = bottom;
		cursor_move(&h3270,0);
	} else {
		scroll_top = 1;
		scroll_bottom = h3270.rows;
	}
	return DATA;
}

static enum state
xterm_text_mode(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	nx = 0;
	n[0] = 0;
	return TEXT;
}

static enum state
xterm_text_semicolon(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	tx = 0;
	return TEXT2;
}

static enum state
xterm_text(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	if (tx < NT)
		text[tx++] = ansi_ch;
	return state;
}

static enum state
xterm_text_do(H3270 *hSession, int ig1 unused, int ig2 unused)
{
/*
#if defined(X3270_DISPLAY) || defined(WC3270)
	text[tx] = '\0';
#endif

#if defined(X3270_DISPLAY)
	switch (n[0]) {
	    case 0:	// icon name and window title
		XtVaSetValues(toplevel, XtNiconName, text, NULL);
		XtVaSetValues(toplevel, XtNtitle, text, NULL);
		break;
	    case 1:	// icon name
		XtVaSetValues(toplevel, XtNiconName, text, NULL);
		break;
	    case 2:	// window_title
		XtVaSetValues(toplevel, XtNtitle, text, NULL);
		break;
	    case 50:	// font
		screen_newfont(text, False, False);
		break;
	    default:
		break;
	}
#endif


#if defined(WC3270)
	switch (n[0]) {
	    case 0:	// icon name and window title
	    case 2:	// window_title
		screen_title(text);
		break;
	    default:
		break;
	}
#endif
*/

	return DATA;
}

static enum state
ansi_htab_set(H3270 *hSession, int ig1 unused, int ig2 unused)
{
	register int col = h3270.cursor_addr % h3270.cols;

	tabs[col/8] |= 1<<(col%8);
	return DATA;
}

static enum state
ansi_htab_clear(H3270 *hSession, int nn, int ig2 unused)
{
	register int col, i;

	switch (nn) {
	    case 0:
		col = h3270.cursor_addr % h3270.cols;
		tabs[col/8] &= ~(1<<(col%8));
		break;
	    case 3:
		for (i = 0; i < (h3270.cols+7)/8; i++)
			tabs[i] = 0;
		break;
	}
	return DATA;
}

/*
 * Scroll the screen or the scrolling region.
 */
static void
ansi_scroll(void)
{
	held_wrap = False;

	/* Save the top line */
	if (scroll_top == 1 && scroll_bottom == h3270.rows)
	{
//		if (!h3270.is_altbuffer)
//			scroll_save(1, False);
		ctlr_scroll(&h3270);
		return;
	}

	/* Scroll all but the last line up */
	if (scroll_bottom > scroll_top)
		ctlr_bcopy(scroll_top * h3270.cols,
		    (scroll_top - 1) * h3270.cols,
		    (scroll_bottom - scroll_top) * h3270.cols,
		    1);

	/* Clear the last line */
	ctlr_aclear(&h3270, (scroll_bottom - 1) * h3270.cols, h3270.cols, 1);
}

/* Callback for when we enter ANSI mode. */
void ansi_in3270(H3270 *session, int in3270, void *dunno)
{
	if (!in3270)
		(void) ansi_reset(session, 0, 0);
}

#if defined(X3270_DBCS) /*[*/
static void
trace_pending_mb(void)
{
	int i;

	for (i = 0; i < mb_pending; i++) {
		trace_ds(" %02x", mb_buffer[i] & 0xff);
	}
}
#endif /*]*/


/*
 * External entry points
 */
void
ansi_process(unsigned int c)
{
	afn_t fn;

	c &= 0xff;
	ansi_ch = c;

//	scroll_to_bottom();

#if defined(X3270_TRACE) /*[*/
	if (lib3270_get_toggle(&h3270,LIB3270_TOGGLE_SCREEN_TRACE))
		trace_char((char)c);
#endif /*]*/

	fn = ansi_fn[st[(int)state][c]];
#if defined(X3270_DBCS) /*[*/
	if (mb_pending && fn != &ansi_printing) {
		trace_ds("Dropped incomplete multi-byte character");
		trace_pending_mb();
		trace_ds("\n");
		mb_pending = 0;
	}
#endif /*]*/
	state = (*fn)(&h3270, n[0], n[1]);
}

void
ansi_send_up(void)
{
	if (appl_cursor)
		net_sends("\033OA");
	else
		net_sends("\033[A");
}

void
ansi_send_down(void)
{
	if (appl_cursor)
		net_sends("\033OB");
	else
		net_sends("\033[B");
}

void
ansi_send_right(void)
{
	if (appl_cursor)
		net_sends("\033OC");
	else
		net_sends("\033[C");
}

void
ansi_send_left(void)
{
	if (appl_cursor)
		net_sends("\033OD");
	else
		net_sends("\033[D");
}

void
ansi_send_home(void)
{
	net_sends("\033[H");
}

void
ansi_send_clear(void)
{
	net_sends("\033[2K");
}

void
ansi_send_pf(int nn)
{
	static char fn_buf[6];
	static int code[] = {
		/*
		 * F1 through F12 are VT220 codes. (Note the discontinuity --
		 * \E[16~ is missing)
		 */
		11, 12, 13, 14, 15, 17, 18, 19, 20, 21, 23, 24,
		/*
		 * F13 through F20 are defined for xterm.
		 */
		25, 26, 28, 29, 31, 32, 33, 34,
		/*
		 * F21 through F24 are x3270 extensions.
		 */
		35, 36, 37, 38
	};

	if (nn < 1 || nn > sizeof(code)/sizeof(code[0]))
		return;
	(void) sprintf(fn_buf, "\033[%d~", code[nn-1]);
	net_sends(fn_buf);
}

void
ansi_send_pa(int nn)
{
	static char fn_buf[4];
	static char code[4] = { 'P', 'Q', 'R', 'S' };

	if (nn < 1 || nn > 4)
		return;
	(void) sprintf(fn_buf, "\033O%c", code[nn-1]);
	net_sends(fn_buf);
}

void toggle_lineWrap(H3270 *session, struct lib3270_toggle *t unused, LIB3270_TOGGLE_TYPE type unused)
{
	if (lib3270_get_toggle(&h3270,LIB3270_TOGGLE_LINE_WRAP))
		wraparound_mode = 1;
	else
		wraparound_mode = 0;
}

#if defined(X3270_DBCS) /*[*/
/* Accumulate and process pending DBCS characters. */
static int
dbcs_process(int ch, unsigned char ebc[])
{
	UChar Ubuf[2];
	UErrorCode err = U_ZERO_ERROR;

	/* See if we have too many. */
	if (mb_pending >= MB_MAX) {
		trace_ds("Multi-byte character ");
		trace_pending_mb();
		trace_ds(" too long, dropping\n");
		mb_pending = 0;
		return 0;
	}


	/* Store it and see if we're done. */
	mb_buffer[mb_pending++] = ch & 0xff;
	/* An interesting idea. */
	if (mb_pending == 1)
	    	return 0;

	if (mb_to_unicode(mb_buffer, mb_pending, Ubuf, 2, &err) > 0) {
		/* It translated! */
		if (dbcs_map8(Ubuf[0], ebc)) {
			mb_pending = 0;
			return 1;
		} else if (dbcs_map16(Ubuf[0], ebc)) {
			mb_pending = 0;
			return 2;
		} else {
			trace_ds("Can't map multi-byte character");
			trace_pending_mb();
			trace_ds(" -> U+%04x to SBCS or DBCS, dropping\n",
			    Ubuf[0] & 0xffff);
			mb_pending = 0;
			return 0;
		}
	}

	/* It failed.  See why. */
	switch (err) {
	case U_TRUNCATED_CHAR_FOUND:
		/* 'Cause we're not finished. */
		return 0;
	case U_INVALID_CHAR_FOUND:
	case U_ILLEGAL_CHAR_FOUND:
		trace_ds("Invalid multi-byte character");
		trace_pending_mb();
		trace_ds(", dropping\n");
		break;
	default:
		trace_ds("Unexpected ICU error %d translating multi-type "
		    "character", (int)err);
		trace_pending_mb();
		trace_ds(", dropping\n");
		break;
	}
	mb_pending = 0;
	return 0;
}
#endif /*]*/

#endif /*]*/
