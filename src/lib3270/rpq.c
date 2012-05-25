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
 * Este programa está nomeado como rpq.c e possui 762 linhas de código.
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
 *	rpq.c
 *		RPQNAMES structured field support.
 *
 */

#include "globals.h"
#include <errno.h>
#if !defined(_WIN32) /*[*/
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /*]*/
#include <sys/types.h>
#if !defined(_WIN32) /*[*/
#include <sys/socket.h>
#include <netdb.h>
#endif /*]*/

#ifndef ANDROID
	#include <stdlib.h>
#endif // !ANDROID

#include "api.h"

#include <assert.h>
#include <stdarg.h>
#include "3270ds.h"
//#include "appres.h"

#include "popupsc.h"
#include "tablesc.h"
#include "telnetc.h"
#include "trace_dsc.h"
#include "utilc.h"
#include "sf.h"

/* Statics */
static Boolean select_rpq_terms(void);
static int get_rpq_timezone(void);
static int get_rpq_user(unsigned char buf[], const int buflen);
#if !defined(_WIN32) /*[*/
static int get_rpq_address(unsigned char buf[], const int buflen);
#endif /*]*/
static void rpq_warning(const char *fmt, ...);
static void rpq_dump_warnings(void);
static Boolean rpq_complained = False;

#if !defined(_WIN32) /*[*/
static Boolean omit_due_space_limit = False;
#endif /*]*/

/*
 * Define symbolic names for RPQ self-defining terms.
 * (Numbering is arbitrary, but must be 0-255 inclusive.
 * Do not renumber existing items because these identify the
 * self-defining term to the mainframe software. Changing pre-existing
 * values will possibly impact host based software.
 */
#define	RPQ_ADDRESS	0
#define	RPQ_TIMESTAMP	1
#define	RPQ_TIMEZONE	2
#define	RPQ_USER	3
#define	RPQ_VERSION	4

/*
 * Define a table of RPQ self-defing terms.
 * NOTE: Synonyms could be specified by coding different text items but using
 * the same "id" value.
 * Items should be listed in alphabetical order by "text" name so if the user
 * specifies abbreviations, they work in a predictable manner.  E.g., "TIME"
 * should match TIMESTAMP instead of TIMEZONE.
 */
static struct rpq_keyword {
	Boolean omit;	/* set from X3270RPQ="kw1:kw2..." environment var */
	int oride;	/* displacement */
	const Boolean allow_oride;
	const unsigned char id;
	const char *text;

} rpq_keywords[] = {
	{True, 0, 	True,	RPQ_ADDRESS,	"ADDRESS"},
	{True, 0, 	False,	RPQ_TIMESTAMP,	"TIMESTAMP"},
	{True, 0, 	True,	RPQ_TIMEZONE,	"TIMEZONE"},
	{True, 0, 	True,	RPQ_USER,	"USER"},
	{True, 0, 	False,	RPQ_VERSION,	"VERSION"},
};
#define NS_RPQ (sizeof(rpq_keywords)/sizeof(rpq_keywords[0]))

static char *x3270rpq;

/*
 * RPQNAMES query reply.
 */
void do_qr_rpqnames(void)
{
	#define TERM_PREFIX_SIZE 2	/* Each term has 1 byte length and 1
					   byte id */

	unsigned char *rpql, *p_term;
	int term_id,i,j,x;
	int remaining = 254;	/* maximum data area for rpqname reply */
	Boolean omit_due_space_limit;

	trace_ds("> QueryReply(RPQNames)\n");

	/*
	 * Allocate enough space for the maximum allowed item.
	 * By pre-allocating the space I don't have to worry about the
	 * possibility of addresses changing.
	 */
	space3270out(4+4+1+remaining);	/* Maximum space for an RPQNAME item */

	SET32(obptr, 0);		/* Device number, 0 = All */
	SET32(obptr, 0);		/* Model number, 0 = All */

	rpql = obptr++;			/* Save address to place data length. */

	/* Create fixed length portion - program id: x3270 */
	for (j = 0; j < 5; j++) {
		*obptr++ = asc2ebc[(int)"x3270"[j]];
		remaining--;
	}

	/* Create user selected variable-length self-defining terms. */
	select_rpq_terms();

	for (j=0; j<NS_RPQ; j++) {
		if (rpq_keywords[j].omit)
			continue;

		omit_due_space_limit = False;

		term_id = rpq_keywords[j].id;

		p_term = obptr;		/* save starting address (to insert
					   length later) */
		obptr++;		/* skip length of term, fill in
					   later */
		*obptr++ = term_id;	/* identify this term */

		/*
		 * Adjust remaining space by the term prefix size so each case
		 * can use the "remaining" space without concern for the
		 * prefix.  This subtraction is accounted for after the item
		 * is built and the updated remaining space is determined.
		 */
		remaining -= TERM_PREFIX_SIZE;

		switch (term_id) {	/* build the term based on id */
		case RPQ_USER:		/* User text from env. vars */
			obptr += get_rpq_user(obptr, remaining);
			break;

		case RPQ_TIMEZONE:	/* UTC time offset */
			omit_due_space_limit = (remaining < 2);
			if (!omit_due_space_limit)
				SET16(obptr, get_rpq_timezone());
			break;

		case RPQ_ADDRESS:	/* Workstation address */
#if !defined(_WIN32) /*[*/
			obptr += get_rpq_address(obptr, remaining);
#endif /*]*/
			break;

		case RPQ_VERSION:	/* program version */
			x = strlen(build_rpq_version);
			omit_due_space_limit = (x > remaining);
			if (!omit_due_space_limit) {
				for (i = 0; i < x; i++) {
					*obptr++ = asc2ebc[(int)(*(build_rpq_version+i) & 0xff)];
				}
			}
			break;

		case RPQ_TIMESTAMP:	/* program build time (yyyymmddhhmmss bcd) */
			x = strlen(build_rpq_timestamp);
			omit_due_space_limit = ((x+1)/2 > remaining);
			if (!omit_due_space_limit) {
				for (i=0; i < x; i+=2) {
					*obptr++ = ((*(build_rpq_timestamp+i) - '0') << 4)
						+ (*(build_rpq_timestamp+i+1) - '0');
				}
			}
			break;

		default:		/* unsupported ID, (can't happen) */
			Error(NULL,"Unsupported RPQ term");
			break;
		}

		if (omit_due_space_limit)
			rpq_warning("RPQ %s term omitted due to insufficient space", rpq_keywords[j].text);
		/*
		 * The item is built, insert item length as needed and
		 * adjust space remaining.
		 * obptr now points at "next available byte".
		 */
		x = obptr-p_term;
		if (x > TERM_PREFIX_SIZE) {
			*p_term = x;
			remaining -= x;	/* This includes length and id fields,
					   correction below */
		} else {
			/* We didn't add an item after all, reset pointer. */
			obptr = p_term;
		}
		/*
		 * When we calculated the length of the term, a few lines
		 * above, that length included the term length and term id
		 * prefix too. (TERM_PREFIX_SIZE)
		 * But just prior to the switch statement, we decremented the
		 * remaining space by that amount so subsequent routines would
		 * be told how much space they have for their data, without
		 * each routine having to account for that prefix.
		 * That means the remaining space is actually more than we
		 * think right now, by the length of the prefix.... add that
		 * back so the remaining space is accurate.
		 *
		 * And... if there was no item added, we still have to make the
		 * same correction to "claim back" the term prefix area so it
		 * may be used by the next possible term.
		 */
		remaining += TERM_PREFIX_SIZE;
	}

	/* Fill in overall length of RPQNAME info */
	*rpql = (obptr - rpql);

	rpq_dump_warnings();
}

/* Utility function used by the RPQNAMES query reply. */
static Boolean
select_rpq_terms(void)
{
	int i,j,k,len;
	char *uplist;
	char *p1, *p2;
	char *kw;
	Boolean is_no_form;


	/* See if the user wants any rpqname self-defining terms returned */
	if ((x3270rpq = getenv("X3270RPQ")) == NULL)
		return False;

	/*
	 * Make an uppercase copy of the user selections so I can match
	 * keywords more easily.
	 * If there are override values, I'll get those from the ORIGINAL
	 * string so upper/lower case is preserved as necessary.
	 */
	uplist = (char *) lib3270_malloc(strlen(x3270rpq)+1);
	assert(uplist != NULL);
	p1 = uplist;
	p2 = x3270rpq;
	do {
		*p1++ = toupper(*p2++);
	} while (*p2);
	*p1 = '\0';

	for (i=0; i<strlen(x3270rpq); ) {
		kw = uplist+i;
		i++;
		if (isspace(*kw))
			continue;	/* skip leading white space */
		if (*kw == ':') {
			continue;
		}

		/* : separates terms, but \: is literal : */
		p1 = kw;
		do {
			p1 = strchr(p1+1,':');
			if (p1 == NULL) break;
		} while (*(p1-1) == '\\');
		/* p1 points to the : separating a term, or is NULL */
		if (p1 != NULL) *p1 = '\0';
		/* kw is now a string of the entire, single term. */

		i = (kw - uplist) + strlen(kw) + 1;
		/* It might be a keyword=value item... */

		for (p1=kw; *p1; p1++) {
			if (!isupper(*p1))
				break;
		}
		len = p1-kw;

		is_no_form = ((len > 2) && (strncmp("NO", kw, 2) == 0));
		if (is_no_form) {
			kw+=2;		/* skip "NO" prefix for matching
					   keyword */
			len-=2;		/* adjust keyword length */
		}
		for (j=0; j < NS_RPQ; j++) {
			if (strncmp(kw, rpq_keywords[j].text, len) == 0) {
				rpq_keywords[j].omit = is_no_form;
				if (*p1 == '=') {
					if (rpq_keywords[j].allow_oride) {
						rpq_keywords[j].oride = p1-uplist+1;
					} else {
						rpq_warning("RPQ %s term "
								"override "
								"ignored", p1);
					}
				}
				break;
			}
		}
		if (j >= NS_RPQ) {
			/* unrecognized keyword... */
			if (strcmp(kw,"ALL") == 0) {
				for (k=0; k < NS_RPQ; k++)
					rpq_keywords[k].omit = is_no_form;
			} else {
				rpq_warning("RPQ term \"%s\" is unrecognized",
						kw);
			}
		}
	}

	free(uplist);

	/*
	 * Return to caller with indication (T/F) of any items
	 * to be selected (T) or are all terms suppressed? (F)
	 */
	for (i=0; i<NS_RPQ; i++) {
		if (!rpq_keywords[i].omit)
			return True;
	}

	return False;
}

/* Utility function used by the RPQNAMES query reply. */
static int get_rpq_timezone(void)
{
	/*
	 * Return the signed number of minutes we're offset from UTC.
	 * Example: North America Pacific Standard Time = UTC - 8 Hours, so we
	 * return (-8) * 60 = -480.
	 * Since the smallest variance between two timezones is 15 minutes,
	 * use small, positive values to represent various errors:
	 * 1 - Cannot determine local calendar time
	 * 2 - Cannot determine UTC
	 * 3 - Difference exceeds 12 hours
	 * 4 - User override is invalid
	 */
	time_t here;
	struct tm here_tm;
	struct tm *utc_tm;
	double delta;
	char *p1, *p2;
	struct rpq_keyword *kw;


	/* id isn't necessarily the table index... locate item */
	for (kw = &rpq_keywords[0]; kw -> id != RPQ_TIMEZONE; kw++) {
	}

	/* Is there a user override? */
	if ((kw->allow_oride) && (kw->oride > 0)) {
		ldiv_t hhmm;
		long x;

		p1 = x3270rpq+kw->oride;

		x = strtol(p1, &p2, 10);
		if (errno != 0) {
			rpq_warning("RPQ TIMEZONE term is invalid - use +/-hhmm");
			return 4;
		}
		if ((*p2 != '\0') && (*p2 != ':') && (!isspace(*p2)))
			return 4;

		hhmm = ldiv(x, 100L);

		if (hhmm.rem > 59L) {
			rpq_warning("RPQ TIMEZONE term is invalid - use +/-hhmm");
			return 4;
		}

		delta = (labs(hhmm.quot) * 60L) + hhmm.rem;
		if (hhmm.quot < 0L) delta = -delta;
	} else {
		/*
		 * No override specified, try to get information from the
		 * system.
		 */
		if ((here = time(NULL)) == (time_t)(-1))
		{
			rpq_warning("RPQ: Unable to determine workstation local time");
			return 1;
		}
		memcpy(&here_tm, localtime(&here), sizeof(struct tm));

		if ((utc_tm = gmtime(&here)) == NULL)
		{
			rpq_warning("RPQ: Unable to determine workstation UTC time");
			return 2;
		}

		/*
	 	 * Do not take Daylight Saving Time into account.
	 	 * We just want the "raw" time difference.
	 	 */
		here_tm.tm_isdst = 0;
		utc_tm->tm_isdst = 0;
		delta = difftime(mktime(&here_tm), mktime(utc_tm)) / 60L;
	}

	/* sanity check: difference cannot exceed +/- 12 hours */
	if (labs(delta) > 720L)
		rpq_warning("RPQ timezone exceeds 12 hour UTC offset");
	return (labs(delta) > 720L)? 3 : (int) delta;
}


/* Utility function used by the RPQNAMES query reply. */
static int
get_rpq_user(unsigned char buf[], const int buflen)
{
	/*
	 * Text may be specified in one of two ways, but not both.
	 * An environment variable provides the user interface:
	 *    - X3270RPQ: Keyword USER=
	 *
	 *    NOTE: If the string begins with 0x then no ASCII/EBCDIC
	 *    translation is done.  The hex characters will be sent as true hex
	 *    data.  E.g., X3270RPQ="user=0x ab 12 EF" will result in 3 bytes
	 *    sent as 0xAB12EF.  White space is optional in hex data format.
	 *    When hex format is required, the 0x prefix must be the first two
	 *    characters of the string.  E.g., X3270RPQ="user= 0X AB" will
	 *    result in 6 bytes sent as 0x40F0E740C1C2 because the text is
	 *    accepted "as is" then translated from ASCII to EBCDIC.
	 */
	const char *rpqtext = CN;
	int x;
	struct rpq_keyword *kw;

	/* id isn't necessarily the table index... locate item */
	for (kw = &rpq_keywords[0]; kw -> id != RPQ_USER; kw++) {
	}

	if ((!kw->allow_oride) || (kw->oride <= 0)) return 0;

	rpqtext = x3270rpq + kw->oride;

	if ((*rpqtext == '0') && (toupper(*(rpqtext+1)) == 'X')) {
		/* text has 0x prefix... interpret as hex, no translation */
		char hexstr[512];	/* more than enough room to copy */
		char * p_h;
		char c;
		int x;
		Boolean is_first_hex_digit;

		p_h = &hexstr[0];
		/* copy the hex digits from X3270RPQ, removing white
		 * space, and using all upper case for the hex digits a-f.
		 */
		rpqtext += 2;	/* skip 0x prefix */
		for (*p_h = '\0'; *rpqtext; rpqtext++) {
			c  = toupper(*rpqtext);
			if ((c==':') || (c=='\0'))
				break;
			if (isspace(c))
				continue;	 /* skip white space */
			if (!isxdigit(c)) {
				rpq_warning("RPQ USER term has non-hex character");
				break;
			}
			x = (p_h - hexstr)/2;
			if (x >= buflen) {
				x = buflen;
				rpq_warning("RPQ USER term truncated after %d bytes", x);
				break; /* too long, truncate */
			}

			*p_h++ = c;	/* copy (upper case) character */
			*p_h = '\0';	/* keep string properly terminated */
		}
		/*
		 * 'hexstr' is now a character string of 0-9, A-F only,
		 * (a-f were converted to upper case).
		 * There may be an odd number of characters, implying a leading
		 * 0.  The string is also known to fit in the area specified.
		 */

		/* hex digits are handled in pairs, set a flag so we keep track
		 * of which hex digit we're currently working with.
		 */
		is_first_hex_digit = ((strlen(hexstr) % 2) == 0);
		if (!is_first_hex_digit)
			rpq_warning("RPQ USER term has odd number of hex digits");
		*buf = 0;	/* initialize first byte for possible implied
				   leading zero */
		for (p_h = &hexstr[0]; *p_h; p_h++) {
			int n;
			/* convert the hex character to a value 0-15 */
			n = isdigit(*p_h) ? *p_h - '0' : *p_h - 'A' + 10;
			if (is_first_hex_digit) {
				*buf = n << 4;
			} else {
				*buf++ |= n;
			}
			is_first_hex_digit = !is_first_hex_digit;
		}
		return (strlen(hexstr)+1)/2;
	}

	/* plain text - subject to ascii/ebcdic translation */
	for (x=0; ; rpqtext++) {
		/* colon ends term (unless preceded by \) */
		if ((*rpqtext == ':') || (*rpqtext == '\0'))
			break;

		if ( x >= buflen) {
			x = buflen;
			rpq_warning("RPQ USER term truncated after %d characters", x);
			break;
		}

		/*
		 * \ means take next char as literal. Skip \ take next char.
		 * If there is no next char, then we'll take the \
		 */
		if ((*rpqtext == '\\') && (*(rpqtext+1) != '\0'))
			rpqtext++;

		*buf++ = asc2ebc[(int)(*rpqtext & 0xff)];
		x++;
	}
	return x;
}

#if !defined(_WIN32) /*[*/
static int
get_rpq_address(unsigned char *buf, const int maxlen)
{
	struct rpq_keyword *kw;
	int x = 0;

	if (maxlen < 2) {
		omit_due_space_limit = True;
		return 0;
	}

	/* id isn't necessarily the table index... locate item */
	for (kw = &rpq_keywords[0]; kw->id != RPQ_ADDRESS; kw++) {
	}

	/* Is there a user override? */
	if ((kw->allow_oride) && (kw->oride > 0)) {
		char *p1, *p2, *rpqtext;
#if defined(AF_INET6) /*[*/
		struct addrinfo *res;
		int ga_err;
#else /*][*/
		in_addr_t ia;
#endif /*]*/

		p1 = x3270rpq + kw->oride;
		rpqtext = (char *) lib3270_malloc(strlen(p1) + 1);
		for (p2=rpqtext;*p1; p2++) {
			if (*p1 == ':')
				break;
			if ((*p1 == '\\') && (*(p1+1) == ':'))
				p1++;
			*p2 = *p1;
			p1++;
		}
		*p2 = '\0';

#if defined(AF_INET6) /*[*/
		ga_err = getaddrinfo(rpqtext, NULL, NULL, &res);
		if (ga_err == 0) {
			void *src = NULL;
			int len = 0;

			SET16(buf, res->ai_family);
			x += 2;

			switch (res->ai_family) {
			case AF_INET:
				src = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
				len = sizeof(struct in_addr);
				break;
			case AF_INET6:
				src = &((struct sockaddr_in6 *)res->ai_addr)->sin6_addr;
				len = sizeof(struct in6_addr);
				break;
			default:
				rpq_warning("RPQ ADDRESS term has unrecognized family %u",
						res->ai_family);
				break;
			}

			if (x + len <= maxlen) {
				x += len;
				(void) memcpy(buf, src, len);
			} else {
				rpq_warning("RPQ ADDRESS term incomplete due to space limit");
			}
			/* Give back storage obtained by getaddrinfo */
			freeaddrinfo(res);
		} else {
			rpq_warning("RPQ: can't resolve '%s': %s",rpqtext, gai_strerror(ga_err));
		}
#else /*][*/
		/*
		 * No IPv6 support.
		 * Use plain old inet_addr() and gethostbyname().
		 */
		ia = inet_addr(rpqtext);
		if (ia == htonl(INADDR_NONE)) {
			struct hostent *h;

			h = gethostbyname(rpqtext);
			if (h == NULL || h->h_addrtype != AF_INET) {
				rpq_warning("RPQ: gethostbyname error");
				return 0;
			}
			(void) memcpy(&ia, h->h_addr_list[0], h->h_length);
		}
		SET16(buf, AF_INET);
		x += 2;
		if (x + sizeof(in_addr_t) <= maxlen) {
			(void) memcpy(buf, &ia, sizeof(in_addr_t));
			x += sizeof(in_addr_t);
		} else {
			rpq_warning("RPQ ADDRESS term incomplete due to space limit");
		}
#endif /*]*/
		free(rpqtext);
	} else {
		/* No override... get our address from the actual socket */
		union {
			struct sockaddr sa;
			struct sockaddr_in sa4;
#if defined(AF_INET6) /*[*/
			struct sockaddr_in6 sa6;
#endif /*]*/
		} u;
		int addrlen = sizeof(u);
		void *src = NULL;
		int len = 0;

		if(net_getsockname(&h3270, &u, &addrlen) < 0)
			return 0;
		SET16(buf, u.sa.sa_family);
		x += 2;
		switch (u.sa.sa_family) {
		case AF_INET:
			src = &u.sa4.sin_addr;
			len = sizeof(struct in_addr);
			break;
#if defined(AF_INET6) /*[*/
		case AF_INET6:
			src = &u.sa6.sin6_addr;
			len = sizeof(struct in6_addr);
			break;
#endif /*]*/
		default:
			rpq_warning("RPQ ADDRESS term has unrecognized family %u", u.sa.sa_family);
			break;
		}
		if (x + len <= maxlen) {
			(void) memcpy(buf, src, len);
			x += len;
		} else {
			rpq_warning("RPQ ADDRESS term incomplete due to space limit");
		}
	}
	return x;
}
#endif /*]*/

#define RPQ_WARNBUF_SIZE	1024
static char * rpq_warnbuf	= CN;
static int    rpq_wbcnt 	= 0;

static void rpq_warning(const char *fmt, ...)
{
	va_list a;

	va_start(a, fmt);
	lib3270_write_va_log(&h3270,"RPQ",fmt,a);
	va_end(a);

	/*
	 * Only accumulate RPQ warnings if they
	 * have not been displayed already.
	 */
	if (!rpq_complained)
	{
		va_start(a, fmt);
		if (rpq_warnbuf == CN)
			rpq_warnbuf = lib3270_malloc(RPQ_WARNBUF_SIZE);

		if (rpq_wbcnt < RPQ_WARNBUF_SIZE)
		{
			*(rpq_warnbuf + rpq_wbcnt++) = '\n';
			*(rpq_warnbuf + rpq_wbcnt) = '\0';
		}

		if (rpq_wbcnt < RPQ_WARNBUF_SIZE)
		{
			rpq_wbcnt += vsnprintf(rpq_warnbuf + rpq_wbcnt,RPQ_WARNBUF_SIZE - rpq_wbcnt, fmt, a);
		}
		va_end(a);
	}
}

static void rpq_dump_warnings(void)
{
	/* If there's something to complain about, only complain once. */
	if (!rpq_complained && rpq_wbcnt)
	{
		popup_an_error(NULL,rpq_warnbuf);
		rpq_wbcnt = 0;
		rpq_complained = True;

		free(rpq_warnbuf);
		rpq_warnbuf = CN;
	}
}
