/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob o nome G3270.
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
 * Este programa está nomeado como plugin.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

#ifndef PW3270_PLUGIN_INCLUDED

	#define PW3270_PLUGIN_INCLUDED 1

	#include <gtk/gtk.h>
	#include <lib3270.h>

#ifdef __cplusplus
	extern "C" {
#endif

	LIB3270_EXPORT int pw3270_plugin_init(void);
	LIB3270_EXPORT int pw3270_plugin_deinit(void);

	LIB3270_EXPORT int pw3270_plugin_start(GtkWidget *window);
	LIB3270_EXPORT int pw3270_plugin_stop(GtkWidget *window);

	// plugins
	LIB3270_EXPORT void pw3270_load_plugins(const gchar *path);
	LIB3270_EXPORT void pw3270_unload_plugins(void);

	LIB3270_EXPORT void pw3270_start_plugins(GtkWidget *widget);
	LIB3270_EXPORT void pw3270_stop_plugins(GtkWidget *widget);

	LIB3270_EXPORT int  pw3270_setup_plugin_action(GtkAction *action, GtkWidget *widget, const gchar *name);

#ifdef __cplusplus
	}
#endif

#endif // PW3270_PLUGIN_INCLUDED

