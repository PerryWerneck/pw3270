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
 * Este programa está nomeado como resources.c e possui 154 linhas de código.
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


#include <stdio.h>
#include <string.h>

#include "globals.h"
#include "utilc.h"

extern String fallbacks[];

/* s3270 substitute Xt resource database. */

#if defined(C3270) /*[*/
/*
 * These should be properly #ifdef'd in X3270.xad, but it would turn it into
 * spaghetti.
 */
static struct {
        char *name;
        char *value;
} rdb[] = {
	{ "message.hour",       "hour" },
	{ "message.hours",      "hours" },
	{ "message.minute",     "minute" },
	{ "message.buildDisabled",	"disabled" },
	{ "message.buildEnabled",	"enabled" },
	{ "message.buildOpts",	"Build options:" },
	{ "message.byte",       "byte" },
	{ "message.bytes",      "bytes" },
	{ "message.characterSet",       "EBCDIC character set:" },
	{ "message.charMode",   "NVT character mode" },
	{ "message.columns",    "columns" },
	{ "message.connectedTo",        "Connected to:" },
	{ "message.connectionPending",  "Connection pending to:" },
	{ "message.defaultCharacterSet",        "Default (us) EBCDIC character set" },
	{ "message.dsMode",     "3270 mode" },
	{ "message.extendedDs", "extended data stream" },
	{ "message.fullColor",  "color" },
	{ "message.hostCodePage", "Host code page:" },
	{ "message.keyboardMap",        "Keyboard map:" },
	{ "message.lineMode",   "NVT line mode" },
	{ "message.localeCodeset",	"Locale codeset:" },
	{ "message.luName",     "LU name:" },
	{ "message.minute",     "minute" },
	{ "message.minutes",    "minutes" },
	{ "message.model",      "Model" },
	{ "message.mono",       "monochrome" },
	{ "message.notConnected",       "Not connected" },
	{ "message.port",       "Port:" },
	{ "message.proxyType",  "Proxy type:" },
	{ "message.Received",   "Received" },
	{ "message.received",   "received" },
	{ "message.record",     "record" },
	{ "message.records",    "records" },
	{ "message.rows",       "rows" },
	{ "message.second",     "second" },
	{ "message.seconds",    "seconds" },
	{ "message.secure",     "via TLS/SSL" },
	{ "message.sent",       "Sent" },
	{ "message.server",     "Server:" },
	{ "message.specialCharacters",  "Special characters:" },
	{ "message.sscpMode",   "SSCP-LU mode" },
	{ "message.standardDs", "standard data stream" },
	{ "message.terminalName",       "Terminal name:" },
	{ "message.tn3270eNoOpts",      "No TN3270E options" },
	{ "message.tn3270eOpts",        "TN3270E options:" },
#if defined(_WIN32) /*[*/
	{ "message.windowsCodePage",	"Windows code page:" },
#endif /*][*/
	{ NULL, NULL }
};
#endif /*]*/

static struct dresource {
	struct dresource *next;
	const char *name;
	const char *value;
} *drdb = NULL, **drdb_next = &drdb;

void add_resource(const char *name, const char *value)
{
	struct dresource *d;

	for (d = drdb; d != NULL; d = d->next) {
		if (!strcmp(d->name, name)) {
			d->value = value;
			return;
		}
	}
	d = lib3270_malloc(sizeof(struct dresource));
	d->next = NULL;
	d->name = name;
	d->value = value;
	*drdb_next = d;
	drdb_next = &d->next;
}


const char * get_resource(const char *name)
{
	struct dresource *d;
	int i;

	for (d = drdb; d != NULL; d = d->next)
	{
		if (!strcmp(d->name, name))
		{
			lib3270_write_log(&h3270,"resource","%s=\"%s\"",name,d->value);
			return d->value;
		}
	}

	for (i = 0; fallbacks[i] != NULL; i++)
	{
		if (!strncmp(fallbacks[i], name, strlen(name)) && *(fallbacks[i] + strlen(name)) == ':')
		{
			const char *ret =  fallbacks[i] + strlen(name) + 2;
			lib3270_write_log(&h3270,"resource","%s=\"%s\"",name,ret);
			return ret;
		}
	}

#if defined(C3270)
	for (i = 0; rdb[i].name != (char *)NULL; i++)
	{
		if (!strcmp(rdb[i].name, name))
		{
			lib3270_write_log(&h3270,"resource","%s=\"%s\"",name,rdb[i].value);
			return rdb[i].value;
		}
	}
#endif
	return NULL;
}

/* A version of get_resource that accepts sprintf arguments. */
const char * get_fresource(const char *fmt, ...)
{
	va_list args;
	char *name;
	const char *r;

	va_start(args, fmt);
	name = xs_vsprintf(fmt, args);
	va_end(args);
	r = get_resource(name);
	lib3270_free(name);
	return r;
}

