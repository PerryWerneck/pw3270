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
 * Este programa está nomeado como get.cc e possui - linhas de código.
 *
 * Contatos:
 *
 *	perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 *	erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 * Referência:
 *
 *	https://wiki.openoffice.org/wiki/Documentation/DevGuide/WritingUNO/C%2B%2B/Class_Definition_with_Helper_Template_Classes
 *
 */

 #include "globals.hpp"
 #include <exception>
 #include <com/sun/star/uno/RuntimeException.hdl>
 #include <com/sun/star/lang/IllegalArgumentException.hpp>
 #include "pw3270/lib3270.hpp"

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

 using namespace pw3270_impl;
 using namespace com::sun::star::uno;

 ::sal_Int16 SAL_CALL session_impl::setSessionName( const ::rtl::OUString& name ) throw (::com::sun::star::uno::RuntimeException)
 {
 	if(hSession)
	{
		// Remove old session
		delete hSession;
		hSession = NULL;
	}

	OString vlr = rtl::OUStringToOString( name , RTL_TEXTENCODING_UNICODE );

	trace("%s(\"%s\")",__FUNCTION__,vlr.getStr());

	try
	{
		string charset;

		hSession = h3270::session::create(((const char *) vlr.getStr()));

	 	trace("%s: hSession(\"%s\"=%p",__FUNCTION__,vlr.getStr(),hSession);

	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);

		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));

		return -1;

	}

	string charset = hSession->get_display_charset();

	trace("Charset=\"%s\"",charset.c_str());

	if(!charset.compare("ISO-8859-1"))
	{
		encoding = RTL_TEXTENCODING_ISO_8859_1;
	}
	else if(!charset.compare("UTF-8"))
	{
		encoding = RTL_TEXTENCODING_UTF8;
	}
	else
	{
		string s = "Unable to convert the host's display charset " + charset + ".";

		throw lang::IllegalArgumentException(
					OUString( RTL_CONSTASCII_USTRINGPARAM(s.c_str()) ),
					(::cppu::OWeakObject *)this,
					0 );

	}

	return 0;

 }

 ::sal_Int16 SAL_CALL session_impl::setHost( const ::rtl::OUString& url ) throw (::com::sun::star::uno::RuntimeException)
 {
	if(!hSession)
		hSession = h3270::session::get_default();

	OString vlr = rtl::OUStringToOString( url , RTL_TEXTENCODING_UNICODE );

	try
	{

		return hSession->set_url(vlr.getStr());

	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);

		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));

	}

	return -1;
 }

 ::sal_Int16 SAL_CALL session_impl::setTextAt( ::sal_Int16 row, ::sal_Int16 col, const ::rtl::OUString& str ) throw (::com::sun::star::uno::RuntimeException)
 {
	try
	{
		CHECK_SESSION_HANDLE
		OString vlr = rtl::OUStringToOString(str,encoding);
		return hSession->set_text_at(row,col,vlr.getStr());

	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);
		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));
	}

	return -1;

 }

 ::sal_Int16 SAL_CALL session_impl::enter() throw (::com::sun::star::uno::RuntimeException)
 {
	try
	{
		CHECK_SESSION_HANDLE
		return hSession->enter();

	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);
		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));
	}

	return -1;

 }

 ::sal_Int16 SAL_CALL session_impl::pfkey( ::sal_Int16 key ) throw (::com::sun::star::uno::RuntimeException)
 {
	try
	{
		CHECK_SESSION_HANDLE
		return hSession->pfkey(key);

	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);
		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));
	}

	return -1;

 }

 ::sal_Int16 SAL_CALL session_impl::pakey( ::sal_Int16 key ) throw (::com::sun::star::uno::RuntimeException)
 {
	try
	{
		CHECK_SESSION_HANDLE
		return hSession->pakey(key);

	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);
		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));
	}

	return -1;

 }

::sal_Int32 SAL_CALL session_impl::setCursorAt( ::sal_Int16 row, ::sal_Int16 col ) throw (::com::sun::star::uno::RuntimeException)
 {
	try
	{
		CHECK_SESSION_HANDLE
		return hSession->set_cursor_position(row,col);

	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);
		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));
	}

	return -1;

 }

 ::sal_Int32 SAL_CALL session_impl::setCursorAddress( ::sal_Int32 addr ) throw (::com::sun::star::uno::RuntimeException)
 {
	try
	{
		CHECK_SESSION_HANDLE
		return hSession->set_cursor_addr(addr);

	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);
		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));
	}

	return -1;

 }

 ::sal_Int16 SAL_CALL session_impl::eraseEOF() throw (::com::sun::star::uno::RuntimeException)
 {
	try
	{
		CHECK_SESSION_HANDLE
		return hSession->erase_eof();

	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);
		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));
	}

	return -1;

 }
