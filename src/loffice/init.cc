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
 * Este programa está nomeado como init.cc e possui - linhas de código.
 *
 * Contatos:
 *
 *	perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 *	erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 * Referência:
 *
 *	https://wiki.openoffice.org/wiki/Documentation/DevGuide/WritingUNO/C%2B%2B/Create_Instance_with_Arguments
 *
 */

 #include "globals.hpp"
 #include <com/sun/star/lang/IllegalArgumentException.hpp>
 #include "pw3270/lib3270.hpp"

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

using namespace pw3270_impl;


session_impl::session_impl()
{
	this->hSession	= NULL;
	this->encoding	= RTL_TEXTENCODING_ISO_8859_1;
}

session_impl::~session_impl()
{
	if(this->hSession)
		delete this->hSession;
}


// XInitialization implementation
void session_impl::initialize( Sequence< Any > const & args ) throw (Exception)
{
	/*
	if (1 != args.getLength())
	{
		throw lang::IllegalArgumentException(
					OUString( RTL_CONSTASCII_USTRINGPARAM("give a string instanciating this component!") ),
					(::cppu::OWeakObject *)this,
					0 );
	}
	*/

	trace("%s args=%d",__FUNCTION__,(int) args.getLength());

	// Process arguments
	for(int f = 0; f < args.getLength();f++)
	{
		trace("Arg(%d) is %s",f,args[f].getValueType().getTypeName().getStr());

	}


}

// XMain
::sal_Int32 SAL_CALL session_impl::run( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aArguments ) throw (Exception)
{


	return 0;
}

::sal_Int16 SAL_CALL session_impl::iterate( ::sal_Bool wait ) throw (::com::sun::star::uno::RuntimeException)
{
	try
	{
		CHECK_SESSION_HANDLE
		return hSession->iterate(wait);

	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);
		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));
	}
	return -1;
}

 ::sal_Int16 SAL_CALL session_impl::quit() throw (::com::sun::star::uno::RuntimeException)
 {
	try
	{
		CHECK_SESSION_HANDLE
		return hSession->quit();
	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);
		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));
	}

	return -1;

 }



