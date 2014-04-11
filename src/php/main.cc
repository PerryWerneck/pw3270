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

/*--[ Globals ]----------------------------------------------------------------------------------------------------*/

static zend_class_entry		* tn3270_ce = NULL;
static zend_object_handlers	  tn3270_object_handlers;

/*--[ Implement ]--------------------------------------------------------------------------------------------------*/

zend_function_entry tn3270_methods[] =
{
    PHP_ME( tn3270,	__construct,	NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)

    PHP_ME( tn3270,	connect,		NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME( tn3270,	disconnect,		NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)

    PHP_ME( tn3270,	isconnected,	NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME( tn3270,	isready,		NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)

    PHP_ME( tn3270,	waitforready,	NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME( tn3270,	wait,			NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME( tn3270,	iterate,		NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)

    PHP_ME( tn3270,	pfkey,			NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME( tn3270,	pakey,			NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME( tn3270,	enter,			NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)

    PHP_ME( tn3270,	getstringat,	NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME( tn3270,	setstringat,	NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    PHP_ME( tn3270,	cmpstringat,	NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)

    {NULL, NULL, NULL}
};

void tn3270_free_storage(void *object TSRMLS_DC)
{
	tn3270_object *obj = (tn3270_object *)object;

	trace("%s",__FUNCTION__);

	zend_object_std_dtor(&obj->std TSRMLS_CC);
	delete obj->hSession;

	efree(obj);
}

zend_object_value tn3270_create_handler(zend_class_entry *type TSRMLS_DC)
{
    zend_object_value	  retval;
	tn3270_object		* obj = (tn3270_object *) emalloc(sizeof(tn3270_object));

	trace("%s",__FUNCTION__);

    memset(obj, 0, sizeof(tn3270_object));

	zend_object_std_init( &(obj->std), type TSRMLS_CC );

	object_properties_init((zend_object*) &(obj->std), type);

    retval.handle	= zend_objects_store_put(obj, NULL, tn3270_free_storage, NULL TSRMLS_CC);
    retval.handlers	= &tn3270_object_handlers;

    return retval;
}

PHP_MINIT_FUNCTION(tn3270)
{
	zend_class_entry ce;

	trace("%s",__FUNCTION__);

	INIT_CLASS_ENTRY(ce, "tn3270", tn3270_methods);

	tn3270_ce = zend_register_internal_class(&ce TSRMLS_CC);
	tn3270_ce->create_object = tn3270_create_handler;

	memcpy(&tn3270_object_handlers,zend_get_std_object_handlers(), sizeof(zend_object_handlers));
    tn3270_object_handlers.clone_obj = NULL;

    return SUCCESS;
}

zend_module_entry lib3270_module_entry =
{
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP3270_EXTNAME,
    NULL,                  /* Functions */
    PHP_MINIT(tn3270),
    NULL,                  /* MSHUTDOWN */
    NULL,                  /* RINIT */
    NULL,                  /* RSHUTDOWN */
    NULL,                  /* MINFO */
#if ZEND_MODULE_API_NO >= 20010901
    PHP3270_EXTVER,
#endif
    STANDARD_MODULE_PROPERTIES
};

// #ifdef COMPILE_DL_LIB3270
extern "C"
{
	ZEND_GET_MODULE(lib3270)
}
// #endif

