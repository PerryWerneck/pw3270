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
 #include <lib3270/config.h>
 #include "pw3270/lib3270.hpp"

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

 using namespace pw3270_impl;

 ::rtl::OUString session_impl::getVersion() throw (RuntimeException)
 {
 	trace("%s: hSession=%p",__FUNCTION__,hSession);
	return OUString( RTL_CONSTASCII_USTRINGPARAM(PACKAGE_VERSION) );
 }

 ::rtl::OUString session_impl::getRevision() throw (RuntimeException)
 {
 	trace("%s: hSession=%p",__FUNCTION__,hSession);
	return OUString( RTL_CONSTASCII_USTRINGPARAM(PACKAGE_REVISION) );
 }

 ::rtl::OUString SAL_CALL session_impl::getTextAt( ::sal_Int16 row, ::sal_Int16 col, ::sal_Int16 size ) throw (::com::sun::star::uno::RuntimeException)
 {
 	string s;

	try
	{
		CHECK_SESSION_HANDLE
		s = hSession->get_text_at(row,col,size);

	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);
		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));
	}

	return OUString(s.c_str(), s.length(), encoding, RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);
 }

 ::sal_Int16 SAL_CALL session_impl::waitForTextAt( ::sal_Int16 row, ::sal_Int16 col, const ::rtl::OUString& str, ::sal_Int16 seconds ) throw (::com::sun::star::uno::RuntimeException)
 {
	try
	{
		CHECK_SESSION_HANDLE
		OString vlr = rtl::OUStringToOString(str,encoding);
		return hSession->wait_for_text_at(row,col,vlr.getStr(),seconds);

	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);
		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));
	}

	return -1;

 }

 ::sal_Int32 SAL_CALL session_impl::getCursorAddress() throw (::com::sun::star::uno::RuntimeException)
 {
	try
	{
		CHECK_SESSION_HANDLE
		return hSession->get_cursor_addr();

	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);
		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));
	}

	return -1;

 }

 ::sal_Int32 SAL_CALL session_impl::getFieldStart( ::sal_Int32 addr ) throw (::com::sun::star::uno::RuntimeException)
 {
	try
	{
		CHECK_SESSION_HANDLE
		return hSession->get_field_start(addr);

	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);
		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));
	}

	return -1;

 }

 ::sal_Int32 SAL_CALL session_impl::getFieldLen( ::sal_Int32 addr ) throw (::com::sun::star::uno::RuntimeException)
 {
	try
	{
		CHECK_SESSION_HANDLE
		return hSession->get_field_len(addr);

	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);
		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));
	}

	return -1;

 }

 ::sal_Int32 SAL_CALL session_impl::getNextUnprotected( ::sal_Int32 addr ) throw (::com::sun::star::uno::RuntimeException)
 {
	try
	{
		CHECK_SESSION_HANDLE
		return hSession->get_next_unprotected(addr);

	} catch(std::exception &e)
	{
		OUString msg = OUString(e.what(),strlen(e.what()),RTL_TEXTENCODING_UTF8,RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_IGNORE);
		throw css::uno::RuntimeException(msg,static_cast< cppu::OWeakObject * >(this));
	}

	return -1;

 }
