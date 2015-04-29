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
 * Este programa está nomeado como connect.cc e possui - linhas de código.
 *
 * Contatos:
 *
 *	perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 *	erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 *
 */

 #include "globals.hpp"
 #include "pw3270/lib3270.hpp"
 #include <exception>
 #include <com/sun/star/uno/RuntimeException.hdl>

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

 using namespace pw3270_impl;

/*
::sal_Int16 SAL_CALL session_impl::Reconnect() throw (::com::sun::star::uno::RuntimeException)
 {
 	trace("%s: hSession=%p",__FUNCTION__,hSession);

	try
	{
		if(!hSession)
			hSession = h3270::session::get_default();

		return hSession->connect();

	} catch(std::exception &e)
	{
		trace("%s failed: %s",__FUNCTION__,e.what());

		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);

		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));

	}

	return -1;


 }
*/

 ::sal_Int16 SAL_CALL session_impl::Connect(const ::rtl::OUString& url, ::sal_Bool wait) throw (::com::sun::star::uno::RuntimeException)
 {
 	trace("%s: hSession=%p",__FUNCTION__,hSession);

	OString vlr = rtl::OUStringToOString( url , RTL_TEXTENCODING_UNICODE );

	try
	{
		if(!hSession)
			hSession = h3270::session::get_default();

		return hSession->connect(vlr.getStr(),wait);

	} catch(std::exception &e)
	{
		trace("%s failed: %s",__FUNCTION__,e.what());

		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);

		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));

	}

	return -1;


 }

 ::sal_Int16 SAL_CALL session_impl::Disconnect() throw (::com::sun::star::uno::RuntimeException)
 {
	try
	{
		CHECK_SESSION_HANDLE

		return hSession->disconnect();

	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);

		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));

	}

	return -1;

 }

 ::sal_Bool SAL_CALL session_impl::isConnected() throw (::com::sun::star::uno::RuntimeException)
 {
	try
	{
		CHECK_SESSION_HANDLE

		return hSession->is_connected();

	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);

		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));

	}

	return -1;
 }

 ::sal_Bool SAL_CALL session_impl::isReady() throw (::com::sun::star::uno::RuntimeException)
 {
	try
	{
		CHECK_SESSION_HANDLE

		return hSession->is_ready();

	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);

		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));

	}

	return -1;

 }

 ::sal_Int16 SAL_CALL session_impl::waitForReady( ::sal_Int16 seconds ) throw (::com::sun::star::uno::RuntimeException)
 {
	try
	{
		CHECK_SESSION_HANDLE

		return hSession->wait_for_ready(seconds);

	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);

		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));

	}

	return -1;

 }


