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
 * Este programa está nomeado como log.c e possui 151 linhas de código.
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
#include <stdarg.h>
#include <lib3270/config.h>
#include <lib3270.h>
#include <lib3270/log.h>
#include "api.h"

/*---[ Prototipes ]-----------------------------------------------------------------------------------------*/

 static void defaultlog(H3270 *session, const char *module, int rc, const char *fmt, va_list arg_ptr);

/*---[ Constants ]------------------------------------------------------------------------------------------*/

 static void (*loghandler)(H3270 *session, const char *module, int rc, const char *fmt, va_list arg_ptr) = defaultlog;

/*---[ Implementacao ]--------------------------------------------------------------------------------------*/

 LIB3270_EXPORT void lib3270_set_log_handler(void (*handler)(H3270 *, const char *, int, const char *, va_list))
 {
	loghandler = handler ? handler : defaultlog;
 }

 LIB3270_EXPORT int lib3270_write_log(H3270 *session, const char *module, const char *fmt, ...)
 {
	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	loghandler(session,module,0,fmt,arg_ptr);
	va_end(arg_ptr);
    return 0;
 }

 LIB3270_EXPORT int lib3270_write_rc(H3270 *session, const char *module, int rc, const char *fmt, ...)
 {
	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	loghandler(session,module,rc,fmt,arg_ptr);
	va_end(arg_ptr);
    return rc;
 }

 LIB3270_EXPORT void lib3270_write_va_log(H3270 *session, const char *module, const char *fmt, va_list arg)
 {
	loghandler(session,module,0,fmt,arg);
 }

 static void defaultlog(H3270 *session, const char *module, int rc, const char *fmt, va_list arg_ptr)
 {
 	fprintf(stderr,"%s:\t",module);
	vfprintf(stderr,fmt,arg_ptr);
	fprintf(stderr,"\n");
	fflush(stderr);
 }

