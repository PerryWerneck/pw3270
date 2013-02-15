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
 * Este programa está nomeado como common.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#ifndef COMMON_H_INCLUDED

 #define COMMON_H_INCLUDED 1

 // "PW" Standards
 #include <gtk/gtk.h>
 #include <errno.h>
 #include <lib3270/config.h>

 #ifdef WIN32

	#include <windows.h>

	#ifndef KEY_WOW64_64KEY
		#define KEY_WOW64_64KEY 0x0100
	#endif // KEY_WOW64_64KEY

	#ifndef KEY_WOW64_32KEY
		#define KEY_WOW64_32KEY	0x0200
	#endif // KEY_WOW64_64KEY

 #endif // WIN32

 #define ENABLE_NLS

 #ifndef GETTEXT_PACKAGE
	#define GETTEXT_PACKAGE PACKAGE_NAME
 #endif

 #include <libintl.h>
 #include <glib/gi18n.h>
 #include <gtk/gtk.h>

 #if defined( DEBUG )
	#define trace(x, ...)	fprintf(stderr,"%s(%d):\t" x "\n",__FILE__,__LINE__, __VA_ARGS__); fflush(stderr);
 #else
	#define trace(x, ...)	/* */
 #endif

 // Configuration
 void		  configuration_init(void);
 void		  configuration_deinit(void);

 gchar		* get_string_from_config(const gchar *group, const gchar *key, const gchar *def);
 gboolean	  get_boolean_from_config(const gchar *group, const gchar *key, gboolean def);
 gint 		  get_integer_from_config(const gchar *group, const gchar *key, gint def);

 void		  set_string_to_config(const gchar *group, const gchar *key, const gchar *fmt, ...);
 void		  set_boolean_to_config(const gchar *group, const gchar *key, gboolean val);
 void		  set_integer_to_config(const gchar *group, const gchar *key, gint val);

 gchar 		* build_data_filename(const gchar *first_element, ...);
 gchar		* filename_from_va(const gchar *first_element, va_list args);


 void 		  save_window_state_to_config(const gchar *group, const gchar *key, GdkWindowState CurrentState);
 void		  save_window_size_to_config(const gchar *group, const gchar *key, GtkWidget *hwnd);

 void		  restore_window_from_config(const gchar *group, const gchar *key, GtkWidget *hwnd);

#ifdef HAVE_WIN_REGISTRY
 gboolean	  get_registry_handle(const gchar *group, HKEY *hKey, REGSAM samDesired);
 void		  registry_foreach(HKEY parent, const gchar *name,void (*cbk)(const gchar *key, const gchar *val, gpointer *user_data), gpointer *user_data);
 void 		  registry_set_double(HKEY hKey, const gchar *key, gdouble value);
 gboolean	  registry_get_double(HKEY hKey, const gchar *key, gdouble *value);
#else
 GKeyFile	* get_application_keyfile(void);
#endif // HAVE_WIN_REGISTRY


#endif
