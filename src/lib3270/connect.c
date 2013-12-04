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
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como connect.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#if defined(_WIN32)
	#include <winsock2.h>
	#include <windows.h>
#endif

#include "globals.h"
#include <errno.h>

#if defined(_WIN32)
	#include <ws2tcpip.h>
#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <unistd.h>
	#include <fcntl.h>
#endif


#if defined(_WIN32) /*[*/
	#define SOCK_CLOSE(s)	closesocket(s->sock); s->sock = -1;
#else /*][*/
	#define SOCK_CLOSE(s)	close(s->sock); s->sock = -1;
#endif /*]*/

#include <stdlib.h>
#include "statusc.h"
#include "hostc.h"
#include "trace_dsc.h"
#include "utilc.h"
#include <lib3270/internals.h>

/*---[ Implement ]-------------------------------------------------------------------------------*/


static void net_connected(H3270 *hSession)
{
	trace("***************** %s",__FUNCTION__);

}

#if defined(_WIN32) /*[*/
 static void sockstart(H3270 *session)
 {
	static int initted = 0;
	WORD wVersionRequested;
	WSADATA wsaData;

	if (initted)
		return;

	initted = 1;

	wVersionRequested = MAKEWORD(2, 2);

	if (WSAStartup(wVersionRequested, &wsaData) != 0)
	{
		lib3270_popup_dialog(	session,
								LIB3270_NOTIFY_CRITICAL,
								N_( "Network startup error" ),
								N_( "WSAStartup failed" ),
								"%s", win32_strerror(GetLastError()) );

		_exit(1);
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		lib3270_popup_dialog(	session,
								LIB3270_NOTIFY_CRITICAL,
								N_( "Network startup error" ),
								N_( "Bad winsock version" ),
								N_( "Can't use winsock version %d.%d" ), LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
		_exit(1);
	}
 }
#endif /*]*/

 static void set_ssl_state(H3270 *hSession, LIB3270_SSL_STATE state)
 {
	if(state == hSession->secure)
		return;

	trace_dsn(hSession,"SSL state changes to %d\n",(int) state);

	hSession->update_ssl(hSession,hSession->secure = state);
 }

 LIB3270_EXPORT int lib3270_connect_host(H3270 *hSession, const char *hostname, const char *srvc)
 {
 	int					  s;
 	int					  sock			= -1;
	struct addrinfo		  hints;
	struct addrinfo 	* result		= NULL;
	struct addrinfo 	* rp			= NULL;

	if(!hostname)
		return EINVAL;

	if(!srvc)
		srvc = "telnet";

	CHECK_SESSION_HANDLE(hSession);

	lib3270_main_iterate(hSession,0);

	if(hSession->auto_reconnect_inprogress)
		return EAGAIN;

	if(PCONNECTED)
		return EBUSY;

#if defined(_WIN32)
	sockstart(hSession);
#endif

	set_ssl_state(hSession,LIB3270_SSL_UNSECURE);

	hSession->ever_3270	= False;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family 	= AF_UNSPEC;	/* Allow IPv4 or IPv6 */
	hints.ai_socktype	= SOCK_STREAM;	/* Stream socket */
	hints.ai_flags		= AI_PASSIVE;	/* For wildcard IP address */
	hints.ai_protocol	= 0;			/* Any protocol */
	hints.ai_canonname	= NULL;
	hints.ai_addr		= NULL;
	hints.ai_next		= NULL;

	if(*hostname == '$')
	{
		const char *name = getenv(hostname+1);
		if(!name)
		{
			lib3270_popup_dialog(	hSession,
									LIB3270_NOTIFY_ERROR,
									_( "Connection error" ),
									_( "Unable to find selected hostname." ),
									_( "Can't determine value for environment variable \"%s\" " ),
									hostname);
			lib3270_set_disconnected(hSession);
			return -1;
		}
		hostname = name;
	}

	status_changed(hSession,LIB3270_STATUS_RESOLVING);

	s = getaddrinfo(hostname, srvc, &hints, &result);

	if(s != 0)
	{
		lib3270_popup_dialog(	hSession,
								LIB3270_NOTIFY_ERROR,
								_( "Connection error" ),
								_( "Can't resolve hostname." ),
								"%s",
								gai_strerror(s));

		lib3270_set_disconnected(hSession);
		return -1;
	}

	status_changed(hSession,LIB3270_STATUS_CONNECTING);

	for(rp = result; sock < 0 && rp != NULL; rp = rp->ai_next)
	{
		hSession->sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(hSession->sock < 0)
			continue;

#ifdef WIN32
		u_long block;
		u_int  len		= sizeof(int);

		WSASetLastError(0);
		block = 0;

		if(ioctlsocket(hSession->sock,FIONBIO,&block))
		{
			lib3270_popup_dialog(	hSession,
									LIB3270_NOTIFY_ERROR,
									_( "Connection error" ),
									_( "ioctlsocket(FIONBIO) failed." ),
									"Windows error %d",
									WSAGetLastError() );

			SOCK_CLOSE(hSession);
		}
		else if(connect(hSession->sock, rp->ai_addr, rp->ai_addrlen))
		{
			int err = WSAGetLastError();
			if(err != WSAEWOULDBLOCK)
			{
				lib3270_popup_dialog(	hSession,
										LIB3270_NOTIFY_ERROR,
										_( "Connection error" ),
										_( "Can't connect to host." ),
										"Windows error %d",
										WSAGetLastError() );
				SOCK_CLOSE(hSession);
			}
		}

#else
		fcntl(hSession->sock, F_SETFL,fcntl(sock,F_GETFL,0)|O_NONBLOCK);

		errno = 0;
		if(connect(hSession->sock, rp->ai_addr, rp->ai_addrlen))
		{
			if( errno != EINPROGRESS )
			{
				lib3270_popup_dialog(	hSession,
										LIB3270_NOTIFY_ERROR,
										_( "Connection error" ),
										_( "Can't connect to host." ),
										"%s",
										strerror(errno));
				SOCK_CLOSE(hSession);
			}
		}

#endif // WIN32
	}

	freeaddrinfo(result);

	// set options for inline out-of-band data and keepalives

	/*
	int on = 1;
	if (setsockopt(hSession->sock, SOL_SOCKET, SO_OOBINLINE, (char *)&on,sizeof(on)) < 0)
	{
		popup_a_sockerr(hSession, N_( "setsockopt(%s)" ), "SO_OOBINLINE");
		SOCK_CLOSE(hSession);
	}

#if defined(OMTU)
	else if (setsockopt(hSession->sock, SOL_SOCKET, SO_SNDBUF, (char *)&mtu,sizeof(mtu)) < 0)
	{
		popup_a_sockerr(hSession, N_( "setsockopt(%s)" ), "SO_SNDBUF");
		SOCK_CLOSE(hSession);
	}
#endif

	*/

	if(hSession->sock < 0)
	{
		lib3270_set_disconnected(hSession);
		return -1;
	}

#if !defined(_WIN32)
	/* don't share the socket with our children */
	(void) fcntl(hSession->sock, F_SETFD, 1);
#endif

	// Connecting, set callbacks, wait for connection
	trace_dsn(hSession,"Half-connected.\n");

	lib3270_st_changed(hSession, LIB3270_STATE_HALF_CONNECT, True);

	hSession->ns_write_id = AddOutput(hSession->sock, hSession, net_connected);

	return 0;

 }

