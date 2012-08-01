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
 * Este programa está nomeado como screenc.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

#ifndef SCREENC_H_INCLUDED

#define SCREENC_H_INCLUDED 1
/* c3270 version of screenc.h */

// #define blink_start()
#define display_heightMM()	100
#define display_height()	1
#define display_widthMM()	100
#define display_width()		1
// #define screen_obscured()	False
// #define screen_scroll()		screen_disp()
// #define screen_132()	/* */
// #define screen_80()		/* */

LIB3270_INTERNAL int		screen_init(H3270 *session);
// LIB3270_INTERNAL Boolean	screen_new_display_charsets(char *cslist, char *csname);
LIB3270_INTERNAL void		mcursor_set(H3270 *session,LIB3270_CURSOR m);

#define mcursor_locked(x) mcursor_set(x,CURSOR_MODE_LOCKED)
#define mcursor_normal(x) mcursor_set(x,CURSOR_MODE_NORMAL)
#define mcursor_waiting(x) mcursor_set(x,CURSOR_MODE_WAITING)

LIB3270_INTERNAL void notify_toggle_changed(H3270 *session, LIB3270_TOGGLE ix, unsigned char value, LIB3270_TOGGLE_TYPE reason);
LIB3270_INTERNAL void set_viewsize(H3270 *session, int rows, int cols);

// LIB3270_INTERNAL Boolean escaped;


// LIB3270_INTERNAL void screen_title(char *text);

#endif // SCREENC_H_INCLUDED
