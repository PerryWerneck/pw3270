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

 #include "private.h"
 #include <pw3270/actions.h>

 static void session_changed(GtkWidget *terminal, GtkWidget *label) {

	gtk_label_set_text(GTK_LABEL(label),v3270_get_session_name(terminal));

	// Do I have to change the window title?
	GtkWidget * toplevel = gtk_widget_get_toplevel(terminal);
	if(PW3270_IS_APPLICATION_WINDOW(toplevel)) {

		pw3270ApplicationWindow * window = PW3270_APPLICATION_WINDOW(toplevel);

		if(window->terminal == terminal) {
			g_autofree gchar * title = v3270_get_session_title(terminal);
			gtk_window_set_title(GTK_WINDOW(window), title);
		}

	}

 }

 static gboolean on_terminal_focus(GtkWidget *terminal, GdkEvent *event, pw3270ApplicationWindow * window) {

	// Store the active terminal widget.
	window->terminal = terminal;

	// Change window title
	g_autofree gchar * title = v3270_get_session_title(terminal);
	gtk_window_set_title(GTK_WINDOW(window), title);

 	return FALSE;
 }

 GtkWidget * pw3270_terminal_new(GtkWidget *widget) {

	g_return_val_if_fail(PW3270_IS_APPLICATION_WINDOW(widget),NULL);

	pw3270ApplicationWindow * window = PW3270_APPLICATION_WINDOW(widget);
 	GtkWidget * terminal = v3270_new();
 	GtkWidget * label = gtk_label_new(v3270_get_session_name(terminal));

	g_signal_connect(G_OBJECT(terminal), "focus-in-event", G_CALLBACK(on_terminal_focus), widget);
	g_signal_connect(G_OBJECT(terminal), "session_changed", G_CALLBACK(session_changed),label);

	gtk_widget_show_all(terminal);
	gtk_widget_show_all(label);

	gtk_notebook_append_page(window->notebook,terminal,label);
	gtk_notebook_set_show_tabs(window->notebook,gtk_notebook_get_n_pages(GTK_NOTEBOOK(window->notebook)) > 1);

	gtk_notebook_set_tab_detachable(window->notebook,terminal,TRUE);
	gtk_notebook_set_tab_reorderable(window->notebook,terminal,TRUE);

	return terminal;

 }

