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
 * Este programa está nomeado como main.cc e possui - linhas de código.
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

 #include "php3270.h"

/*--[ Implement ]--------------------------------------------------------------------------------------------------*/

PHP_METHOD(tn3270, connect)
{
	const char		* host;
	int				  szHost;
	zend_bool		  wait	= 0;
	int				  rc	= 0;
	tn3270_object	* obj	= (tn3270_object *) zend_object_store_get_object(getThis() TSRMLS_CC);

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sb", &host, &szHost, &wait) == FAILURE)
		RETURN_NULL();

	if(szHost)
	{
		char text[szHost+1];
		strncpy(text,host,szHost);
		text[szHost] = 0;
		rc = obj->hSession->connect(text,wait);
	}
	else
	{
		rc = obj->hSession->connect();
	}

	RETURN_LONG(rc);
}

PHP_METHOD(tn3270, disconnect)
{
	tn3270_object	* obj = (tn3270_object *) zend_object_store_get_object(getThis() TSRMLS_CC);
	RETURN_LONG(obj->hSession->disconnect());
}

PHP_METHOD(tn3270, waitforready)
{
	long			  seconds;
	tn3270_object	* obj		= (tn3270_object *) zend_object_store_get_object(getThis() TSRMLS_CC);

	// http://www.php.net/manual/pt_BR/internals2.funcs.php
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &seconds) == FAILURE)
		RETURN_NULL();

	RETURN_LONG(obj->hSession->wait_for_ready((int) seconds));
}

PHP_METHOD(tn3270, waity)
{
	long			  seconds;
	tn3270_object	* obj		= (tn3270_object *) zend_object_store_get_object(getThis() TSRMLS_CC);

	// http://www.php.net/manual/pt_BR/internals2.funcs.php
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &seconds) == FAILURE)
		RETURN_NULL();

	RETURN_LONG(obj->hSession->wait((int) seconds));
}

PHP_METHOD(tn3270, wait)
{
	long			  seconds;
	tn3270_object	* obj		= (tn3270_object *) zend_object_store_get_object(getThis() TSRMLS_CC);

	// http://www.php.net/manual/pt_BR/internals2.funcs.php
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &seconds) == FAILURE)
		RETURN_NULL();

	RETURN_LONG(obj->hSession->wait((int) seconds));
}


PHP_METHOD(tn3270, iterate)
{
	zend_bool		  wait	= 0;
	tn3270_object	* obj	= (tn3270_object *) zend_object_store_get_object(getThis() TSRMLS_CC);

	// http://www.php.net/manual/pt_BR/internals2.funcs.php
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &wait) == FAILURE)
		RETURN_NULL();

	RETURN_LONG(obj->hSession->iterate(wait));
}
