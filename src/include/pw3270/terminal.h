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
 * Este programa está nomeado como - e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

/**
 * @brief Declares the pw3270 terminal window.
 *
 */

#ifndef PW3270_TERMINAL_H_INCLUDED

	#define PW3270_TERMINAL_H_INCLUDED

	#include <gtk/gtk.h>
	#include <v3270.h>

	G_BEGIN_DECLS

	#define PW3270_TYPE_TERMINAL						(pw3270Terminal_get_type ())
	#define PW3270_TERMINAL(inst)						(G_TYPE_CHECK_INSTANCE_CAST ((inst), \
														PW3270_TYPE_TERMINAL, pw3270Terminal))
	#define PW3270_TERMINAL_CLASS(class)				(G_TYPE_CHECK_CLASS_CAST ((class),   \
														PW3270_TYPE_TERMINAL, pw3270TerminalClass))
	#define PW3270_IS_TERMINAL(inst)					(G_TYPE_CHECK_INSTANCE_TYPE ((inst), \
														PW3270_TYPE_TERMINAL))
	#define PW3270_IS_TERMINAL_CLASS(class)				(G_TYPE_CHECK_CLASS_TYPE ((class),   \
														PW3270_TYPE_TERMINAL))
	#define PW3270_TERMINAL_GET_CLASS(inst)				(G_TYPE_INSTANCE_GET_CLASS ((inst),  \
														GTK_TYPE_APPLICATION_WINDOW, pw3270TerminalClass))


	typedef struct _pw3270TerminalClass   pw3270TerminalClass;
	typedef struct _pw3270Terminal        pw3270Terminal;

	GType		  pw3270Terminal_get_type();
	GtkWidget	* pw3270_terminal_new();

	G_END_DECLS


#endif // PW3270_TERMINAL_H_INCLUDED
