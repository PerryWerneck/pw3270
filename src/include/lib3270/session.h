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
 * Este programa está nomeado como session.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #ifndef LIB3270_SESSION_H_INCLUDED

	#define LIB3270_SESSION_H_INCLUDED 1

	#ifdef WIN32
		#include <winsock2.h>
		#include <windows.h>
		#include <ws2tcpip.h>
	#else
		#include <sys/socket.h>
	#endif // WIN32

	#include <lib3270/popup.h>

	struct lib3270_session_callbacks
	{
		int	 (*write)(H3270 *hSession, unsigned const char *buf, int len);
		void (*disconnect)(H3270 *hSession);

		void (*configure)(H3270 *session, unsigned short rows, unsigned short cols);
		void (*update)(H3270 *session, int baddr, unsigned char c, unsigned short attr, unsigned char cursor);
		void (*changed)(H3270 *session, int offset, int len);
		void (*display)(H3270 *session);
		void (*set_width)(H3270 *session, int width);

		void (*update_cursor)(H3270 *session, unsigned short row, unsigned short col, unsigned char c, unsigned short attr);
		void (*update_oia)(H3270 *session, LIB3270_FLAG id, unsigned char on);
		void (*update_toggle)(H3270 *session, LIB3270_TOGGLE ix, unsigned char value, LIB3270_TOGGLE_TYPE reason, const char *name);
		void (*update_luname)(H3270 *session, const char *name);
		void (*update_status)(H3270 *session, LIB3270_MESSAGE id);
		void (*update_connect)(H3270 *session, unsigned char connected);
		void (*update_model)(H3270 *session, const char *name, int model, int rows, int cols);
		void (*update_selection)(H3270 *session, int start, int end);
		void (*update_ssl)(H3270 *session, LIB3270_SSL_STATE state);

		void (*set_timer)(H3270 *session, unsigned char on);
		void (*erase)(H3270 *session);
		void (*suspend)(H3270 *session);
		void (*resume)(H3270 *session);
		void (*cursor)(H3270 *session, LIB3270_POINTER id);
		void (*set_selection)(H3270 *session, unsigned char on);
		void (*ctlr_done)(H3270 *session);
		void (*autostart)(H3270 *session);
		int  (*print)(H3270 *session);

		void (*message)(H3270 *session, LIB3270_NOTIFY id , const char *title, const char *message, const char *text);

	};

	/**
	 * Register application Handlers.
	 *
	 * @param cbk	Structure with the application I/O handles to set.
	 *
	 * @return 0 if ok, error code if not.
	 *
	 */
	int LIB3270_EXPORT lib3270_set_session_callbacks(const struct lib3270_callbacks *cbk);

	LIB3270_EXPORT int lib3270_getpeername(H3270 *hSession, struct sockaddr *addr, socklen_t *addrlen);
	LIB3270_EXPORT int lib3270_getsockname(H3270 *hSession, struct sockaddr *addr, socklen_t *addrlen);

	LIB3270_EXPORT struct lib3270_session_callbacks * lib3270_get_session_callbacks(H3270 *session, unsigned short sz);

#endif // LIB3270_SESSION_H_INCLUDED


