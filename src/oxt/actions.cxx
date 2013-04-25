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
 * Este programa está nomeado como actions.cxx e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

 #include "globals.hpp"
 #include <unistd.h>
 #include <string.h>

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

::sal_Int16 SAL_CALL pw3270::uno_impl::enter(  ) throw (::com::sun::star::uno::RuntimeException)
{
	return hSession->enter();
}

::sal_Int16 SAL_CALL pw3270::uno_impl::pfkey( ::sal_Int16 keycode ) throw (::com::sun::star::uno::RuntimeException)
{
	return hSession->pfkey((int) keycode);
}

::sal_Int16 SAL_CALL pw3270::uno_impl::pakey( ::sal_Int16 keycode ) throw (::com::sun::star::uno::RuntimeException)
{
	return hSession->pakey((int) keycode);
}

::sal_Int16 SAL_CALL pw3270::uno_impl::cmpTextAt( ::sal_Int16 row, ::sal_Int16 col, const ::rtl::OUString& text ) throw (::com::sun::star::uno::RuntimeException)
{
	return hSession->cmp_text_at((int) row, (int) col, rtl::OUStringToOString(text,hSession->get_encoding()).getStr());
}

::sal_Bool SAL_CALL pw3270::uno_impl::isReady(  ) throw (::com::sun::star::uno::RuntimeException)
{
	if(!hSession->in_tn3270e())
		return false;

	if(hSession->get_state() == LIB3270_MESSAGE_NONE)
		return true;

	return false;
}

::sal_Bool SAL_CALL pw3270::uno_impl::isConnected(  ) throw (::com::sun::star::uno::RuntimeException)
{
	return hSession->in_tn3270e() != 0;
}

::sal_Bool SAL_CALL pw3270::uno_impl::hasTextAt( ::sal_Int16 row, ::sal_Int16 col, const ::rtl::OUString& text ) throw (::com::sun::star::uno::RuntimeException)
{
	if(!hSession->in_tn3270e())
		return false;

	return cmpTextAt(row,col,text) == 0;
}

::sal_Int16 SAL_CALL pw3270::uno_impl::waitForReady( ::sal_Int16 seconds ) throw (::com::sun::star::uno::RuntimeException)
{
	time_t end = time(0) + seconds;

	while(time(0) < end)
	{
		osl_yieldThread();

		switch(hSession->get_state())
		{
		case LIB3270_MESSAGE_NONE:
			return 0;

		case LIB3270_MESSAGE_DISCONNECTED:
			return ENOTCONN;

		case LIB3270_MESSAGE_MINUS:
		case LIB3270_MESSAGE_PROTECTED:
		case LIB3270_MESSAGE_NUMERIC:
		case LIB3270_MESSAGE_OVERFLOW:
		case LIB3270_MESSAGE_INHIBIT:
		case LIB3270_MESSAGE_KYBDLOCK:
			return EPROTO;


		}

		sleep(1);
	}

	return ETIMEDOUT;
}
