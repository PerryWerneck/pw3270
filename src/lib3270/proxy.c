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
 * Este programa está nomeado como proxy.c e possui - linhas de código.
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
 *	proxy.c
 *		This module implements various kinds of proxies.
 */

#ifdef _WIN32
	#include <winsock2.h>
	#include <windows.h>
#endif // _WIN32

#include "globals.h"
#include "utilc.h"

//#include "appres.h"
#include "resources.h"

#ifndef ANDROID
	#include <stdlib.h>
#endif

#if defined(_WIN32)
	#include <ws2tcpip.h>
	#include "w3miscc.h"
#else

	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <arpa/inet.h>

//	#if defined(HAVE_SYS_SELECT_H)
//	#include <sys/select.h>
//	#endif

#endif

#include "api.h"

#include "3270ds.h"
#include "popupsc.h"
#include "proxyc.h"
#include "resolverc.h"
#include "telnetc.h"
#include "trace_dsc.h"
#include "w3miscc.h"
#include "screen.h"

// #if defined(PR3287)
// extern char *proxy_spec;
// #endif

/*
 * Supported proxy types.
 *
 * At some point we will add SOCKS.
 */
enum
{
	PT_NONE,
	PT_PASSTHRU,	/**< Sun telnet-passthru */
	PT_HTTP,		/**< RFC 2817 CONNECT tunnel */
	PT_TELNET,		/**< 'connect host port' proxy */
	PT_SOCKS4,		/**< SOCKS version 4 (or 4A if necessary) */
	PT_SOCKS4A,		/**< SOCKS version 4A (force remote name resolution) */
	PT_SOCKS5,		/**< SOCKS version 5 (RFC 1928) */
	PT_SOCKS5D,		/**< SOCKS version 5 (force remote name resolution) */
	PT_MAX
} proxytype_t;

/* proxy type names -- keep these in sync with proxytype_t! */
char *type_name[] =
{
	"unknown",
	"passthru",
	"HTTP",
	"TELNET",
	"SOCKS4",
	"SOCKS4A",
	"SOCKS5",
	"SOCKS5D"
};

#define PROXY_PASSTHRU	"passthru"
#define PORT_PASSTHRU	"3514"

#define PROXY_HTTP	"http"
#define PORT_HTTP	"3128"

#define PROXY_TELNET	"telnet"

#define PROXY_SOCKS4	"socks4"
#define PORT_SOCKS4	"1080"

#define PROXY_SOCKS4A	"socks4a"
#define PORT_SOCKS4A	"1080"

#define PROXY_SOCKS5	"socks5"
#define PORT_SOCKS5	"1080"

#define PROXY_SOCKS5D	"socks5d"
#define PORT_SOCKS5D	"1080"

static int parse_host_port(char *s, char **phost, char **pport);

static int proxy_passthru(int fd, char *host, unsigned short port);
static int proxy_http(int fd, char *host, unsigned short port);
static int proxy_telnet(int fd, char *host, unsigned short port);
static int proxy_socks4(int fd, char *host, unsigned short port, int force_a);
static int proxy_socks5(int fd, char *host, unsigned short port, int force_d);


char * proxy_type_name(int type)
{
    	if (type < 1 || type >= PT_MAX)
	    	return "unknown";
	else
	    	return type_name[type];
}

/*
 * Resolve the type, hostname and port for a proxy.
 * Returns -1 for failure, 0 for no proxy, >0 (the proxy type) for success.
 */
int proxy_setup(H3270 *session, char **phost, char **pport)
{
	char	* proxy = session->proxy;
	char	* colon;
	int		  sl;

	if(!proxy)
		return PT_NONE;

	if ((colon = strchr(proxy, ':')) == CN || (colon == proxy))
	{
    	popup_an_error(session,_( "Invalid proxy syntax" ) );
		return -1;
	}

	sl = colon - proxy;
	if (sl == strlen(PROXY_PASSTHRU) && !strncasecmp(proxy, PROXY_PASSTHRU, sl))
	{
		if (parse_host_port(colon + 1, phost, pport) < 0)
	    	return -1;

		if (*pport == CN)
	    	*pport = NewString(PORT_PASSTHRU);

		return PT_PASSTHRU;
	}

	if (sl == strlen(PROXY_HTTP) && !strncasecmp(proxy, PROXY_HTTP, sl))
	{

		if (parse_host_port(colon + 1, phost, pport) < 0)
		    	return -1;
		if (*pport == CN)
		    	*pport = NewString(PORT_HTTP);
		return PT_HTTP;
	}

	if (sl == strlen(PROXY_TELNET) && !strncasecmp(proxy, PROXY_TELNET, sl))
	{

		if (parse_host_port(colon + 1, phost, pport) < 0)
		    	return -1;
		if (*pport == CN)
		{
		   	popup_an_error(session,_( "Must specify port for telnet proxy" ) );
			return -1;
		}
		return PT_TELNET;
	}

	if (sl == strlen(PROXY_SOCKS4) && !strncasecmp(proxy, PROXY_SOCKS4, sl))
	{
		if (parse_host_port(colon + 1, phost, pport) < 0)
	    	return -1;

		if (*pport == CN)
	    	*pport = NewString(PORT_SOCKS4);

		return PT_SOCKS4;
	}

	if (sl == strlen(PROXY_SOCKS4A) && !strncasecmp(proxy, PROXY_SOCKS4A, sl))
	{
		if (parse_host_port(colon + 1, phost, pport) < 0)
	    	return -1;

		if (*pport == CN)
	    	*pport = NewString(PORT_SOCKS4A);

		return PT_SOCKS4A;
	}

	if (sl == strlen(PROXY_SOCKS5) && !strncasecmp(proxy, PROXY_SOCKS5, sl))
	{
		if (parse_host_port(colon + 1, phost, pport) < 0)
	    	return -1;

		if (*pport == CN)
		    *pport = NewString(PORT_SOCKS5);

		return PT_SOCKS5;
	}

	if (sl == strlen(PROXY_SOCKS5D) && !strncasecmp(proxy, PROXY_SOCKS5D, sl))
	{
		if (parse_host_port(colon + 1, phost, pport) < 0)
	    	return -1;

		if (*pport == CN)
	    	*pport = NewString(PORT_SOCKS5D);

		return PT_SOCKS5D;
	}

	popup_an_error(session,"Invalid proxy type '%.*s'", sl, proxy);

	return -1;
}

/*
 * Parse host[:port] from a string.
 * 'host' can be in square brackets to allow numeric IPv6 addresses.
 * Returns the host name and port name in heap memory.
 * Returns -1 for failure, 0 for success.
 */
static int
parse_host_port(char *s, char **phost, char **pport)
{
	char *colon;
	char *hstart;
	int hlen;

    	if (*s == '[') {
	    	char *rbrack;

	    	/* Hostname in square brackets. */
		hstart = s + 1;
	    	rbrack = strchr(s, ']');
		if (rbrack == CN ||
			rbrack == s + 1 ||
			(*(rbrack + 1) != '\0' && *(rbrack + 1) != ':')) {

			popup_an_error(NULL,"Invalid proxy hostname syntax");
			return -1;
		}
		if (*(rbrack + 1) == ':')
			colon = rbrack + 1;
		else
		    	colon = NULL;
		hlen = rbrack - (s + 1);
	} else {
		hstart = s;
	    	colon = strchr(s, ':');
		if (colon == s) {
			popup_an_error(NULL,"Invalid proxy hostname syntax");
			return -1;
		}
		if (colon == NULL)
		    	hlen = strlen(s);
		else
		    	hlen = colon - s;
	}

	/* Parse the port. */
	if (colon == CN || !*(colon + 1))
	    	*pport = CN;
	else
	    	*pport = NewString(colon + 1);

	/* Copy out the hostname. */
	*phost = lib3270_malloc(hlen + 1);
	strncpy(*phost, hstart, hlen);
	(*phost)[hlen] = '\0';
	return 0;
}

/*
 * Negotiate with the proxy server.
 * Returns -1 for failure, 0 for success.
 */
int
proxy_negotiate(int type, int fd, char *host, unsigned short port)
{
	switch (type) {
	case PT_NONE:
	    	return 0;
	case PT_PASSTHRU:
		return proxy_passthru(fd, host, port);
	case PT_HTTP:
		return proxy_http(fd, host, port);
	case PT_TELNET:
		return proxy_telnet(fd, host, port);
	case PT_SOCKS4:
		return proxy_socks4(fd, host, port, 0);
	case PT_SOCKS4A:
		return proxy_socks4(fd, host, port, 1);
	case PT_SOCKS5:
		return proxy_socks5(fd, host, port, 0);
	case PT_SOCKS5D:
		return proxy_socks5(fd, host, port, 1);
	default:
		return -1;
	}
}

/* Sun PASSTHRU proxy. */
static int
proxy_passthru(int fd, char *host, unsigned short port)
{
	char *buf;

	buf = lib3270_malloc(strlen(host) + 32);
	(void) sprintf(buf, "%s %u\r\n", host, port);

#if defined(X3270_TRACE) /*[*/
    trace_dsn("Passthru Proxy: xmit '%.*s'", (int) (strlen(buf) - 2), buf);
	trace_netdata('>', (unsigned char *)buf, (int) strlen(buf));
#endif /*]*/

	if (send(fd, buf, strlen(buf), 0) < 0) {
	    	popup_a_sockerr(NULL,"Passthru Proxy: send error");
		lib3270_free(buf);
		return -1;
	}
	lib3270_free(buf);

    	return 0;
}

/* HTTP (RFC 2817 CONNECT tunnel) proxy. */
static int
proxy_http(int fd, char *host, unsigned short port)
{
    	char *buf;
	char *colon;
	char rbuf[1024];
	int nr;
	int nread = 0;
	char *space;

	/* Send the CONNECT request. */
	buf = lib3270_malloc(64 + strlen(host));
	colon = strchr(host, ':');
	sprintf(buf, "CONNECT %s%s%s:%u HTTP/1.1\r\n",
		(colon? "[": ""),
		host,
		(colon? "]": ""),
		port);

#if defined(X3270_TRACE) /*[*/
	trace_dsn("HTTP Proxy: xmit '%.*s'\n", (int) (strlen(buf) - 2), buf);
	trace_netdata('>', (unsigned char *)buf, (int) strlen(buf));
#endif /*]*/

	if (send(fd, buf, strlen(buf), 0) < 0) {
	    	popup_a_sockerr(NULL,"HTTP Proxy: send error");
		lib3270_free(buf);
		return -1;
	}

	sprintf(buf, "Host: %s%s%s:%u\r\n",
		(colon? "[": ""),
		host,
		(colon? "]": ""),
		port);

#if defined(X3270_TRACE) /*[*/
	trace_dsn("HTTP Proxy: xmit '%.*s'\n", (int) (strlen(buf) - 2), buf);
	trace_netdata('>', (unsigned char *)buf, (int) strlen(buf));
#endif /*]*/

	if (send(fd, buf, strlen(buf), 0) < 0) {
	    	popup_a_sockerr(NULL,"HTTP Proxy: send error");
		lib3270_free(buf);
		return -1;
	}

	strcpy(buf, "\r\n");
#if defined(X3270_TRACE) /*[*/
	trace_dsn("HTTP Proxy: xmit ''\n");
	trace_netdata('>', (unsigned char *)buf, strlen(buf));
#endif /*]*/

	if (send(fd, buf, strlen(buf), 0) < 0) {
	    	popup_a_sockerr(NULL,"HTTP Proxy: send error");
		lib3270_free(buf);
		return -1;
	}
	lib3270_free(buf);

	/*
	 * Process the reply.
	 * Read a byte at a time until \n or EOF.
	 */
	for (;;) {
	    	fd_set rfds;
		struct timeval tv;

		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		tv.tv_sec = 15;
		tv.tv_usec = 0;
		if (select(fd + 1, &rfds, NULL, NULL, &tv) < 0) {
		    	popup_an_error(NULL,"HTTP Proxy: server timeout");
#if defined(X3270_TRACE) /*[*/
		    	if (nread)
				trace_netdata('<', (unsigned char *)rbuf, nread);
#endif /*]*/
			return -1;
		}

	    	nr = recv(fd, &rbuf[nread], 1, 0);
		if (nr < 0) {
			popup_a_sockerr(NULL,"HTTP Proxy: receive error");
#if defined(X3270_TRACE) /*[*/
		    	if (nread)
				trace_netdata('<', (unsigned char *)rbuf, nread);
#endif /*]*/
			return -1;
		}
		if (nr == 0) {
#if defined(X3270_TRACE) /*[*/
		    	if (nread)
				trace_netdata('<', (unsigned char *)rbuf, nread);
#endif /*]*/
			popup_an_error(NULL,"HTTP Proxy: unexpected EOF");
			return -1;
		}
		if (rbuf[nread] == '\r')
		    	continue;
		if (rbuf[nread] == '\n')
		    	break;
		if (++nread >= sizeof(rbuf)) {
			nread = sizeof(rbuf) - 1;
		    	break;
		}
	}
	rbuf[nread] = '\0';

#if defined(X3270_TRACE) /*[*/
	trace_netdata('<', (unsigned char *)rbuf, nread);
	trace_dsn("HTTP Proxy: recv '%s'\n", rbuf);
#endif /*]*/

	if (strncmp(rbuf, "HTTP/", 5) || (space = strchr(rbuf, ' ')) == CN) {
	    	popup_an_error(NULL,"HTTP Proxy: unrecognized reply");
		return -1;
	}
	if (*(space + 1) != '2') {
	    	popup_an_error(NULL,"HTTP Proxy: CONNECT failed:\n%s", rbuf);
		return -1;
	}

    	return 0;
}

/* TELNET proxy. */
static int
proxy_telnet(int fd, char *host, unsigned short port)
{
	char *buf;

	buf = lib3270_malloc(strlen(host) + 32);
	(void) sprintf(buf, "connect %s %u\r\n", host, port);

#if defined(X3270_TRACE) /*[*/
	trace_dsn("TELNET Proxy: xmit '%.*s'", (int) (strlen(buf) - 2), buf);
	trace_netdata('>', (unsigned char *)buf, (int) strlen(buf));
#endif /*]*/

	if (send(fd, buf, strlen(buf), 0) < 0) {
	    	popup_a_sockerr(NULL,"TELNET Proxy: send error");
		lib3270_free(buf);
		return -1;
	}
	lib3270_free(buf);

    	return 0;
}

/* SOCKS version 4 proxy. */
#if defined(HAVE_GETADDRINFO)
static int proxy_socks4(int fd, char *host, unsigned short port, int force_a)
{
	popup_an_error(NULL,"%s", _( "Unsupported socks 4 proxy" ) );
}
#else
static int proxy_socks4(int fd, char *host, unsigned short port, int force_a)
{
    	struct hostent *hp;
	struct in_addr ipaddr;
	int use_4a = 0;
	char *user;
    	char *buf;
	char *s;
	char rbuf[8];
	int nr;
	int nread = 0;
#if defined(X3270_TRACE) /*[*/
	unsigned short rport;
#endif

	/* Resolve the hostname to an IPv4 address. */
	if (force_a)
	    	use_4a = 1;
	else {
		hp = gethostbyname(host);
		if (hp != NULL) {
			memcpy(&ipaddr, hp->h_addr, hp->h_length);
		} else {
			ipaddr.s_addr = inet_addr(host);
			if (ipaddr.s_addr == -1)
				use_4a = 1;
		}
	}

	/* Resolve the username. */
#ifdef ANDROID
	user = "android";
#else
	user = getenv("USER");
	if (user == CN)
	    	user = "nobody";
#endif

	/* Send the request to the server. */
	if (use_4a) {
	    	buf = lib3270_malloc(32 + strlen(user) + strlen(host));
		s = buf;
		*s++ = 0x04;
		*s++ = 0x01;
		SET16(s, port);
		SET32(s, 0x00000001);
		strcpy(s, user);
		s += strlen(user) + 1;
		strcpy(s, host);
		s += strlen(host) + 1;

#if defined(X3270_TRACE) /*[*/
		trace_dsn("SOCKS4 Proxy: version 4 connect port %u "
			"address 0.0.0.1 user '%s' host '%s'\n",
			port, user, host);
		trace_netdata('>', (unsigned char *)buf, s - buf);
#endif /*]*/

		if (send(fd, buf, s - buf, 0) < 0) {
		    	popup_a_sockerr(NULL,"SOCKS4 Proxy: send error");
			lib3270_free(buf);
			return -1;
		}
		lib3270_free(buf);
	} else {
	    	unsigned long u;

	    	buf = lib3270_malloc(32 + strlen(user));
		s = buf;
		*s++ = 0x04;
		*s++ = 0x01;
		SET16(s, port);
		u = ntohl(ipaddr.s_addr);
		SET32(s, u);
		strcpy(s, user);
		s += strlen(user) + 1;

#if defined(X3270_TRACE) /*[*/
		trace_dsn("SOCKS4 Proxy: xmit version 4 connect port %u "
			"address %s user '%s'\n",
			port, inet_ntoa(ipaddr), user);
		trace_netdata('>', (unsigned char *)buf, s - buf);
#endif /*]*/

		if (send(fd, buf, s - buf, 0) < 0) {
			lib3270_free(buf);
		    	popup_a_sockerr(NULL,"SOCKS4 Proxy: send error");
			return -1;
		}
		lib3270_free(buf);
	}

	/*
	 * Process the reply.
	 * Read 8 bytes of response.
	 */
	for (;;) {
	    	fd_set rfds;
		struct timeval tv;

		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		tv.tv_sec = 15;
		tv.tv_usec = 0;
		if (select(fd + 1, &rfds, NULL, NULL, &tv) < 0) {
		    	popup_an_error(NULL,"SOCKS4 Proxy: server timeout");
			return -1;
		}

	    	nr = recv(fd, &rbuf[nread], 1, 0);
		if (nr < 0) {
			popup_a_sockerr(NULL,"SOCKS4 Proxy: receive error");
			return -1;
		}
		if (nr == 0)
		    	break;
		if (++nread >= sizeof(rbuf))
		    	break;
	}

#if defined(X3270_TRACE) /*[*/
	trace_netdata('<', (unsigned char *)rbuf, nread);
	if (use_4a) {
	    	struct in_addr a;

	    	rport = (rbuf[2] << 8) | rbuf[3];
	    	memcpy(&a, &rbuf[4], 4);
		trace_dsn("SOCKS4 Proxy: recv status 0x%02x port %u "
			"address %s\n",
			rbuf[1],
			rport,
			inet_ntoa(a));
	} else
		trace_dsn("SOCKS4 Proxy: recv status 0x%02x\n", rbuf[1]);
#endif /*]*/

	switch (rbuf[1]) {
	case 0x5a:
	    	break;
	case 0x5b:
		popup_an_error(NULL,"SOCKS4 Proxy: request rejected or failed");
		return -1;
	case 0x5c:
		popup_an_error(NULL,"SOCKS4 Proxy: client is not reachable");
		return -1;
	case 0x5d:
		popup_an_error(NULL,"SOCKS4 Proxy: userid error");
		return -1;
	default:
		popup_an_error(NULL,"SOCKS4 Proxy: unknown status 0x%02x",
			rbuf[1]);
		return -1;
	}

    	return 0;
}
#endif // HAVE_GETADDRINFO

/* SOCKS version 5 (RFC 1928) proxy. */
static int
proxy_socks5(int fd, char *host, unsigned short port, int force_d)
{
	union {
	    	struct sockaddr sa;
		struct sockaddr_in sin;
#if defined(AF_INET6) /*[*/
		struct sockaddr_in6 sin6;
#endif /*]*/
	} ha;
	socklen_t ha_len = 0;
	int use_name = 0;
    	char *buf;
	char *s;
	unsigned char rbuf[8];
	int nr;
	int nread = 0;
	int n2read = 0;
	char nbuf[256];
	int done = 0;
#if defined(X3270_TRACE) /*[*/
	char *atype_name[] = {
	    "",
	    "IPv4",
	    "",
	    "domainname",
	    "IPv6"
	};
	unsigned char *portp;
#endif /*]*/
	unsigned short rport;

	if (force_d)
	    	use_name = 1;
	else {
	    	char errmsg[1024];
		int rv;

		/* Resolve the hostname. */
		rv = resolve_host_and_port(&h3270,host, CN, &rport, &ha.sa, &ha_len,errmsg, sizeof(errmsg));
		if (rv == -2)
		    	use_name = 1;
		else if (rv < 0) {
			popup_an_error(NULL,"SOCKS5 proxy: %s/%u: %s", host, port,
				errmsg);
			return -1;
		}
	}

	/* Send the authentication request to the server. */
	strcpy((char *)rbuf, "\005\001\000");
#if defined(X3270_TRACE) /*[*/
	trace_dsn("SOCKS5 Proxy: xmit version 5 nmethods 1 (no auth)\n");
	trace_netdata('>', rbuf, 3);
#endif /*]*/
	if (send(fd, (const char *) rbuf, 3, 0) < 0) {
		popup_a_sockerr(NULL,"SOCKS5 Proxy: send error");
		return -1;
	}

	/*
	 * Wait for the server reply.
	 * Read 2 bytes of response.
	 */
	nread = 0;
	for (;;) {
	    	fd_set rfds;
		struct timeval tv;

		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		tv.tv_sec = 15;
		tv.tv_usec = 0;
		if (select(fd + 1, &rfds, NULL, NULL, &tv) < 0) {
		    	popup_an_error(NULL,"SOCKS5 Proxy: server timeout");
#if defined(X3270_TRACE) /*[*/
		    	if (nread)
				trace_netdata('<', rbuf, nread);
#endif /*]*/
			return -1;
		}

	    	nr = recv(fd, (char *) &rbuf[nread], 1, 0);
		if (nr < 0) {
			popup_a_sockerr(NULL,"SOCKS5 Proxy: receive error");
#if defined(X3270_TRACE) /*[*/
		    	if (nread)
				trace_netdata('<', rbuf, nread);
#endif /*]*/
			return -1;
		}
		if (nr == 0) {
			popup_a_sockerr(NULL,"SOCKS5 Proxy: unexpected EOF");
#if defined(X3270_TRACE) /*[*/
		    	if (nread)
				trace_netdata('<', rbuf, nread);
#endif /*]*/
			return -1;
		}
		if (++nread >= 2)
		    	break;
	}

#if defined(X3270_TRACE) /*[*/
	trace_netdata('<', rbuf, nread);
#endif /*]*/

	if (rbuf[0] != 0x05 || (rbuf[1] != 0 && rbuf[1] != 0xff)) {
	    	popup_an_error(NULL,"SOCKS5 Proxy: bad authentication response");
		return -1;
	}

#if defined(X3270_TRACE) /*[*/
	trace_dsn("SOCKS5 Proxy: recv version %d method %d\n", rbuf[0],
		rbuf[1]);
#endif /*]*/

	if (rbuf[1] == 0xff) {
	    	popup_an_error(NULL,"SOCKS5 Proxy: authentication failure");
		return -1;
	}

	/* Send the request to the server. */
	buf = lib3270_malloc(32 + strlen(host));
	s = buf;
	*s++ = 0x05;		/* protocol version 5 */
	*s++ = 0x01;		/* CONNECT */
	*s++ = 0x00;		/* reserved */
	if (use_name) {
	    	*s++ = 0x03;	/* domain name */
		*s++ = strlen(host);
		strcpy(s, host);
		s += strlen(host);
	} else if (ha.sa.sa_family == AF_INET) {
	    	*s++ = 0x01;	/* IPv4 */
		memcpy(s, &ha.sin.sin_addr, 4);
		s += 4;
		strcpy(nbuf, inet_ntoa(ha.sin.sin_addr));
#if defined(AF_INET6) /*[*/
	} else {
	    	*s++ = 0x04;	/* IPv6 */
		memcpy(s, &ha.sin6.sin6_addr, sizeof(struct in6_addr));
		s += sizeof(struct in6_addr);
		(void) inet_ntop(AF_INET6, &ha.sin6.sin6_addr, nbuf, sizeof(nbuf));
#endif /*]*/
	}
	SET16(s, port);

#if defined(X3270_TRACE) /*[*/
	trace_dsn("SOCKS5 Proxy: xmit version 5 connect %s %s port %u\n",
		use_name? "domainname":
			  ((ha.sa.sa_family == AF_INET)? "IPv4": "IPv6"),
		use_name? host: nbuf,
		port);
	trace_netdata('>', (unsigned char *)buf, s - buf);
#endif /*]*/

	if (send(fd, buf, s - buf, 0) < 0) {
		popup_a_sockerr(NULL,"SOCKS5 Proxy: send error");
		lib3270_free(buf);
		return -1;
	}
	lib3270_free(buf);

	/*
	 * Process the reply.
	 * Only the first two bytes of the response are interesting;
	 * skip the rest.
	 */
	nread = 0;
	done = 0;
	buf = NULL;
	while (!done) {
	    	fd_set rfds;
		struct timeval tv;
		unsigned char r;

		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		tv.tv_sec = 15;
		tv.tv_usec = 0;
		if (select(fd + 1, &rfds, NULL, NULL, &tv) < 0) {
		    	popup_an_error(NULL,"SOCKS5 Proxy: server timeout");
			return -1;
		}

	    	nr = recv(fd, (char *) &r, 1, 0);
		if (nr < 0) {
			popup_a_sockerr(NULL,"SOCKS5 Proxy: receive error");
#if defined(X3270_TRACE) /*[*/
		    	if (nread)
				trace_netdata('<', (unsigned char *)buf, nread);
#endif /*]*/
			return -1;
		}
		if (nr == 0) {
			popup_an_error(NULL,"SOCKS5 Proxy: unexpected EOF");
#if defined(X3270_TRACE) /*[*/
		    	if (nread)
				trace_netdata('<', (unsigned char *)buf, nread);
#endif /*]*/
			return -1;
		}

		buf = Realloc(buf, nread + 1);
		buf[nread] = r;

		switch (nread++) {
		case 0:
		    	if (r != 0x05) {
			    	popup_an_error(NULL,"SOCKS5 Proxy: incorrect "
					"reply version 0x%02x", r);
#if defined(X3270_TRACE) /*[*/
				if (nread)
					trace_netdata('<', (unsigned char *)buf, nread);
#endif /*]*/
				return -1;
			}
			break;
		case 1:
#if defined(X3270_TRACE) /*[*/
			if (r != 0x00)
				trace_netdata('<', (unsigned char *)buf, nread);
#endif /*]*/
		    	switch (r) {
			case 0x00:
			    	break;
			case 0x01:
			    	popup_an_error(NULL,"SOCKS5 Proxy: server failure");
				return -1;
			case 0x02:
			    	popup_an_error(NULL,"SOCKS5 Proxy: connection not "
					"allowed");
				return -1;
			case 0x03:
			    	popup_an_error(NULL,"SOCKS5 Proxy: network "
					"unreachable");
				return -1;
			case 0x04:
			    	popup_an_error(NULL,"SOCKS5 Proxy: host "
					"unreachable");
				return -1;
			case 0x05:
			    	popup_an_error(NULL,"SOCKS5 Proxy: connection "
					"refused");
				return -1;
			case 0x06:
			    	popup_an_error(NULL,"SOCKS5 Proxy: ttl expired");
				return -1;
			case 0x07:
			    	popup_an_error(NULL,"SOCKS5 Proxy: command not "
					"supported");
				return -1;
			case 0x08:
			    	popup_an_error(NULL,"SOCKS5 Proxy: address type "
					"not supported");
				return -1;
			default:
				popup_an_error(NULL,"SOCKS5 Proxy: unknown server "
					"error 0x%02x", r);
				return -1;
			}
			break;
		case 2:
			break;
		case 3:
			switch (r) {
			case 0x01:
			    	n2read = 6;
				break;
			case 0x03:
				n2read = -1;
				break;
#if defined(AF_INET6) /*[*/
			case 0x04:
				n2read = sizeof(struct in6_addr) + 2;
				break;
#endif /*]*/
			default:
				popup_an_error(NULL,"SOCKS5 Proxy: unknown server "
					"address type 0x%02x", r);
#if defined(X3270_TRACE) /*[*/
				if (nread)
					trace_netdata('<', (unsigned char *)buf, nread);
#endif /*]*/
				return -1;
			}
			break;
		default:
			if (n2read == -1)
			    	n2read = r + 2;
			else if (!--n2read)
			    	done = 1;
			break;
		}
	}

#if defined(X3270_TRACE) /*[*/
	trace_netdata('<', (unsigned char *)buf, nread);
	switch (buf[3]) {
	case 0x01: /* IPv4 */
	    	memcpy(&ha.sin.sin_addr, &buf[4], 4);
		strcpy(nbuf, inet_ntoa(ha.sin.sin_addr));
		portp = (unsigned char *)&buf[4 + 4];
		break;
	case 0x03: /* domainname */
	    	strncpy(nbuf, &buf[5], buf[4]);
		nbuf[(unsigned char)buf[4]] = '\0';
		portp = (unsigned char *)&buf[5 + (unsigned char)buf[4]];
		break;
#if defined(AF_INET6) /*[*/
	case 0x04: /* IPv6 */
	    	memcpy(&ha.sin6.sin6_addr, &buf[4], sizeof(struct in6_addr));
		(void) inet_ntop(AF_INET6, &ha.sin6.sin6_addr, nbuf,
				 sizeof(nbuf));
		portp = (unsigned char *)&buf[4 + sizeof(struct in6_addr)];
		break;
#endif /*]*/
	default:
		/* can't happen */
		nbuf[0] = '\0';
		portp = (unsigned char *)buf;
		break;
	}
	rport = (*portp << 8) + *(portp + 1);
	trace_dsn("SOCKS5 Proxy: recv version %d status 0x%02x address %s %s "
		"port %u\n",
		buf[0], buf[1],
		atype_name[(unsigned char)buf[3]],
		nbuf,
		rport);
#endif /*]*/
	lib3270_free(buf);

    	return 0;
}
