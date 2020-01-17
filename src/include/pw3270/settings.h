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

#ifndef PW3270SETTINGS_H_INCLUDED

 #define PW3270SETTINGS_H_INCLUDED 1

#ifdef _WIN32
	#include <windows.h>
#endif // _WIN32

 #include <gtk/gtk.h>

 G_BEGIN_DECLS

/*--[ PW3270 Settings Widget ]-----------------------------------------------------------------------*/

 #define GTK_TYPE_PW3270_SETTINGS					(PW3270Settings_get_type())
 #define GTK_PW3270_SETTINGS(obj)					(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_PW3270_SETTINGS, PW3270Settings))
 #define GTK_PW3270_SETTINGS_CLASS(klass)			(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_PW3270_SETTINGS, PW3270SettingsClass))
 #define GTK_IS_PW3270_SETTINGS(obj)				(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_PW3270_SETTINGS))
 #define GTK_IS_PW3270_SETTINGS_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_PW3270_SETTINGS))
 #define GTK_PW3270_SETTINGS_GET_CLASS(obj)			(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_PW3270_SETTINGS, PW3270SettingsClass))

 GType PW3270Settings_get_type(void);

 typedef struct _PW3270SettingsPrivate PW3270SettingsPrivate;

 typedef struct _PW3270Settings {

	GtkGrid parent;

	PW3270SettingsPrivate	* settings;											///< @brief Private Data.
	const gchar				* label;											///< @brief Page lagel.
	const gchar				* title;											///< @brief Page title.

 	void (*load)(GtkWidget *widget, PW3270SettingsPrivate *settings);			///< @brief Method to load the properties from terminal widget
 	void (*apply)(GtkWidget *widget, PW3270SettingsPrivate *settings);			///< @brief Method for GTK_RESPONSE_APPLY
 	void (*revert)(GtkWidget *widget, PW3270SettingsPrivate *settings);			///< @brief Method for GTK_RESPONSE_CANCEL

 } PW3270Settings;

 typedef struct _PW3270SettingsClass	{

 	GtkGridClass parent_class;

 } PW3270SettingsClass;

 PW3270Settings * pw3270_settings_new();

/*--[ PW3270 Settings Dialog ]-----------------------------------------------------------------------*/

 #define GTK_TYPE_PW3270_SETTINGS_DIALOG				(PW3270SettingsDialog_get_type())
 #define GTK_PW3270_SETTINGS_DIALOG(obj)	   			(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_PW3270_SETTINGS_DIALOG, PW3270SettingsDialog))
 #define GTK_PW3270_SETTINGS_DIALOG_CLASS(klass)	    (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_PW3270_SETTINGS_DIALOG, PW3270SettingsDialogClass))
 #define GTK_IS_PW3270_SETTINGS_DIALOG(obj)				(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_PW3270_SETTINGS_DIALOG))
 #define GTK_IS_PW3270_SETTINGS_DIALOG_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_PW3270_SETTINGS_DIALOG))
 #define GTK_PW3270_SETTINGS_DIALOG_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_PW3270_SETTINGS_DIALOG, PW3270SettingsDialogClass))

 typedef struct _PW3270SettingsDialog		PW3270SettingsDialog;
 typedef struct _PW3270SettingsDialogClass	PW3270SettingsDialogClass;

 GType		  PW3270SettingsDialog_get_type(void);
 GtkWidget	* pw3270_settings_dialog_new(GAction *action);

 G_END_DECLS

#endif // V3270SETTINGS_H_INCLUDED
