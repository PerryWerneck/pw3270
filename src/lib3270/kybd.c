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
 * Este programa está nomeado como kybd.c e possui - linhas de código.
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
 *	kybd.c
 *		This module handles the keyboard for the 3270 emulator.
 */

#include "globals.h"

#ifndef ANDROID
	#include <stdlib.h>
#endif // !ANDROID

#if defined(X3270_DISPLAY) /*[*/
#include <X11/Xatom.h>
#endif
#define XK_3270
#if defined(X3270_APL) /*[*/
#define XK_APL
#endif /*]*/
// #include <X11/keysym.h>

#include <fcntl.h>
#include "3270ds.h"
// #include "appres.h"
// #include "ctlr.h"
#include "resources.h"

#include "actionsc.h"
#include "ansic.h"
//#include "aplc.h"
#include "ctlrc.h"
#include "ftc.h"
#include "hostc.h"
// #include "keypadc.h"
#include "kybdc.h"
#include "popupsc.h"
#include "printc.h"
#include "screenc.h"
// #if defined(X3270_DISPLAY) /*[*/
// #include "selectc.h"
// #endif /*]*/
#include "statusc.h"
#include "tablesc.h"
#include "telnetc.h"
#include "togglesc.h"
#include "trace_dsc.h"
#include "utf8c.h"
#include "utilc.h"
#if defined(X3270_DBCS) /*[*/
#include "widec.h"
#endif /*]*/
#include "api.h"


/*#define KYBDLOCK_TRACE	1*/

/* Statics */
// static enum	{ NONE, COMPOSE, FIRST } composing = NONE;

#ifdef X3270_TRACE
static const char *ia_name[] =
{
	"String", "Paste", "Screen redraw", "Keypad", "Default", "Key",
	"Macro", "Script", "Peek", "Typeahead", "File transfer", "Command",
	"Keymap", "Idle"
};
#endif // X3270_TRACE

static unsigned char pf_xlate[] = {
	AID_PF1,  AID_PF2,  AID_PF3,  AID_PF4,  AID_PF5,  AID_PF6,
	AID_PF7,  AID_PF8,  AID_PF9,  AID_PF10, AID_PF11, AID_PF12,
	AID_PF13, AID_PF14, AID_PF15, AID_PF16, AID_PF17, AID_PF18,
	AID_PF19, AID_PF20, AID_PF21, AID_PF22, AID_PF23, AID_PF24
};
static unsigned char pa_xlate[] = {
	AID_PA1, AID_PA2, AID_PA3
};
#define PF_SZ	(sizeof(pf_xlate)/sizeof(pf_xlate[0]))
#define PA_SZ	(sizeof(pa_xlate)/sizeof(pa_xlate[0]))
// static void * unlock_id;
// static time_t unlock_delay_time;
#define UNLOCK_MS		350	/* 0.35s after last unlock */
static Boolean key_Character(int code, Boolean with_ge, Boolean pasting,Boolean *skipped);
static Boolean flush_ta(void);
static void key_AID(H3270 *session, unsigned char aid_code);
static void kybdlock_set(unsigned int bits, const char *cause);
// static KeySym MyStringToKeysym(char *s, enum keytype *keytypep);

/*
#if defined(X3270_DBCS)
Boolean key_WCharacter(unsigned char code[], Boolean *skipped);
#endif
*/

static int nxk = 0;
static struct xks {
	KeySym key;
	KeySym assoc;
} *xk;

// static Boolean		reverse = False;	/* reverse-input mode */

/* Globals */
// unsigned int	kybdlock = KL_NOT_CONNECTED;
//unsigned char	aid = AID_NO;		/* current attention ID */

/* Composite key mappings. */

struct akeysym {
	KeySym keysym;
	enum keytype keytype;
};
/*
static struct akeysym cc_first;
static struct composite {
	struct akeysym k1, k2;
	struct akeysym translation;
} *composites = NULL;
static int n_composites = 0;
*/

#define ak_eq(k1, k2)	(((k1).keysym  == (k2).keysym) && \
			 ((k1).keytype == (k2).keytype))

static struct ta
{
	struct ta 		*next;

	enum _ta_type
	{
		TA_TYPE_DEFAULT,
		TA_TYPE_KEY_AID,
		TA_TYPE_USER
	} type;

	H3270 * session;
	void (*fn)(H3270 *, const char *, const char *);
	char *parm[2];
	unsigned char aid_code;
} *ta_head = (struct ta *) NULL,
  *ta_tail = (struct ta *) NULL;

#define ENQUEUE_ACTION(x) enq_ta(hSession, (void (*)(H3270 *, const char *, const char *)) x, NULL, NULL)

static const char dxl[] = "0123456789abcdef";
#define FROM_HEX(c)	(strchr(dxl, tolower(c)) - dxl)
#define KYBDLOCK_IS_OERR	(h3270.kybdlock && !(h3270.kybdlock & ~KL_OERR_MASK))


/*
 * Check if the typeahead queue is available
 */
static int enq_chk(void)
{
	/* If no connection, forget it. */
	if (!CONNECTED)
	{
		trace_event("  dropped (not connected)\n");
		return -1;
	}

	/* If operator error, complain and drop it. */
	if (h3270.kybdlock & KL_OERR_MASK)
	{
		lib3270_ring_bell(NULL);
		trace_event("  dropped (operator error)\n");
		return -1;
	}

	/* If scroll lock, complain and drop it. */
	if (h3270.kybdlock & KL_SCROLLED)
	{
		lib3270_ring_bell(&h3270);
		trace_event("  dropped (scrolled)\n");
		return -1;
	}

	/* If typeahead disabled, complain and drop it. */
	if (!h3270.typeahead)
	{
		trace_event("  dropped (no typeahead)\n");
		return -1;
	}

	return 0;
}

/*
 * Put a "Key-aid" on the typeahead queue
 */
 static void enq_key(unsigned char aid_code)
 {
	struct ta *ta;

 	if(enq_chk())
		return;

	ta = (struct ta *) lib3270_malloc(sizeof(*ta));
	ta->next 		= (struct ta *) NULL;
	ta->type 		= TA_TYPE_KEY_AID;
	ta->aid_code	= aid_code;

	trace("Adding key %02x on queue",(int) aid_code);

	if (ta_head)
	{
		ta_tail->next = ta;
	}
	else
	{
		ta_head = ta;
		status_typeahead(&h3270,True);
	}
	ta_tail = ta;

	trace_event("  Key-aid queued (kybdlock 0x%x)\n", h3270.kybdlock);
 }

/*
 * Put an action on the typeahead queue.
 */
static void enq_ta(H3270 *hSession, void (*fn)(H3270 *, const char *, const char *), const char *parm1, const char *parm2)
{
	struct ta *ta;

 	if(enq_chk())
		return;

	CHECK_SESSION_HANDLE(hSession);

	ta = (struct ta *) lib3270_malloc(sizeof(*ta));
	ta->session	= hSession;
	ta->next	= (struct ta *) NULL;
	ta->type	= TA_TYPE_DEFAULT;
	ta->fn		= fn;

	if (parm1)
		ta->parm[0] = NewString(parm1);

	if (parm2)
		ta->parm[1] = NewString(parm2);

	if (ta_head)
	{
		ta_tail->next = ta;
	}
	else
	{
		ta_head = ta;
		status_typeahead(hSession,True);
	}
	ta_tail = ta;

	trace_event("  action queued (kybdlock 0x%x)\n", h3270.kybdlock);
}

/*
 * Execute an action from the typeahead queue.
 */
Boolean run_ta(void)
{
	struct ta *ta;

	if (h3270.kybdlock || (ta = ta_head) == (struct ta *)NULL)
		return False;

	if ((ta_head = ta->next) == (struct ta *)NULL)
	{
		ta_tail = (struct ta *)NULL;
		status_typeahead(&h3270,False);
	}

	switch(ta->type)
	{
	case TA_TYPE_DEFAULT:
		ta->fn(ta->session,ta->parm[0],ta->parm[1]);
		lib3270_free(ta->parm[0]);
		lib3270_free(ta->parm[1]);
		break;

	case TA_TYPE_KEY_AID:
		trace("Sending enqueued key %02x",ta->aid_code);
		key_AID(ta->session,ta->aid_code);
		break;

	default:
		popup_an_error(ta->session, _( "Unexpected type %d in typeahead queue" ), ta->type);

	}

	lib3270_free(ta);

	return True;
}

/*
 * Flush the typeahead queue.
 * Returns whether or not anything was flushed.
 */
static Boolean
flush_ta(void)
{
	struct ta *ta, *next;
	Boolean any = False;

	for (ta = ta_head; ta != (struct ta *) NULL; ta = next)
	{
		lib3270_free(ta->parm[0]);
		lib3270_free(ta->parm[1]);
		next = ta->next;
		lib3270_free(ta);
		any = True;
	}
	ta_head = ta_tail = (struct ta *) NULL;
	status_typeahead(&h3270,False);
	return any;
}

/* Set bits in the keyboard lock. */
static void
kybdlock_set(unsigned int bits, const char *cause unused)
{
	unsigned int n;

	n = h3270.kybdlock | bits;
	if (n != h3270.kybdlock) {
#if defined(KYBDLOCK_TRACE) /*[*/
	       trace_event("  %s: kybdlock |= 0x%04x, 0x%04x -> 0x%04x\n",
		    cause, bits, kybdlock, n);
#endif /*]*/
		if ((h3270.kybdlock ^ bits) & KL_DEFERRED_UNLOCK) {
			/* Turned on deferred unlock. */
			h3270.unlock_delay_time = time(NULL);
		}
		h3270.kybdlock = n;
		status_changed(&h3270,LIB3270_STATUS_KYBDLOCK);
	}
}

/* Clear bits in the keyboard lock. */
void
kybdlock_clr(H3270 *session, unsigned int bits, const char *cause unused)
{
	unsigned int n;

	n = session->kybdlock & ~bits;

	if (n != session->kybdlock)
	{
#if defined(KYBDLOCK_TRACE)
		trace_event("  %s: kybdlock &= ~0x%04x, 0x%04x -> 0x%04x\n",cause, bits, kybdlock, n);
#endif
		if ((session->kybdlock ^ n) & KL_DEFERRED_UNLOCK)
		{
			/* Turned off deferred unlock. */
			session->unlock_delay_time = 0;
		}
		session->kybdlock = n;
		status_changed(session,LIB3270_STATUS_KYBDLOCK);
	}
}

/**
 * Set or clear enter-inhibit mode.
 *
 * @param session	Session handle
 * @param inhibit	New state
 *
 */
void kybd_inhibit(H3270 *session, Boolean inhibit)
{
	if (inhibit)
	{
		kybdlock_set(KL_ENTER_INHIBIT, "kybd_inhibit");
		if (session->kybdlock == KL_ENTER_INHIBIT)
			status_reset(session);
	}
	else
	{
		kybdlock_clr(session,KL_ENTER_INHIBIT, "kybd_inhibit");
		if (!session->kybdlock)
			status_reset(session);
	}
}

/*
 * Called when a host connects or disconnects.
 */
void kybd_connect(H3270 *session, int connected, void *dunno)
{
	if (session->kybdlock & KL_DEFERRED_UNLOCK)
		RemoveTimeOut(session->unlock_id);

	kybdlock_clr(session, -1, "kybd_connect");

	if (connected) {
		/* Wait for any output or a WCC(restore) from the host */
		kybdlock_set(KL_AWAITING_FIRST, "kybd_connect");
	} else {
		kybdlock_set(KL_NOT_CONNECTED, "kybd_connect");
		(void) flush_ta();
	}
}

/*
 * Called when we switch between 3270 and ANSI modes.
 */
void kybd_in3270(H3270 *session, int in3270 unused, void *dunno)
{
	if (session->kybdlock & KL_DEFERRED_UNLOCK)
		RemoveTimeOut(session->unlock_id);
	kybdlock_clr(session,~KL_AWAITING_FIRST, "kybd_in3270");

	/* There might be a macro pending. */
	if (CONNECTED)
		ps_process();
}

/*
 * Called to initialize the keyboard logic.
 */ /*
void kybd_init(void)
{
	// Register kybd interest in connect and disconnect events.
	trace("%s",__FUNCTION__);
	lib3270_register_schange(NULL,LIB3270_STATE_CONNECT,kybd_connect,NULL);
	lib3270_register_schange(NULL,LIB3270_STATE_3270_MODE,kybd_in3270,NULL);
}
*/

/*
 * Toggle reverse mode.
 */
 /*
static void
reverse_mode(Boolean on)
{
	reverse = on;
	status_reverse_mode(on);
}
*/

/*
 * Lock the keyboard because of an operator error.
 */
static void
operator_error(int error_type)
{
//	if (sms_redirect())
//		popup_an_error("Keyboard locked");

	if(h3270.oerr_lock)
	{
		status_oerr(NULL,error_type);
		mcursor_locked(&h3270);
		kybdlock_set((unsigned int)error_type, "operator_error");
		flush_ta();
	}
	else
	{
		lib3270_ring_bell(NULL);
	}
}


/*
 * Handle an AID (Attention IDentifier) key.  This is the common stuff that
 * gets executed for all AID keys (PFs, PAs, Clear and etc).
 */
static void key_AID(H3270 *session, unsigned char aid_code)
{
#if defined(X3270_ANSI) /*[*/
	if (IN_ANSI) {
		register unsigned i;

		trace("aid_code: %02x IN_ANSI: %d",aid_code,IN_ANSI);

		if (aid_code == AID_ENTER) {
			net_sendc('\r');
			return;
		}
		for (i = 0; i < PF_SZ; i++)
			if (aid_code == pf_xlate[i]) {
				ansi_send_pf(i+1);
				return;
			}
		for (i = 0; i < PA_SZ; i++)
			if (aid_code == pa_xlate[i]) {
				ansi_send_pa(i+1);
				return;
			}
		return;
	}
#endif /*]*/

#if defined(X3270_PLUGIN) /*[*/
	plugin_aid(aid_code);
#endif /*]*/

	trace("IN_SSCP: %d cursor_addr: %d",IN_SSCP,h3270.cursor_addr);

	if (IN_SSCP) {
		if (h3270.kybdlock & KL_OIA_MINUS)
			return;
		if (aid_code != AID_ENTER && aid_code != AID_CLEAR) {
			status_changed(&h3270,LIB3270_STATUS_MINUS);
			kybdlock_set(KL_OIA_MINUS, "key_AID");
			return;
		}
	}
	if (IN_SSCP && aid_code == AID_ENTER)
	{
		/* Act as if the host had written our input. */
		h3270.buffer_addr = h3270.cursor_addr;
	}
	if (!IN_SSCP || aid_code != AID_CLEAR)
	{
		status_twait(&h3270);
		mcursor_waiting(&h3270);
		lib3270_set_toggle(&h3270,LIB3270_TOGGLE_INSERT,0);
		kybdlock_set(KL_OIA_TWAIT | KL_OIA_LOCKED, "key_AID");
	}
	h3270.aid = aid_code;
	ctlr_read_modified(h3270.aid, False);
	ticking_start(&h3270,False);
	status_ctlr_done(&h3270);
}

LIB3270_FKEY_ACTION( pfkey )
{

	if (key < 1 || key > PF_SZ)
		return EINVAL;

	if (hSession->kybdlock & KL_OIA_MINUS)
		return -1;
	else if (hSession->kybdlock)
		enq_key(pf_xlate[key-1]);
	else
		key_AID(hSession,pf_xlate[key-1]);

	return 0;
}

LIB3270_FKEY_ACTION( pakey )
{
	if (key < 1 || key > PA_SZ)
	{
		return EINVAL;
	}

	if (hSession->kybdlock & KL_OIA_MINUS)
		return -1;
	else if (hSession->kybdlock)
		enq_key(pa_xlate[key-1]);
	else
		key_AID(hSession,pa_xlate[key-1]);

	return 0;
}

LIB3270_ACTION(break)
{
	if (!IN_3270)
		return 0;

	net_break();

	return 0;
}

/*
 * ATTN key, per RFC 2355.  Sends IP, regardless.
 */
LIB3270_ACTION(attn)
{
	if (!IN_3270)
		return 0;

	net_interrupt();

	return 0;
}

/*
 * IAC IP, which works for 5250 System Request and interrupts the program
 * on an AS/400, even when the keyboard is locked.
 *
 * This is now the same as the Attn action.
 */ /*
void
Interrupt_action(Widget w unused, XEvent *event, String *params,
    Cardinal *num_params)
{
	if (!IN_3270)
		return;

//	reset_idle_timer();

	net_interrupt();
}

*/

/*
 * Prepare for an insert of 'count' bytes.
 * Returns True if the insert is legal, False otherwise.
 */
static Boolean ins_prep(int faddr, int baddr, int count)
{
	int next_faddr;
	int xaddr;
	int need;
	int ntb;
	int tb_start = -1;
	int copy_len;

	/* Find the end of the field. */
	if (faddr == -1) {
		/* Unformatted.  Use the end of the line. */
		next_faddr = (((baddr / h3270.cols) + 1) * h3270.cols) % (h3270.rows*h3270.cols);
	} else {
		next_faddr = faddr;
		INC_BA(next_faddr);
		while (next_faddr != faddr && !h3270.ea_buf[next_faddr].fa) {
			INC_BA(next_faddr);
		}
	}

	/* Are there enough NULLs or trailing blanks available? */
	xaddr = baddr;
	need = count;
	ntb = 0;
	while (need && (xaddr != next_faddr)) {
		if (h3270.ea_buf[xaddr].cc == EBC_null)
			need--;
		else if (lib3270_get_toggle(&h3270,LIB3270_TOGGLE_BLANK_FILL) &&
			((h3270.ea_buf[xaddr].cc == EBC_space) ||
			 (h3270.ea_buf[xaddr].cc == EBC_underscore))) {
			if (tb_start == -1)
				tb_start = xaddr;
			ntb++;
		} else {
			tb_start = -1;
			ntb = 0;
		}
		INC_BA(xaddr);
	}
#if defined(_ST) /*[*/
	printf("need %d at %d, tb_start at %d\n", count, baddr, tb_start);
#endif /*]*/
	if (need - ntb > 0) {
		operator_error(KL_OERR_OVERFLOW);
		return False;
	}

	/*
	 * Shift the buffer to the right until we've consumed the available
	 * (and needed) NULLs.
	 */
	need = count;
	xaddr = baddr;
	while (need && (xaddr != next_faddr)) {
		int n_nulls = 0;
		int first_null = -1;

		while (need &&
		       ((h3270.ea_buf[xaddr].cc == EBC_null) ||
		        (tb_start >= 0 && xaddr >= tb_start))) {
			need--;
			n_nulls++;
			if (first_null == -1)
				first_null = xaddr;
			INC_BA(xaddr);
		}
		if (n_nulls) {
			int to;

			/* Shift right n_nulls worth. */
			copy_len = first_null - baddr;
			if (copy_len < 0)
				copy_len += h3270.rows*h3270.cols;
			to = (baddr + n_nulls) % (h3270.rows*h3270.cols);
#if defined(_ST) /*[*/
			printf("found %d NULLs at %d\n", n_nulls, first_null);
			printf("copying %d from %d to %d\n", copy_len, to,
			    first_null);
#endif /*]*/
			if (copy_len)
				ctlr_wrapping_memmove(to, baddr, copy_len);
		}
		INC_BA(xaddr);
	}

	return True;

}

#define GE_WFLAG	0x100
#define PASTE_WFLAG	0x200

static void key_Character_wrapper(H3270 *param, const char *param1, const char *param2)
{
	int code;
	Boolean with_ge = False;
	Boolean pasting = False;

	code = atoi(param1);

	if (code & GE_WFLAG)
	{
		with_ge = True;
		code &= ~GE_WFLAG;
	}

	if (code & PASTE_WFLAG)
	{
		pasting = True;
		code &= ~PASTE_WFLAG;
	}

//	trace_event(" %s -> Key(%s\"%s\")\n",ia_name[(int) ia_cause],with_ge ? "GE " : "",ctl_see((int) ebc2asc[code]));

	(void) key_Character(code, with_ge, pasting, NULL);
}

/*
 * Handle an ordinary displayable character key.  Lots of stuff to handle
 * insert-mode, protected fields and etc.
 */
static Boolean key_Character(int code, Boolean with_ge, Boolean pasting, Boolean *skipped)
{
	register int	baddr, faddr, xaddr;
	register unsigned char	fa;
	enum dbcs_why why;

	if (skipped != NULL)
		*skipped = False;

	if (h3270.kybdlock)
	{
		char codename[64];

		(void) sprintf(codename, "%d", code |(with_ge ? GE_WFLAG : 0) | (pasting ? PASTE_WFLAG : 0));
		enq_ta(&h3270,key_Character_wrapper, codename, CN);
		return False;
	}
	baddr = h3270.cursor_addr;
	faddr = find_field_attribute(&h3270,baddr);
	fa = get_field_attribute(&h3270,baddr);
	if (h3270.ea_buf[baddr].fa || FA_IS_PROTECTED(fa)) {
		operator_error(KL_OERR_PROTECTED);
		return False;
	}
	if (h3270.numeric_lock && FA_IS_NUMERIC(fa) &&
	    !((code >= EBC_0 && code <= EBC_9) ||
	      code == EBC_minus || code == EBC_period)) {
		operator_error(KL_OERR_NUMERIC);
		return False;
	}

	/* Can't put an SBCS in a DBCS field. */
	if (h3270.ea_buf[faddr].cs == CS_DBCS) {
		operator_error(KL_OERR_DBCS);
		return False;
	}

	/* If it's an SI (end of DBCS subfield), move over one position. */
	if (h3270.ea_buf[baddr].cc == EBC_si) {
		INC_BA(baddr);
		if (baddr == faddr) {
			operator_error(KL_OERR_OVERFLOW);
			return False;
		}
	}

	/* Add the character. */
	if (h3270.ea_buf[baddr].cc == EBC_so) {

		if (lib3270_get_toggle(&h3270,LIB3270_TOGGLE_INSERT)) {
			if (!ins_prep(faddr, baddr, 1))
				return False;
		} else {
			Boolean was_si = False;

			/*
			 * Overwriting an SO (start of DBCS subfield).
			 * If it's followed by an SI, replace the SO/SI
			 * pair with x/space.  If not, replace it and
			 * the following DBCS character with
			 * x/space/SO.
			 */
			xaddr = baddr;
			INC_BA(xaddr);
			was_si = (h3270.ea_buf[xaddr].cc == EBC_si);
			ctlr_add(xaddr, EBC_space, CS_BASE);
			ctlr_add_fg(xaddr, 0);
#if defined(X3270_ANSI) /*[*/
			ctlr_add_bg(xaddr, 0);
#endif /*]*/
			if (!was_si) {
				INC_BA(xaddr);
				ctlr_add(xaddr, EBC_so, CS_BASE);
				ctlr_add_fg(xaddr, 0);
#if defined(X3270_ANSI) /*[*/
				ctlr_add_bg(xaddr, 0);
#endif /*]*/
			}
		}

	} else switch (ctlr_lookleft_state(baddr, &why)) {
	case DBCS_RIGHT:
		DEC_BA(baddr);
		/* fall through... */
	case DBCS_LEFT:
		if (why == DBCS_ATTRIBUTE) {
			if (lib3270_get_toggle(&h3270,LIB3270_TOGGLE_INSERT)) {
				if (!ins_prep(faddr, baddr, 1))
					return False;
			} else {
				/*
				 * Replace single DBCS char with
				 * x/space.
				 */
				xaddr = baddr;
				INC_BA(xaddr);
				ctlr_add(xaddr, EBC_space, CS_BASE);
				ctlr_add_fg(xaddr, 0);
				ctlr_add_gr(xaddr, 0);
			}
		} else {
			Boolean was_si;

			if (lib3270_get_toggle(&h3270,LIB3270_TOGGLE_INSERT)) {
				/*
				 * Inserting SBCS into a DBCS subfield.
				 * If this is the first position, we
				 * can just insert one character in
				 * front of the SO.  Otherwise, we'll
				 * need room for SI (to end subfield),
				 * the character, and SO (to begin the
				 * subfield again).
				 */
				xaddr = baddr;
				DEC_BA(xaddr);
				if (h3270.ea_buf[xaddr].cc == EBC_so) {
					DEC_BA(baddr);
					if (!ins_prep(faddr, baddr, 1))
						return False;
				} else {
					if (!ins_prep(faddr, baddr, 3))
						return False;
					xaddr = baddr;
					ctlr_add(xaddr, EBC_si,
					    CS_BASE);
					ctlr_add_fg(xaddr, 0);
					ctlr_add_gr(xaddr, 0);
					INC_BA(xaddr);
					INC_BA(baddr);
					INC_BA(xaddr);
					ctlr_add(xaddr, EBC_so,
					    CS_BASE);
					ctlr_add_fg(xaddr, 0);
					ctlr_add_gr(xaddr, 0);
				}
			} else {
				/* Overwriting part of a subfield. */
				xaddr = baddr;
				ctlr_add(xaddr, EBC_si, CS_BASE);
				ctlr_add_fg(xaddr, 0);
				ctlr_add_gr(xaddr, 0);
				INC_BA(xaddr);
				INC_BA(baddr);
				INC_BA(xaddr);
				was_si = (h3270.ea_buf[xaddr].cc == EBC_si);
				ctlr_add(xaddr, EBC_space, CS_BASE);
				ctlr_add_fg(xaddr, 0);
				ctlr_add_gr(xaddr, 0);
				if (!was_si) {
					INC_BA(xaddr);
					ctlr_add(xaddr, EBC_so,
					    CS_BASE);
					ctlr_add_fg(xaddr, 0);
					ctlr_add_gr(xaddr, 0);
				}
			}
		}
		break;
	default:
	case DBCS_NONE:
		if (lib3270_get_toggle(&h3270,LIB3270_TOGGLE_INSERT) && !ins_prep(faddr, baddr, 1))
			return False;
		break;
	}
	ctlr_add(baddr, (unsigned char)code,(unsigned char)(with_ge ? CS_GE : 0));
	ctlr_add_fg(baddr, 0);
	ctlr_add_gr(baddr, 0);
	INC_BA(baddr);

	/* Replace leading nulls with blanks, if desired. */
	if (h3270.formatted && lib3270_get_toggle(&h3270,LIB3270_TOGGLE_BLANK_FILL)) {
		register int	baddr_fill = baddr;

		DEC_BA(baddr_fill);
		while (baddr_fill != faddr) {

			/* Check for backward line wrap. */
			if ((baddr_fill % h3270.cols) == h3270.cols - 1) {
				Boolean aborted = True;
				register int baddr_scan = baddr_fill;

				/*
				 * Check the field within the preceeding line
				 * for NULLs.
				 */
				while (baddr_scan != faddr) {
					if (h3270.ea_buf[baddr_scan].cc != EBC_null) {
						aborted = False;
						break;
					}
					if (!(baddr_scan % h3270.cols))
						break;
					DEC_BA(baddr_scan);
				}
				if (aborted)
					break;
			}

			if (h3270.ea_buf[baddr_fill].cc == EBC_null)
				ctlr_add(baddr_fill, EBC_space, 0);
			DEC_BA(baddr_fill);
		}
	}

	mdt_set(h3270.cursor_addr);

	/*
	 * Implement auto-skip, and don't land on attribute bytes.
	 * This happens for all pasted data (even DUP), and for all
	 * keyboard-generated data except DUP.
	 */
	if (pasting || (code != EBC_dup)) {
		while (h3270.ea_buf[baddr].fa) {
			if (skipped != NULL)
				*skipped = True;
			if (FA_IS_SKIP(h3270.ea_buf[baddr].fa))
				baddr = next_unprotected(&h3270,baddr);
			else
				INC_BA(baddr);
		}
		cursor_move(&h3270,baddr);
	}

	(void) ctlr_dbcs_postprocess();
	return True;
}

/*
#if defined(X3270_DBCS)
static void
key_WCharacter_wrapper(Widget w unused, XEvent *event unused, String *params, Cardinal *num_params unused)
{
	int code;
	unsigned char codebuf[2];

	code = atoi(params[0]);
	trace_event(" %s -> Key(0x%04x)\n",
	    ia_name[(int) ia_cause], code);
	codebuf[0] = (code >> 8) & 0xff;
	codebuf[1] = code & 0xff;
	(void) key_WCharacter(codebuf, NULL);
}

//
// Input a DBCS character.
// Returns True if a character was stored in the buffer, False otherwise.
//
Boolean key_WCharacter(unsigned char code[], Boolean *skipped)
{
	int baddr;
	register unsigned char fa;
	int faddr;
	enum dbcs_state d;
	int xaddr;
	Boolean done = False;
	Boolean no_si = False;
	extern unsigned char reply_mode; // XXX

	reset_idle_timer();

	if (kybdlock) {
		char codename[64];

		(void) sprintf(codename, "%d", (code[0] << 8) | code[1]);
		enq_ta(key_WCharacter_wrapper, codename, CN);
		return False;
	}

	if (skipped != NULL)
		*skipped = False;

	// In DBCS mode?
	if (!dbcs) {
		trace_event("DBCS character received when not in DBCS mode, "
		    "ignoring.\n");
		return True;
	}

#if defined(X3270_ANSI)
	// In ANSI mode?
	if (IN_ANSI) {
	    char mb[16];

	    dbcs_to_mb(code[0], code[1], mb);
	    net_sends(mb);
	    return True;
	}
#endif

	baddr = cursor_addr;
	fa = get_field_attribute(baddr);
	faddr = find_field_attribute(baddr);

	// Protected?
	if (ea_buf[baddr].fa || FA_IS_PROTECTED(fa)) {
		operator_error(KL_OERR_PROTECTED);
		return False;
	}

	// Numeric?
	if (h3270.numeric_lock && FA_IS_NUMERIC(fa)) {
		operator_error(KL_OERR_NUMERIC);
		return False;
	}

	//
	// Figure our what to do based on the DBCS state of the buffer.
	// Leaves baddr pointing to the next unmodified position.
	//
retry:
	switch (d = ctlr_dbcs_state(baddr)) {
	case DBCS_RIGHT:
	case DBCS_RIGHT_WRAP:
		// Back up one position and process it as a LEFT.
		DEC_BA(baddr);
		// fall through...
	case DBCS_LEFT:
	case DBCS_LEFT_WRAP:
		// Overwrite the existing character.
		if (insert) {
			if (!ins_prep(faddr, baddr, 2)) {
				return False;
			}
		}
		ctlr_add(baddr, code[0], ea_buf[baddr].cs);
		INC_BA(baddr);
		ctlr_add(baddr, code[1], ea_buf[baddr].cs);
		INC_BA(baddr);
		done = True;
		break;
	case DBCS_SB:
		// Back up one position and process it as an SI.
		DEC_BA(baddr);
		// fall through...
	case DBCS_SI:
		// Extend the subfield to the right.
		if (insert) {
			if (!ins_prep(faddr, baddr, 2)) {
				return False;
			}
		} else {
			// Don't overwrite a field attribute or an SO.
			xaddr = baddr;
			INC_BA(xaddr);	// C1
			if (ea_buf[xaddr].fa)
				break;
			if (ea_buf[xaddr].cc == EBC_so)
				no_si = True;
			INC_BA(xaddr);	// SI
			if (ea_buf[xaddr].fa || ea_buf[xaddr].cc == EBC_so)
				break;
		}
		ctlr_add(baddr, code[0], ea_buf[baddr].cs);
		INC_BA(baddr);
		ctlr_add(baddr, code[1], ea_buf[baddr].cs);
		if (!no_si) {
			INC_BA(baddr);
			ctlr_add(baddr, EBC_si, ea_buf[baddr].cs);
		}
		done = True;
		break;
	case DBCS_DEAD:
		break;
	case DBCS_NONE:
		if (ea_buf[faddr].ic) {
			Boolean extend_left = FALSE;

			// Is there room?
			if (insert) {
				if (!ins_prep(faddr, baddr, 4)) {
					return False;
				}
			} else {
				xaddr = baddr;	// baddr, SO
				if (ea_buf[xaddr].cc == EBC_so) {
					//
					// (baddr), where we would have put the
					// SO, is already an SO.  Move to
					// (baddr+1) and try again.
					//
#if defined(DBCS_RIGHT_DEBUG)
					printf("SO in position 0\n");
#endif
					INC_BA(baddr);
					goto retry;
				}

				INC_BA(xaddr);	// baddr+1, C0
				if (ea_buf[xaddr].fa)
					break;
				if (ea_buf[xaddr].cc == EBC_so) {
					enum dbcs_state e;

					//
					// (baddr+1), where we would have put
					// the left side of the DBCS, is a SO.
					// If there's room, we can extend the
					// subfield to the left.  If not, we're
					// stuck.
					//
					DEC_BA(xaddr);
					DEC_BA(xaddr);
					e = ctlr_dbcs_state(xaddr);
					if (e == DBCS_NONE || e == DBCS_SB) {
						extend_left = True;
						no_si = True;
#if defined(DBCS_RIGHT_DEBUG)
						printf("SO in position 1, "
							"extend left\n");
#endif
					} else {
						//
						// Won't actually happen,
						// because this implies that
						// the buffer addr at baddr
						// is an SB.
						//
#if defined(DBCS_RIGHT_DEBUG)
						printf("SO in position 1, "
							"no room on left, "
							"fail\n");
#endif
						break;
					}
				}

				INC_BA(xaddr); // baddr+2, C1
				if (ea_buf[xaddr].fa)
					break;
				if (ea_buf[xaddr].cc == EBC_so) {
					//
					// (baddr+2), where we want to put the
					// right half of the DBCS character, is
					// a SO.  This is a natural extension
					// to the left -- just make sure we
					// don't write an SI.
					//
					no_si = True;
#if defined(DBCS_RIGHT_DEBUG)
					printf("SO in position 2, no SI\n");
#endif
				}

				//
				// Check the fourth position only if we're
				// not doing an extend-left.
				///
				if (!no_si) {
					INC_BA(xaddr); // baddr+3, SI
					if (ea_buf[xaddr].fa)
						break;
					if (ea_buf[xaddr].cc == EBC_so) {
						//
						// (baddr+3), where we want to
						// put an
						// SI, is an SO.  Forget it.
						//
#if defined(DBCS_RIGHT_DEBUG)
						printf("SO in position 3, "
							"retry right\n");
						INC_BA(baddr);
						goto retry;
#endif
						break;
					}
				}
			}
			// Yes, add it.
			if (extend_left)
				DEC_BA(baddr);
			ctlr_add(baddr, EBC_so, ea_buf[baddr].cs);
			INC_BA(baddr);
			ctlr_add(baddr, code[0], ea_buf[baddr].cs);
			INC_BA(baddr);
			ctlr_add(baddr, code[1], ea_buf[baddr].cs);
			if (!no_si) {
				INC_BA(baddr);
				ctlr_add(baddr, EBC_si, ea_buf[baddr].cs);
			}
			done = True;
		} else if (reply_mode == SF_SRM_CHAR) {
			// Use the character attribute.
			if (insert) {
				if (!ins_prep(faddr, baddr, 2)) {
					return False;
				}
			} else {
				xaddr = baddr;
				INC_BA(xaddr);
				if (ea_buf[xaddr].fa)
					break;
			}
			ctlr_add(baddr, code[0], CS_DBCS);
			INC_BA(baddr);
			ctlr_add(baddr, code[1], CS_DBCS);
			INC_BA(baddr);
			done = True;
		}
		break;
	}

	if (done) {
		// Implement blank fill mode.
		if (lib3270_get_toggle(&h3270,LIB3270_TOGGLE_BLANK_FILL)) {
			xaddr = faddr;
			INC_BA(xaddr);
			while (xaddr != baddr) {
				if (ea_buf[xaddr].cc == EBC_null)
					ctlr_add(xaddr, EBC_space, CS_BASE);
				else
					break;
				INC_BA(xaddr);
			}
		}

		mdt_set(cursor_addr);

		// Implement auto-skip.
		while (ea_buf[baddr].fa) {
			if (skipped != NULL)
				*skipped = True;
			if (FA_IS_SKIP(ea_buf[baddr].fa))
				baddr = next_unprotected(&h3270,baddr);
			else
				INC_BA(baddr);
		}
		cursor_move(baddr);
		(void) ctlr_dbcs_postprocess();
		return True;
	} else {
		operator_error(KL_OERR_DBCS);
		return False;
	}
}
#endif
*/

/*
 * Handle an ordinary character key, given an ASCII code.
 */
void key_ACharacter(unsigned char c, enum keytype keytype, enum iaction cause,Boolean *skipped)
{
	if (skipped != NULL)
		*skipped = False;

	trace_event(" %s -> Key(\"%s\")\n",ia_name[(int) cause], ctl_see((int) c));

	if (IN_3270)
	{
		if (c < ' ')
		{
			trace_event("  dropped (control char)\n");
			return;
		}
		(void) key_Character((int) asc2ebc[c], keytype == KT_GE, False, skipped);
	}
#if defined(X3270_ANSI) /*[*/
	else if (IN_ANSI)
	{
		net_sendc((char) c);
	}
#endif /*]*/
	else
	{
		trace_event("  dropped (not connected)\n");
	}
}


/*
 * Simple toggles.
 */
/*
#if defined(X3270_DISPLAY)
void
AltCursor_action(Widget w unused, XEvent *event, String *params,
    Cardinal *num_params)
{
	reset_idle_timer();
	do_toggle(ALT_CURSOR);
}
#endif
*/

/*
void
MonoCase_action(Widget w unused, XEvent *event, String *params,
    Cardinal *num_params)
{
	reset_idle_timer();
	do_toggle(MONOCASE);
}
*/

/*
 * Flip the display left-to-right
 */
 /*
void
Flip_action(Widget w unused, XEvent *event, String *params,
    Cardinal *num_params)
{

//	reset_idle_timer();

	screen_flip();
}
*/


/*
 * Tab forward to next field.
 */
/*
void
Tab_action(Widget w unused, XEvent *event, String *params, Cardinal *num_params)
{
	action_NextField();
}
*/

LIB3270_KEY_ACTION( tab )
{

//	reset_idle_timer();

	if (hSession->kybdlock) {
		if (KYBDLOCK_IS_OERR) {
			kybdlock_clr(hSession,KL_OERR_MASK, "Tab");
			status_reset(hSession);
		} else {
			ENQUEUE_ACTION(lib3270_tab);
			return 0;
		}
	}
#if defined(X3270_ANSI) /*[*/
	if (IN_ANSI) {
		net_sendc('\t');
		return 0;
	}
#endif /*]*/
	cursor_move(hSession,next_unprotected(hSession,hSession->cursor_addr));
	return 0;
}


/*
 * Tab backward to previous field.
 */
LIB3270_KEY_ACTION( backtab )
{
	register int	baddr, nbaddr;
	int		sbaddr;

//	reset_idle_timer();

	if (hSession->kybdlock) {
		if (KYBDLOCK_IS_OERR) {
			kybdlock_clr(hSession,KL_OERR_MASK, "BackTab");
			status_reset(hSession);
		} else {
			ENQUEUE_ACTION(lib3270_backtab);
			return 0;
		}
	}
	if (!IN_3270)
		return 0;
	baddr = hSession->cursor_addr;
	DEC_BA(baddr);
	if (hSession->ea_buf[baddr].fa)	/* at bof */
		DEC_BA(baddr);
	sbaddr = baddr;
	while (True) {
		nbaddr = baddr;
		INC_BA(nbaddr);
		if (hSession->ea_buf[baddr].fa &&
		    !FA_IS_PROTECTED(hSession->ea_buf[baddr].fa) &&
		    !hSession->ea_buf[nbaddr].fa)
			break;
		DEC_BA(baddr);
		if (baddr == sbaddr) {
			cursor_move(hSession,0);
			return 0;
		}
	}
	INC_BA(baddr);
	cursor_move(hSession,baddr);
	return 0;
}


/*
 * Deferred keyboard unlock.
 */

static void defer_unlock(H3270 *session)
{
	kybdlock_clr(session,KL_DEFERRED_UNLOCK, "defer_unlock");
	status_reset(session);
	if (CONNECTED)
		ps_process();
}

/*
 * Reset keyboard lock.
 */
void do_reset(H3270 *session, Boolean explicit)
{
	/*
	 * If explicit (from the keyboard) and there is typeahead or
	 * a half-composed key, simply flush it.
	 */

	if (explicit
#if defined(X3270_FT) /*[*/
	    || lib3270_get_ft_state(session) != LIB3270_FT_STATE_NONE
#endif /*]*/
	    ) {
		Boolean half_reset = False;

		if (flush_ta())
			half_reset = True;

		if (half_reset)
			return;
	}

	/* Always clear insert mode. */
	lib3270_set_toggle(session,LIB3270_TOGGLE_INSERT,0);

	/* Otherwise, if not connect, reset is a no-op. */
	if (!CONNECTED)
		return;

	/*
	 * Remove any deferred keyboard unlock.  We will either unlock the
	 * keyboard now, or want to defer further into the future.
	 */
	if (session->kybdlock & KL_DEFERRED_UNLOCK)
		RemoveTimeOut(session->unlock_id);

	/*
	 * If explicit (from the keyboard), unlock the keyboard now.
	 * Otherwise (from the host), schedule a deferred keyboard unlock.
	 */
	if (explicit
#if defined(X3270_FT) /*[*/
	    || lib3270_get_ft_state(session) != LIB3270_FT_STATE_NONE
#endif /*]*/
	    || (!session->unlock_delay) // && !sms_in_macro())
	    || (session->unlock_delay_time != 0 && (time(NULL) - session->unlock_delay_time) > 1)) {
		kybdlock_clr(session,-1, "do_reset");
	} else if (session->kybdlock &
  (KL_DEFERRED_UNLOCK | KL_OIA_TWAIT | KL_OIA_LOCKED | KL_AWAITING_FIRST)) {
		kybdlock_clr(session,~KL_DEFERRED_UNLOCK, "do_reset");
		kybdlock_set(KL_DEFERRED_UNLOCK, "do_reset");
		session->unlock_id = AddTimeOut(UNLOCK_MS, session, defer_unlock);
	}

	/* Clean up other modes. */
	status_reset(session);
	mcursor_normal(session);

}

LIB3270_ACTION( reset )
{
	lib3270_unselect(hSession);
	do_reset(hSession,True);
	return 0;
}


/*
 * Move to first unprotected field on screen.
 */
LIB3270_ACTION( firstfield )
{
//	reset_idle_timer();
	if (hSession->kybdlock)
	{
		ENQUEUE_ACTION(lib3270_firstfield);
		return 0;
	}
#if defined(X3270_ANSI) /*[*/
	if (IN_ANSI) {
		ansi_send_home();
		return 0;
	}
#endif /*]*/
	if (!hSession->formatted) {
		cursor_move(hSession,0);
		return 0;
	}
	cursor_move(hSession,next_unprotected(hSession,hSession->rows*hSession->cols-1));

	return 0;
}


/*
 * Cursor left 1 position.
 */
static void
do_left(void)
{
	register int	baddr;
	enum dbcs_state d;

	baddr = h3270.cursor_addr;
	DEC_BA(baddr);
	d = ctlr_dbcs_state(baddr);
	if (IS_LEFT(d))
		DEC_BA(baddr);
	cursor_move(&h3270,baddr);
}

/*
void Left_action(Widget w unused, XEvent *event, String *params, Cardinal *num_params)
{
	action_CursorLeft();
}
*/

LIB3270_CURSOR_ACTION( left )
{
	if (hSession->kybdlock)
	{
		if (KYBDLOCK_IS_OERR)
		{
			kybdlock_clr(hSession,KL_OERR_MASK, "Left");
			status_reset(&h3270);
		}
		else
		{
			ENQUEUE_ACTION(lib3270_cursor_left);
			return 0;
		}
	}
#if defined(X3270_ANSI) /*[*/
	if (IN_ANSI)
	{
		ansi_send_left();
		return 0;
	}
#endif /*]*/

	if (!hSession->flipped)
	{
		do_left();
	}
	else
	{
		register int	baddr;

		baddr = hSession->cursor_addr;
		INC_BA(baddr);
		/* XXX: DBCS? */
		lib3270_set_cursor_address(hSession,baddr);
	}
	return 0;
}


/*
 * Delete char key.
 * Returns "True" if succeeds, "False" otherwise.
 */
static Boolean
do_delete(void)
{
	register int	baddr, end_baddr;
	int xaddr;
	register unsigned char	fa;
	int ndel;
	register int i;

	baddr = h3270.cursor_addr;

	/* Can't delete a field attribute. */
	fa = get_field_attribute(&h3270,baddr);
	if (FA_IS_PROTECTED(fa) || h3270.ea_buf[baddr].fa) {
		operator_error(KL_OERR_PROTECTED);
		return False;
	}
	if (h3270.ea_buf[baddr].cc == EBC_so || h3270.ea_buf[baddr].cc == EBC_si) {
		/*
		 * Can't delete SO or SI, unless it's adjacent to its
		 * opposite.
		 */
		xaddr = baddr;
		INC_BA(xaddr);
		if (h3270.ea_buf[xaddr].cc == SOSI(h3270.ea_buf[baddr].cc)) {
			ndel = 2;
		} else {
			operator_error(KL_OERR_PROTECTED);
			return False;
		}
	} else if (IS_DBCS(h3270.ea_buf[baddr].db)) {
		if (IS_RIGHT(h3270.ea_buf[baddr].db))
			DEC_BA(baddr);
		ndel = 2;
	} else
		ndel = 1;

	/* find next fa */
	if (h3270.formatted) {
		end_baddr = baddr;
		do {
			INC_BA(end_baddr);
			if (h3270.ea_buf[end_baddr].fa)
				break;
		} while (end_baddr != baddr);
		DEC_BA(end_baddr);
	} else {
		if ((baddr % h3270.cols) == h3270.cols - ndel)
			return True;
		end_baddr = baddr + (h3270.cols - (baddr % h3270.cols)) - 1;
	}

	/* Shift the remainder of the field left. */
	if (end_baddr > baddr) {
		ctlr_bcopy(baddr + ndel, baddr, end_baddr - (baddr + ndel) + 1,
		    0);
	} else if (end_baddr != baddr) {
		/* XXX: Need to verify this. */
		ctlr_bcopy(baddr + ndel, baddr,
		    ((h3270.rows * h3270.cols) - 1) - (baddr + ndel) + 1, 0);
		ctlr_bcopy(0, (h3270.rows * h3270.cols) - ndel, ndel, 0);
		ctlr_bcopy(ndel, 0, end_baddr - ndel + 1, 0);
	}

	/* NULL fill at the end. */
	for (i = 0; i < ndel; i++)
		ctlr_add(end_baddr - i, EBC_null, 0);

	/* Set the MDT for this field. */
	mdt_set(h3270.cursor_addr);

	/* Patch up the DBCS state for display. */
	(void) ctlr_dbcs_postprocess();
	return True;
}

LIB3270_ACTION( delete )
{
	if (hSession->kybdlock)
	{
		ENQUEUE_ACTION(lib3270_delete);
		return 0;
	}
#if defined(X3270_ANSI) /*[*/
	if (IN_ANSI) {
		net_sendc('\177');
		return 0;
	}
#endif /*]*/
	if (!do_delete())
		return 0;
	if (hSession->reverse)
	{
		int baddr = hSession->cursor_addr;

		DEC_BA(baddr);
		if (!hSession->ea_buf[baddr].fa)
			cursor_move(hSession,baddr);
	}
	hSession->display(hSession);
	return 0;
}


/*
 * 3270-style backspace.
 */
LIB3270_ACTION( backspace )
{
	if (hSession->kybdlock)
	{
		ENQUEUE_ACTION( lib3270_backspace );
		return 0;
	}
#if defined(X3270_ANSI) /*[*/
	if (IN_ANSI) {
		net_send_erase();
		return 0;
	}
#endif /*]*/
	if (hSession->reverse)
		(void) do_delete();
	else if (!hSession->flipped)
		do_left();
	else {
		register int	baddr;

		baddr = hSession->cursor_addr;
		DEC_BA(baddr);
		cursor_move(hSession,baddr);
	}
	hSession->display(hSession);
	return 0;
}


/*
 * Destructive backspace, like Unix "erase".
 */
static void
do_erase(void)
{
	int	baddr, faddr;
	enum dbcs_state d;

	baddr = h3270.cursor_addr;
	faddr = find_field_attribute(&h3270,baddr);
	if (faddr == baddr || FA_IS_PROTECTED(h3270.ea_buf[baddr].fa)) {
		operator_error(KL_OERR_PROTECTED);
		return;
	}
	if (baddr && faddr == baddr - 1)
		return;
	do_left();

	/*
	 * If we are now on an SI, move left again.
	 */
	if (h3270.ea_buf[h3270.cursor_addr].cc == EBC_si) {
		baddr = h3270.cursor_addr;
		DEC_BA(baddr);
		cursor_move(&h3270,baddr);
	}

	/*
	 * If we landed on the right-hand side of a DBCS character, move to the
	 * left-hand side.
	 * This ensures that if this is the end of a DBCS subfield, we will
	 * land on the SI, instead of on the character following.
	 */
	d = ctlr_dbcs_state(h3270.cursor_addr);
	if (IS_RIGHT(d)) {
		baddr = h3270.cursor_addr;
		DEC_BA(baddr);
		cursor_move(&h3270,baddr);
	}

	/*
	 * Try to delete this character.
	 */
	if (!do_delete())
		return;

	/*
	 * If we've just erased the last character of a DBCS subfield, erase
	 * the SO/SI pair as well.
	 */
	baddr = h3270.cursor_addr;
	DEC_BA(baddr);
	if (h3270.ea_buf[baddr].cc == EBC_so && h3270.ea_buf[h3270.cursor_addr].cc == EBC_si) {
		cursor_move(&h3270,baddr);
		(void) do_delete();
	}
	h3270.display(&h3270);
}

LIB3270_ACTION( erase )
{
//	reset_idle_timer();
	if (hSession->kybdlock) {
		ENQUEUE_ACTION(lib3270_erase);
		return 0;
	}
#if defined(X3270_ANSI) /*[*/
	if (IN_ANSI) {
		net_send_erase();
		return 0;
	}
#endif /*]*/
	do_erase();
	return 0;
}

/**
 * Cursor right 1 position.
 */
LIB3270_CURSOR_ACTION( right )
{
	register int	baddr;
	enum dbcs_state d;

	if (hSession->kybdlock)
	{
		if (KYBDLOCK_IS_OERR)
		{
			kybdlock_clr(hSession,KL_OERR_MASK, "Right");
			status_reset(hSession);
		}
		else
		{
			ENQUEUE_ACTION(lib3270_cursor_right);
			return 0;
		}
	}
#if defined(X3270_ANSI) /*[*/
	if (IN_ANSI) {
		ansi_send_right();
		return 0;
	}
#endif /*]*/
	if (!hSession->flipped)
	{
		baddr = hSession->cursor_addr;
		INC_BA(baddr);
		d = ctlr_dbcs_state(baddr);
		if (IS_RIGHT(d))
			INC_BA(baddr);
		lib3270_set_cursor_address(hSession,baddr);
	}
	else
	{
		do_left();
	}
	return 0;
}


/*
 * Cursor to previous word.
 */
LIB3270_ACTION( previousword )
{
	register int baddr;
	int baddr0;
	unsigned char  c;
	Boolean prot;

//	reset_idle_timer();
	if (hSession->kybdlock) {
		ENQUEUE_ACTION(lib3270_previousword);
//		enq_ta(PreviousWord_action, CN, CN);
		return 0;
	}
#if defined(X3270_ANSI) /*[*/
	if (IN_ANSI)
		return 0;
#endif /*]*/
	if (!hSession->formatted)
		return 0;

	baddr = hSession->cursor_addr;
	prot = FA_IS_PROTECTED(get_field_attribute(hSession,baddr));

	/* Skip to before this word, if in one now. */
	if (!prot) {
		c = hSession->ea_buf[baddr].cc;
		while (!hSession->ea_buf[baddr].fa && c != EBC_space && c != EBC_null) {
			DEC_BA(baddr);
			if (baddr == hSession->cursor_addr)
				return 0;
			c = hSession->ea_buf[baddr].cc;
		}
	}
	baddr0 = baddr;

	/* Find the end of the preceding word. */
	do {
		c = hSession->ea_buf[baddr].cc;
		if (hSession->ea_buf[baddr].fa) {
			DEC_BA(baddr);
			prot = FA_IS_PROTECTED(get_field_attribute(hSession,baddr));
			continue;
		}
		if (!prot && c != EBC_space && c != EBC_null)
			break;
		DEC_BA(baddr);
	} while (baddr != baddr0);

	if (baddr == baddr0)
		return 0;

	/* Go it its front. */
	for (;;) {
		DEC_BA(baddr);
		c = hSession->ea_buf[baddr].cc;
		if (hSession->ea_buf[baddr].fa || c == EBC_space || c == EBC_null) {
			break;
		}
	}
	INC_BA(baddr);
	cursor_move(hSession,baddr);
	return 0;
}


/* Find the next unprotected word, or -1 */
static int
nu_word(int baddr)
{
	int baddr0 = baddr;
	unsigned char c;
	Boolean prot;

	prot = FA_IS_PROTECTED(get_field_attribute(&h3270,baddr));

	do {
		c = h3270.ea_buf[baddr].cc;
		if (h3270.ea_buf[baddr].fa)
			prot = FA_IS_PROTECTED(h3270.ea_buf[baddr].fa);
		else if (!prot && c != EBC_space && c != EBC_null)
			return baddr;
		INC_BA(baddr);
	} while (baddr != baddr0);

	return -1;
}

/* Find the next word in this field, or -1 */
static int
nt_word(int baddr)
{
	int baddr0 = baddr;
	unsigned char c;
	Boolean in_word = True;

	do {
		c = h3270.ea_buf[baddr].cc;
		if (h3270.ea_buf[baddr].fa)
			return -1;
		if (in_word) {
			if (c == EBC_space || c == EBC_null)
				in_word = False;
		} else {
			if (c != EBC_space && c != EBC_null)
				return baddr;
		}
		INC_BA(baddr);
	} while (baddr != baddr0);

	return -1;
}


/*
 * Cursor to next unprotected word.
 */
LIB3270_ACTION( nextword )
{
	register int	baddr;
	unsigned char c;

//	reset_idle_timer();
	if (hSession->kybdlock) {
		ENQUEUE_ACTION( lib3270_nextword );
//		enq_ta(NextWord_action, CN, CN);
		return 0;
	}
#if defined(X3270_ANSI) /*[*/
	if (IN_ANSI)
		return 0;
#endif /*]*/
	if (!hSession->formatted)
		return 0;

	/* If not in an unprotected field, go to the next unprotected word. */
	if (hSession->ea_buf[hSession->cursor_addr].fa ||
	    FA_IS_PROTECTED(get_field_attribute(hSession,hSession->cursor_addr))) {
		baddr = nu_word(hSession->cursor_addr);
		if (baddr != -1)
			cursor_move(hSession,baddr);
		return 0;
	}

	/* If there's another word in this field, go to it. */
	baddr = nt_word(hSession->cursor_addr);
	if (baddr != -1) {
		cursor_move(hSession,baddr);
		return 0;
	}

	/* If in a word, go to just after its end. */
	c = hSession->ea_buf[hSession->cursor_addr].cc;
	if (c != EBC_space && c != EBC_null) {
		baddr = hSession->cursor_addr;
		do {
			c = hSession->ea_buf[baddr].cc;
			if (c == EBC_space || c == EBC_null) {
				cursor_move(hSession,baddr);
				return 0;
			} else if (hSession->ea_buf[baddr].fa) {
				baddr = nu_word(baddr);
				if (baddr != -1)
					cursor_move(hSession,baddr);
				return 0;
			}
			INC_BA(baddr);
		} while (baddr != hSession->cursor_addr);
	}
	/* Otherwise, go to the next unprotected word. */
	else {
		baddr = nu_word(hSession->cursor_addr);
		if (baddr != -1)
			cursor_move(hSession,baddr);
	}

	return 0;
}



/**
 * Cursor up 1 position.
 *
 * @return 0
 *
 */
LIB3270_CURSOR_ACTION( up )
{
	register int	baddr;

//	reset_idle_timer();
	if (hSession->kybdlock) {
		if (KYBDLOCK_IS_OERR)
		{
			kybdlock_clr(hSession,KL_OERR_MASK, "Up");
			status_reset(hSession);
		}
		else
		{
			ENQUEUE_ACTION(lib3270_cursor_up);
//			enq_ta(Up_action, CN, CN);
			return 0;
		}
	}
#if defined(X3270_ANSI) /*[*/
	if (IN_ANSI) {
		ansi_send_up();
		return 0;
	}
#endif /*]*/
	baddr = hSession->cursor_addr - hSession->cols;
	if (baddr < 0)
		baddr = (hSession->cursor_addr + (hSession->rows * hSession->cols)) - hSession->cols;
	lib3270_set_cursor_address(hSession,baddr);
	return 0;
}

/**
 * Cursor down 1 position.
 *
 * @return 0
 *
 */
LIB3270_CURSOR_ACTION( down )
{
	register int	baddr;

//	reset_idle_timer();
	if (hSession->kybdlock)
	{
		if (KYBDLOCK_IS_OERR)
		{
			kybdlock_clr(hSession,KL_OERR_MASK, "Down");
			status_reset(hSession);
		} else
		{
			ENQUEUE_ACTION(lib3270_cursor_down);
//			enq_ta(Down_action, CN, CN);
			return 0;
		}
	}
#if defined(X3270_ANSI) /*[*/
	if (IN_ANSI)
	{
		ansi_send_down();
		return 0;
	}
#endif /*]*/
	baddr = (hSession->cursor_addr + hSession->cols) % (hSession->cols * hSession->rows);
	lib3270_set_cursor_address(hSession,baddr);
	return 0;
}


/**
 * Cursor to first field on next line or any lines after that.
 */
LIB3270_CURSOR_ACTION( newline )
{
	register int	baddr, faddr;
	register unsigned char	fa;

	if (hSession->kybdlock)
	{
		ENQUEUE_ACTION(lib3270_cursor_newline);
		return 0;
	}
#if defined(X3270_ANSI) /*[*/
	if (IN_ANSI)
	{
		net_sendc('\n');
		return 0;
	}
#endif /*]*/
	baddr = (hSession->cursor_addr + hSession->cols) % (hSession->cols * hSession->rows);	/* down */
	baddr = (baddr / hSession->cols) * hSession->cols;			/* 1st col */
	faddr = find_field_attribute(hSession,baddr);
	fa = hSession->ea_buf[faddr].fa;
	if (faddr != baddr && !FA_IS_PROTECTED(fa))
		cursor_move(hSession,baddr);
	else
		cursor_move(hSession,next_unprotected(hSession,baddr));

	return 0;
}


/*
 * DUP key
 */
LIB3270_ACTION( dup )
{
	if (hSession->kybdlock)
	{
		ENQUEUE_ACTION(lib3270_dup);
		return 0;
	}
#if defined(X3270_ANSI)
	if (IN_ANSI)
		return 0;
#endif
	if (key_Character(EBC_dup, False, False, NULL))
	{
		hSession->display(hSession);
		cursor_move(hSession,next_unprotected(hSession,hSession->cursor_addr));
	}

	return 0;
}

/*
 * FM key
 */
LIB3270_ACTION( fieldmark )
{
	if (hSession->kybdlock)
	{
		ENQUEUE_ACTION(lib3270_fieldmark);
		return 0;
	}
#if defined(X3270_ANSI)
	if (IN_ANSI)
		return 0 ;
#endif
	(void) key_Character(EBC_fm, False, False, NULL);

	return 0;
}

/**
 * Send an "Enter" action.
 *
 * Called when the user press the key enter.
 *
 * @return 0 if ok, -1 if the action can't be performed.
 *
 */
LIB3270_KEY_ACTION( enter )
{
	trace("%s (kybdlock & KL_OIA_MINUS): %d kybdlock: %d",__FUNCTION__,(hSession->kybdlock & KL_OIA_MINUS),hSession->kybdlock);

	if (hSession->kybdlock & KL_OIA_MINUS)
		return -1;
	else if (hSession->kybdlock)
		ENQUEUE_ACTION(lib3270_enter);
	else
		key_AID(hSession,AID_ENTER);

	return 0;
}

LIB3270_ACTION( sysreq )
{
//	reset_idle_timer();
	if (IN_ANSI)
		return 0;
#if defined(X3270_TN3270E) /*[*/
	if (IN_E) {
		net_abort();
	} else
#endif /*]*/
	{
		if (hSession->kybdlock & KL_OIA_MINUS)
			return 0;
		else if (hSession->kybdlock)
			ENQUEUE_ACTION(lib3270_sysreq);
		else
			key_AID(hSession,AID_SYSREQ);
	}
	return 0;
}


/*
 * Clear AID key
 */
LIB3270_ACTION( clear )
{
//	reset_idle_timer();
	if (hSession->kybdlock & KL_OIA_MINUS)
		return 0;
	if (hSession->kybdlock && CONNECTED) {
		ENQUEUE_ACTION(lib3270_clear);
		return 0;
	}
#if defined(X3270_ANSI) /*[*/
	if (IN_ANSI) {
		ansi_send_clear();
		return 0;
	}
#endif /*]*/
	h3270.buffer_addr = 0;
	ctlr_clear(hSession,True);
	cursor_move(hSession,0);
	if (CONNECTED)
		key_AID(hSession,AID_CLEAR);
	return 0;
}


/*
 * Cursor Select key (light pen simulator).
 */
 /*
static void
lightpen_select(int baddr)
{
	int faddr;
	register unsigned char	fa;
	int designator;
#if defined(X3270_DBCS)
	int designator2;
#endif

	faddr = find_field_attribute(baddr);
	fa = ea_buf[faddr].fa;
	if (!FA_IS_SELECTABLE(fa)) {
		lib3270_ring_bell();
		return;
	}
	designator = faddr;
	INC_BA(designator);

#if defined(X3270_DBCS)
	if (dbcs) {
		if (ea_buf[baddr].cs == CS_DBCS) {
			designator2 = designator;
			INC_BA(designator2);
			if ((ea_buf[designator].db != DBCS_LEFT &&
			     ea_buf[designator].db != DBCS_LEFT_WRAP) &&
			    (ea_buf[designator2].db != DBCS_RIGHT &&
			     ea_buf[designator2].db != DBCS_RIGHT_WRAP)) {
				lib3270_ring_bell();
				return;
			}
			if (ea_buf[designator].cc == 0x42 &&
			    ea_buf[designator2].cc == EBC_greater) {
				ctlr_add(designator2, EBC_question, CS_DBCS);
				mdt_clear(faddr);
			} else if (ea_buf[designator].cc == 0x42 &&
				   ea_buf[designator2].cc == EBC_question) {
				ctlr_add(designator2, EBC_greater, CS_DBCS);
				mdt_clear(faddr);
			} else if ((ea_buf[designator].cc == EBC_space &&
				    ea_buf[designator2].cc == EBC_space) ||
			           (ea_buf[designator].cc == EBC_null &&
				    ea_buf[designator2].cc == EBC_null)) {
				ctlr_add(designator2, EBC_greater, CS_DBCS);
				mdt_set(faddr);
				key_AID(AID_SELECT);
			} else if (ea_buf[designator].cc == 0x42 &&
				   ea_buf[designator2].cc == EBC_ampersand) {
				mdt_set(faddr);
				key_AID(AID_ENTER);
			} else {
				lib3270_ring_bell();
			}
			return;
		}
	}
#endif

	switch (ea_buf[designator].cc) {
	    case EBC_greater:
		ctlr_add(designator, EBC_question, 0);
		mdt_clear(faddr);
		break;
	    case EBC_question:
		ctlr_add(designator, EBC_greater, 0);
		mdt_set(faddr);
		break;
	    case EBC_space:
	    case EBC_null:
		mdt_set(faddr);
		key_AID(AID_SELECT);
		break;
	    case EBC_ampersand:
		mdt_set(faddr);
		key_AID(AID_ENTER);
		break;
	    default:
		lib3270_ring_bell();
		break;
	}
}
*/

/*
 * Cursor Select key (light pen simulator) -- at the current cursor location.
 */
/*
void
CursorSelect_action(Widget w unused, XEvent *event, String *params,
    Cardinal *num_params)
{
//	reset_idle_timer();
	if (kybdlock) {
		enq_ta(CursorSelect_action, CN, CN);
		return;
	}

#if defined(X3270_ANSI)
	if (IN_ANSI)
		return;
#endif
	lightpen_select(cursor_addr);
}
*/

/**
 * Erase End Of Line Key.
 *
 */
LIB3270_ACTION( eraseeol )
{
	register int	baddr;
	register unsigned char	fa;
	enum dbcs_state d;
	enum dbcs_why why = DBCS_FIELD;

//	reset_idle_timer();
	if (hSession->kybdlock)
	{
		ENQUEUE_ACTION(lib3270_eraseeol);
		return 0;
	}
#if defined(X3270_ANSI) /*[*/
	if (IN_ANSI)
		return 0;
#endif /*]*/

	baddr = h3270.cursor_addr;
	fa = get_field_attribute(&h3270,baddr);
	if (FA_IS_PROTECTED(fa) || h3270.ea_buf[baddr].fa)
	{
		operator_error(KL_OERR_PROTECTED);
		return -1;
	}

	if (h3270.formatted)
	{
		/* erase to next field attribute or current line */
		do
		{
			ctlr_add(baddr, EBC_null, 0);
			INC_BA(baddr);
		} while (!h3270.ea_buf[baddr].fa && BA_TO_COL(baddr) > 0);

		mdt_set(h3270.cursor_addr);
	}
	else
	{
		/* erase to end of current line */
		do
		{
			ctlr_add(baddr, EBC_null, 0);
			INC_BA(baddr);
		} while(baddr != 0 && BA_TO_COL(baddr) > 0);
	}

	/* If the cursor was in a DBCS subfield, re-create the SI. */
	d = ctlr_lookleft_state(cursor_addr, &why);
	if (IS_DBCS(d) && why == DBCS_SUBFIELD) {
		if (d == DBCS_RIGHT) {
			baddr = h3270.cursor_addr;
			DEC_BA(baddr);
			h3270.ea_buf[baddr].cc = EBC_si;
		} else
			h3270.ea_buf[h3270.cursor_addr].cc = EBC_si;
	}
	(void) ctlr_dbcs_postprocess();
	h3270.display(&h3270);
	return 0;
}

/**
 * Erase End Of Field Key.
 *
 */
LIB3270_ACTION( eraseeof )
{
	register int	baddr;
	register unsigned char	fa;
	enum dbcs_state d;
	enum dbcs_why why = DBCS_FIELD;

//	reset_idle_timer();
	if (hSession->kybdlock)
	{
		ENQUEUE_ACTION(lib3270_eraseeof);
		return 0;
	}
#if defined(X3270_ANSI) /*[*/
	if (IN_ANSI)
		return 0;
#endif /*]*/
	baddr = hSession->cursor_addr;
	fa = get_field_attribute(hSession,baddr);
	if (FA_IS_PROTECTED(fa) || h3270.ea_buf[baddr].fa) {
		operator_error(KL_OERR_PROTECTED);
		return -1;
	}
	if (hSession->formatted) {	/* erase to next field attribute */
		do {
			ctlr_add(baddr, EBC_null, 0);
			INC_BA(baddr);
		} while (!h3270.ea_buf[baddr].fa);
		mdt_set(hSession->cursor_addr);
	} else {	/* erase to end of screen */
		do {
			ctlr_add(baddr, EBC_null, 0);
			INC_BA(baddr);
		} while (baddr != 0);
	}

	/* If the cursor was in a DBCS subfield, re-create the SI. */
	d = ctlr_lookleft_state(cursor_addr, &why);
	if (IS_DBCS(d) && why == DBCS_SUBFIELD) {
		if (d == DBCS_RIGHT) {
			baddr = hSession->cursor_addr;
			DEC_BA(baddr);
			h3270.ea_buf[baddr].cc = EBC_si;
		} else
			h3270.ea_buf[hSession->cursor_addr].cc = EBC_si;
	}
	(void) ctlr_dbcs_postprocess();
	hSession->display(hSession);
	return 0;
}

LIB3270_ACTION( eraseinput )
{
	register int	baddr, sbaddr;
	unsigned char	fa;
	Boolean		f;

//	reset_idle_timer();
	if (hSession->kybdlock) {
		ENQUEUE_ACTION( lib3270_eraseinput );
		return 0;
	}
#if defined(X3270_ANSI) /*[*/
	if (IN_ANSI)
		return 0;
#endif /*]*/
	if (hSession->formatted) {
		/* find first field attribute */
		baddr = 0;
		do {
			if (hSession->ea_buf[baddr].fa)
				break;
			INC_BA(baddr);
		} while (baddr != 0);
		sbaddr = baddr;
		f = False;
		do {
			fa = hSession->ea_buf[baddr].fa;
			if (!FA_IS_PROTECTED(fa)) {
				mdt_clear(baddr);
				do {
					INC_BA(baddr);
					if (!f) {
						cursor_move(hSession,baddr);
						f = True;
					}
					if (!hSession->ea_buf[baddr].fa) {
						ctlr_add(baddr, EBC_null, 0);
					}
				} while (!hSession->ea_buf[baddr].fa);
			} else {	/* skip protected */
				do {
					INC_BA(baddr);
				} while (!hSession->ea_buf[baddr].fa);
			}
		} while (baddr != sbaddr);
		if (!f)
			cursor_move(hSession,0);
	} else {
		ctlr_clear(hSession,True);
		cursor_move(hSession,0);
	}
	hSession->display(hSession);
	return 0;
}



/*
 * Delete word key.  Backspaces the cursor until it hits the front of a word,
 * deletes characters until it hits a blank or null, and deletes all of these
 * but the last.
 *
 * Which is to say, does a ^W.
 */
LIB3270_ACTION( deleteword )
{
	register int baddr;
	register unsigned char	fa;

//	reset_idle_timer();
	if (hSession->kybdlock) {
		ENQUEUE_ACTION(lib3270_deleteword);
//		enq_ta(DeleteWord_action, CN, CN);
		return 0;
	}
#if defined(X3270_ANSI) /*[*/
	if (IN_ANSI) {
		net_send_werase();
		return 0;
	}
#endif /*]*/
	if (!hSession->formatted)
		return 0;

	baddr = hSession->cursor_addr;
	fa = get_field_attribute(hSession,baddr);

	/* Make sure we're on a modifiable field. */
	if (FA_IS_PROTECTED(fa) || hSession->ea_buf[baddr].fa) {
		operator_error(KL_OERR_PROTECTED);
		return -1;
	}

	/* Backspace over any spaces to the left of the cursor. */
	for (;;) {
		baddr = hSession->cursor_addr;
		DEC_BA(baddr);
		if (hSession->ea_buf[baddr].fa)
			return 0;
		if (hSession->ea_buf[baddr].cc == EBC_null ||
		    hSession->ea_buf[baddr].cc == EBC_space)
			do_erase();
		else
			break;
	}

	/* Backspace until the character to the left of the cursor is blank. */
	for (;;) {
		baddr = hSession->cursor_addr;
		DEC_BA(baddr);
		if (hSession->ea_buf[baddr].fa)
			return 0;
		if (hSession->ea_buf[baddr].cc == EBC_null ||
		    hSession->ea_buf[baddr].cc == EBC_space)
			break;
		else
			do_erase();
	}
	hSession->display(hSession);
	return 0;
}



/*
 * Delete field key.  Similar to EraseEOF, but it wipes out the entire field
 * rather than just to the right of the cursor, and it leaves the cursor at
 * the front of the field.
 *
 * Which is to say, does a ^U.
 */
LIB3270_ACTION( deletefield )
{
	register int	baddr;
	register unsigned char	fa;

//	reset_idle_timer();
	if (hSession->kybdlock) {
		ENQUEUE_ACTION(lib3270_deletefield);
//		enq_ta(DeleteField_action, CN, CN);
		return 0;
	}
#if defined(X3270_ANSI) /*[*/
	if (IN_ANSI) {
		net_send_kill();
		return 0;
	}
#endif /*]*/
	if (!hSession->formatted)
		return 0;

	baddr = hSession->cursor_addr;
	fa = get_field_attribute(hSession,baddr);
	if (FA_IS_PROTECTED(fa) || hSession->ea_buf[baddr].fa) {
		operator_error(KL_OERR_PROTECTED);
		return -1;
	}
	while (!hSession->ea_buf[baddr].fa)
		DEC_BA(baddr);
	INC_BA(baddr);
	mdt_set(hSession->cursor_addr);
	cursor_move(hSession,baddr);
	while (!hSession->ea_buf[baddr].fa) {
		ctlr_add(baddr, EBC_null, 0);
		INC_BA(baddr);
	}
	hSession->display(hSession);
	return 0;
}



/*
 * Set insert mode key.
 */ /*
void
Insert_action(Widget w unused, XEvent *event, String *params, Cardinal *num_params)
{
//	reset_idle_timer();
	if (kybdlock) {
		enq_ta(Insert_action, CN, CN);
		return;
	}
#if defined(X3270_ANSI)
	if (IN_ANSI)
		return;
#endif
	set_toggle(INSERT,True);
}
*/


/*
 * Toggle insert mode key.
 */ /*
void
ToggleInsert_action(Widget w unused, XEvent *event, String *params, Cardinal *num_params)
{
//	reset_idle_timer();
	if (kybdlock) {
		enq_ta(ToggleInsert_action, CN, CN);
		return;
	}
#if defined(X3270_ANSI)
	if (IN_ANSI)
		return;
#endif

	do_toggle(INSERT);
}
*/


/*
 * Toggle reverse mode key.
 */ /*
void
ToggleReverse_action(Widget w unused, XEvent *event, String *params, Cardinal *num_params)
{
//	reset_idle_timer();
	if (kybdlock) {
		enq_ta(ToggleReverse_action, CN, CN);
		return;
	}
#if defined(X3270_ANSI)
	if (IN_ANSI)
		return;
#endif
	reverse_mode(!reverse);
} */


/*
 * Move the cursor to the first blank after the last nonblank in the
 * field, or if the field is full, to the last character in the field.
 */
LIB3270_ACTION( fieldend )
{
	int	baddr, faddr;
	unsigned char	fa, c;
	int	last_nonblank = -1;

	if (hSession->kybdlock)
	{
		ENQUEUE_ACTION( lib3270_fieldend );
		return 0;
	}
#if defined(X3270_ANSI) /*[*/
	if (IN_ANSI)
		return 0;
#endif /*]*/
	if (!hSession->formatted)
		return 0;
	baddr = hSession->cursor_addr;
	faddr = find_field_attribute(hSession,baddr);
	fa = hSession->ea_buf[faddr].fa;
	if (faddr == baddr || FA_IS_PROTECTED(fa))
		return 0;

	baddr = faddr;
	while (True) {
		INC_BA(baddr);
		c = hSession->ea_buf[baddr].cc;
		if (hSession->ea_buf[baddr].fa)
			break;
		if (c != EBC_null && c != EBC_space)
			last_nonblank = baddr;
	}

	if (last_nonblank == -1) {
		baddr = faddr;
		INC_BA(baddr);
	} else {
		baddr = last_nonblank;
		INC_BA(baddr);
		if (hSession->ea_buf[baddr].fa)
			baddr = last_nonblank;
	}
	cursor_move(hSession,baddr);
	return 0;
}

/* PA key action for String actions */
static void
do_pa(unsigned n)
{
	if (n < 1 || n > PA_SZ) {
		popup_an_error(NULL, _( "Unknown PA key %d" ), n);
		return;
	}

	lib3270_pakey(&h3270,n);

}

/* PF key action for String actions */
static void do_pf(unsigned n)
{
	if (n < 1 || n > PF_SZ) {
		popup_an_error(NULL, _( "Unknown PF key %d" ), n);
		return;
	}

	lib3270_pfkey(&h3270,n);
}

/*
 * Set or clear the keyboard scroll lock.
 */
void
kybd_scroll_lock(Boolean lock)
{
	if (!IN_3270)
		return;
	if (lock)
		kybdlock_set(KL_SCROLLED, "kybd_scroll_lock");
	else
		kybdlock_clr(&h3270, KL_SCROLLED, "kybd_scroll_lock");
}

/*
 * Move the cursor back within the legal paste area.
 * Returns a Boolean indicating success.
 */
static Boolean
remargin(int lmargin)
{
	Boolean ever = False;
	int baddr, b0 = 0;
	int faddr;
	unsigned char fa;

	baddr = h3270.cursor_addr;
	while (BA_TO_COL(baddr) < lmargin) {
		baddr = ROWCOL_TO_BA(BA_TO_ROW(baddr), lmargin);
		if (!ever) {
			b0 = baddr;
			ever = True;
		}
		faddr = find_field_attribute(&h3270,baddr);
		fa = h3270.ea_buf[faddr].fa;
		if (faddr == baddr || FA_IS_PROTECTED(fa)) {
			baddr = next_unprotected(&h3270,baddr);
			if (baddr <= b0)
				return False;
		}
	}

	cursor_move(&h3270,baddr);
	return True;
}

LIB3270_EXPORT int lib3270_emulate_input(H3270 *session, char *s, int len, int pasting)
{
	enum { BASE, BACKSLASH, BACKX, BACKP, BACKPA, BACKPF, OCTAL, HEX, XGE } state = BASE;
	int literal = 0;
	int nc = 0;
	enum iaction ia = pasting ? IA_PASTE : IA_STRING;
	int orig_addr;
	int orig_col;
	Boolean skipped = False;

#if defined(X3270_DBCS) /*[*/
	unsigned char ebc[2];
	unsigned char cx;
	static UChar *w_ibuf = NULL;
	static size_t w_ibuf_len = 0;
	UChar c;
	UChar *ws;
#else /*][*/
	char c;
	char *ws;
#endif /*]*/

	CHECK_SESSION_HANDLE(session);

	orig_addr = session->cursor_addr;
	orig_col  = BA_TO_COL(session->cursor_addr);

	if(len < 0)
		len = strlen(s);

	/*
	 * Convert from a multi-byte string to a Unicode string.
	 */
#if defined(X3270_DBCS) /*[*/
	if (len > w_ibuf_len) {
		w_ibuf_len = len;
		w_ibuf = (UChar *)Realloc(w_ibuf, w_ibuf_len * sizeof(UChar));
	}
	len = mb_to_unicode(s, len, w_ibuf, w_ibuf_len, NULL);
	if (len < 0) {
		return 0; /* failed */
	}
	ws = w_ibuf;
#else /*][*/
	ws = s;
#endif /*]*/

	/*
	 * In the switch statements below, "break" generally means "consume
	 * this character," while "continue" means "rescan this character."
	 */
	while (len) {

		/*
		 * It isn't possible to unlock the keyboard from a string,
		 * so if the keyboard is locked, it's fatal
		 */
		if (session->kybdlock)
		{
			trace_event("  keyboard locked, string dropped\n");
			return 0;
		}

		if (pasting && IN_3270) {

			/* Check for cursor wrap to top of screen. */
			if (session->cursor_addr < orig_addr)
				return len-1;		/* wrapped */

			/* Jump cursor over left margin. */
			if (lib3270_get_toggle(&h3270,LIB3270_TOGGLE_MARGINED_PASTE) &&
			    BA_TO_COL(session->cursor_addr) < orig_col) {
				if (!remargin(orig_col))
					return len-1;
				skipped = True;
			}
		}

		c = *ws;

		switch (state) {
		    case BASE:
			switch (c) {
			    case '\b':
			    lib3270_cursor_left(session);
				skipped = False;
				break;
			    case '\f':
				if (pasting) {
					key_ACharacter((unsigned char) ' ',
					    KT_STD, ia, &skipped);
				} else {
					lib3270_clear(session);
					skipped = False;
					if (IN_3270)
						return len-1;
				}
				break;
			    case '\n':
				if (pasting) {
					if (!skipped)
						lib3270_cursor_newline(session);
//						action_internal(Newline_action,ia, CN, CN);
					skipped = False;
				} else {
					lib3270_enter(session);
					skipped = False;
					if (IN_3270)
						return len-1;
				}
				break;
			    case '\r':	/* ignored */
				break;
			    case '\t':
			    lib3270_tab(session);
				skipped = False;
				break;
			    case '\\':	/* backslashes are NOT special when
					   pasting */
				if (!pasting)
					state = BACKSLASH;
				else
					key_ACharacter((unsigned char) c,
					    KT_STD, ia, &skipped);
				break;
			    case '\033': /* ESC is special only when pasting */
				if (pasting)
					state = XGE;
				break;
			    case '[':	/* APL left bracket */
					key_ACharacter((unsigned char) c, KT_STD, ia, &skipped);
				break;
			    case ']':	/* APL right bracket */
					key_ACharacter((unsigned char) c, KT_STD, ia, &skipped);
				break;
			default:
/*
#if defined(X3270_DBCS)
				//
				// Try mapping it to the 8-bit character set,
				// otherwise to the 16-bit character set.
				//
				if (dbcs_map8(c, &cx)) {
					key_ACharacter((unsigned char)cx,
					    KT_STD, ia_cause, &skipped);
					break;
				} else if (dbcs_map16(c, ebc)) {
					(void) key_WCharacter(ebc, &skipped);
					break;
				} else {
					trace_event("Cannot convert U+%04x to "
					    "EBCDIC\n", c & 0xffff);
					break;
				}
#endif */
				key_ACharacter((unsigned char) c, KT_STD, ia, &skipped);
				break;
			}
			break;
		    case BACKSLASH:	/* last character was a backslash */
			switch (c) {
			    case 'a':
				popup_an_error(NULL,"%s: Bell not supported",action_name(String_action));
//				cancel_if_idle_command();
				state = BASE;
				break;
			    case 'b':
				lib3270_cursor_left(session);
//				action_internal(Left_action, ia, CN, CN);
				skipped = False;
				state = BASE;
				break;
			    case 'f':
			    lib3270_clear(session);
				skipped = False;
				state = BASE;
				if (IN_3270)
					return len-1;
				else
					break;
			    case 'n':
				lib3270_enter(session);
 				skipped = False;
				state = BASE;
				if (IN_3270)
					return len-1;
				else
					break;
			    case 'p':
				state = BACKP;
				break;

			    case 'r':
					lib3270_cursor_newline(session);
//					action_internal(Newline_action, ia, CN, CN);
					skipped = False;
					state = BASE;
					break;

			    case 't':
			    lib3270_tab(session);
				skipped = False;
				state = BASE;
				break;
			    case 'T':
			    lib3270_tab(session);
				skipped = False;
				state = BASE;
				break;
			    case 'v':
				popup_an_error(NULL,"%s: Vertical tab not supported",action_name(String_action));
//				cancel_if_idle_command();
				state = BASE;
				break;
			    case 'x':
				state = BACKX;
				break;
			    case '\\':
				key_ACharacter((unsigned char) c, KT_STD, ia,&skipped);
				state = BASE;
				break;
			    case '0':
			    case '1':
			    case '2':
			    case '3':
			    case '4':
			    case '5':
			    case '6':
			    case '7':
				state = OCTAL;
				literal = 0;
				nc = 0;
				continue;
			default:
				state = BASE;
				continue;
			}
			break;
		    case BACKP:	/* last two characters were "\p" */
			switch (c) {
			    case 'a':
				literal = 0;
				nc = 0;
				state = BACKPA;
				break;
			    case 'f':
				literal = 0;
				nc = 0;
				state = BACKPF;
				break;
			    default:
				popup_an_error(NULL,"%s: Unknown character after \\p",
				    action_name(String_action));
//				cancel_if_idle_command();
				state = BASE;
				break;
			}
			break;
		    case BACKPF: /* last three characters were "\pf" */
			if (nc < 2 && isdigit(c)) {
				literal = (literal * 10) + (c - '0');
				nc++;
			} else if (!nc) {
				popup_an_error(NULL,"%s: Unknown character after \\pf",
				    action_name(String_action));
//				cancel_if_idle_command();
				state = BASE;
			} else {
				do_pf(literal);
				skipped = False;
				if (IN_3270)
					return len-1;
				state = BASE;
				continue;
			}
			break;
		    case BACKPA: /* last three characters were "\pa" */
			if (nc < 1 && isdigit(c)) {
				literal = (literal * 10) + (c - '0');
				nc++;
			} else if (!nc) {
				popup_an_error(NULL,"%s: Unknown character after \\pa",
				    action_name(String_action));
//				cancel_if_idle_command();
				state = BASE;
			} else {
				do_pa(literal);
				skipped = False;
				if (IN_3270)
					return len-1;
				state = BASE;
				continue;
			}
			break;
		    case BACKX:	/* last two characters were "\x" */
			if (isxdigit(c)) {
				state = HEX;
				literal = 0;
				nc = 0;
				continue;
			} else {
				popup_an_error(NULL,"%s: Missing hex digits after \\x",
				    action_name(String_action));
//				cancel_if_idle_command();
				state = BASE;
				continue;
			}
		    case OCTAL:	/* have seen \ and one or more octal digits */
			if (nc < 3 && isdigit(c) && c < '8') {
				literal = (literal * 8) + FROM_HEX(c);
				nc++;
				break;
			} else {
				key_ACharacter((unsigned char) literal, KT_STD,
				    ia, &skipped);
				state = BASE;
				continue;
			}
		    case HEX:	/* have seen \ and one or more hex digits */
			if (nc < 2 && isxdigit(c)) {
				literal = (literal * 16) + FROM_HEX(c);
				nc++;
				break;
			} else {
				key_ACharacter((unsigned char) literal, KT_STD,
				    ia, &skipped);
				state = BASE;
				continue;
			}
		    case XGE:	/* have seen ESC */
			switch (c) {
			    case ';':	/* FM */
				key_Character(EBC_fm, False, True, &skipped);
				break;
			    case '*':	/* DUP */
				key_Character(EBC_dup, False, True, &skipped);
				break;
			    default:
				key_ACharacter((unsigned char) c, KT_GE, ia,
						&skipped);
				break;
			}
			state = BASE;
			break;
		}
		ws++;
		len--;
	}

	switch (state) {
	    case BASE:
		if (lib3270_get_toggle(&h3270,LIB3270_TOGGLE_MARGINED_PASTE) &&
		    BA_TO_COL(session->cursor_addr) < orig_col) {
			(void) remargin(orig_col);
		}
		break;
	    case OCTAL:
	    case HEX:
		key_ACharacter((unsigned char) literal, KT_STD, ia, &skipped);
		state = BASE;
		if (lib3270_get_toggle(&h3270,LIB3270_TOGGLE_MARGINED_PASTE) &&
		    BA_TO_COL(session->cursor_addr) < orig_col) {
			(void) remargin(orig_col);
		}
		break;
	    case BACKPF:
		if (nc > 0) {
			do_pf(literal);
			state = BASE;
		}
		break;
	    case BACKPA:
		if (nc > 0) {
			do_pa(literal);
			state = BASE;
		}
		break;
	    default:
		popup_an_error(NULL,"%s: Missing data after \\",
		    action_name(String_action));
//		cancel_if_idle_command();
		break;
	}

	session->display(session);
	return len;
}

#if defined(X3270_FT) /*[*/
/*
 * Set up the cursor and input field for command input.
 * Returns the length of the input field, or 0 if there is no field
 * to set up.
 */
int
kybd_prime(void)
{
	int baddr;
	register unsigned char fa;
	int len = 0;

	/*
	 * No point in trying if the screen isn't formatted, the keyboard
	 * is locked, or we aren't in 3270 mode.
	 */
	if (!h3270.formatted || h3270.kybdlock || !IN_3270)
		return 0;

	fa = get_field_attribute(&h3270,h3270.cursor_addr);
	if (h3270.ea_buf[h3270.cursor_addr].fa || FA_IS_PROTECTED(fa)) {
		/*
		 * The cursor is not in an unprotected field.  Find the
		 * next one.
		 */
		baddr = next_unprotected(&h3270,h3270.cursor_addr);

		/* If there isn't any, give up. */
		if (!baddr)
			return 0;

		/* Move the cursor there. */
	} else {
		/* Already in an unprotected field.  Find its start. */
		baddr = h3270.cursor_addr;
		while (!h3270.ea_buf[baddr].fa) {
			DEC_BA(baddr);
		}
		INC_BA(baddr);
	}

	/* Move the cursor to the beginning of the field. */
	cursor_move(&h3270,baddr);

	/* Erase it. */
	while (!h3270.ea_buf[baddr].fa) {
		ctlr_add(baddr, 0, 0);
		len++;
		INC_BA(baddr);
	}

	/* Return the field length. */
	return len;
}
#endif /*]*/

/*
 * Translate a keysym name to a keysym, including APL and extended
 * characters.
 */ /*
static KeySym
MyStringToKeysym(char *s, enum keytype *keytypep)
{
	KeySym k;
	int cc;
	char *ptr;
	unsigned char xc;


#if defined(X3270_APL)
	if (!strncmp(s, "apl_", 4)) {
		int is_ge;

		k = APLStringToKeysym(s, &is_ge);
		if (is_ge)
			*keytypep = KT_GE;
		else
			*keytypep = KT_STD;
	} else
#endif
	{
		k = StringToKeysym(s);
		*keytypep = KT_STD;
	}
	if (k == NoSymbol && ((xc = utf8_lookup(s, NULL, NULL)) != 0))
		k = xc;
	if (k == NoSymbol && !strcasecmp(s, "euro"))
		k = 0xa4;
	if (k == NoSymbol && strlen(s) == 1)
		k = s[0] & 0xff;
	if (k < ' ')
		k = NoSymbol;
	else if (k > 0xff) {
		int i;

		for (i = 0; i < nxk; i++)
			if (xk[i].key == k) {
				k = xk[i].assoc;
				break;
			}
		if (k > 0xff)
			k &= 0xff;
	}

	// Allow arbitrary values, e.g., 0x03 for ^C.
	if (k == NoSymbol &&
	    (cc = strtoul(s, &ptr, 0)) > 0 &&
	    cc < 0xff &&
	    ptr != s &&
	    *ptr == '\0')
		k = cc;

	return k;
}
*/

/* Add a key to the extended association table. */
void
add_xk(KeySym key, KeySym assoc)
{
	int i;

	for (i = 0; i < nxk; i++)
		if (xk[i].key == key) {
			xk[i].assoc = assoc;
			return;
		}
	xk = (struct xks *) Realloc(xk, (nxk + 1) * sizeof(struct xks));
	xk[nxk].key = key;
	xk[nxk].assoc = assoc;
	nxk++;
}

/* Clear the extended association table. */
void
clear_xks(void)
{
	if (nxk) {
		lib3270_free(xk);
		xk = (struct xks *)NULL;
		nxk = 0;
	}
}


