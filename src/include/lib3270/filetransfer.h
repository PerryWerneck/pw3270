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
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como filetransfer.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas de Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */

#ifndef LIB3270_FILETRANSFER_INCLUDED

	#define LIB3270_FILETRANSFER_INCLUDED 1
	#include <stdio.h>

	typedef enum _lib3270_FT_FLAG
	{
		LIB3270_FT_OPTION_RECEIVE		= 0x0001,
		LIB3270_FT_OPTION_ASCII			= 0x0002,
		LIB3270_FT_OPTION_CRLF			= 0x0004,
		LIB3270_FT_OPTION_APPEND		= 0x0008,
		LIB3270_FT_OPTION_TSO			= 0x0010,
		LIB3270_FT_OPTION_REMAP_ASCII	= 0x0020
	} LIB3270_FT_OPTION;

	typedef struct _h3270ft
	{
		unsigned short	  sz;					/**< Size of FT data structure */
		H3270			* host;
		void			* widget;				/**< File transfer dialog handle */
		FILE 			* ft_local_file;		/**< File descriptor for local file */

	} H3270FT;

	/**
	 * Start a new file transfer session.
	 *
	 * @param session
	 * @param flags
	 * @param local
	 * @param remote
	 * @param lrecl
	 * @param blksize
	 * @param primspace
	 * @param secspace
	 * @param dft
	 * @param msg			Pointer to error message.
	 *
	 * @return Filetransfer handle if ok, NULL if failed
	 *
	 */
	LIB3270_EXPORT H3270FT * lib3270_ft_start(H3270 *session, LIB3270_FT_OPTION flags, const char *local, const char *remote, int lrecl, int blksize, int primspace, int secspace, int dft, const char **msg);

	LIB3270_EXPORT int lib3270_ft_cancel(H3270FT *ft, int force);

#endif // LIB3270_FILETRANSFER_INCLUDED
