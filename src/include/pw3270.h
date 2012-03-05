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
 * Este programa está nomeado como widget.h e possui - linhas de código.
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

 #ifndef PW3270_H_INCLUDED


	#define PW3270_H_INCLUDED 1

	#ifdef _WIN32
		#include <windows.h>
	#endif

	#include <gtk/gtk.h>

	// Trace
	#include <stdio.h>

	#if defined( DEBUG )
		#define trace(x, ...)          fprintf(stderr,"%s(%d):\t" x "\n",__FILE__,__LINE__, __VA_ARGS__); fflush(stderr);
	#else
		#define trace(x, ...) /* */
	#endif

	// Error management
	#include <errno.h>

	#ifndef ETIMEDOUT
		#define ETIMEDOUT -1238
	#endif

	#ifndef ECANCELED
		#ifdef EINTR
			#define ECANCELED EINTR
		#else
			#define ECANCELED -1125
		#endif
	#endif

	#ifndef ENOTCONN
		#define ENOTCONN -1107
	#endif

	// Windows
	#ifdef _WIN32

		#ifndef WINVER
			#define WINVER 0x0501
		#endif

		#ifndef _WIN32_WINNT
			#define _WIN32_WINNT WINVER
		#endif

	#endif





#endif // PW3270_H_INCLUDED
