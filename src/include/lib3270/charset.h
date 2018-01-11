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
 * Este programa está nomeado como charset.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

#ifndef LIB3270_CHARSET_H_INCLUDED

	#define LIB3270_CHARSET_H_INCLUDED 1

#ifdef __cplusplus
	extern "C" {
#endif

	struct lib3270_charset
	{
		char			* host;
		char			* display;
		unsigned long	  cgcsgid;

		// Translation tables
		unsigned short		  ebc2asc[256];
		unsigned short 		  asc2ebc[256];

		unsigned short		  asc2uc[256];

	};

	typedef enum
	{
		CS_ONLY,
		FT_ONLY,
		BOTH
	} lib3270_remap_scope;

	LIB3270_EXPORT int			  lib3270_set_host_charset(H3270 *hSession, const char *name);
	LIB3270_EXPORT const char	* lib3270_get_host_charset(H3270 *hSession);
	LIB3270_EXPORT void 		  lib3270_reset_charset(H3270 *hSession, const char * host, const char * display, unsigned long cgcsgid);

	LIB3270_EXPORT void			  lib3270_remap_char(H3270 *hSession, unsigned short ebc, unsigned short iso, lib3270_remap_scope scope, unsigned char one_way);
	LIB3270_EXPORT const char	* lib3270_ebc2asc(H3270 *hSession, unsigned char *buffer, int sz);
	LIB3270_EXPORT const char	* lib3270_asc2ebc(H3270 *hSession, unsigned char *buffer, int sz);


#ifdef __cplusplus
	}
#endif

#endif // LIB3270_CHARSET_H_INCLUDED
