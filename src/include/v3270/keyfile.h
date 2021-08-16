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
 * @brief Declares the v3270 keyfile object.
 *
 */

#ifndef V3270_KEYFILE_H_INCLUDED

#define V3270_KEYFILE_H_INCLUDED

#include <glib.h>

G_BEGIN_DECLS

typedef struct _V3270KeyFile V3270KeyFile;

void			  v3270_set_default_session(GtkWidget *terminal);
gchar			* v3270_keyfile_get_default_filename(void);
gchar			* v3270_key_file_get_default_path(GtkWidget *terminal);

V3270KeyFile	* v3270_key_file_open(GtkWidget *terminal, const gchar *name, GError **error);
void			  v3270_key_file_close(GtkWidget *terminal);

void			  v3270_key_file_save(GtkWidget *terminal, GError **error);
void			  v3270_key_file_save_to_file(GtkWidget * terminal, const gchar *filename, GError **error);

/// @brief Get current key filename
const gchar		* v3270_key_file_get_filename(GtkWidget *terminal);

/// @brief Build a writable key filename
gchar			* v3270_key_file_build_filename(GtkWidget *terminal);

GKeyFile		* v3270_key_file_get(GtkWidget *terminal);

gboolean		  v3270_key_file_can_write(GtkWidget *widget);

void			  v3270_key_file_set_boolean(GtkWidget *terminal, const gchar *group_name, const gchar *key, gboolean value);

G_END_DECLS

#endif // PW3270_H_INCLUDED
