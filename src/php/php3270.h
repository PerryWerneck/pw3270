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
 * Este programa está nomeado como php3270.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 * Referências:
 *
 * http://devzone.zend.com/1435/wrapping-c-classes-in-a-php-extension/
 *
 */

#ifndef PHP_LIB3270_INCLUDED

	#define PHP_LIB3270_INCLUDED 1

	#define PHP_LIB3270_EXTNAME		"lib3270"
	#define PHP_LIB3270_EXTVER		"5.0"

	extern "C"
	{
		#include "php.h"
	}

	extern zend_module_entry lib3270_module_entry;
	#define phpext_lib3270_ptr &lib3270_module_entry;

	// 3270 session methods
	PHP_METHOD(tn3270,__construct);
	PHP_METHOD(tn3270,connect);
	PHP_METHOD(tn3270,disconnect);
	PHP_METHOD(tn3270,isconnected);
	PHP_METHOD(tn3270,isready);
	PHP_METHOD(tn3270,waitforready);
	PHP_METHOD(tn3270,wait);
	PHP_METHOD(tn3270,iterate);

	PHP_METHOD(tn3270,pfkey);
	PHP_METHOD(tn3270,pakey);
	PHP_METHOD(tn3270,enter);


	#undef PACKAGE_NAME
	#undef PACKAGE_VERSION
	#undef HAVE_MALLOC_H
	#include <pw3270/class.h>

	// PHP object
	using namespace PW3270_NAMESPACE;

	struct tn3270_object
	{
		zend_object	  std;
		session		* hSession;
	};


#endif // PHP_LIB3270_INCLUDED
