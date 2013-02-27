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
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como main.cxx e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

#include "globals.hpp"

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/lang/XSingleComponentFactory.hpp>

using namespace com::sun::star::registry;	// for XRegistryKey
using namespace com::sun::star::lang;		// for XSingleComponentFactory

/*---[ Statics ]-------------------------------------------------------------------------------------------*/



/*---[ Implement ]-----------------------------------------------------------------------------------------*/

static Sequence< OUString > getSupportedServiceNames()
{
	Sequence<OUString> names(1);

	trace("%s returns: %s",__FUNCTION__, SERVICENAME);
	names[0] = OUString( RTL_CONSTASCII_USTRINGPARAM( SERVICENAME ) );

	return names;
}

static Reference< XInterface > SAL_CALL CreateInstance( const Reference< XComponentContext > & xContext )
{
  return static_cast< XTypeProvider * >( new pw3270::uno_impl( xContext ) );
}

/*---[ Implement exported calls ]--------------------------------------------------------------------------*/

/**************************************************************
 * Function to determine the environment of the implementation.
 *
 * If the environment is NOT session specific
 * (needs no additional context), then this function
 * should return the environment type name and leave ppEnv (0).
 *
 * @param ppEnvTypeName	environment type name;
 *							string must be constant
 * @param ppEnv			function returns its environment
 *							if the environment is session specific,
 *							i.e. has special context
 */
extern "C" void SAL_CALL component_getImplementationEnvironment(const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv)
{
#ifdef LANGUAGE_BINDING_NAME
	trace("%s set envtype to %s\n",__FUNCTION__,LANGUAGE_BINDING_NAME);
	*ppEnvTypeName = LANGUAGE_BINDING_NAME;
#else
	trace("%s set envtype to %s\n",__FUNCTION__,"msci");
	*ppEnvTypeName = "msci";
#endif
}

/************************************************************
 * Optional function to retrieve a component description.
 *
 * @return an XML formatted string containing a short
 *         component description
 */
// typedef const sal_Char * (SAL_CALL * component_getDescriptionFunc)(void);

/**********************************************************
 * Writes component registry info, at least writing the
 * supported service names.
 *
 * @param pServiceManager	a service manager
 *								(the type is XMultiServiceFactory
 *								to be used by the environment
 *								returned by
 *								component_getImplementationEnvironment)
 *
 * @param pRegistryKey		a registry key
 *								(the type is XRegistryKey to be used
 *								by the environment returned by
 *								component_getImplementationEnvironment)
 *
 * @return	true if everything went fine
 */
extern "C" sal_Bool SAL_CALL component_writeInfo(void * pServiceManager, void * pRegistryKey)
{
	sal_Bool result = sal_False;

	trace("%s",__FUNCTION__);

	if (pRegistryKey)
	{
		try
		{
			Reference< XRegistryKey > xNewKey(
				reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
					OUString( RTL_CONSTASCII_USTRINGPARAM("/" IMPLNAME "/UNO/SERVICES") ) ) );

			const Sequence< OUString > & rSNL = getSupportedServiceNames();
			const OUString * pArray = rSNL.getConstArray();

			for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
				xNewKey->createKey( pArray[nPos] );

			return sal_True;
		}
		catch (InvalidRegistryException &)
		{
			// we should not ignore exceptions
		}
	}

	return result;
}

/*********************************************************
 * Retrieves a factory to create component instances.
 *
 * @param pImplName			desired implementation name
 *
 * @param pServiceManager 	a service manager
 *								(the type is XMultiServiceFactory
 *								to be used by the environment
 *								returned by
 *								component_getImplementationEnvironment)
 *
 * @param pRegistryKey		a registry key
 *								(the type is XRegistryKey to be used
 *								by the environment returned by
 *								component_getImplementationEnvironment)
 *
 * @return						acquired component factory
 *								(the type is XInterface to be used by the
 *								environment returned by
 *								component_getImplementationEnvironment)
 */
extern "C" void * SAL_CALL component_getFactory(const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey)
{
	void * pRet = 0;

	trace("%s",__FUNCTION__);

	if(pServiceManager && rtl_str_compare( pImplName, IMPLNAME ) == 0)
	{
		Reference< XSingleComponentFactory > xFactory( ::cppu::createSingleComponentFactory(
					CreateInstance, OUString::createFromAscii( IMPLNAME ), getSupportedServiceNames() ));


		if (xFactory.is())
		{
			xFactory->acquire();
			pRet = xFactory.get();
		}
	}

	return pRet;
}

/*---[ Implement XInitialization ]-------------------------------------------------------------------------*/

void SAL_CALL pw3270::uno_impl::initialize( Sequence< Any > const & args ) throw (Exception)
{
	trace("%s",__FUNCTION__);
}

/*---[ Implement XServiceInfo ]----------------------------------------------------------------------------*/

OUString SAL_CALL pw3270::uno_impl::getImplementationName(  ) throw(RuntimeException)
{
	trace("%s",__FUNCTION__);
	return OUString( RTL_CONSTASCII_USTRINGPARAM(IMPLNAME) );
}

sal_Bool SAL_CALL pw3270::uno_impl::supportsService( const OUString& ServiceName ) throw(RuntimeException)
{
	trace("%s",__FUNCTION__);
	return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("IMPLNAME") );
}

Sequence< OUString > pw3270::uno_impl::getSupportedServiceNames() throw (RuntimeException)
{
	return getSupportedServiceNames();
}

/*---[ Implement pw3270 ]----------------------------------------------------------------------------------*/

pw3270::uno_impl::uno_impl( const Reference< XComponentContext > & xContext )
{
	this->hSession = new lib3270_session();
}

pw3270::uno_impl::~uno_impl()
{
	delete this->hSession;
}

pw3270::session::session()
{
	trace("%s",__FUNCTION__);
}

pw3270::session::~session()
{
	trace("%s",__FUNCTION__);
}

rtl_TextEncoding pw3270::session::getEncoding()
{
	return RTL_TEXTENCODING_ISO_8859_1;
}
