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
 * Este programa está nomeado como ssl.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */


#include <lib3270/config.h>
#if defined(HAVE_LIBSSL)
	#include <openssl/ssl.h>
	#include <openssl/err.h>
#endif

#include "globals.h"
#include <errno.h>
#include <lib3270/internals.h>
#include <lib3270/trace.h>
#include "trace_dsc.h"

static int ssl_3270_ex_index = -1;	/**< Index of h3270 handle in SSL session */

/*--[ Implement ]------------------------------------------------------------------------------------*/

#if defined(HAVE_LIBSSL)
int ssl_negotiate(H3270 *hSession)
{
	int rv;

	trace("%s",__FUNCTION__);

	set_ssl_state(hSession,LIB3270_SSL_NEGOTIATING);
	non_blocking(hSession,False);

	/* Initialize the SSL library. */
	ssl_init(hSession);
	if(hSession->ssl_con == NULL)
	{
		/* Failed. */
		popup_an_error(hSession,_( "SSL init failed!"));
		lib3270_disconnect(hSession);
		return -1;
	}

	/* Set up the TLS/SSL connection. */
	if(SSL_set_fd(hSession->ssl_con, hSession->sock) != 1)
	{
		trace_dsn(hSession,"SSL_set_fd failed!\n");
		#warning Show a better popup here
		// popup_an_error(hSession,_( "SSL_set_fd failed!"));
		lib32070_disconnect(hSession);
		return -1;
	}

	trace("%s: Running SSL_connect",__FUNCTION__);
	rv = SSL_connect(hSession->ssl_con);
	trace("%s: SSL_connect exits with rc=%d",__FUNCTION__,rv);

	if (rv != 1)
	{
		int ssl_error =  SSL_get_error(hSession->ssl_con,rv);

		if(ssl_error == SSL_ERROR_SYSCALL)
		{
			if(!hSession->ssl_error)
			{
				trace_dsn(hSession,"SSL_connect failed (ssl_error=%lu)\n",hSession->ssl_error);
				popup_an_error(hSession,_( "SSL connect failed!"));
			}
			else
			{
				trace_dsn(hSession,"SSL_connect failed: %s %s\n",
						ERR_lib_error_string(hSession->ssl_error),
						ERR_reason_error_string(hSession->ssl_error));
				popup_an_error(hSession,"%s",_( ERR_reason_error_string(hSession->ssl_error) ));
			}

		}
		else
		{
			trace_dsn(hSession,"SSL_connect failed (ssl_error=%d errno=%d)\n",ssl_error,errno);
			popup_an_error(hSession,_( "SSL connect failed!"));
		}

		lib3270_disconnect(hSession);
		return -1;
	}

	/* Success. */
	if(lib3270_get_toggle(hSession,LIB3270_TOGGLE_DS_TRACE))
	{
		char				  buffer[4096];
		int 				  alg_bits		= 0;
		const SSL_CIPHER	* cipher		= SSL_get_current_cipher(hSession->ssl_con);
		X509				* peer			= SSL_get_peer_certificate(hSession->ssl_con);

		trace_dsn(hSession,"TLS/SSL negotiated connection complete. Connection is now secure.\n");

		trace_dsn(hSession,"TLS/SSL cipher description: %s",SSL_CIPHER_description((SSL_CIPHER *) cipher, buffer, 4095));
		SSL_CIPHER_get_bits(cipher, &alg_bits);
		trace_dsn(hSession,"%s version %s with %d bits verify=%ld\n",
						SSL_CIPHER_get_name(cipher),
						SSL_CIPHER_get_version(cipher),
						alg_bits,
						SSL_get_verify_result(hSession->ssl_con));

		if(peer)
		{
			BIO				* out	= BIO_new(BIO_s_mem());
			unsigned char	* data;
			unsigned char	* text;
			int				  n;

			X509_print(out,peer);

			n		= BIO_get_mem_data(out, &data);
			text	= (unsigned char *) malloc (n+1);
			text[n]	='\0';
			memcpy(text,data,n);

			trace_dsn(hSession,"TLS/SSL peer certificate:\n%s\n",text);

			free(text);
			BIO_free(out);
			X509_free(peer);

		}
	}

	if(!SSL_get_verify_result(hSession->ssl_con))
		set_ssl_state(hSession,LIB3270_SSL_SECURE);

	/* Tell the world that we are (still) connected, now in secure mode. */
	lib3270_set_connected(hSession);
	return 0;
}
#endif // HAVE_LIBSSL

#if defined(HAVE_LIBSSL) /*[*/

/* Initialize the OpenSSL library. */
void ssl_init(H3270 *session)
{
	static SSL_CTX *ssl_ctx = NULL;

	session->ssl_error = 0;
	set_ssl_state(session,LIB3270_SSL_UNDEFINED);

	if(ssl_ctx == NULL)
	{
		lib3270_write_log(session,"SSL","%s","Initializing SSL context");
		SSL_load_error_strings();
		SSL_library_init();
		ssl_ctx = SSL_CTX_new(SSLv23_method());
		if(ssl_ctx == NULL)
		{
			popup_an_error(session,"SSL_CTX_new failed");
			session->ssl_host = False;
			return;
		}
		SSL_CTX_set_options(ssl_ctx, SSL_OP_ALL);
		SSL_CTX_set_info_callback(ssl_ctx, ssl_info_callback);
		SSL_CTX_set_default_verify_paths(ssl_ctx);

#if defined(_WIN32)
		{
			HKEY hKey = 0;

			if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Software\\" PACKAGE_NAME,0,KEY_QUERY_VALUE,&hKey) == ERROR_SUCCESS)
			{
				char			data[4096];
				unsigned long	datalen	= sizeof(data);		// data field length(in), data returned length(out)
				unsigned long	datatype;					// #defined in winnt.h (predefined types 0-11)

				if(RegQueryValueExA(hKey,"datadir",NULL,&datatype,(LPBYTE) data,&datalen) == ERROR_SUCCESS)
				{
					strncat(data,"\\certs",4095);

					trace("Loading certs from \"%s\"",data);
					SSL_CTX_load_verify_locations(ssl_ctx,NULL,data);
				}
				RegCloseKey(hKey);
			}


		}

#endif // _WIN32

		ssl_3270_ex_index = SSL_get_ex_new_index(0,NULL,NULL,NULL,NULL);


	}

	if(session->ssl_con)
		SSL_free(session->ssl_con);

	session->ssl_con = SSL_new(ssl_ctx);
	if(session->ssl_con == NULL)
	{
		popup_an_error(session,"SSL_new failed");
		session->ssl_host = False;
		return;
	}

	SSL_set_ex_data(session->ssl_con,ssl_3270_ex_index,(char *) session);

//	SSL_set_verify(session->ssl_con, SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
	SSL_set_verify(session->ssl_con, 0, NULL);

}

/* Callback for tracing protocol negotiation. */
void ssl_info_callback(INFO_CONST SSL *s, int where, int ret)
{
//	H3270 *hSession = lib3270_get_default_session_handle(); // TODO: Find a better way!
	H3270 *hSession = (H3270 *) SSL_get_ex_data(s,ssl_3270_ex_index);

#ifdef DEBUG
	trace("%s: hsession=%p, session=%p",__FUNCTION__,hSession,lib3270_get_default_session_handle());
	if(hSession != lib3270_get_default_session_handle())
		exit(-1);
#endif // DEBUG

	switch(where)
	{
	case SSL_CB_CONNECT_LOOP:
		trace_dsn(hSession,"SSL_connect: %s %s\n",SSL_state_string(s), SSL_state_string_long(s));
		break;

	case SSL_CB_CONNECT_EXIT:

		trace_dsn(hSession,"%s: SSL_CB_CONNECT_EXIT\n",__FUNCTION__);

		if (ret == 0)
		{
			trace_dsn(hSession,"SSL_connect: failed in %s\n",SSL_state_string_long(s));
		}
		else if (ret < 0)
		{
			unsigned long e = ERR_get_error();
			char err_buf[1024];

			if(e != 0)
			{
				hSession->ssl_error = e;
				(void) ERR_error_string_n(e, err_buf, 1023);
			}
#if defined(_WIN32)
			else if (GetLastError() != 0)
			{
				strncpy(err_buf,lib3270_win32_strerror(GetLastError()),1023);
			}
#else
			else if (errno != 0)
			{
				strncpy(err_buf, strerror(errno),1023);
			}
#endif
			else
			{
				err_buf[0] = '\0';
			}

			trace_dsn(hSession,"SSL Connect error %d\nMessage: %s\nState: %s\nAlert: %s\n",
							ret,
							err_buf,
							SSL_state_string_long(s),
							SSL_alert_type_string_long(ret)
						);

		}


	default:
		trace_dsn(hSession,"SSL Current state is \"%s\"\n",SSL_state_string_long(s));
	}

//	trace("%s: state=%04x where=%04x ret=%d",__FUNCTION__,SSL_state(s),where,ret);

#ifdef DEBUG
	if(where & SSL_CB_EXIT)
	{
		trace("%s: SSL_CB_EXIT ret=%d\n",__FUNCTION__,ret);
	}
#endif

	if(where & SSL_CB_ALERT)
		trace_dsn(hSession,"SSL ALERT: %s\n",SSL_alert_type_string_long(ret));

	if(where & SSL_CB_HANDSHAKE_DONE)
	{
		trace_dsn(hSession,"%s: SSL_CB_HANDSHAKE_DONE state=%04x\n",__FUNCTION__,SSL_state(s));
		if(SSL_state(s) == 0x03)
			set_ssl_state(hSession,LIB3270_SSL_NEGOTIATED);
		else
			set_ssl_state(hSession,LIB3270_SSL_UNSECURE);
	}
}

#endif /*]*/

LIB3270_EXPORT LIB3270_SSL_STATE lib3270_get_secure(H3270 *session)
{
	CHECK_SESSION_HANDLE(session);
	return session->secure;
}

LIB3270_EXPORT int lib3270_is_secure(H3270 *hSession)
{
	return lib3270_get_secure(hSession) == LIB3270_SSL_SECURE;
}

LIB3270_EXPORT long lib3270_get_SSL_verify_result(H3270 *hSession)
{
	CHECK_SESSION_HANDLE(hSession);
#if defined(HAVE_LIBSSL)
	if(hSession->ssl_con)
		return SSL_get_verify_result(hSession->ssl_con);
#endif // HAVE_LIBSSL
	return -1;
}

void set_ssl_state(H3270 *session, LIB3270_SSL_STATE state)
{
	if(state == session->secure)
		return;

	trace_dsn(session,"SSL state changes to %d\n",(int) state);

	session->update_ssl(session,session->secure = state);
}
