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
 * Este programa está nomeado como remote.cxx e possui - linhas de código.
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
 #include <errno.h>
 #include <string.h>

/*---[ Statics ]-------------------------------------------------------------------------------------------*/


/*---[ Implement ]-----------------------------------------------------------------------------------------*/

pw3270::ipc3270_session::ipc3270_session(const char *name) : pw3270::session()
{
#ifdef HAVE_DBUS

#else

#endif // HAVE_DBUS
}

pw3270::ipc3270_session::~ipc3270_session()
{
#ifdef HAVE_DBUS

#endif // HAVE_DBUS
}

int pw3270::ipc3270_session::get_revision(void)
{
#ifdef HAVE_DBUS

	return -1;

#else

	return -1;

#endif // HAVE_DBUS
}

LIB3270_MESSAGE pw3270::ipc3270_session::get_state(void)
{
#ifdef HAVE_DBUS

	return (LIB3270_MESSAGE) -1;

#else

	return (LIB3270_MESSAGE) -1;

#endif // HAVE_DBUS
}

char * pw3270::ipc3270_session::get_text_at(int row, int col, int len)
{
#ifdef HAVE_DBUS

	return NULL;

#else

	return NULL;

#endif // HAVE_DBUS
}

int pw3270::ipc3270_session::set_text_at(int row, int col, const char *text)
{
#ifdef HAVE_DBUS

	return -1;

#else

	return -1;

#endif // HAVE_DBUS
}

int pw3270::ipc3270_session::cmp_text_at(int row, int col, const char *text)
{
#ifdef HAVE_DBUS

	return -1;

#else

	return -1;

#endif // HAVE_DBUS
}

void pw3270::ipc3270_session::set_toggle(LIB3270_TOGGLE toggle, bool state)
{
}

int pw3270::ipc3270_session::connect(const char *uri)
{
#ifdef HAVE_DBUS

	return -1;

#else

	return -1;

#endif // HAVE_DBUS
}

int pw3270::ipc3270_session::disconnect(void)
{
#ifdef HAVE_DBUS

	return -1;

#else

	return -1;

#endif // HAVE_DBUS
}

bool pw3270::ipc3270_session::connected(void)
{
#ifdef HAVE_DBUS

	return false;

#else

	return false;

#endif // HAVE_DBUS
}

int pw3270::ipc3270_session::enter(void)
{
#ifdef HAVE_DBUS

	return -1;

#else

	return -1;

#endif // HAVE_DBUS
}

int pw3270::ipc3270_session::pfkey(int key)
{
#ifdef HAVE_DBUS

	return -1;

#else

	return -1;

#endif // HAVE_DBUS
}

int	pw3270::ipc3270_session::pakey(int key)
{
#ifdef HAVE_DBUS

	return -1;

#else

	return -1;

#endif // HAVE_DBUS
}

bool pw3270::ipc3270_session::in_tn3270e()
{
#ifdef HAVE_DBUS

	return false;

#else

	return false;

#endif // HAVE_DBUS
}

void pw3270::ipc3270_session::mem_free(void *ptr)
{
#ifdef HAVE_DBUS


#else


#endif // HAVE_DBUS
}


