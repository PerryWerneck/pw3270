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


	#include <cppuhelper/supportsservice.hxx>

	#include <cppuhelper/implbase2.hxx> // "3" implementing three interfaces
//	#include <cppuhelper/factory.hxx>
//	#include <cppuhelper/implementationentry.hxx>
//	#include <com/sun/star/lang/XServiceInfo.hpp>
//	#include <com/sun/star/lang/IllegalArgumentException.hpp>

	#include <pw3270/lib3270.hpp>


	using namespace ::rtl; 					// for OUString
	using namespace ::com::sun::star; 		// for sdk interfaces
	using namespace ::com::sun::star::uno;	// for basic types


	namespace pw3270_impl
	{
		// https://wiki.openoffice.org/wiki/Documentation/DevGuide/WritingUNO/C%2B%2B/Class_Definition_with_Helper_Template_Classes
		class sessionImpl
		// : public lang::XServiceInfo
		// ::cppu::WeakImplHelper2< ::pw3270::lib3270, lang::XServiceInfo >
		{
		};

		/*
		// https://wiki.openoffice.org/wiki/Documentation/DevGuide/WritingUNO/C%2B%2B/Implementing_without_Helpers
		class sessionImpl
		{
			oslInterlockedCount m_refcount;

		public:
			inline sessionImpl() throw () : m_refcount( 0 )
			{
			}

			// XInterface
			virtual Any SAL_CALL queryInterface( Type const & type ) throw (RuntimeException);
			virtual void SAL_CALL acquire() throw ();
			virtual void SAL_CALL release() throw ();
		};
		*/

	};


#endif // PW3270_OXT_GLOBALS_HPP_INCLUDED
