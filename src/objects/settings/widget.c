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

#ifdef _WIN32
	#include <winsock2.h>
	#include <windows.h>
#endif // _WIN32

 #include <pw3270.h>
 #include <pw3270/settings.h>
 #include <lib3270.h>
 #include <lib3270/log.h>

 G_DEFINE_TYPE(PW3270Settings, PW3270Settings, GTK_TYPE_GRID);

 static void load(GtkWidget *widget, PW3270SettingsPrivate *settings);
 static void apply(GtkWidget *widget, PW3270SettingsPrivate *settings);
 static void revert(GtkWidget *widget, PW3270SettingsPrivate *settings);
 static void finalize(GObject *object);

/*--[ Implement ]------------------------------------------------------------------------------------*/

 PW3270Settings * pw3270_settings_new() {
	return GTK_PW3270_SETTINGS(g_object_new(GTK_TYPE_PW3270_SETTINGS,NULL));
 }

 static void PW3270Settings_class_init(PW3270SettingsClass *klass) {

	G_OBJECT_CLASS(klass)->finalize = finalize;
 }

 static void PW3270Settings_init(PW3270Settings *widget) {

	// https://developer.gnome.org/hig/stable/visual-layout.html.en
 	gtk_grid_set_row_spacing(GTK_GRID(widget),6);
 	gtk_grid_set_column_spacing(GTK_GRID(widget),12);

 	widget->settings = NULL;	// Just in case.
	widget->load = load;
	widget->apply = apply;
	widget->revert = revert;

 }

 static void finalize(GObject *object) {

	PW3270Settings * settings = GTK_PW3270_SETTINGS(object);
	if(settings->settings) {
		g_free(settings->settings);
		settings->settings = NULL;
	}

}

 void load(GtkWidget G_GNUC_UNUSED(*widget), PW3270SettingsPrivate G_GNUC_UNUSED(*settings)) {
 }

 void apply(GtkWidget G_GNUC_UNUSED(*widget), PW3270SettingsPrivate G_GNUC_UNUSED(*settings)) {
 }

 void revert(GtkWidget G_GNUC_UNUSED(*widget), PW3270SettingsPrivate G_GNUC_UNUSED(*settings)) {
 }

