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
 #include "appres.h"
 #include <lib3270.h>
 #include <lib3270/session.h>
 #include <lib3270/selection.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void update_selected_rectangle(H3270 *session)
{
	struct
	{
		int row;
		int col;
	} p[2];

	int begin	= session->selected.begin;
	int end		= session->selected.end;
	int row, col, baddr;

	if(begin > end)
	{
		baddr 	= end;
		end		= begin;
		begin	= baddr;
	}

	// Get start & end posision
	p[0].row = (begin/session->cols);
	p[0].col = (begin%session->cols);
	p[1].row = (end/session->cols);
	p[1].col = (end%session->cols);

	// First remove unselected areas
	baddr = 0;
	for(row=0;row < session->rows;row++)
	{
		for(col = 0; col < session->cols;col++)
		{
			if(!(row >= p[0].row && row <= p[1].row && col >= p[0].col && col <= p[1].col) && (ea_buf[baddr].attr & LIB3270_ATTR_SELECTED))
			{
				ea_buf[baddr].attr &= ~LIB3270_ATTR_SELECTED;
				session->update(session,baddr,ea_buf[baddr].chr,ea_buf[baddr].attr,baddr == session->cursor_addr);
			}
			baddr++;
		}
	}

	// Then, draw selected ones
	baddr = 0;
	for(row=0;row < session->rows;row++)
	{
		for(col = 0; col < session->cols;col++)
		{
			if((row >= p[0].row && row <= p[1].row && col >= p[0].col && col <= p[1].col) && !(ea_buf[baddr].attr & LIB3270_ATTR_SELECTED))
			{
				ea_buf[baddr].attr |= LIB3270_ATTR_SELECTED;
				session->update(session,baddr,ea_buf[baddr].chr,ea_buf[baddr].attr,baddr == session->cursor_addr);
			}
			baddr++;
		}
	}

}

static void update_selected_region(H3270 *session)
{
	int baddr;
	int begin	= session->selected.begin;
	int end		= session->selected.end;
	int len 	= session->rows*session->cols;

	if(begin > end)
	{
		baddr 	= end;
		end		= begin;
		begin	= baddr;
	}

	// First remove unselected areas
	for(baddr = 0; baddr < begin; baddr++)
	{
		if(ea_buf[baddr].attr & LIB3270_ATTR_SELECTED)
		{
			ea_buf[baddr].attr &= ~LIB3270_ATTR_SELECTED;
			session->update(session,baddr,ea_buf[baddr].chr,ea_buf[baddr].attr,baddr == session->cursor_addr);
		}
	}

	for(baddr = end+1; baddr < len; baddr++)
	{
		if(ea_buf[baddr].attr & LIB3270_ATTR_SELECTED)
		{
			ea_buf[baddr].attr &= ~LIB3270_ATTR_SELECTED;
			session->update(session,baddr,ea_buf[baddr].chr,ea_buf[baddr].attr,baddr == session->cursor_addr);
		}
	}

	// Then draw the selected ones
	for(baddr = begin; baddr <= end; baddr++)
	{
		if(!(ea_buf[baddr].attr & LIB3270_ATTR_SELECTED))
		{
			ea_buf[baddr].attr |= LIB3270_ATTR_SELECTED;
			session->update(session,baddr,ea_buf[baddr].chr,ea_buf[baddr].attr,baddr == session->cursor_addr);
		}
	}

}

void update_selection(H3270 *session)
{
	if(lib3270_get_toggle(session,LIB3270_TOGGLE_RECTANGLE_SELECT))
		update_selected_rectangle(session);
	else
		update_selected_region(session);
}

void toggle_rectselect(H3270 *session, struct toggle *t, LIB3270_TOGGLE_TYPE tt)
{
	if(session->selected.begin < 0)
		return;

	if(t->value)
		update_selected_rectangle(session);
	else
		update_selected_region(session);
}

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

	session->set_selection(session,0);

}


LIB3270_EXPORT void lib3270_select_to(H3270 *session, int baddr)
{
	CHECK_SESSION_HANDLE(session);

	lib3270_set_cursor_address(session,session->selected.end = baddr);

	if(session->selected.begin < 0)
	{
		session->selected.begin = session->cursor_addr;
		session->set_selection(session,1);
	}

	update_selection(session);

}

