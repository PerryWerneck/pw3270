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
 * Este programa está nomeado como info.cc e possui - linhas de código.
 *
 * Contatos:
 *
 *	perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 *	erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 * Referência:
 *
 *	https://wiki.openoffice.org/wiki/Documentation/DevGuide/WritingUNO/C%2B%2B/Providing_a_Single_Factory_Using_a_Helper_Method
 *
 */

 #include "globals.hpp"
 #include <cppuhelper/implementationentry.hxx>
 #include "pw3270/lib3270.hpp"

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

namespace pw3270_impl
{
	Sequence< OUString > SAL_CALL getSupportedServiceNames_session_impl()
	{
		Sequence<OUString> names(1);
		names[0] = OUString(RTL_CONSTASCII_USTRINGPARAM("pw3270.session"));
		return names;
	}

	OUString SAL_CALL getImplementationName_session_impl()
	{
		return OUString( RTL_CONSTASCII_USTRINGPARAM("pw3270.lib3270.session") );
	}

	Reference< XInterface > SAL_CALL create_session_impl(Reference< XComponentContext > const & xContext ) SAL_THROW( () )
	{
		return static_cast< lang::XTypeProvider * >( new session_impl() );
	}

	static struct ::cppu::ImplementationEntry s_component_entries [] =
	{
		{
			create_session_impl,
			getImplementationName_session_impl,
			getSupportedServiceNames_session_impl,
			::cppu::createSingleComponentFactory,
			0,
			0
		},
		{
			0,
			0,
			0,
			0,
			0,
			0
		}
	};

}

extern "C"
{
	DLL_PUBLIC void * SAL_CALL component_getFactory(sal_Char const * implName, lang::XMultiServiceFactory * xMgr,registry::XRegistryKey * xRegistry )
	{
		return ::cppu::component_getFactoryHelper(implName, xMgr, xRegistry, ::pw3270_impl::s_component_entries );
	}

	DLL_PUBLIC sal_Bool SAL_CALL component_writeInfo(lang::XMultiServiceFactory * xMgr, registry::XRegistryKey * xRegistry )
	{
		return ::cppu::component_writeInfoHelper(xMgr, xRegistry, ::pw3270_impl::s_component_entries );
	}

	DLL_PUBLIC void SAL_CALL component_getImplementationEnvironment(sal_Char const ** ppEnvTypeName, uno_Environment ** ppEnv )
	{
        * ppEnvTypeName = LANGUAGE_BINDING_NAME;
	}

}
