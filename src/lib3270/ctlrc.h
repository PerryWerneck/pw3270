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
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como ctlrc.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

/*
 *	ctlrc.h
 *		Global declarations for ctlr.c.
 */

enum pds {
	PDS_OKAY_NO_OUTPUT = 0,	/* command accepted, produced no output */
	PDS_OKAY_OUTPUT = 1,	/* command accepted, produced output */
	PDS_BAD_CMD = -1,	/* command rejected */
	PDS_BAD_ADDR = -2	/* command contained a bad address */
};

LIB3270_INTERNAL void ctlr_aclear(H3270 *session, int baddr, int count, int clear_ea);
LIB3270_INTERNAL void ctlr_add(H3270 *hSession, int baddr, unsigned char c, unsigned char cs);
LIB3270_INTERNAL void ctlr_add_bg(H3270 *hSession, int baddr, unsigned char color);
LIB3270_INTERNAL void ctlr_add_cs(H3270 *hSession, int baddr, unsigned char cs);
LIB3270_INTERNAL void ctlr_add_fa(H3270 *hSession, int baddr, unsigned char fa, unsigned char cs);
LIB3270_INTERNAL void ctlr_add_fg(H3270 *hSession, int baddr, unsigned char color);
LIB3270_INTERNAL void ctlr_add_gr(H3270 *hSession, int baddr, unsigned char gr);
LIB3270_INTERNAL void ctlr_altbuffer(H3270 *session, int alt);
LIB3270_INTERNAL int  ctlr_any_data(H3270 *session);
LIB3270_INTERNAL void ctlr_bcopy(int baddr_from, int baddr_to, int count, int move_ea);
// LIB3270_INTERNAL void ctlr_changed(int bstart, int bend);
LIB3270_INTERNAL void ctlr_clear(H3270 *hSession, Boolean can_snap);
LIB3270_INTERNAL void ctlr_erase_all_unprotected(H3270 *hSession);
LIB3270_INTERNAL void ctlr_init(H3270 *session, unsigned cmask);
LIB3270_INTERNAL void ctlr_read_buffer(H3270 *session, unsigned char aid_byte);
LIB3270_INTERNAL void ctlr_read_modified(unsigned char aid_byte, Boolean all);
LIB3270_INTERNAL void ctlr_reinit(H3270 *session, unsigned cmask);
LIB3270_INTERNAL void ctlr_scroll(H3270 *hSession);
// LIB3270_INTERNAL void ctlr_shrink(void);
// LIB3270_INTERNAL void ctlr_snap_buffer(void);
// LIB3270_INTERNAL Boolean ctlr_snap_modes(void);
LIB3270_INTERNAL void ctlr_wrapping_memmove(H3270 *session, int baddr_to, int baddr_from, int count);
LIB3270_INTERNAL enum pds ctlr_write(unsigned char buf[], int buflen, Boolean erase);
LIB3270_INTERNAL void ctlr_write_sscp_lu(H3270 *session, unsigned char buf[], int buflen);
// LIB3270_INTERNAL struct ea *fa2ea(int baddr);

// LIB3270_INTERNAL Boolean get_bounded_field_attribute(register int baddr, register int bound, unsigned char *fa_out);
LIB3270_INTERNAL void mdt_clear(int baddr);
LIB3270_INTERNAL void mdt_set(int baddr);
LIB3270_INTERNAL int next_unprotected(H3270 *session, int baddr0);
LIB3270_INTERNAL enum pds process_ds(unsigned char *buf, int buflen);
LIB3270_INTERNAL void ps_process(void);

LIB3270_INTERNAL void update_model_info(H3270 *session, int model, int cols, int rows);
LIB3270_INTERNAL void ctlr_set_rows_cols(H3270 *session, int mn, int ovc, int ovr);
LIB3270_INTERNAL void ctlr_erase(H3270 *session, int alt);

LIB3270_INTERNAL void ticking_start(H3270 *session, Boolean anyway);
// LIB3270_INTERNAL void toggle_nop(H3270 *session, struct lib3270_toggle *t, LIB3270_TOGGLE_TYPE tt);
// LIB3270_INTERNAL void toggle_showTiming(struct lib3270_toggle *t, LIB3270_TOGGLE_TYPE tt);

enum dbcs_state {
	DBCS_NONE = 0,		/* position is not DBCS */
	DBCS_LEFT,			/* position is left half of DBCS character */
	DBCS_RIGHT,			/* position is right half of DBCS character */
	DBCS_SI,			/* position is SI terminating DBCS subfield */
	DBCS_SB,			/* position is SBCS character after the SI */
	DBCS_LEFT_WRAP,		/* position is left half of split DBCS */
	DBCS_RIGHT_WRAP,	/* position is right half of split DBCS */
	DBCS_DEAD			/* position is dead left-half DBCS */
};
#define IS_LEFT(d)	((d) == DBCS_LEFT || (d) == DBCS_LEFT_WRAP)
#define IS_RIGHT(d)	((d) == DBCS_RIGHT || (d) == DBCS_RIGHT_WRAP)
#define IS_DBCS(d)	(IS_LEFT(d) || IS_RIGHT(d))

/*
#define MAKE_LEFT(b)	{ \
	if (((b) % COLS) == ((ROWS * COLS) - 1)) \
		ea_buf[(b)].db = DBCS_LEFT_WRAP; \
	else \
		ea_buf[(b)].db = DBCS_LEFT; \
}
#define MAKE_RIGHT(b)	{ \
	if (!((b) % COLS)) \
		ea_buf[(b)].db = DBCS_RIGHT_WRAP; \
	else \
		ea_buf[(b)].db = DBCS_RIGHT; \
}
*/

#define SOSI(c)	(((c) == EBC_so)? EBC_si: EBC_so)

enum dbcs_why { DBCS_FIELD, DBCS_SUBFIELD, DBCS_ATTRIBUTE };

#if defined(X3270_DBCS) /*[*/
LIB3270_INTERNAL enum dbcs_state ctlr_dbcs_state(int baddr);
LIB3270_INTERNAL enum dbcs_state ctlr_lookleft_state(int baddr, enum dbcs_why *why);
LIB3270_INTERNAL int ctlr_dbcs_postprocess(void);
#else /*][*/
	#define ctlr_dbcs_state(b)		DBCS_NONE
	#define ctlr_lookleft_state(b, w)	DBCS_NONE
	#define ctlr_dbcs_postprocess()		0
#endif /*]*/
