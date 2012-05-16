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
 * Este programa está nomeado como state.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#include "globals.h"

/*---[ Implement ]------------------------------------------------------------------------------------------------------------*/

LIB3270_EXPORT LIB3270_CSTATE lib3270_get_connection_state(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return h->cstate;
}

LIB3270_EXPORT int lib3270_pconnected(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return (((int) h->cstate) >= (int)RESOLVING);
}

LIB3270_EXPORT int lib3270_half_connected(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return (h->cstate == RESOLVING || h->cstate == PENDING);
}

LIB3270_EXPORT int lib3270_connected(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return ((int) h->cstate >= (int)CONNECTED_INITIAL);
}

LIB3270_EXPORT int lib3270_in_neither(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return (h->cstate == CONNECTED_INITIAL);
}

LIB3270_EXPORT int lib3270_in_ansi(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return (h->cstate == CONNECTED_ANSI || h->cstate == CONNECTED_NVT);
}

LIB3270_EXPORT int lib3270_in_3270(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return (h->cstate == CONNECTED_3270 || h->cstate == CONNECTED_TN3270E || h->cstate == CONNECTED_SSCP);
}

LIB3270_EXPORT int lib3270_in_sscp(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return (h->cstate == CONNECTED_SSCP);
}

LIB3270_EXPORT int lib3270_in_tn3270e(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return (h->cstate == CONNECTED_TN3270E);
}

LIB3270_EXPORT int lib3270_in_e(H3270 *h)
{
	CHECK_SESSION_HANDLE(h);
	return (h->cstate >= CONNECTED_INITIAL_E);
}


