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

 #include "../private.h"
 #include <pw3270.h>
 #include <pw3270/application.h>
 #include <pw3270/settings.h>

 typedef struct _Pw3270SettingsDialog {
 	GApplication	* application;
 	GSimpleAction	* action;
 	GtkDialog		* dialog;
 	GtkNotebook		* notebook;
 	GSList			* pages;
 } Pw3270SettingsDialog;

 static void on_destroy(GtkWidget G_GNUC_UNUSED(*dialog), Pw3270SettingsDialog *settings) {
 	settings->dialog = NULL;
	g_slist_free_full(settings->pages,g_free);
	g_free(settings);
	g_simple_action_set_enabled(settings->action,TRUE);
 }

 void on_response(GtkDialog *dialog, gint response_id, Pw3270SettingsDialog * settings) {

	if(response_id== GTK_RESPONSE_APPLY) {

		g_message("Aplying application settings");

		GSList * page;
		for(page = settings->pages;page;page = page->next) {
			Pw3270SettingsPage * widget = (Pw3270SettingsPage *) page->data;
			if(widget->apply) {
				widget->apply(widget,GTK_APPLICATION(settings->application));
			}
		}

	}

	gtk_widget_destroy(GTK_WIDGET(dialog));
 }

 static void on_page_added(GtkNotebook G_GNUC_UNUSED(*notebook), GtkWidget *widget, guint G_GNUC_UNUSED(page_num), Pw3270SettingsDialog G_GNUC_UNUSED(*settings)) {

	// https://developer.gnome.org/hig/stable/visual-layout.html.en

	if(GTK_IS_GRID(widget)) {
		gtk_grid_set_row_spacing(GTK_GRID(widget),6);
		gtk_grid_set_column_spacing(GTK_GRID(widget),12);
	}

	if(GTK_IS_CONTAINER(widget)) {
		gtk_container_set_border_width(GTK_CONTAINER(widget),18);
	}

 }

 static void on_switch_page(GtkNotebook G_GNUC_UNUSED(*notebook), GtkWidget *widget, guint G_GNUC_UNUSED(page_num), Pw3270SettingsDialog * settings)
 {

 	debug("%s: %p",__FUNCTION__,settings->dialog);

	if(!settings->dialog)
		return;

	GtkWidget * header_bar = gtk_dialog_get_header_bar(settings->dialog);

	if(header_bar) {
		GSList * page;
		for(page = settings->pages;page;page = page->next) {
			Pw3270SettingsPage * pg = (Pw3270SettingsPage *) page->data;
			if(pg->widget == widget) {
				if(pg->title)
					gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header_bar),pg->title);
				return;
			}
		}
	}

 }

 void pw3270_application_settings_dialog_add_page(Pw3270SettingsDialog *settings, Pw3270SettingsPage *page) {

	settings->pages = g_slist_prepend(settings->pages,page);

	GtkWidget * label = NULL;
	if(page->label)
		label = gtk_label_new(page->label);

	gtk_notebook_append_page(
		settings->notebook,
		page->widget,
		label
	);

 }

 void pw3270_application_preferences_activated(GSimpleAction *action, GVariant G_GNUC_UNUSED(*parameter), gpointer application) {

	size_t ix;

	debug("%s",__FUNCTION__);

	// Create dialog.
	GtkWidget * dialog = pw3270_settings_dialog_new(
								_("Application preferences"),
								gtk_application_get_active_window(GTK_APPLICATION(application))
							);

	// Create setttings data.
	Pw3270SettingsDialog * settings = g_new0(Pw3270SettingsDialog,1);
	settings->action = action;
	settings->dialog = GTK_DIALOG(dialog);
	settings->application = G_APPLICATION(application);

	g_simple_action_set_enabled(action,FALSE);

	// Create settings notebook.

	settings->notebook = GTK_NOTEBOOK(gtk_notebook_new());

	gtk_notebook_set_scrollable(settings->notebook,TRUE);
	gtk_notebook_set_show_tabs(settings->notebook,TRUE);
	gtk_notebook_set_show_border(settings->notebook, FALSE);

	g_signal_connect(G_OBJECT(settings->notebook), "page-added", G_CALLBACK(on_page_added), settings);
//	g_signal_connect(G_OBJECT(settings->notebook), "page-removed", G_CALLBACK(on_page_changed), settings);
	g_signal_connect(G_OBJECT(settings->notebook), "switch-page", G_CALLBACK(on_switch_page), settings);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),GTK_WIDGET(settings->notebook),TRUE,TRUE,0);

	// Connection signals.
	g_signal_connect(dialog,"destroy",G_CALLBACK(on_destroy),settings);
	g_signal_connect(dialog,"response",G_CALLBACK(on_response),settings);

	// Load pages.
	Pw3270SettingsPage * pages[] = {
		pw3270_toolbar_settings_new()
	};

	for(ix = 0; ix < G_N_ELEMENTS(pages); ix++) {
		pw3270_application_settings_dialog_add_page(settings,pages[ix]);
	}

	// Load page contents.
	GSList * page;
	for(page = settings->pages;page;page = page->next) {
		Pw3270SettingsPage * widget = (Pw3270SettingsPage *) page->data;
		if(widget->load) {
			widget->load(widget,GTK_APPLICATION(settings->application));
		}
	}

	// Show dialog.
	gtk_widget_show_all(dialog);

 }

