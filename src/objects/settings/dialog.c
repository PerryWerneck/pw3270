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

 #include <pw3270.h>
 #include <pw3270/settings.h>
 #include <lib3270.h>
 #include <lib3270/log.h>

 struct _PW3270SettingsDialog {
  	GtkDialog		  parent;
  	GtkNotebook		* tabs;
  	gboolean		  has_subtitle;
 };

 struct _PW3270SettingsDialogClass	{
 	GtkDialogClass parent_class;
 };

 G_DEFINE_TYPE(PW3270SettingsDialog, PW3270SettingsDialog, GTK_TYPE_DIALOG);

 static void add(GtkContainer *container, GtkWidget *widget);
 static void page_changed(GtkNotebook *notebook, GtkWidget *child, guint page_num, PW3270SettingsDialog *dialog);
 static void switch_page(GtkNotebook *notebook, GtkWidget *settings, guint page_num, PW3270SettingsDialog *dialog);
 static void dialog_close(GtkDialog *dialog);
 static void response(GtkDialog *dialog, gint response_id);

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void PW3270SettingsDialog_class_init(PW3270SettingsDialogClass *klass) {
	GTK_CONTAINER_CLASS(klass)->add = add;
	GTK_DIALOG_CLASS(klass)->close = dialog_close;
	GTK_DIALOG_CLASS(klass)->response = response;
}

static void PW3270SettingsDialog_init(PW3270SettingsDialog *dialog)
{
	GtkWidget * content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

	// Get use of header bar.
	g_object_get(gtk_settings_get_default(), "gtk-dialogs-use-header", &dialog->has_subtitle, NULL);

	// https://developer.gnome.org/hig/stable/visual-layout.html.en
	//gtk_box_set_spacing(GTK_BOX(content_area),18);
	//gtk_container_set_border_width(GTK_CONTAINER(content_area),18);

	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);
    gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);

	gtk_dialog_add_buttons(
		GTK_DIALOG(dialog),
		_("_Cancel"), GTK_RESPONSE_CANCEL,
		_("_Apply"), GTK_RESPONSE_APPLY,
		NULL
	);

	// Create notebook for settings widgets
	dialog->tabs = GTK_NOTEBOOK(gtk_notebook_new());

	gtk_notebook_set_scrollable(dialog->tabs,TRUE);
	gtk_notebook_set_show_tabs(dialog->tabs,FALSE);
	g_signal_connect(G_OBJECT(dialog->tabs), "page-added", G_CALLBACK(page_changed), dialog);
	g_signal_connect(G_OBJECT(dialog->tabs), "page-removed", G_CALLBACK(page_changed), dialog);
	g_signal_connect(G_OBJECT(dialog->tabs), "switch-page", G_CALLBACK(switch_page), dialog);
	gtk_box_pack_start(GTK_BOX(content_area),GTK_WIDGET(dialog->tabs),TRUE,TRUE,0);

	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);

}

GtkWidget * pw3270_settings_dialog_new() {
#if GTK_CHECK_VERSION(3,12,0)

	gboolean use_header;
	g_object_get(gtk_settings_get_default(), "gtk-dialogs-use-header", &use_header, NULL);

	GtkWidget * dialog =
		GTK_WIDGET(g_object_new(
			GTK_TYPE_PW3270_SETTINGS_DIALOG,
			"use-header-bar", (use_header ? 1 : 0),
			NULL
		));

#else

	GtkWidget * dialog = GTK_WIDGET(g_object_new(GTK_TYPE_PW3270_SETTINGS_DIALOG, NULL));

#endif	// GTK 3.12

 	return dialog;

}

void dialog_close(GtkDialog *dialog) {
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

void response(GtkDialog *dialog, gint response_id) {

	debug("%s(%d)",__FUNCTION__,response_id);

	gtk_widget_destroy(GTK_WIDGET(dialog));
}

void add(GtkContainer *container, GtkWidget *widget) {

	GtkWidget * label = NULL;

	gtk_notebook_append_page(
		GTK_PW3270_SETTINGS_DIALOG(container)->tabs,
		widget,
		label
	);

}

void page_changed(GtkNotebook *notebook, GtkWidget G_GNUC_UNUSED(*child), guint G_GNUC_UNUSED(page_num), PW3270SettingsDialog G_GNUC_UNUSED(*dialog)) {
 	gtk_notebook_set_show_tabs(notebook,gtk_notebook_get_n_pages(notebook) > 1);
}

void switch_page(GtkNotebook G_GNUC_UNUSED(*notebook), GtkWidget *settings, guint G_GNUC_UNUSED(page_num), PW3270SettingsDialog *dialog) {

	/*
	debug("title: %s",settings->title);
	debug("label: %s",settings->label);

	if(dialog->has_subtitle) {
		GtkWidget * header_bar = gtk_dialog_get_header_bar(GTK_DIALOG(dialog));
		if(header_bar) {
			gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header_bar),settings->title);
		}
	}
	*/
}

