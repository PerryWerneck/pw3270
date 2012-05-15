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
 * Este programa está nomeado como util.c e possui 978 linhas de código.
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
 *	util.c
 *		Utility functions for x3270/c3270/s3270/tcl3270
 */

#if defined(_WIN32)
	#include <winsock2.h>
	#include <windows.h>
#endif // _WIN32

#include "globals.h"

#if defined(_WIN32)

	#include "winversc.h"
	#include <ws2tcpip.h>
	#include <stdio.h>
	#include <errno.h>
	#include "w3miscc.h"

#else
	#include <pwd.h>
#endif // _WIN32

#ifdef HAVE_MALLOC_H
	#include <malloc.h>
#endif

#ifndef ANDROID
	#include <stdlib.h>
#endif // !ANDROID

#include <stdarg.h>
#include "resources.h"

#include "utilc.h"
#include "api.h"

#define my_isspace(c)	isspace((unsigned char)c)


#if defined(_WIN32)

int is_nt = 1;
int has_ipv6 = 1;

int get_version_info(void)
{
	OSVERSIONINFO info;

	// Figure out what version of Windows this is.
	memset(&info, '\0', sizeof(info));
	info.dwOSVersionInfoSize = sizeof(info);
	if(GetVersionEx(&info) == 0)
	{
		lib3270_write_log(NULL,"lib3270","%s","Can't get Windows version");
		return -1;
	}

	// Yes, people still run Win98.
	if (info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		is_nt = 0;

	// Win2K and earlier is IPv4-only.  WinXP and later can have IPv6.
	if (!is_nt || info.dwMajorVersion < 5 || (info.dwMajorVersion == 5 && info.dwMinorVersion < 1))
	{
		has_ipv6 = 0;
	}

	return 0;
}

// Convert a network address to a string.
const char * inet_ntop(int af, const void *src, char *dst, socklen_t cnt)
{
    	union {
	    	struct sockaddr sa;
		struct sockaddr_in sin;
		struct sockaddr_in6 sin6;
	} sa;
	DWORD ssz;
	DWORD sz = cnt;

	memset(&sa, '\0', sizeof(sa));

	switch (af) {
	case AF_INET:
	    	sa.sin = *(struct sockaddr_in *)src;	// struct copy
		ssz = sizeof(struct sockaddr_in);
		break;
	case AF_INET6:
	    	sa.sin6 = *(struct sockaddr_in6 *)src;	// struct copy
		ssz = sizeof(struct sockaddr_in6);
		break;
	default:
	    	if (cnt > 0)
			dst[0] = '\0';
		return NULL;
	}

	sa.sa.sa_family = af;

	if (WSAAddressToString(&sa.sa, ssz, NULL, dst, &sz) != 0) {
	    	if (cnt > 0)
			dst[0] = '\0';
		return NULL;
	}

	return dst;
}

// Decode a Win32 error number.
const char * win32_strerror(int e)
{
	static char buffer[4096];

	if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
	    NULL,
	    e,
	    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	    buffer,
	    sizeof(buffer),
	    NULL) == 0) {

	    sprintf(buffer, "Windows error %d", e);
	}

	return buffer;
}


#endif // _WIN32

/*
 * Cheesy internal version of sprintf that allocates its own memory.
 */
char * xs_vsprintf(const char *fmt, va_list args)
{
	char *r;
#if defined(HAVE_VASPRINTF) /*[*/
	(void) vasprintf(&r, fmt, args);
	if(!r)
		Error(NULL,"Out of memory in %s",__FUNCTION__);
	return r;
#else /*][*/
	char buf[16384];
	int nc;

	nc = vsprintf(buf, fmt, args);
	if (nc > sizeof(buf))
		Error(NULL,"Internal buffer overflow");
	r = lib3270_malloc(nc + 1);
	return strcpy(r, buf);
#endif /*]*/
}

/*
 * Common helper functions to insert strings, through a template, into a new
 * buffer.
 * 'format' is assumed to be a printf format string with '%s's in it.
 */
char *
xs_buffer(const char *fmt, ...)
{
	va_list args;
	char *r;

	va_start(args, fmt);
	r = xs_vsprintf(fmt, args);
	va_end(args);
	return r;
}

/* Common uses of xs_buffer. */
void
xs_warning(const char *fmt, ...)
{
	va_list args;
	char *r;

	va_start(args, fmt);
	r = xs_vsprintf(fmt, args);
	va_end(args);
	Warning(NULL,r);
	lib3270_free(r);
}

void
xs_error(const char *fmt, ...)
{
	va_list args;
	char *r;

	va_start(args, fmt);
	r = xs_vsprintf(fmt, args);
	va_end(args);
	Error(NULL,r);
	lib3270_free(r);
}

/* Prettyprinter for strings with unprintable data. */
void
fcatv(FILE *f, char *s)
{
	char c;

	while ((c = *s++)) {
		switch (c) {
		    case '\n':
			(void) fprintf(f, "\\n");
			break;
		    case '\t':
			(void) fprintf(f, "\\t");
			break;
		    case '\b':
			(void) fprintf(f, "\\b");
			break;
		    default:
			if ((c & 0x7f) < ' ')
				(void) fprintf(f, "\\%03o", c & 0xff);
			else
				fputc(c, f);
			break;
		}
	}
}

/* String version of fcatv. */
char *
scatv(const char *s, char *buf, size_t len)
{
	char c;
	char *dst = buf;

	while ((c = *s++) && len > 0) {
		char cbuf[5];
		char *t = cbuf;

		/* Expand this character. */
		switch (c) {
		    case '\n':
			(void) strcpy(cbuf, "\\n");
			break;
		    case '\t':
			(void) strcpy(cbuf, "\\t");
			break;
		    case '\b':
			(void) strcpy(cbuf, "\\b");
			break;
		    default:
			if ((c & 0x7f) < ' ')
				(void) sprintf(cbuf, "\\%03o", c & 0xff);
			else {
				cbuf[0] = c;
				cbuf[1] = '\0';
			}
			break;
		}
		/* Copy as much as will fit. */
		while ((c = *t++) && len > 0) {
			*dst++ = c;
			len--;
		}
	}
	if (len > 0)
		*dst = '\0';

	return buf;
}

/*
 * Definition resource splitter, for resources of the repeating form:
 *	left: right\n
 *
 * Can be called iteratively to parse a list.
 * Returns 1 for success, 0 for EOF, -1 for error.
 *
 * Note: Modifies the input string.
 */
int
split_dresource(char **st, char **left, char **right)
{
	char *s = *st;
	char *t;
	Boolean quote;

	/* Skip leading white space. */
	while (my_isspace(*s))
		s++;

	/* If nothing left, EOF. */
	if (!*s)
		return 0;

	/* There must be a left-hand side. */
	if (*s == ':')
		return -1;

	/* Scan until an unquoted colon is found. */
	*left = s;
	for (; *s && *s != ':' && *s != '\n'; s++)
		if (*s == '\\' && *(s+1) == ':')
			s++;
	if (*s != ':')
		return -1;

	/* Stip white space before the colon. */
	for (t = s-1; my_isspace(*t); t--)
		*t = '\0';

	/* Terminate the left-hand side. */
	*(s++) = '\0';

	/* Skip white space after the colon. */
	while (*s != '\n' && my_isspace(*s))
		s++;

	/* There must be a right-hand side. */
	if (!*s || *s == '\n')
		return -1;

	/* Scan until an unquoted newline is found. */
	*right = s;
	quote = False;
	for (; *s; s++) {
		if (*s == '\\' && *(s+1) == '"')
			s++;
		else if (*s == '"')
			quote = !quote;
		else if (!quote && *s == '\n')
			break;
	}

	/* Strip white space before the newline. */
	if (*s) {
		t = s;
		*st = s+1;
	} else {
		t = s-1;
		*st = s;
	}
	while (my_isspace(*t))
		*t-- = '\0';

	/* Done. */
	return 1;
}

/*
 * Split a DBCS resource into its parts.
 * Returns the number of parts found:
 *	-1 error (empty sub-field)
 *	 0 nothing found
 *	 1 one and just one thing found
 *	 2 two things found
 *	 3 more than two things found
 */
int
split_dbcs_resource(const char *value, char sep, char **part1, char **part2)
{
	int n_parts = 0;
	const char *s = value;
	const char *f_start = CN;	/* start of sub-field */
	const char *f_end = CN;		/* end of sub-field */
	char c;
	char **rp;

	*part1 = CN;
	*part2 = CN;

	for( ; ; ) {
		c = *s;
		if (c == sep || c == '\0') {
			if (f_start == CN)
				return -1;
			if (f_end == CN)
				f_end = s;
			if (f_end == f_start) {
				if (c == sep) {
					if (*part1) {
						lib3270_free(*part1);
						*part1 = NULL;
					}
					if (*part2) {
						lib3270_free(*part2);
						*part2 = NULL;
					}
					return -1;
				} else
					return n_parts;
			}
			switch (n_parts) {
			case 0:
				rp = part1;
				break;
			case 1:
				rp = part2;
				break;
			default:
				return 3;
			}
			*rp = lib3270_malloc(f_end - f_start + 1);
			strncpy(*rp, f_start, f_end - f_start);
			(*rp)[f_end - f_start] = '\0';
			f_end = CN;
			f_start = CN;
			n_parts++;
			if (c == '\0')
				return n_parts;
		} else if (isspace(c)) {
			if (f_end == CN)
				f_end = s;
		} else {
			if (f_start == CN)
				f_start = s;
			f_end = CN;
		}
		s++;
	}
}

#if defined(X3270_DISPLAY) /*[*/
/*
 * List resource splitter, for lists of elements speparated by newlines.
 *
 * Can be called iteratively.
 * Returns 1 for success, 0 for EOF, -1 for error.
 */
int
split_lresource(char **st, char **value)
{
	char *s = *st;
	char *t;
	Boolean quote;

	/* Skip leading white space. */
	while (my_isspace(*s))
		s++;

	/* If nothing left, EOF. */
	if (!*s)
		return 0;

	/* Save starting point. */
	*value = s;

	/* Scan until an unquoted newline is found. */
	quote = False;
	for (; *s; s++) {
		if (*s == '\\' && *(s+1) == '"')
			s++;
		else if (*s == '"')
			quote = !quote;
		else if (!quote && *s == '\n')
			break;
	}

	/* Strip white space before the newline. */
	if (*s) {
		t = s;
		*st = s+1;
	} else {
		t = s-1;
		*st = s;
	}
	while (my_isspace(*t))
		*t-- = '\0';

	/* Done. */
	return 1;
}
#endif /*]*/


/*
#if !defined(LIB3270)

const char *
get_message(const char *key)
{
	static char namebuf[128];
	char *r;

	(void) sprintf(namebuf, "%s.%s", ResMessage, key);
	if ((r = get_resource(namebuf)) != CN)
		return r;
	else {
		(void) sprintf(namebuf, "[missing \"%s\" message]", key);
		return namebuf;
	}
}

#endif
*/

// #define ex_getenv getenv

/* Variable and tilde substitution functions. */
static char *
var_subst(const char *s)
{
	enum { VS_BASE, VS_QUOTE, VS_DOLLAR, VS_BRACE, VS_VN, VS_VNB, VS_EOF }
	    state = VS_BASE;
	char c;
	int o_len = strlen(s) + 1;
	char *ob;
	char *o;
	const char *vn_start = CN;

	if (strchr(s, '$') == CN)
		return NewString(s);

	o_len = strlen(s) + 1;
	ob = lib3270_malloc(o_len);
	o = ob;
#	define LBR	'{'
#	define RBR	'}'

	while (state != VS_EOF) {
		c = *s;
		switch (state) {
		    case VS_BASE:
			if (c == '\\')
			    state = VS_QUOTE;
			else if (c == '$')
			    state = VS_DOLLAR;
			else
			    *o++ = c;
			break;
		    case VS_QUOTE:
			if (c == '$') {
				*o++ = c;
				o_len--;
			} else {
				*o++ = '\\';
				*o++ = c;
			}
			state = VS_BASE;
			break;
		    case VS_DOLLAR:
			if (c == LBR)
				state = VS_BRACE;
			else if (isalpha(c) || c == '_') {
				vn_start = s;
				state = VS_VN;
			} else {
				*o++ = '$';
				*o++ = c;
				state = VS_BASE;
			}
			break;
		    case VS_BRACE:
			if (isalpha(c) || c == '_') {
				vn_start = s;
				state = VS_VNB;
			} else {
				*o++ = '$';
				*o++ = LBR;
				*o++ = c;
				state = VS_BASE;
			}
			break;
		    case VS_VN:
		    case VS_VNB:
			if (!(isalnum(c) || c == '_')) {
				int vn_len;
				char *vn;
				char *vv;

				vn_len = s - vn_start;
				if (state == VS_VNB && c != RBR) {
					*o++ = '$';
					*o++ = LBR;
					(void) strncpy(o, vn_start, vn_len);
					o += vn_len;
					state = VS_BASE;
					continue;	/* rescan */
				}
				vn = lib3270_malloc(vn_len + 1);
				(void) strncpy(vn, vn_start, vn_len);
				vn[vn_len] = '\0';

#ifndef ANDROID
				if((vv = getenv(vn)))
				{
					*o = '\0';
					o_len = o_len
					    - 1			/* '$' */
					    - (state == VS_VNB)	/* { */
					    - vn_len		/* name */
					    - (state == VS_VNB)	/* } */
					    + strlen(vv);
					ob = Realloc(ob, o_len);
					o = strchr(ob, '\0');
					(void) strcpy(o, vv);
					o += strlen(vv);
				}
#endif // !ANDROID

				lib3270_free(vn);
				if (state == VS_VNB) {
					state = VS_BASE;
					break;
				} else {
					/* Rescan this character */
					state = VS_BASE;
					continue;
				}
			}
			break;
		    case VS_EOF:
			break;
		}
		s++;
		if (c == '\0')
			state = VS_EOF;
	}
	return ob;
}

#if !defined(_WIN32) /*[*/
/*
 * Do tilde (home directory) substitution on a string.  Returns a malloc'd
 * result.
 */
static char *
tilde_subst(const char *s)
{
	char *slash;
	const char *name;
	const char *rest;
	struct passwd *p;
	char *r;
	char *mname = CN;

	/* Does it start with a "~"? */
	if (*s != '~')
		return NewString(s);

	/* Terminate with "/". */
	slash = strchr(s, '/');
	if (slash) {
		int len = slash - s;

		mname = lib3270_malloc(len + 1);
		(void) strncpy(mname, s, len);
		mname[len] = '\0';
		name = mname;
		rest = slash;
	} else {
		name = s;
		rest = strchr(name, '\0');
	}

	/* Look it up. */
	if (!strcmp(name, "~"))	/* this user */
		p = getpwuid(getuid());
	else			/* somebody else */
		p = getpwnam(name + 1);

	/* Free any temporary copy. */
	lib3270_free(mname);

	/* Substitute and return. */
	if (p == (struct passwd *)NULL)
		r = NewString(s);
	else {
		r = lib3270_malloc(strlen(p->pw_dir) + strlen(rest) + 1);
		(void) strcpy(r, p->pw_dir);
		(void) strcat(r, rest);
	}
	return r;
}
#endif /*]*/

char *
do_subst(const char *s, Boolean do_vars, Boolean do_tilde)
{
	if (!do_vars && !do_tilde)
		return NewString(s);

	if (do_vars) {
		char *t;

		t = var_subst(s);
#if !defined(_WIN32) /*[*/
		if (do_tilde) {
			char *u;

			u = tilde_subst(t);
			lib3270_free(t);
			return u;
		}
#endif /*]*/
		return t;
	}

#if !defined(_WIN32) /*[*/
	return tilde_subst(s);
#else /*][*/
	return NewString(s);
#endif /*]*/
}

/*
 * ctl_see
 *	Expands a character in the manner of "cat -v".
 */
char *
ctl_see(int c)
{
	static char	buf[64];
	char	*p = buf;

	c &= 0xff;
	if ((c & 0x80) && (c <= 0xa0)) {
		*p++ = 'M';
		*p++ = '-';
		c &= 0x7f;
	}
	if (c >= ' ' && c != 0x7f) {
		*p++ = c;
	} else {
		*p++ = '^';
		if (c == 0x7f) {
			*p++ = '?';
		} else {
			*p++ = c + '@';
		}
	}
	*p = '\0';
	return buf;
}

/*
 * Whitespace stripper.
 */
char *
strip_whitespace(const char *s)
{
	char *t = NewString(s);

	while (*t && my_isspace(*t))
		t++;
	if (*t) {
		char *u = t + strlen(t) - 1;

		while (my_isspace(*u)) {
			*u-- = '\0';
		}
	}
	return t;
}

/*
 * Hierarchy (a>b>c) splitter.
 */
Boolean
split_hier(char *label, char **base, char ***parents)
{
	int n_parents = 0;
	char *gt;
	char *lp;

	label = NewString(label);
	for (lp = label; (gt = strchr(lp, '>')) != CN; lp = gt + 1) {
		if (gt == lp)
			return False;
		n_parents++;
	}
	if (!*lp)
		return False;

	if (n_parents) {
		*parents = (char **)Calloc(n_parents + 1, sizeof(char *));
		for (n_parents = 0, lp = label;
		     (gt = strchr(lp, '>')) != CN;
		     lp = gt + 1) {
			(*parents)[n_parents++] = lp;
			*gt = '\0';
		}
		*base = lp;
	} else {
		(*parents) = NULL;
		(*base) = label;
	}
	return True;
}

/*
 * Incremental, reallocing version of snprintf.
 */
#define RPF_BLKSIZE	4096
#define SP_TMP_LEN	16384

/* Initialize an RPF structure. */
void
rpf_init(rpf_t *r)
{
	r->buf = NULL;
	r->alloc_len = 0;
	r->cur_len = 0;
}

/* Reset an initialized RPF structure (re-use with length 0). */
void
rpf_reset(rpf_t *r)
{
	r->cur_len = 0;
}

/* Append a string to a dynamically-allocated buffer. */
void
rpf(rpf_t *r, char *fmt, ...)
{
	va_list a;
	Boolean need_realloc = False;
	int ns;
	char tbuf[SP_TMP_LEN];

	/* Figure out how much space would be needed. */
	va_start(a, fmt);
	ns = vsprintf(tbuf, fmt, a); /* XXX: dangerous, but so is vsnprintf */
	va_end(a);
	if (ns >= SP_TMP_LEN)
	    Error(NULL,"rpf overrun");

	/* Make sure we have that. */
	while (r->alloc_len - r->cur_len < ns + 1) {
		r->alloc_len += RPF_BLKSIZE;
		need_realloc = True;
	}
	if (need_realloc) {
		r->buf = Realloc(r->buf, r->alloc_len);
	}

	/* Scribble onto the end of that. */
	(void) strcpy(r->buf + r->cur_len, tbuf);
	r->cur_len += ns;
}

/* Free resources associated with an RPF. */
void
rpf_free(rpf_t *r)
{
	lib3270_free(r->buf);
	r->buf = NULL;
	r->alloc_len = 0;
	r->cur_len = 0;
}

/*
#if defined(X3270_DISPLAY)

// Glue between x3270 and the X libraries.

//
// A way to work around problems with Xt resources.  It seems to be impossible
// to get arbitrarily named resources.  Someday this should be hacked to
// add classes too.
//
char * get_resource(const char *name)
{
	XrmValue value;
	char *type;
	char *str;
	char *r = CN;

	str = xs_buffer("%s.%s", XtName(toplevel), name);
	if ((XrmGetResource(rdb, str, 0, &type, &value) == True) && *value.addr)
		r = value.addr;
	XtFree(str);

	lib3270_write_log(&h3270,"resource","%s=\"%s\"",name,r);

	return r;
}

//
// Input callbacks.
//
typedef void voidfn(void);

typedef struct iorec {
	voidfn		*fn;
	XtInputId	 id;
	struct iorec	*next;
} iorec_t;

static iorec_t *iorecs = NULL;

static void
io_fn(XtPointer closure, int *source unused, XtInputId *id)
{
	iorec_t *iorec;

	for (iorec = iorecs; iorec != NULL; iorec = iorec->next) {
	    if (iorec->id == *id) {
		(*iorec->fn)();
		break;
	    }
	}
}

unsigned long
AddInput(int sock, voidfn *fn)
{
	iorec_t *iorec;

	iorec = (iorec_t *)XtMalloc(sizeof(iorec_t));
	iorec->fn = fn;
	iorec->id = XtAppAddInput(appcontext, sock,
		(XtPointer) XtInputReadMask, io_fn, NULL);

	iorec->next = iorecs;
	iorecs = iorec;

	return iorec->id;
}

unsigned long
AddExcept(int sock, voidfn *fn)
{
	iorec_t *iorec;

	iorec = (iorec_t *)XtMalloc(sizeof(iorec_t));
	iorec->fn = fn;
	iorec->id = XtAppAddInput(appcontext, sock,
		(XtPointer) XtInputExceptMask, io_fn, NULL);
	iorec->next = iorecs;
	iorecs = iorec;

	return iorec->id;
}

unsigned long
AddOutput(int sock, voidfn *fn)
{
	iorec_t *iorec;

	iorec = (iorec_t *)XtMalloc(sizeof(iorec_t));
	iorec->fn = fn;
	iorec->id = XtAppAddInput(appcontext, sock,
		(XtPointer) XtInputWriteMask, io_fn, NULL);
	iorec->next = iorecs;
	iorecs = iorec;

	return iorec->id;
}

void
RemoveInput(unsigned long cookie)
{
	iorec_t *iorec;
	iorec_t *prev = NULL;

	for (iorec = iorecs; iorec != NULL; iorec = iorec->next) {
	    if (iorec->id == (XtInputId)cookie) {
		break;
	    }
	    prev = iorec;
	}

	if (iorec != NULL) {
		XtRemoveInput((XtInputId)cookie);
		if (prev != NULL)
			prev->next = iorec->next;
		else
			iorecs = iorec->next;
		XtFree((XtPointer)iorec);
	}
}

//
/ Timer callbacks.
//

typedef struct torec {
	voidfn		*fn;
	XtIntervalId	 id;
	struct torec	*next;
} torec_t;

static torec_t *torecs = NULL;

static void
to_fn(XtPointer closure, XtIntervalId *id)
{
	torec_t *torec;
	torec_t *prev = NULL;
	voidfn *fn = NULL;

	for (torec = torecs; torec != NULL; torec = torec->next) {
		if (torec->id == *id) {
			break;
		}
		prev = torec;
	}

	if (torec != NULL) {

    	// Remember the record.
		fn = torec->fn;

		// Free the record.
		if (prev != NULL)
			prev->next = torec->next;
		else
			torecs = torec->next;
		XtFree((XtPointer)torec);

		// Call the function.
		(*fn)();
	}
}

unsigned long
AddTimeOut(unsigned long msec, voidfn *fn)
{
	torec_t *torec;

	torec = (torec_t *)XtMalloc(sizeof(torec_t));
	torec->fn = fn;
	torec->id = XtAppAddTimeOut(appcontext, msec, to_fn, NULL);
	torec->next = torecs;
	torecs = torec;
	return (unsigned long)torec->id;
}

void
RemoveTimeOut(unsigned long cookie)
{
	torec_t *torec;
	torec_t *prev = NULL;

	for (torec = torecs; torec != NULL; torec = torec->next) {
		if (torec->id == (XtIntervalId)cookie) {
			break;
		}
		prev = torec;
	}

	if (torec != NULL) {
		XtRemoveTimeOut((XtIntervalId)cookie);
		if (prev != NULL)
			prev->next = torec->next;
		else
			torecs = torec->next;
		XtFree((XtPointer)torec);
	} else {
		Error("RemoveTimeOut: Can't find");
	}
}

KeySym
StringToKeysym(char *s)
{
	return XStringToKeysym(s);
}
#endif
*/

LIB3270_EXPORT void lib3270_free(void *p)
{
	if(p)
		free(p);
}

LIB3270_EXPORT void * lib3270_realloc(void *p, int len)
{
	p = realloc(p, len);
	if(!p)
		Error(NULL,"Out of memory in %s",__FUNCTION__);
	return p;
}

LIB3270_EXPORT void * lib3270_calloc(int elsize, int nelem, void *ptr)
{
	size_t sz = nelem * elsize;

	if(ptr)
		ptr = realloc(ptr,sz);
	else
		ptr = malloc(sz);

	if(ptr)
		memset(ptr,0,sz);
	else
		Error(NULL,"Out of memory in %s",__FUNCTION__);

	return ptr;
}


LIB3270_EXPORT void * lib3270_malloc(int len)
{
	char *r;

	r = malloc(len);
	if (r == (char *)NULL)
	{
		Error(NULL,"Out of memory in %s",__FUNCTION__);
		return 0;
	}

	memset(r,0,len);
	return r;
}

/*
void * Calloc(size_t nelem, size_t elsize)
{
	int		  sz = nelem * elsize;
	char	* r = malloc(sz);

	if(!r)
		Error(NULL,"Out of memory in %s",__FUNCTION__);

	memset(r, 0, sz);
	return r;
}
*/

LIB3270_EXPORT char * lib3270_get_resource_string(const char *first_element, ...)
{
#ifdef ANDROID

	#warning Work in progress

#else

	char 		* str		= lib3270_malloc(4097);
	char 		* ptr 		= str;
	const char	* element;
	va_list		  args;
	const char	* res;

	va_start(args, first_element);

	for(element = first_element;element;element = va_arg(args, const char *))
    {
    	if(ptr != str)
			*(ptr++) = '.';

		strncpy(ptr,element,4096-strlen(str));
		ptr += strlen(ptr);
    }

	va_end(args);

	*ptr = 0;

	res = get_resource(str);

	trace("%s(%s)=%s",__FUNCTION__,str,res ? res : "NULL");

	lib3270_free(str);

	if(res)
		return strdup(res);
#endif
	return NULL;
}

LIB3270_EXPORT const char * lib3270_get_version(void)
{
	return build_rpq_version;
}

LIB3270_EXPORT const char * lib3270_get_revision(void)
{
	return build_rpq_revision;
}
