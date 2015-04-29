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
		#define HAVE_GCC_VISIBILITY_FEATURE 1
	#endif


	#include <cppuhelper/implbase4.hxx>
	#include <cppuhelper/factory.hxx>
	#include <com/sun/star/lang/XInitialization.hpp>
	#include <com/sun/star/lang/XServiceInfo.hpp>
	#include <com/sun/star/lang/XMain.hpp>

	#include <com/sun/star/uno/RuntimeException.hpp>

	#ifdef DEBUG
		#include <stdio.h>
		#define trace( fmt, ... )	fprintf(stderr, "%s(%d) " fmt "\n", __FILE__, __LINE__, __VA_ARGS__ ); fflush(stderr);
	#else
		#define trace(x, ...) 		// __VA_ARGS__
	#endif

	#include <pw3270/lib3270.hpp>
	#include <pw3270/class.h>

	#define DLL_PUBLIC __attribute__((visibility("default")))

	#define CHECK_SESSION_HANDLE if(!hSession) hSession = h3270::session::get_default();


	using namespace ::rtl; 						// for OUString
	using namespace ::com::sun::star; 			// for sdk interfaces
	using namespace ::com::sun::star::lang; 	// for sdk interfaces
	using namespace ::com::sun::star::uno;		// for basic types
	using namespace PW3270_NAMESPACE;

 	namespace pw3270_impl
	{
		// https://wiki.openoffice.org/wiki/Documentation/DevGuide/WritingUNO/C%2B%2B/Class_Definition_with_Helper_Template_Classes
		class DLL_PUBLIC session_impl : public ::cppu::WeakImplHelper4< ::pw3270::lib3270, XServiceInfo, XMain, XInitialization >
		{
		public:

			session_impl();
			virtual ~session_impl();

			// XMain
		    virtual ::sal_Int32 SAL_CALL run( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aArguments ) throw (Exception);

			// XInitialization will be called upon createInstanceWithArguments[AndContext]()
			virtual void SAL_CALL initialize( Sequence< Any > const & args ) throw (Exception);

			// XServiceInfo
			virtual OUString SAL_CALL getImplementationName() throw (RuntimeException);
			virtual sal_Bool SAL_CALL supportsService( OUString const & serviceName ) throw (RuntimeException);
			virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (RuntimeException);

			// lib3270
			virtual ::rtl::OUString SAL_CALL getVersion() throw (RuntimeException);
			virtual ::rtl::OUString SAL_CALL getRevision() throw (RuntimeException);
			virtual ::sal_Int16 SAL_CALL iterate( ::sal_Bool wait ) throw (::com::sun::star::uno::RuntimeException);

			virtual ::sal_Int16 SAL_CALL setSessionName( const ::rtl::OUString& name ) throw (::com::sun::star::uno::RuntimeException);

			virtual ::sal_Int16 SAL_CALL Connect( const ::rtl::OUString& url, ::sal_Bool wait) throw (::com::sun::star::uno::RuntimeException);
			virtual ::sal_Int16 SAL_CALL Disconnect() throw (::com::sun::star::uno::RuntimeException);

			// State
			virtual ::sal_Bool SAL_CALL isConnected() throw (::com::sun::star::uno::RuntimeException);
			virtual ::sal_Bool SAL_CALL isReady() throw (::com::sun::star::uno::RuntimeException);

			// Screen contents
			virtual ::rtl::OUString SAL_CALL getTextAt( ::sal_Int16 row, ::sal_Int16 col, ::sal_Int16 size ) throw (::com::sun::star::uno::RuntimeException);
			virtual ::sal_Int16 SAL_CALL setTextAt( ::sal_Int16 row, ::sal_Int16 col, const ::rtl::OUString& str ) throw (::com::sun::star::uno::RuntimeException);
			virtual ::sal_Int32 SAL_CALL getFieldStart( ::sal_Int32 addr ) throw (::com::sun::star::uno::RuntimeException);
			virtual ::sal_Int32 SAL_CALL getFieldLen( ::sal_Int32 addr ) throw (::com::sun::star::uno::RuntimeException);
			virtual ::sal_Int32 SAL_CALL getNextUnprotected( ::sal_Int32 addr ) throw (::com::sun::star::uno::RuntimeException);

			// Wait
			virtual ::sal_Int16 SAL_CALL waitForReady( ::sal_Int16 seconds ) throw (::com::sun::star::uno::RuntimeException);
			virtual ::sal_Int16 SAL_CALL waitForTextAt( ::sal_Int16 row, ::sal_Int16 col, const ::rtl::OUString& str, ::sal_Int16 seconds ) throw (::com::sun::star::uno::RuntimeException);

			// Actions
			virtual ::sal_Int16 SAL_CALL enter() throw (::com::sun::star::uno::RuntimeException);
			virtual ::sal_Int16 SAL_CALL pfkey( ::sal_Int16 key ) throw (::com::sun::star::uno::RuntimeException);
			virtual ::sal_Int16 SAL_CALL pakey( ::sal_Int16 key ) throw (::com::sun::star::uno::RuntimeException);
			virtual ::sal_Int16 SAL_CALL quit() throw (::com::sun::star::uno::RuntimeException);
			virtual ::sal_Int16 SAL_CALL eraseEOF() throw (::com::sun::star::uno::RuntimeException);

			// Cursor
			virtual ::sal_Int32 SAL_CALL setCursorAt( ::sal_Int16 row, ::sal_Int16 col ) throw (::com::sun::star::uno::RuntimeException);
			virtual ::sal_Int32 SAL_CALL setCursorAddress( ::sal_Int32 addr ) throw (::com::sun::star::uno::RuntimeException);
			virtual ::sal_Int32 SAL_CALL getCursorAddress() throw (::com::sun::star::uno::RuntimeException);

		private:
			h3270::session		* hSession;
			rtl_TextEncoding	  encoding;

		};

		extern Sequence< OUString > SAL_CALL getSupportedServiceNames_session_impl();
		extern OUString SAL_CALL getImplementationName_session_impl();
		extern Reference< XInterface > SAL_CALL create_session_impl(Reference< XComponentContext > const & xContext ) SAL_THROW( () );

	};


#endif // PW3270_OXT_GLOBALS_HPP_INCLUDED
