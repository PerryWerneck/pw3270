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

/*---[ Implement ]-------------------------------------------------------------------------------*/

 LIB3270_EXPORT int lib3270_sock_connect(H3270 *hSession, const char *hostname, const char *srvc, int timeout)
 {
 	int					  s;
 	int					  sock			= -1;
	struct addrinfo		  hints;
	struct addrinfo 	* result		= NULL;
	struct addrinfo 	* rp			= NULL;
	LIB3270_MESSAGE		  saved_status	= hSession->oia_status;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family 	= AF_UNSPEC;	/* Allow IPv4 or IPv6 */
	hints.ai_socktype	= SOCK_STREAM;	/* Stream socket */
	hints.ai_flags		= AI_PASSIVE;	/* For wildcard IP address */
	hints.ai_protocol	= 0;			/* Any protocol */
	hints.ai_canonname	= NULL;
	hints.ai_addr		= NULL;
	hints.ai_next		= NULL;

	if(timeout < 0)
		timeout = 10;

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

		status_changed(hSession,saved_status);

		return -1;
	}

	status_changed(hSession,LIB3270_STATUS_CONNECTING);

	for(rp = result; sock < 0 && rp != NULL; rp = rp->ai_next)
	{
		sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(sock < 0)
			continue;

#ifdef WIN32
		u_long block;
		u_int  len		= sizeof(int);

		WSASetLastError(0);
		block = 1;

		if(ioctlsocket(sock,FIONBIO,&block))
		{
			lib3270_popup_dialog(	hSession,
									LIB3270_NOTIFY_ERROR,
									_( "Connection error" ),
									_( "ioctlsocket(FIONBIO) failed." ),
									"Windows error %d",
									WSAGetLastError() );
			close(sock);
			sock = -1;
		}
		else if(connect(sock, rp->ai_addr, rp->ai_addrlen))
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
				close(sock);
				sock = -1;
			}
		}

		#error Implementar

		/*
		if(sock > 0)
		{
			// Connection in progress, wait until socket is available for write
			fd_set				wr;
			struct timeval		tm;
			int					status;
			int					err;
			socklen_t			len		= sizeof(err);

			FD_ZERO(&wr);
			FD_SET(sock, &wr);
			memset(&tm,0,sizeof(tm));
			tm.tv_sec = timeout;

			switch(select(sock+1, NULL, &wr, NULL, &tm))
			{
			case 0:
				lib3270_popup_dialog(	hSession,
										LIB3270_NOTIFY_ERROR,
										_( "Connection error" ),
										_( "Can't connect to host." ),
										"%s",
										strerror(errno = ETIMEDOUT));
				close(sock);
				sock = -1;
				break;

			case -1:
				lib3270_popup_dialog(	hSession,
										LIB3270_NOTIFY_ERROR,
										_( "Connection error" ),
										_( "select() error when connecting to host." ),
										"%s",
										strerror(errno));
				close(sock);
				sock = -1;
				break;

			default:
				// Se o socket nao esta disponivel para gravacao o connect falhou
				if(!FD_ISSET(sock,&wr))
				{
					lib3270_popup_dialog(	hSession,
											LIB3270_NOTIFY_ERROR,
											_( "Connection error" ),
											_( "Error when connecting to host." ),
											"%s",
											_( "Socket was not available after connect" ));
					close(sock);
					sock = -1;
				}
				else if(getsockopt(sock, SOL_SOCKET, SO_ERROR, (void *) &err, &len) < 0)
				{
					lib3270_popup_dialog(	hSession,
											LIB3270_NOTIFY_ERROR,
											_( "Connection error" ),
											_( "Error getting connection state." ),
											"%s",
											strerror(errno));

					close(sock);
					sock = -1;
				}
				else if(err)
				{
					lib3270_popup_dialog(	hSession,
											LIB3270_NOTIFY_ERROR,
											_( "Connection error" ),
											_( "socket error when connecting to host." ),
											"Socket error %d",
											err);

					close(sock);
					sock = -1;
				}
			}
		}
		*/

#else
		fcntl(sock, F_SETFL,fcntl(sock,F_GETFL,0)|O_NONBLOCK);

		errno = 0;
		if(connect(sock, rp->ai_addr, rp->ai_addrlen))
		{
			if( errno != EINPROGRESS )
			{
				lib3270_popup_dialog(	hSession,
										LIB3270_NOTIFY_ERROR,
										_( "Connection error" ),
										_( "Can't connect to host." ),
										"%s",
										strerror(errno));
				close(sock);
				sock = -1;
			}
		}

		#error Implementar

/*
		if(sock > 0)
		{
			// Connection in progress, wait until socket is available for write
			fd_set				wr;
			struct timeval		tm;
			int					status;
			int					err;
			socklen_t			len		= sizeof(err);

			FD_ZERO(&wr);
			FD_SET(sock, &wr);
			memset(&tm,0,sizeof(tm));
			tm.tv_sec = timeout;

			switch(select(sock+1, NULL, &wr, NULL, &tm))
			{
			case 0:
				lib3270_popup_dialog(	hSession,
										LIB3270_NOTIFY_ERROR,
										_( "Connection error" ),
										_( "Can't connect to host." ),
										"%s",
										strerror(errno = ETIMEDOUT));
				close(sock);
				sock = -1;
				break;

			case -1:
				lib3270_popup_dialog(	hSession,
										LIB3270_NOTIFY_ERROR,
										_( "Connection error" ),
										_( "select() error when connecting to host." ),
										"%s",
										strerror(errno));
				close(sock);
				sock = -1;
				break;

			default:

				// Se o socket nao esta disponivel para gravacao o connect falhou
				if(!FD_ISSET(sock,&wr))
				{
					lib3270_popup_dialog(	hSession,
											LIB3270_NOTIFY_ERROR,
											_( "Connection error" ),
											_( "Error when connecting to host." ),
											"%s",
											_( "Socket was not available after connect" ));
					close(sock);
					sock = -1;
				}
				else if(getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len) < 0)
				{
					lib3270_popup_dialog(	hSession,
											LIB3270_NOTIFY_ERROR,
											_( "Connection error" ),
											_( "Error getting connection state." ),
											"%s",
											strerror(errno));

					close(sock);
					sock = -1;
				}
				else if(err)
				{
					lib3270_popup_dialog(	hSession,
											LIB3270_NOTIFY_ERROR,
											_( "Connection error" ),
											_( "socket error when connecting to host." ),
											"Socket error %d",
											err);

					close(sock);
					sock = -1;
				}
			}
		}
		*/
#endif // WIN32
	}

	freeaddrinfo(result);

	status_changed(hSession,saved_status);

	return 0;

 }

