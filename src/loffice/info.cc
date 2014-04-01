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
 *	https://wiki.openoffice.org/wiki/Documentation/DevGuide/WritingUNO/C%2B%2B/Class_Definition_with_Helper_Template_Classes
 *
 */

 #include "globals.hpp"
 #include "pw3270/lib3270.hpp"

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

using namespace pw3270_impl;

// XServiceInfo implementation
OUString session_impl::getImplementationName() throw (RuntimeException)
{
	// unique implementation name
	return OUString( RTL_CONSTASCII_USTRINGPARAM("pw3270.pw3270_impl.session") );
}

sal_Bool session_impl::supportsService( OUString const & serviceName ) throw (RuntimeException)
{
	// this object only supports one service, so the test is simple
	return serviceName.equalsAsciiL(RTL_CONSTASCII_STRINGPARAM("pw3270.session") );
}

Sequence< OUString > session_impl::getSupportedServiceNames() throw (RuntimeException)
{
	return getSupportedServiceNames_session_impl();
}

