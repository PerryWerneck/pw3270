/*
 * "Software PW3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
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
 * Este programa está nomeado como actions.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas de Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

	/*
	 * Action call table.
	 *
	 * Usually this definitions are used to declare lib3270's action table but,
	 * if you redefine the creation macros it can be used to build a callback
	 * table for g_object_connect calls.
	 *
	 */

	/* Keyboard actions */
	DECLARE_LIB3270_KEY_ACTION( enter )

	DECLARE_LIB3270_FKEY_ACTION( pfkey )
	DECLARE_LIB3270_FKEY_ACTION( pakey )

	/* Cursor movement */
	DECLARE_LIB3270_CURSOR_ACTION( up )
	DECLARE_LIB3270_CURSOR_ACTION( down )
	DECLARE_LIB3270_CURSOR_ACTION( left )
	DECLARE_LIB3270_CURSOR_ACTION( right )
	DECLARE_LIB3270_CURSOR_ACTION( newline )

	/* Misc actions */
	DECLARE_LIB3270_ACTION( kybdreset )
	DECLARE_LIB3270_ACTION( clear )
	DECLARE_LIB3270_ACTION( eraseinput )

	DECLARE_LIB3270_ACTION( select_field )
	DECLARE_LIB3270_ACTION( select_all )
	DECLARE_LIB3270_ACTION( unselect )
	DECLARE_LIB3270_ACTION( reselect )

	DECLARE_LIB3270_ACTION( firstfield )
	DECLARE_LIB3270_ACTION( eraseeof )
	DECLARE_LIB3270_ACTION( eraseeol )
	DECLARE_LIB3270_ACTION( erase )
	DECLARE_LIB3270_ACTION( delete )
	DECLARE_LIB3270_ACTION( dup )
	DECLARE_LIB3270_ACTION( fieldmark )

	DECLARE_LIB3270_ACTION( backspace )
	DECLARE_LIB3270_ACTION( previousword )
	DECLARE_LIB3270_ACTION( nextword )
	DECLARE_LIB3270_ACTION( fieldend )

	DECLARE_LIB3270_ACTION( nextfield )
	DECLARE_LIB3270_ACTION( previousfield )

	DECLARE_LIB3270_ACTION( attn )
	DECLARE_LIB3270_ACTION( break )
	DECLARE_LIB3270_ACTION( pastenext )

	DECLARE_LIB3270_ACTION( deleteword )
	DECLARE_LIB3270_ACTION( deletefield )
	DECLARE_LIB3270_ACTION( sysreq )

	DECLARE_LIB3270_ACTION( testpattern )
	DECLARE_LIB3270_ACTION( charsettable )


