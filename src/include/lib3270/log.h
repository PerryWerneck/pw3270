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
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

 #ifndef LIB3270_LOG_H_INCLUDED

	#include <stdarg.h>

	#define LIB3270_LOG_H_INCLUDED 1

	#ifdef ANDROID

		#include <android/log.h>

		#define lib3270_write_log(s,m,f,...)	__android_log_print(ANDROID_LOG_VERBOSE, PACKAGE_NAME, f "\n", __VA_ARGS__ )
		#define lib3270_write_rc(s,m,r,f,...)	__android_log_print(ANDROID_LOG_VERBOSE, PACKAGE_NAME, f "\n", __VA_ARGS__ )
		#define lib3270_write_va_log(s,m,f,a)	__android_log_vprint(ANDROID_LOG_VERBOSE, PACKAGE_NAME, f "\n", a)

	#else

		LIB3270_EXPORT void	  lib3270_set_log_handler(void (*loghandler)(H3270 *, const char *, int, const char *, va_list));
		LIB3270_EXPORT int	  lib3270_write_log(H3270 *session, const char *module, const char *fmt, ...) LIB3270_GNUC_FORMAT(3,4);
		LIB3270_EXPORT int	  lib3270_write_rc(H3270 *session, const char *module, int rc, const char *fmt, ...) LIB3270_GNUC_FORMAT(4,5);
		LIB3270_EXPORT void	  lib3270_write_va_log(H3270 *session, const char *module, const char *fmt, va_list arg);

	#endif // ANDROID

	#ifdef DEBUG
		#define trace( fmt, ... )	fprintf(stderr, "%s(%d) " fmt "\n", __FILE__, __LINE__, __VA_ARGS__ ); fflush(stderr);
	#else
		#define trace(x, ...) 		// __VA_ARGS__
	#endif

 #endif // LIB3270_LOG_H_INCLUDED


