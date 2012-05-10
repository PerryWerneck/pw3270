/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob o nome G3270.
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
 * Este programa está nomeado como xio.c e possui 143 linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */

/*
 *	xio.c
 *		Low-level I/O setup functions and exit code.
 */

#include "globals.h"

#include "actionsc.h"
#include "hostc.h"
#include "telnetc.h"
#include "toggle.h"
#include "utilc.h"
#include "xioc.h"

/* Statics. */
// static unsigned long ns_read_id;
// static unsigned long ns_exception_id;
// static Boolean reading = False;
// static Boolean excepting = False;

/*
 * Called to set up input on a new network connection.
 */
/*
void x_add_input(H3270 *h)
{
#ifdef _WIN32
	h->ns_exception_id = AddExcept(h->sockEvent, h, net_exception);
	h->excepting = True;
	h->ns_read_id = AddInput(h->sockEvent, h, net_input);
	h->reading = True;
#else
	h->ns_exception_id = AddExcept(h->sock, h, net_exception);
	h->excepting = True;
	h->ns_read_id = AddInput(h->sock, h, net_input);
	h->reading = True;
#endif // WIN32
}
*/
/*
 * Called when an exception is received to disable further exceptions.
 */ /*
void x_except_off(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);

	if(h->excepting)
	{
		RemoveInput(h->ns_exception_id);
		h->excepting = False;
	}
}
*/

/*
 * Called when exception processing is complete to re-enable exceptions.
 * This includes removing and restoring reading, so the exceptions are always
 * processed first.
 */
void x_except_on(H3270 *h)
{
	if(h->excepting)
		return;

	if(h->reading)
		RemoveInput(h->ns_read_id);

#ifdef WIN32
	h->ns_exception_id = AddExcept(h->sockEvent, h, net_exception);
	h->excepting = True;

	if(h->reading)
		h->ns_read_id = AddInput(h->sockEvent, h, net_input);
#else
	h->ns_exception_id = AddExcept(h->sock, h, net_exception);
	h->excepting = True;

	if(h->reading)
		h->ns_read_id = AddInput(h->sock, h, net_input);
#endif // WIN32
}

/*
 * Called to disable input on a closing network connection.
 */ /*
void x_remove_input(H3270 *h)
{
	if(h->reading)
	{
		RemoveInput(h->ns_read_id);
		h->reading = False;
	}
	if(h->excepting)
	{
		RemoveInput(h->ns_exception_id);
		h->excepting = False;
	}
}
*/
