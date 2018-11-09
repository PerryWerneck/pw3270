/*
 * "Software G3270, desenvolvido com base nos códigos fontes do WC3270  e  X3270
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
 * Este programa está nomeado como html.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 *
 */

#ifndef LIB3270_HTML_H_INCLUDED

	#define LIB3270_HTML_H_INCLUDED 1

#ifdef __cplusplus
	extern "C" {
#endif

	typedef enum _lib3270_html_option
	{
		LIB3270_HTML_OPTION_ALL			= 0x0001,
		LIB3270_HTML_OPTION_HEADERS		= 0x0002,
		LIB3270_HTML_OPTION_FORM		= 0x0004,

	} LIB3270_HTML_OPTION;


	LIB3270_EXPORT char * lib3270_get_as_html(H3270 *session, LIB3270_HTML_OPTION option);

#ifdef __cplusplus
	}
#endif

#endif // LIB3270_HTML_H_INCLUDED

