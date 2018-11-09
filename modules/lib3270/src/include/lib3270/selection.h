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
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
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

	LIB3270_EXPORT int	  lib3270_unselect(H3270 *session);
	LIB3270_EXPORT void	  lib3270_select_to(H3270 *session, int baddr);
	LIB3270_EXPORT int	  lib3270_select_word_at(H3270 *session, int baddr);
	LIB3270_EXPORT int	  lib3270_select_field_at(H3270 *session, int baddr);
	LIB3270_EXPORT int	  lib3270_select_field(H3270 *session);
	LIB3270_EXPORT int	  lib3270_select_all(H3270 *session);

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
	 * Drag selected region.
	 *
	 * Move or resize selected box according to the selection flags.
	 *
	 * @param h			Session handle.
	 * @param flag		Selection flag.
	 * @param origin	Reference position (got from mouse button down or other move action).
	 * @param baddr		New position.
	 *
	 * @return The new reference position.
	 *
	 */
	 LIB3270_EXPORT int lib3270_drag_selection(H3270 *h, unsigned char flag, int origin, int baddr);

	/**
	 * Gets the selected range of characters in the screen
	 *
	 * @param h		Session handle.
	 * @param start	return location for start of selection, as a character offset.
	 * @param end	return location for end of selection, as a character offset.
	 *
	 * @return Non 0 if selection is non-empty
	 *
	 */
	 LIB3270_EXPORT int lib3270_get_selection_bounds(H3270 *hSession, int *start, int *end);

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

	/**
	 * Get a string from required region.
	 *
	 * @param h			Session handle.
	 * @param start_pos	First char to get.
	 * @param end_pos	Last char to get.
	 * @param all		zero to get only selected chars.
	 *
	 * @return String with selected region (release it with free()
	 *
	 */
	 LIB3270_EXPORT char * lib3270_get_region(H3270 *h, int start_pos, int end_pos, unsigned char all);


	/**
	 * Selects a range of characters in the screen.
	 *
	 * @param h				Session handle.
	 * @param start_offset	Start offset.
	 * @param end_offset :	End offset.
	 *
	 */
	 LIB3270_EXPORT int lib3270_select_region(H3270 *h, int start, int end);

 #endif // LIB3270_SELECTION_H_INCLUDED
