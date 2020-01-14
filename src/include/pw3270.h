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
 * @brief Declares the pw3270 Action objects.
 *
 */

#ifndef PW3270_H_INCLUDED

	#define PW3270_H_INCLUDED

	#include <config.h>

	#ifndef GETTEXT_PACKAGE
		#define GETTEXT_PACKAGE PACKAGE_NAME
	#endif

	#include <libintl.h>
	#include <glib/gi18n.h>
	#include <gtk/gtk.h>

	G_BEGIN_DECLS

	/* not really I18N-related, but also a string marker macro */
	#define I_(string) g_intern_static_string (string)


	void pw3270_load_placeholders(GtkBuilder * builder);

	// Application settings widget
	typedef struct _Pw3270SettingsPage Pw3270SettingsPage;

	struct _Pw3270SettingsPage {
		GtkWidget	* widget;		///< @brief Settings widget.
		const gchar	* label;		///< @brief Page lagel.
		const gchar * title;		///< @brief Page title.
		void (*load)(Pw3270SettingsPage *, GtkApplication *);
		void (*apply)(Pw3270SettingsPage *, GtkApplication *);
	};

	void gtk_file_chooser_set_pw3270_filters(GtkFileChooser *chooser);

	const gchar	* v3270_get_session_filename(GtkWidget *widget);
	void		  v3270_set_session_filename(GtkWidget *widget, const gchar *filename);

	/// @brief Check if the terminal has a customized session file.
	gboolean	  v3270_allow_custom_settings(GtkWidget *widget);

	GtkWidget	* pw3270_settings_dialog_new(const gchar *title, GtkWindow *parent);

	G_END_DECLS

#endif // PW3270_H_INCLUDED
