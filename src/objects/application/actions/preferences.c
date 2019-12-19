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

 typedef struct _Pw3270SettingsDialog {
 	GtkApplication	* application;
 	GSimpleAction	* action;
 	GSList			* pages;
 } Pw3270SettingsDialog;

 static void on_destroy(GtkWidget *dialog, Pw3270SettingsDialog *settings) {
	g_slist_free_full(settings->pages,g_free);
	g_free(settings);
	g_simple_action_set_enabled(settings->action,TRUE);
 }

 void on_response(GtkDialog *dialog, gint response_id, Pw3270SettingsDialog * settings) {

	GSList * page;
	GSettings *hSettings = pw3270_application_get_settings(settings->application);

	switch(response_id) {
	case GTK_RESPONSE_CANCEL:
		g_message("Reverting application settings");
		for(page = settings->pages;page;page = page->next) {
			Pw3270SettingsWidget * widget = (Pw3270SettingsWidget *) page->data;
			widget->revert(widget,settings->application,hSettings);
		}
		break;

	case GTK_RESPONSE_APPLY:
		g_message("Aplying application settings");
		for(page = settings->pages;page;page = page->next) {
			Pw3270SettingsWidget * widget = (Pw3270SettingsWidget *) page->data;
			widget->apply(widget,settings->application,hSettings);
		}
		break;
	}

	gtk_widget_destroy(dialog);
 }

 static void on_page_added(GtkNotebook *notebook, GtkWidget *widget, guint page_num, Pw3270SettingsDialog * settings) {

	// https://developer.gnome.org/hig/stable/visual-layout.html.en

	if(GTK_IS_GRID(widget)) {
		gtk_grid_set_row_spacing(GTK_GRID(widget),6);
		gtk_grid_set_column_spacing(GTK_GRID(widget),12);
	}

	if(GTK_IS_CONTAINER(widget)) {
		gtk_container_set_border_width(GTK_CONTAINER(widget),18);
	}

 }

 void pw3270_application_preferences_activated(GSimpleAction *action, GVariant G_GNUC_UNUSED(*parameter), gpointer application) {

	debug("%s",__FUNCTION__);

	// Create dialog.
	gboolean use_header;
	g_object_get(gtk_settings_get_default(), "gtk-dialogs-use-header", &use_header, NULL);

	GtkWidget * dialog =
		GTK_WIDGET(g_object_new(
			GTK_TYPE_DIALOG,
			"use-header-bar", (use_header ? 1 : 0),
			NULL
		));

	gtk_window_set_title(GTK_WINDOW(dialog),_("Application preferences"));
	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);

    gtk_window_set_transient_for(GTK_WINDOW(dialog),gtk_application_get_active_window(GTK_APPLICATION(application)));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog),TRUE);

	gtk_dialog_add_buttons(
		GTK_DIALOG(dialog),
		_("_Cancel"), GTK_RESPONSE_CANCEL,
		_("_Apply"), GTK_RESPONSE_APPLY,
		NULL
	);

	// Create setttings data.
	Pw3270SettingsDialog * settings = g_new0(Pw3270SettingsDialog,1);
	settings->action		= action;
	settings->application	= GTK_APPLICATION(application);

	g_simple_action_set_enabled(action,FALSE);
	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);

	// Create settings notebook.

	GtkNotebook *notebook = GTK_NOTEBOOK(gtk_notebook_new());

	gtk_notebook_set_scrollable(notebook,TRUE);
	gtk_notebook_set_show_tabs(notebook,TRUE);
	gtk_notebook_set_show_border(notebook, FALSE);

	g_signal_connect(G_OBJECT(notebook), "page-added", G_CALLBACK(on_page_added), dialog);
//	g_signal_connect(G_OBJECT(notebook), "page-removed", G_CALLBACK(on_page_changed), dialog);
//	g_signal_connect(G_OBJECT(notebook), "switch-page", G_CALLBACK(on_switch_page), dialog);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),GTK_WIDGET(notebook),TRUE,TRUE,0);

	// Connection signals.
	g_signal_connect(dialog,"destroy",G_CALLBACK(on_destroy),settings);
	g_signal_connect(dialog,"response",G_CALLBACK(on_response),settings);
	g_signal_connect(dialog,"close",G_CALLBACK(gtk_widget_destroy),NULL);

	// Show dialog.
	gtk_widget_show_all(dialog);
	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);

 }

