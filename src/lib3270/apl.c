/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270.
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
 * Este programa está nomeado como apl.c e possui 219 linhas de código.
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
 *	apl.c
 *		This module handles APL-specific actions.
 */

#include "globals.h"

#if defined(X3270_APL) /*[*/

#include <X11/keysym.h>

#include "aplc.h"


/*
 * APL keysym translation.
 *
 * This code looks a little odd because of how an APL font is implemented.
 * An APL font has APL graphics in place of the various accented letters and
 * special symbols in a regular font.  APL keysym translation consists of
 * taking the keysym name for an APL symbol (these names are meaningful only to
 * x3270) and translating it into the keysym for the regular symbol that the
 * desired APL symbol _replaces_.
 *
 * For example, an APL font has the APL "jot" symbol where a regular font has
 * the "registered" symbol.  So we take the keysym name "jot" and translate it
 * into the keysym XK_registered.  When the XK_registered symbol is displayed
 * with an APL font, it appears as a "jot".
 *
 * The specification of which APL symbols replace which regular symbols is in
 * IBM GA27-3831, 3174 Establishment Controller Character Set Reference.
 *
 * In addition, several standard characters have different names for APL,
 * for example, "period" becomes "dot".  These are included in the table as
 * well.
 */
 /*

static struct {
	const char *name;
	KeySym keysym;
	int is_ge;
} axl[] = {
	{ "Aunderbar",		XK_nobreakspace,	1 },
	{ "Bunderbar",		XK_acircumflex,		1 },
	{ "Cunderbar",		XK_adiaeresis,		1 },
	{ "Dunderbar",		XK_agrave,		1 },
	{ "Eunderbar",		XK_aacute,		1 },
	{ "Funderbar",		XK_atilde,		1 },
	{ "Gunderbar",		XK_aring,		1 },
	{ "Hunderbar",		XK_ccedilla,		1 },
	{ "Iunderbar",		XK_ntilde,		1 },
	{ "Junderbar",		XK_eacute,		1 },
	{ "Kunderbar",		XK_ecircumflex,		1 },
	{ "Lunderbar",		XK_ediaeresis,		1 },
	{ "Munderbar",		XK_egrave,		1 },
	{ "Nunderbar",		XK_iacute,		1 },
	{ "Ounderbar",		XK_icircumflex,		1 },
	{ "Punderbar",		XK_idiaeresis,		1 },
	{ "Qunderbar",		XK_igrave,		1 },
	{ "Runderbar",		XK_ssharp,		1 },
	{ "Sunderbar",		XK_Acircumflex,		1 },
	{ "Tunderbar",		XK_Adiaeresis,		1 },
	{ "Uunderbar",		XK_Agrave,		1 },
	{ "Vunderbar",		XK_Aacute,		1 },
	{ "Wunderbar",		XK_Atilde,		1 },
	{ "Xunderbar",		XK_Aring,		1 },
	{ "Yunderbar",		XK_Ccedilla,		1 },
	{ "Zunderbar",		XK_Ntilde,		1 },
	{ "alpha",		XK_asciicircum,		1 },
	{ "bar",		XK_minus,		0 },
	{ "braceleft",		XK_braceleft,		1 },
	{ "braceright",		XK_braceright,		1 },
	{ "bracketleft",	XK_Yacute,		1 },
	{ "bracketright", 	XK_diaeresis,		1 },
	{ "circle",		XK_cedilla,		1 },
	{ "circlebar",		XK_Ograve,		1 },
	{ "circleslope",	XK_otilde,		1 },
	{ "circlestar",		XK_Ugrave,		1 },
	{ "circlestile",	XK_ograve,		1 },
	{ "colon",		XK_colon,		0 },
	{ "comma",		XK_comma,		0 },
	{ "commabar",		XK_W,			1 }, // soliton
	{ "del",		XK_bracketleft,		1 },
	{ "delstile",		XK_udiaeresis,		1 },
	{ "delta",		XK_bracketright,	1 },
	{ "deltastile",		XK_ugrave,		1 },
	{ "deltaunderbar",	XK_Udiaeresis,		1 },
	{ "deltilde",		XK_Ucircumflex,		1 },
	{ "diaeresis",		XK_Ecircumflex,		1 },
	{ "diaeresiscircle",	XK_V,			1 }, // soliton
	{ "diaeresisdot",	XK_Odiaeresis,		1 },
	{ "diaeresisjot",	XK_U,			1 }, // soliton
	{ "diamond",		XK_oslash,		1 },
	{ "dieresis",		XK_Ecircumflex,		1 },
	{ "dieresisdot",	XK_Odiaeresis,		1 },
	{ "divide",		XK_onehalf,		1 },
	{ "dot",		XK_period,		0 },
	{ "downarrow",		XK_guillemotright,	1 },
	{ "downcaret",		XK_Igrave,		1 },
	{ "downcarettilde",	XK_ocircumflex,		1 },
	{ "downshoe",		XK_questiondown,	1 },
	{ "downstile",		XK_thorn,		1 },
	{ "downtack",		XK_ETH,			1 },
	{ "downtackjot",	XK_Uacute,		1 },
	{ "downtackup",		XK_onesuperior,		1 },
	{ "downtackuptack",	XK_onesuperior,		1 },
	{ "epsilon",		XK_sterling,		1 },
	{ "epsilonunderbar",	XK_Iacute,		1 },
	{ "equal",		XK_equal,		0 },
	{ "equalunderbar",	XK_backslash,		1 },
	{ "euro",		XK_X,			1 }, // soliton
	{ "greater",		XK_greater,		0 },
	{ "iota",		XK_yen,			1 },
	{ "iotaunderbar",	XK_Egrave,		1 },
	{ "jot",		XK_registered,		1 },
	{ "leftarrow",		XK_currency,		1 },
	{ "leftbracket",	XK_Yacute,		1 },
	{ "leftparen",		XK_parenleft,		0 },
	{ "leftshoe",		XK_masculine,		1 },
	{ "lefttack",		XK_Icircumflex,		1 },
	{ "less",		XK_less,		0 },
	{ "multiply",		XK_paragraph,		1 },
	{ "notequal",		XK_acute,		1 },
	{ "notgreater",		XK_eth,			1 },
	{ "notless",		XK_THORN,		1 },
	{ "omega",		XK_copyright,		1 },
	{ "overbar",		XK_mu,			1 },
	{ "plus",		XK_plus,		0 },
	{ "plusminus",		XK_AE,			1 },
	{ "quad",		XK_degree,		1 },
	{ "quaddivide",		XK_Oacute,		1 },
	{ "quadjot",		XK_Ediaeresis,		1 },
	{ "quadquote",		XK_uacute,		1 },
	{ "quadslope",		XK_oacute,		1 },
	{ "query",		XK_question,		0 },
	{ "quote",		XK_apostrophe,		0 },
	{ "quotedot",		XK_ucircumflex,		1 },
	{ "rho",		XK_periodcentered,	1 },
	{ "rightarrow",		XK_plusminus,		1 },
	{ "rightbracket", 	XK_diaeresis,		1 },
	{ "rightparen",		XK_parenright,		0 },
	{ "rightshoe",		XK_ordfeminine,		1 },
	{ "righttack",		XK_Idiaeresis,		1 },
	{ "semicolon",		XK_semicolon,		0 },
	{ "slash",		XK_slash,		0 },
	{ "slashbar",		XK_twosuperior,		1 },
	{ "slope",		XK_onequarter,		1 },
	{ "slopebar",		XK_Ocircumflex,		1 },
	{ "slopequad",		XK_oacute,		1 },
	{ "splat",		XK_ae,			1 },
	{ "squad",		XK_odiaeresis,		1 },
	{ "star",		XK_asterisk,		0 },
	{ "stile",		XK_multiply,		1 },
	{ "tilde",		XK_Ooblique,		1 },
	{ "times",		XK_paragraph,		1 },
	{ "underbar",		XK_underscore,		0 },
	{ "uparrow",		XK_guillemotleft,	1 },
	{ "upcaret",		XK_Eacute,		1 },
	{ "upcarettilde",	XK_hyphen,		1 },
	{ "upshoe",		XK_exclamdown,		1 },
	{ "upshoejot",		XK_ydiaeresis,		1 },
	{ "upstile",		XK_yacute,		1 },
	{ "uptack",		XK_macron,		1 },
	{ "uptackjot",		XK_Otilde,		1 },
	{ 0, 0 }
};

//
// Translation from APL ksysym names to indirect APL keysyms.
//
KeySym
APLStringToKeysym(char *s, int *is_gep)
{
	register int i;

	if (strncmp(s, "apl_", 4))
		return NoSymbol;
	s += 4;
	for (i = 0; axl[i].name; i++)
		if (!strcmp(axl[i].name, s)) {
			*is_gep = axl[i].is_ge;
			return axl[i].keysym;
		}
	return NoSymbol;
}
*/

#endif /*]*/
