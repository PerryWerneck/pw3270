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
 * Este programa está nomeado como mkfb.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

/*
 * mkfb.c
 *	Utility to create RDB string definitions from a simple #ifdef'd .ad
 *	file
 */

#include "../include/config.h"


#if defined( WIN32 )
	#include <windows.h>
	#define tmpfile w32_tmpfile
#elif defined( __APPLE__ )
	#define tmpfile osx_tmpfile
#endif // OS

#include <sys/stat.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>

#define BUFSZ	1024		/* input line buffer size */
#define ARRSZ	8192		/* output array size */
#define SSSZ	10		/* maximum nested ifdef */

unsigned aix[ARRSZ];		/* fallback array indices */
unsigned xlno[ARRSZ];		/* fallback array line numbers */
unsigned n_fallbacks = 0;	/* number of fallback entries */

/* ifdef state stack */
#define MODE_COLOR	0x00000001
#define MODE_FT		0x00000002
#define MODE_TRACE	0x00000004
#define MODE_MENUS	0x00000008
#define MODE_ANSI	0x00000010
#define MODE_KEYPAD	0x00000020
#define MODE_APL	0x00000040
#define MODE_PRINTER	0x00000080
#define MODE_STANDALONE	0x00000100
#define MODE_SCRIPT	0x00000200
#define MODE_DBCS	0x00000400
#define MODE__WIN32	0x00000800

#define MODEMASK	0x00000fff

struct {
	unsigned long ifdefs;
	unsigned long ifndefs;
	unsigned lno;
} ss[SSSZ];
int ssp = 0;

struct {
	const char *name;
	unsigned long mask;
} parts[] = {
	{ "COLOR", MODE_COLOR },
	{ "X3270_FT", MODE_FT },
	{ "X3270_TRACE", MODE_TRACE },
	{ "X3270_MENUS", MODE_MENUS },
	{ "X3270_ANSI", MODE_ANSI },
	{ "X3270_KEYPAD", MODE_KEYPAD },
	{ "X3270_APL", MODE_APL },
	{ "X3270_PRINTER", MODE_PRINTER },
	{ "STANDALONE", MODE_STANDALONE },
	{ "X3270_SCRIPT", MODE_SCRIPT },
	{ "X3270_DBCS", MODE_DBCS },
	{ "_WIN32", MODE__WIN32 }
};
#define NPARTS	(sizeof(parts)/sizeof(parts[0]))

unsigned long is_defined =
    MODE_COLOR |
#if defined(X3270_FT)
	MODE_FT
#else
	0
#endif
|
#if defined(X3270_TRACE)
	MODE_TRACE
#else
	0
#endif
|
#if defined(X3270_MENUS)
	MODE_MENUS
#else
	0
#endif
|
#if defined(X3270_ANSI)
	MODE_ANSI
#else
	0
#endif
|
#if defined(X3270_KEYPAD)
	MODE_KEYPAD
#else
	0
#endif
|
#if defined(X3270_APL)
	MODE_APL
#else
	0
#endif
|
#if defined(X3270_PRINTER)
	MODE_PRINTER
#else
	0
#endif
|
#if defined(X3270_SCRIPT)
	MODE_SCRIPT
#else
	0
#endif
|
#if defined(X3270_DBCS)
	MODE_DBCS
#else
	0
#endif
|
#if defined(_WIN32)
	MODE__WIN32
#else
	0
#endif
    ;
unsigned long is_undefined;

char *me;

void emit(FILE *t, int ix, char c);

void
usage(void)
{
	fprintf(stderr, "usage: %s [infile [outfile]]\n", me);
	exit(1);
}

int
main(int argc, char *argv[])
{
	char buf[BUFSZ];
	int lno = 0;
	int cc = 0;
	int i;
	int continued = 0;
	const char *filename = "standard input";
	FILE *u, *t, *tc = NULL, *tm = NULL, *o;
	int cmode = 0;
	unsigned long ifdefs;
	unsigned long ifndefs;
	int last_continue = 0;

	/* Parse arguments. */
	if ((me = strrchr(argv[0], '/')) != (char *)NULL)
		me++;
	else
		me = argv[0];
	if (argc > 1 && !strcmp(argv[1], "-c")) {
	    cmode = 1;
	    is_defined |= MODE_STANDALONE;
	    argc--;
	    argv++;
	}
	switch (argc) {
	    case 1:
		break;
	    case 2:
	    case 3:
		if (strcmp(argv[1], "-")) {
			if (freopen(argv[1], "r", stdin) == (FILE *)NULL) {
				perror(argv[1]);
				exit(1);
			}
			filename = argv[1];
		}
		break;
	    default:
		usage();
	}

	is_undefined = MODE_COLOR | (~is_defined & MODEMASK);

	/* Do #ifdef, comment and whitespace processing first. */
	u = tmpfile();
	if (u == NULL) {
		perror("tmpfile");
		exit(1);
	}

	while (fgets(buf, BUFSZ, stdin) != (char *)NULL) {
		char *s = buf;
		int sl;
		int i;

		lno++;

		/* Skip leading white space. */
		while (isspace(*s))
			s++;
		if (cmode &&
		    (!strncmp(s, "x3270.", 6) || !strncmp(s, "x3270*", 6))) {
			s += 6;
		}

		/* Remove trailing white space. */
		while ((sl = strlen(s)) && isspace(s[sl-1]))
			s[sl-1] = '\0';

		/* Skip comments and empty lines. */
		if ((!last_continue && *s == '!') || !*s)
			continue;

		/* Check for simple if[n]defs. */
		if (*s == '#') {
			int ifnd = 1;

			if (!strncmp(s, "#ifdef ", 7) ||
			    !(ifnd = strncmp(s, "#ifndef ", 8))) {
				char *tk;

				if (ssp >= SSSZ) {
					fprintf(stderr,
					    "%s, line %d: Stack overflow\n",
					    filename, lno);
					exit(1);
				}
				ss[ssp].ifdefs = 0L;
				ss[ssp].ifndefs = 0L;
				ss[ssp].lno = lno;

				tk = s + 7 + !ifnd;
				for (i = 0; i < NPARTS; i++) {
					if (!strcmp(tk, parts[i].name)) {
						if (!ifnd)
							ss[ssp++].ifndefs =
							    parts[i].mask;
						else
							ss[ssp++].ifdefs =
							    parts[i].mask;
						break;
					}
				}
				if (i >= NPARTS) {
					fprintf(stderr,
					    "%s, line %d: Unknown condition\n",
					    filename, lno);
					exit(1);
				}
				continue;
			}

			else if (!strcmp(s, "#else")) {
				unsigned long tmp;

				if (!ssp) {
					fprintf(stderr,
					    "%s, line %d: Missing #if[n]def\n",
					    filename, lno);
					exit(1);
				}
				tmp = ss[ssp-1].ifdefs;
				ss[ssp-1].ifdefs = ss[ssp-1].ifndefs;
				ss[ssp-1].ifndefs = tmp;
			} else if (!strcmp(s, "#endif")) {
				if (!ssp) {
					fprintf(stderr,
					    "%s, line %d: Missing #if[n]def\n",
					    filename, lno);
					exit(1);
				}
				ssp--;
			} else {
				fprintf(stderr,
				    "%s, line %d: Unrecognized # directive\n",
				    filename, lno);
				exit(1);
			}
			continue;
		}

		/* Figure out if there's anything to emit. */

		/* First, look for contradictions. */
		ifdefs = 0;
		ifndefs = 0;
		for (i = 0; i < ssp; i++) {
			ifdefs |= ss[i].ifdefs;
			ifndefs |= ss[i].ifndefs;
		}
		if (ifdefs & ifndefs) {
#ifdef DEBUG_IFDEFS
			fprintf(stderr, "contradiction, line %d\n", lno);
#endif
			continue;
		}

		/* Then, apply the actual values. */
		if (ifdefs && (ifdefs & is_defined) != ifdefs) {
#ifdef DEBUG_IFDEFS
			fprintf(stderr, "ifdef failed, line %d\n", lno);
#endif
			continue;
		}
		if (ifndefs && (ifndefs & is_undefined) != ifndefs) {
#ifdef DEBUG_IFDEFS
			fprintf(stderr, "ifndef failed, line %d\n", lno);
#endif
			continue;
		}

		/* Emit the text. */
		fprintf(u, "%lx %lx %d\n%s\n", ifdefs, ifndefs, lno, s);
		last_continue = strlen(s) > 0 && s[strlen(s) - 1] == '\\';
	}
	if (ssp) {
		fprintf(stderr, "%d missing #endif(s) in %s\n", ssp, filename);
		fprintf(stderr, "last #ifdef was at line %u\n", ss[ssp-1].lno);
		exit(1);
	}

	/* Re-scan, emitting code this time. */
	rewind(u);
	t = tmpfile();
	if (t == NULL) {
		perror("tmpfile");
		exit(1);
	}
	if (!cmode) {
		tc = tmpfile();
		if (tc == NULL) {
			perror("tmpfile");
			exit(1);
		}
		tm = tmpfile();
		if (tm == NULL) {
			perror("tmpfile");
			exit(1);
		}
	}

	/* Emit the initial boilerplate. */
	fprintf(t, "/* This file was created automatically from %s by mkfb. */\n\n",
	    filename);
	if (cmode) {
		fprintf(t, "#include \"private.h\"\n");
		fprintf(t, "static unsigned char fsd[] = {\n");
	} else {
		fprintf(t, "unsigned char common_fallbacks[] = {\n");
		fprintf(tc, "unsigned char color_fallbacks[] = {\n");
		fprintf(tm, "unsigned char mono_fallbacks[] = {\n");
	}

	/* Scan the file, emitting the fsd array and creating the indices. */
	while (fscanf(u, "%lx %lx %d\n", &ifdefs, &ifndefs, &lno) == 3) {
		char *s = buf;
		char c;
		int white;
		FILE *t_this = t;
		int ix = 0;

		if (fgets(buf, BUFSZ, u) == NULL)
			break;
		if (strlen(buf) > 0 && buf[strlen(buf)-1] == '\n')
			buf[strlen(buf)-1] = '\0';

#if 0
		fprintf(stderr, "%lx %lx %d %s\n", ifdefs, ifndefs, lno, buf);
#endif

		/* Add array offsets. */
		if (cmode) {
			/* Ignore color.  Accumulate offsets into an array. */
			if (n_fallbacks >= ARRSZ) {
				fprintf(stderr, "%s, line %d: Buffer overflow\n", filename, lno);
				exit(1);
			}
			aix[n_fallbacks] = cc;
			xlno[n_fallbacks++] = lno;
		} else {
			/* Use color to decide which file to write into. */
			if (!(ifdefs & MODE_COLOR) && !(ifndefs & MODE_COLOR)) {
				/* Both. */
				t_this = t;
				ix = 0;
			} else if (ifdefs & MODE_COLOR) {
				/* Just color. */
				t_this = tc;
				ix = 1;
			} else {
				/* Just mono. */
				t_this = tm;
				ix = 2;
			}
		}

		continued = 0;
		white = 0;
		while ((c = *s++) != '\0') {
			if (c == ' ' || c == '\t')
				white++;
			else if (white) {
				emit(t_this, ix, ' ');
				cc++;
				white = 0;
			}
			switch (c) {
			    case ' ':
			    case '\t':
				break;
			    case '#':
				if (!cmode) {
					emit(t_this, ix, '\\');
					emit(t_this, ix, '#');
					cc += 2;
				} else {
					emit(t_this, ix, c);
					cc++;
				}
				break;
			    case '\\':
				if (*s == '\0') {
					continued = 1;
					break;
				} else if (cmode) {
				    switch ((c = *s++)) {
				    case 't':
					c = '\t';
					break;
				    case 'n':
					c = '\n';
					break;
				    default:
					break;
				    }
				}
				/* else fall through */
			    default:
				emit(t_this, ix, c);
				cc++;
				break;
			}
		}
		if (white) {
			emit(t_this, ix, ' ');
			cc++;
			white = 0;
		}
		if (!continued) {
			if (cmode)
				emit(t_this, ix, 0);
			else
				emit(t_this, ix, '\n');
			cc++;
		}
	}
	fclose(u);
	if (cmode)
		fprintf(t, "};\n\n");
	else {
		emit(t, 0, 0);
		fprintf(t, "};\n\n");
		emit(tc, 0, 0);
		fprintf(tc, "};\n\n");
		emit(tm, 0, 0);
		fprintf(tm, "};\n\n");
	}


	/* Open the output file. */
	if (argc == 3) {
		o = fopen(argv[2], "w");
		if (o == NULL) {
			perror(argv[2]);
			exit(1);
		}
	} else
		o = stdout;

	/* Copy tmp to output. */
	rewind(t);
	if (!cmode) {
		rewind(tc);
		rewind(tm);
	}
	while (fgets(buf, sizeof(buf), t) != NULL) {
		fprintf(o, "%s", buf);
	}
	if (!cmode) {
		while (fgets(buf, sizeof(buf), tc) != NULL) {
			fprintf(o, "%s", buf);
		}
		while (fgets(buf, sizeof(buf), tm) != NULL) {
			fprintf(o, "%s", buf);
		}
	}

	if (cmode) {
		/* Emit the fallback array. */
		fprintf(o, "String fallbacks[%u] = {\n", n_fallbacks + 1);
		for (i = 0; i < n_fallbacks; i++) {
			fprintf(o, "\t(String)&fsd[%u], /* line %u */\n",
					aix[i],
			    xlno[i]);
		}
		fprintf(o, "\t(String)NULL\n};\n\n");

		/* Emit some test code. */
		fprintf(o, "%s", "#if defined(DUMP) /*[*/\n\
#include <stdio.h>\n\
int\n\
main(int argc, char *argv[])\n\
{\n\
	int i;\n\
\n\
	for (i = 0; fallbacks[i] != NULL; i++)\n\
		printf(\"%d: %s\\n\", i, fallbacks[i]);\n\
	return 0;\n\
}\n");
		fprintf(o, "#endif /*]*/\n\n");
	}

	if (o != stdout)
		fclose(o);
	fclose(t);
	if (!cmode) {
		fclose(tc);
		fclose(tm);
	}

	return 0;
}

static int n_out[3] = { 0, 0, 0 };

void
emit(FILE *t, int ix, char c)
{
	if (n_out[ix] >= 19) {
		fprintf(t, "\n");
		n_out[ix] = 0;
	}
	fprintf(t, "%3d,", (unsigned char)c);
	n_out[ix]++;
}

#if defined(_WIN32)
FILE * w32_tmpfile( void )
{
	char *dir;
	char *xtemplate;
	DWORD retval;
	size_t len;
	int fd;
	FILE *file = NULL;

	dir = (char *) malloc(PATH_MAX);
	xtemplate = (char *) malloc(PATH_MAX);

	/* Find Windows temporary file directory.
	We provide this as the directory argument to path_search
	because Windows defines P_tmpdir to "\\" and will therefore
	try to put all temporary files in the root (unless $TMPDIR
	is set). */
	retval = GetTempPath (PATH_MAX, dir);
	if (retval == 0 || retval >= PATH_MAX - 1)
		goto done;

	do
	{
		char *tempname = tempnam(dir,"XXXXXX");
		if(!tempname)
			goto done;

		fd = _open (tempname,_O_BINARY | _O_CREAT | _O_TEMPORARY | _O_EXCL | _O_RDWR,_S_IREAD | _S_IWRITE);
	}
	while (fd < 0 && errno == EEXIST);

	if (fd < 0)
		goto done;

	file = _fdopen (fd, "w+b");
	if (file == NULL)
	{
		int save_errno = errno;
		_close (fd);
		errno = save_errno;
	}

	done:
	free(xtemplate);
	free(dir);
	return file;
}
#elif defined( __APPLE__ )
FILE * osx_tmpfile( void )
{
	int fd = -1;
	FILE *file = NULL;

	do
	{
		char *tempname = tempnam(NULL,"XXXXXX");
		if(!tempname)
			return NULL;
		fd = open (tempname,O_CREAT | O_EXCL | O_RDWR,S_IREAD | S_IWRITE);
	} while (fd < 0 && errno == EEXIST);


	file = fdopen (fd, "w+b");
	if (file == NULL)
	{
		int save_errno = errno;
		close (fd);
		errno = save_errno;
	}

	return file;
}

#endif // _WIN32
