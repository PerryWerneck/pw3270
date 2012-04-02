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
 * Este programa está nomeado como selection.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

 #ifndef LIB3270_SELECTION_H_INCLUDED

	#define LIB3270_SELECTION_H_INCLUDED 1

	LIB3270_EXPORT void	  lib3270_clear_selection(H3270 *session);
	LIB3270_EXPORT void	  lib3270_select_to(H3270 *session, int baddr);
	LIB3270_EXPORT void	  lib3270_select_word(H3270 *session, int baddr);
	LIB3270_EXPORT int	  lib3270_select_field_at(H3270 *session, int baddr);

	/**
	 * "Paste" supplied string.
	 *
	 * @param h		Session handle.
	 * @param str	String to paste.
	 *
	 * @see lib3270_pastenext.
	 *
	 * @return Non 0 if there's more to paste with lib3270_pastenext
	 *
	 */
	 LIB3270_EXPORT int lib3270_paste(H3270 *h, const unsigned char *str);

	/**
	 * Paste remaining string.
	 *
	 * @param h	Session handle.
	 *
	 * @see lib3270_paste.
	 *
	 * @return Non 0 if there's more to paste.
	 *
	 */
	 LIB3270_EXPORT int lib3270_pastenext(H3270 *h);

	/**
	 * Move selected box 1 char in the selected direction.
	 *
	 * @param h		Session handle.
	 * @param dir	Direction to move
	 *
	 * @return 0 if the movement can be done, non zero if failed.
	 */
	 LIB3270_EXPORT int lib3270_move_selection(H3270 *h, LIB3270_DIRECTION dir);

	/**
	 * Move selected box.
	 *
	 * @param h		Session handle.
	 * @param from	Address of origin position inside the selected buffer.
	 * @param to	Address of the new origin position.
	 *
	 * @return The new origin position.
	 *
	 */
	 LIB3270_EXPORT int lib3270_move_selected_area(H3270 *h, int from, int to);

	/**
	 * Get addresses of selected area.
	 *
	 * @param h		Session handle.
	 * @param begin	Pointer to the begin value.
	 * @param end	Pointer to the end value.
	 *
	 * @return 0 if the selected area was get, non zero if unselected or unavailable.
	 *
	 */
	 LIB3270_EXPORT int lib3270_get_selected_addr(H3270 *hSession, int *begin, int *end);

	/**
	 * Get bitmasked flag for the current selection.
	 *
	 * Calculate flags to help drawing of the correct mouse pointer over a selection.
	 *
	 * @param h		Session handle.
	 * @param baddr	Position.
	 *
	 * @return bitmask for mouse pointer.
	 */
	 LIB3270_EXPORT unsigned char lib3270_get_selection_flags(H3270 *h, int baddr);

 #endif // LIB3270_SELECTION_H_INCLUDED
