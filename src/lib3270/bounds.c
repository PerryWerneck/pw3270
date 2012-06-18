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
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como bounds.c e possui - linhas de código.
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

#include "globals.h"

/*--[ Implement ]------------------------------------------------------------------------------------*/

LIB3270_EXPORT int lib3270_get_field_bounds(H3270 *session, int baddr, int *start, int *end)
{
	int first;

	CHECK_SESSION_HANDLE(session);

	if(!lib3270_connected(session))
		return -1;

	first = lib3270_field_addr(session,baddr);

	if(first < 0)
		return -1;

	first++;

	if(start)
		*start = first;

	if(end)
	{
		int maxlen = (session->rows * session->cols)-1;
		*end	= first + lib3270_field_length(session,first);
		if(*end > maxlen)
			*end = maxlen;
	}

	return 0;
}

LIB3270_EXPORT int lib3270_get_word_bounds(H3270 *session, int baddr, int *start, int *end)
{
	int pos;

	CHECK_SESSION_HANDLE(session);

	if(!lib3270_connected(session) || isspace(session->text[baddr].chr))
		return -1;

	if(start)
	{
		for(pos = baddr; pos > 0 && !isspace(session->text[pos].chr);pos--);

		*start = pos > 0 ? pos+1 : 0;
	}

	if(end)
	{
		int maxlen = session->rows * session->cols;
		for(pos = baddr; pos < maxlen && !isspace(session->text[pos].chr);pos++);

		*end = pos < maxlen ? pos-1 : maxlen;
	}

	return 0;
}


