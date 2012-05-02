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
 * Este programa está nomeado como resolver.c e possui 254 linhas de código.
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
 *	resolver.c
 *		Hostname resolution.
 */

#include "globals.h"

#if defined(W3N4) || defined(W3N46) /*[*/
	#error Deprecated
#endif /*]*/

#ifdef WIN32

	/* Compiling for WinXP or later: Expose getaddrinfo()/freeaddrinfo(). */
	#undef _WIN32_WINNT
	#define _WIN32_WINNT 0x0501

	#include <winsock2.h>
	#include <ws2tcpip.h>

#else

	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>

#endif /*]*/

#include <stdio.h>
#include <string.h>
#include "api.h"

#include "resolverc.h"
#include "w3miscc.h"

#pragma pack(1)
struct parms
{
	unsigned short	sz;
	const char			*host;
	char				*portname;
	unsigned short	*pport;
	struct sockaddr	*sa;
	socklen_t 			*sa_len;
	char				*errmsg;
	int					em_len;
};
#pragma pack()

/*
 * Resolve a hostname and port.
 * Returns 0 for success, -1 for fatal error (name resolution impossible),
 *  -2 for simple error (cannot resolve the name).
 */
static int cresolve_host_and_port(H3270 *h, struct parms *p)
{
#ifdef HAVE_GETADDRINFO

	struct addrinfo	 hints, *res;
	int		 rc;

	/* Use getaddrinfo() to resolve the hostname and port together. */
	(void) memset(&hints, '\0', sizeof(struct addrinfo));
	hints.ai_flags = 0;
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	rc = getaddrinfo(p->host, p->portname, &hints, &res);

	if (rc)
	{
#ifdef WIN32
		#warning gai_strerror on windows is returning message in local_charset, need to set it to utf-8!
		snprintf(p->errmsg, p->em_len, _( "Error %d resolving %s" ) , rc, p->host);
#else
		snprintf(p->errmsg, p->em_len, _( "Error resolving %s: %s" ), p->host, gai_strerror(rc));
#endif // WIN32
		return -2;
	}

	switch (res->ai_family)
	{
	case AF_INET:
		*p->pport = ntohs(((struct sockaddr_in *)res->ai_addr)->sin_port);
		break;
	case AF_INET6:
		*p->pport = ntohs(((struct sockaddr_in6 *)res->ai_addr)->sin6_port);
		break;
	default:
		snprintf(p->errmsg, p->em_len, _( "%s: unknown family %d" ), p->host,res->ai_family);
		freeaddrinfo(res);
		return -1;
	}

	(void) memcpy(p->sa, res->ai_addr, res->ai_addrlen);
	*p->sa_len = res->ai_addrlen;

	freeaddrinfo(res);

#else /*][*/

	struct hostent	*hp;
	struct servent	*sp;
	unsigned short	 port;
	unsigned long	 lport;
	char		*ptr;
	struct sockaddr_in *sin = (struct sockaddr_in *)sa;

	/* Get the port number. */
	lport = strtoul(portname, &ptr, 0);
	if (ptr == portname || *ptr != '\0' || lport == 0L || lport & ~0xffff)
	{
		if (!(sp = getservbyname(portname, "tcp")))
		{
			snprintf(p->errmsg, p->em_len,_( "Unknown port number or service: %s" ),portname);
			return -1;
		}
		port = sp->s_port;
	}
	else
	{
		port = htons((unsigned short)lport);
	}
	*pport = ntohs(port);

	/* Use gethostbyname() to resolve the hostname. */
	hp = gethostbyname(host);
	if (hp == (struct hostent *) 0)
	{
		sin->sin_family = AF_INET;
		sin->sin_addr.s_addr = inet_addr(host);
		if (sin->sin_addr.s_addr == (unsigned long)-1)
		{
			snprintf(p->errmsg, p->em_len, _( "Unknown host:\n%s" ), host);
			return -2;
		}
	}
	else
	{
		sin->sin_family = hp->h_addrtype;
		(void) memmove(&sin->sin_addr, hp->h_addr, hp->h_length);
	}
	sin->sin_port = port;
	*sa_len = sizeof(struct sockaddr_in);

#endif

	return 0;
}

int resolve_host_and_port(H3270 *hSession, const char *host, char *portname, unsigned short *pport,struct sockaddr *sa, socklen_t *sa_len, char *errmsg, int em_len)
{
	int				rc;
	LIB3270_STATUS	saved_status	= hSession->oia_status;
	struct parms	p				= { sizeof(struct parms), host, portname, pport, sa, sa_len, errmsg, em_len };

	trace("Calling resolver for %s", p.host);

	status_changed(hSession,LIB3270_STATUS_RESOLVING);
	hSession->cursor(hSession,CURSOR_MODE_LOCKED);

	rc = lib3270_call_thread((int (*)(H3270 *, void *)) cresolve_host_and_port,hSession,&p);

	hSession->cursor(hSession,CURSOR_MODE_NORMAL);
	status_changed(hSession,saved_status);

	trace("Calling resolver for %s exits with %d", p.host, rc);

	return rc;

}
