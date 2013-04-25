

#ifndef GLOBALS_HPP_INCLUDED

	#define GLOBALS_HPP_INCLUDED 1
	#define UNX	1
	#define GCC 1
	#define LINUX 1
	#define CPPU_ENV gcc3
	#define HAVE_GCC_VISIBILITY_FEATURE 1
	#define LANGUAGE_BINDING_NAME "gcc3"

	#include <stdio.h>
	#include <lib3270.h>

#if defined(HAVE_DBUS)
	#include <stdio.h>
	#include <dbus/dbus.h>
#endif // HAVE_DBUS

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

		class uno_impl;

		class session
		{
			public:
				session();
				virtual ~session();
				virtual int					get_revision(void)								= 0;
				virtual LIB3270_MESSAGE 	get_state(void)									= 0;
				virtual char			 *  get_text_at(int row, int col, int len)			= 0;
				virtual int					set_text_at(int row, int col, const char *text) = 0;
				virtual int					cmp_text_at(int row, int col, const char *text) = 0;
				virtual void			    set_toggle(LIB3270_TOGGLE toggle, bool state)	= 0;

				virtual int					connect(const char *uri)						= 0;
				virtual int					disconnect(void)								= 0;
				virtual bool				connected(void)									= 0;

				virtual int					enter(void)										= 0;
				virtual int					pfkey(int key)									= 0;
				virtual int					pakey(int key)									= 0;
				virtual bool 				in_tn3270e()									= 0;
				virtual void				mem_free(void *)								= 0;

				void						sleep(int seconds = 1);

				void					    log(const char *fmt, const char *msg);

				rtl_TextEncoding			get_encoding();

		};

		class lib3270_session : public session
		{
			public:
				lib3270_session(uno_impl *obj) throw( RuntimeException );
				virtual ~lib3270_session();

				virtual int					get_revision(void);
				virtual LIB3270_MESSAGE 	get_state(void);

				virtual int					connect(const char *uri);
				virtual int					disconnect(void);
				virtual bool				connected(void);
				virtual bool 				in_tn3270e(void);

				virtual int					enter(void);
				virtual int					pfkey(int key);
				virtual int					pakey(int key);

				virtual char			 *  get_text_at(int row, int col, int len);
				virtual int					set_text_at(int row, int col, const char *text);
				virtual int					cmp_text_at(int row, int col, const char *text);

				virtual void				mem_free(void *ptr);
				virtual void			    set_toggle(LIB3270_TOGGLE toggle, bool state);

			private:
				bool		  enabled;
				oslModule	  hModule;
				oslThread	  hThread;
				H3270		* hSession;

				/* Internal calls */
				static void start_connect(lib3270_session *session);
				void network_loop(void);

				/* lib3270 entry points */
				const char		* (* _get_revision)(void);
				LIB3270_MESSAGE	  (* _get_program_message)(H3270 *);
				int				  (* _in_tn3270e)(H3270 *);
				char 			* (* _get_text_at)(H3270 *,int,int,int);
				int				  (* _set_text_at)(H3270 *,int,int,const unsigned char *);
				int 			  (* _cmp_text_at)(H3270 *,int,int,const char *);
				int				  (* _enter)(H3270 *);
				int				  (* _pfkey)(H3270 *, int);
				int				  (* _pakey)(H3270 *, int);
				void			* (* _mem_free)(void *);
				void 			  (*_set_toggle)(void *h, LIB3270_TOGGLE ix, int value);


		};

		class ipc3270_session : public session
		{
			public:
				ipc3270_session(uno_impl *obj, const char *name) throw( RuntimeException );
				virtual ~ipc3270_session();
				virtual int					get_revision(void);
				virtual LIB3270_MESSAGE 	get_state(void);
				virtual char			 *  get_text_at(int row, int col, int len);
				virtual int					set_text_at(int row, int col, const char *text);
				virtual int					cmp_text_at(int row, int col, const char *text);
				virtual void			    set_toggle(LIB3270_TOGGLE toggle, bool state);

				virtual int					connect(const char *uri);
				virtual int					disconnect(void);
				virtual bool				connected(void);

				virtual int					enter(void);
				virtual int					pfkey(int key);
				virtual int					pakey(int key);
				virtual bool 				in_tn3270e();
				virtual void				mem_free(void *);

			private:

#if defined(HAVE_DBUS)

				DBusConnection	* conn;
				char			* dest;
				char			* path;
				char			* intf;
				DBusMessage		* create_message(const char *method);
				DBusMessage		* call(DBusMessage *msg);
				char 			* query_string(const char *method);
				int 			  query_intval(const char *method);

#endif // HAVE_DBUS

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
				virtual ::sal_Int16 SAL_CALL getRevision() throw (RuntimeException);
				virtual ::rtl::OUString SAL_CALL getVersion(  ) throw (::com::sun::star::uno::RuntimeException);
				virtual ::sal_Int16 SAL_CALL Connect( const ::rtl::OUString& hostinfo ) throw (::com::sun::star::uno::RuntimeException);
				virtual ::sal_Int16 SAL_CALL Disconnect(  ) throw (::com::sun::star::uno::RuntimeException);
				virtual ::sal_Int16 SAL_CALL getConnectionState(  ) throw (::com::sun::star::uno::RuntimeException);
				virtual ::sal_Int16 SAL_CALL sleep( ::sal_Int16 seconds ) throw (::com::sun::star::uno::RuntimeException);
				virtual ::rtl::OUString SAL_CALL getTextAt( ::sal_Int16 row, ::sal_Int16 col, ::sal_Int16 size ) throw (::com::sun::star::uno::RuntimeException);
				virtual ::sal_Int16 SAL_CALL setTextAt( ::sal_Int16 row, ::sal_Int16 col, const ::rtl::OUString& text ) throw (::com::sun::star::uno::RuntimeException);
				virtual ::sal_Int16 SAL_CALL log(const ::rtl::OUString& text ) throw (::com::sun::star::uno::RuntimeException);
				virtual ::sal_Int16 SAL_CALL enter(  ) throw (::com::sun::star::uno::RuntimeException);
				virtual ::sal_Int16 SAL_CALL pfkey( ::sal_Int16 keycode ) throw (::com::sun::star::uno::RuntimeException);
				virtual ::sal_Int16 SAL_CALL pakey( ::sal_Int16 keycode ) throw (::com::sun::star::uno::RuntimeException);
				virtual ::sal_Int16 SAL_CALL cmpTextAt( ::sal_Int16 row, ::sal_Int16 col, const ::rtl::OUString& text ) throw (::com::sun::star::uno::RuntimeException);
				virtual ::sal_Bool SAL_CALL isReady(  ) throw (::com::sun::star::uno::RuntimeException);
				virtual ::sal_Bool SAL_CALL isConnected(  ) throw (::com::sun::star::uno::RuntimeException);
				virtual ::sal_Bool SAL_CALL hasTextAt( ::sal_Int16 row, ::sal_Int16 col, const ::rtl::OUString& text ) throw (::com::sun::star::uno::RuntimeException);
				virtual ::sal_Int16 SAL_CALL waitForReady( ::sal_Int16 seconds ) throw (::com::sun::star::uno::RuntimeException);
				virtual ::sal_Int16 SAL_CALL dsTrace( ::sal_Bool state ) throw (::com::sun::star::uno::RuntimeException);
				virtual ::sal_Int16 SAL_CALL screenTrace( ::sal_Bool state ) throw (::com::sun::star::uno::RuntimeException);
				virtual ::sal_Int16 SAL_CALL setSession( const ::rtl::OUString& name ) throw (::com::sun::star::uno::RuntimeException);

				void failed(const char *fmt, ...) throw( ::com::sun::star::uno::RuntimeException );

			private:

				session *hSession;

		};

	};




#endif // GLOBALS_HPP_INCLUDED

