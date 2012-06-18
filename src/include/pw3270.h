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
	#include <lib3270.h>

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

	// pw3270 window
	G_BEGIN_DECLS

	#define GTK_TYPE_PW3270				(pw3270_get_type ())
	#define GTK_PW3270(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_PW3270, pw3270))
	#define GTK_PW3270_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_PW3270, pw3270Class))
	#define GTK_IS_PW3270(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_PW3270))
	#define GTK_IS_PW3270_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_PW3270))
	#define GTK_PW3270_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_PW3270, pw3270Class))

	typedef struct _pw3270			pw3270;
	typedef struct _pw3270Class		pw3270Class;


	GtkWidget	* pw3270_new(const gchar *host);
	void		  pw3270_set_host(GtkWidget *widget, const gchar *uri);
	gboolean 	  pw3270_get_toggle(GtkWidget *widget, LIB3270_TOGGLE ix);

	G_END_DECLS

#endif // PW3270_H_INCLUDED
