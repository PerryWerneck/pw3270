

#ifndef GLOBALS_HPP_INCLUDED

	#define GLOBALS_HPP_INCLUDED 1
	#define UNX	1
	#define GCC 1
	#define LINUX 1
	#define CPPU_ENV gcc3
	#define HAVE_GCC_VISIBILITY_FEATURE 1
	#define LANGUAGE_BINDING_NAME "gcc3"

	#include <stdio.h>

	#include <rtl/uuid.h>
	#include <osl/thread.hxx>

	#include <cppuhelper/implbase3.hxx> // "3" implementing three interfaces
	#include <cppuhelper/factory.hxx>
	#include <cppuhelper/implementationentry.hxx>

	#include <com/sun/star/lang/XServiceInfo.hpp>
	#include <com/sun/star/lang/XInitialization.hpp>
	#include <com/sun/star/lang/IllegalArgumentException.hpp>

	#include <pw3270intf.hpp>

	/*---[ Debug macros ]--------------------------------------------------------------------------------------*/

	#ifdef DEBUG
		#define trace(fmt, ... ) fprintf(stderr, "%s(%d) " fmt "\n", __FILE__, __LINE__, __VA_ARGS__ ); fflush(stderr);
	#else
		#define trace( fmt, ... ) /* fmt __VA_ARGS__ */
	#endif

	/*---[ Object implementation ]-----------------------------------------------------------------------------*/

	#define IMPLNAME "br.com.bb.pw3270intf"
	#define SERVICENAME "br.com.bb.pw3270"

	using namespace br::com::bb;
	using namespace ::rtl; // for OUString
	using namespace ::com::sun::star; // for sdk interfaces
	using namespace ::com::sun::star::uno; // for basic types

	namespace pw3270
	{

		class session
		{
			public:
				session();
				virtual ~session();
				virtual int get_revision(void)	= 0;

		};

		class lib3270_session : public session
		{
			public:
				lib3270_session();
				virtual ~lib3270_session();

				virtual int get_revision(void);

			private:
				oslModule	  hModule;
				void		* hSession;

				/* lib3270 entry points */
				const char	* (* _get_revision)(void);

		};

		class uno_impl : public ::cppu::WeakImplHelper3< br::com::bb::pw3270intf, com::sun::star::lang::XServiceInfo, com::sun::star::lang::XInitialization >
		{
			public:

				uno_impl( const com::sun::star::uno::Reference< XComponentContext > & xContext );
				virtual ~uno_impl();

				// XInitialization will be called upon createInstanceWithArguments[AndContext]()
				virtual void SAL_CALL initialize( Sequence< Any > const & args ) throw (Exception);

				// XServiceInfo	implementation
				virtual OUString SAL_CALL getImplementationName(  ) throw(RuntimeException);
				virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(RuntimeException);
				virtual Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(RuntimeException);

				// pw3270 implementation - Main
				virtual sal_Int16 SAL_CALL getRevision() throw (RuntimeException);

			private:

				session *hSession;



		};

	};




#endif // GLOBALS_HPP_INCLUDED

