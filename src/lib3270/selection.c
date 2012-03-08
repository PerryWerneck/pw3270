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
 * Este programa está nomeado como selection.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "globals.h"
 #include "ctlr.h"
 #include <lib3270.h>
 #include <lib3270/session.h>
 #include <lib3270/selection.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

LIB3270_EXPORT void lib3270_clear_selection(H3270 *session)
{
	int a;

	session->selected.begin	= -1;
	session->selected.end	= -1;

	for(a = 0; a < session->rows*session->cols; a++)
	{
		unsigned short attr = ea_buf[a].attr;

		if(ea_buf[a].attr & LIB3270_ATTR_SELECTED)
		{
			ea_buf[a].attr &= ~LIB3270_ATTR_SELECTED;
			if(session->update)
				session->update(session,a,ea_buf[a].chr,ea_buf[a].attr,a == session->cursor_addr);
		}
	}
}

LIB3270_EXPORT void lib3270_select_to(H3270 *session, int baddr)
{
	CHECK_SESSION_HANDLE(session);

	if(session->selected.begin < 0)
		session->selected.begin = session->selected.end = session->cursor_addr;

	if(baddr > session->cursor_addr)
	{
		session->selected.begin	= session->cursor_addr;
		session->selected.end	= baddr;
	}
	else
	{
		session->selected.begin = baddr;
		session->selected.end 	= session->cursor_addr;
	}

	// Update screen contents
	for(baddr = 0; baddr < session->rows*session->cols; baddr++)
	{
		unsigned short attr = ea_buf[baddr].attr;

		if(baddr >= session->selected.begin && baddr <= session->selected.end)
			attr |= LIB3270_ATTR_SELECTED;
		else
			attr &= ~LIB3270_ATTR_SELECTED;

		if(attr != ea_buf[baddr].attr && session->update)
		{
			ea_buf[baddr].attr = attr;
			session->update(session,baddr,ea_buf[baddr].chr,attr,baddr == session->cursor_addr);
		}
	}
}

