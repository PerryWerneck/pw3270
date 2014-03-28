/*
 * "Software PW3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
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
 * Este programa está nomeado como globals.hpp e possui - linhas de código.
 *
 * Contatos:
 *
 *	perry.werneck@gmail.com		(Alexandre Perry de Souza Werneck)
 *	erico.mendonca@gmail.com	(Erico Mascarenhas de Mendonça)
 *
 * Referências:
 *
 *	https://wiki.openoffice.org/wiki/Documentation/DevGuide/WritingUNO/C%2B%2B/C%2B%2B_Component
 *
 *
 */

#ifndef PW3270_OXT_GLOBALS_HPP_INCLUDED

	#define PW3270_OXT_GLOBALS_HPP_INCLUDED 1

	#define CPPUENV "gcc3"

	#ifdef _WIN32
		#define SAL_W32
	#else
		#define UNX 1
		#define GCC 1
		#define LINUX 1
	#endif


	#include <cppuhelper/implbase3.hxx> // "3" implementing three interfaces
	#include <cppuhelper/factory.hxx>
	#include <com/sun/star/lang/XInitialization.hpp>
//	#include <cppuhelper/implementationentry.hxx>
	#include <com/sun/star/lang/XServiceInfo.hpp>
	#include <com/sun/star/uno/RuntimeException.hpp>

	#include <pw3270/lib3270.hpp>


	using namespace ::rtl; 						// for OUString
	using namespace ::com::sun::star; 			// for sdk interfaces
	using namespace ::com::sun::star::lang; 	// for sdk interfaces
	using namespace ::com::sun::star::uno;		// for basic types


	namespace pw3270_impl
	{
		// https://wiki.openoffice.org/wiki/Documentation/DevGuide/WritingUNO/C%2B%2B/Class_Definition_with_Helper_Template_Classes
		class session_impl : public ::cppu::WeakImplHelper3< ::pw3270::lib3270, XServiceInfo, XInitialization >
		{
		public:
			// XInitialization will be called upon createInstanceWithArguments[AndContext]()
			virtual void SAL_CALL initialize( Sequence< Any > const & args ) throw (Exception);

			// XServiceInfo
			virtual OUString SAL_CALL getImplementationName() throw (RuntimeException);
			virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName ) throw (RuntimeException);
			virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (RuntimeException);

			// lib3270
			virtual ::rtl::OUString SAL_CALL getVersion() throw (RuntimeException);

		};

		extern Sequence< OUString > SAL_CALL getSupportedServiceNames_session_impl();
		extern OUString SAL_CALL getImplementationName_session_impl();
		extern Reference< XInterface > SAL_CALL create_session_impl(Reference< XComponentContext > const & xContext ) SAL_THROW( () );

	};


#endif // PW3270_OXT_GLOBALS_HPP_INCLUDED
