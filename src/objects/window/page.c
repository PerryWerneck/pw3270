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
 #include <pw3270/application.h>
 #include <lib3270/toggle.h>
 #include <v3270/settings.h>
 #include <v3270/dialogs.h>
 #include <v3270/actions.h>
 #include <v3270/print.h>
 #include <pw3270.h>

//---[ Gtk Label with customized popup-menu ]---------------------------------------------------------------------------------------

 typedef struct _pw3270TabLabel {

 	GtkLabel parent;

 } pw3270TabLabel;

 typedef struct _pw3270TabLabelClass {

 	GtkLabelClass parent_class;

 } pw3270TabLabelClass;

 G_DEFINE_TYPE(pw3270TabLabel, pw3270TabLabel, GTK_TYPE_LABEL);

 static void popup_menu_detach(GtkWidget G_GNUC_UNUSED(*label), GtkMenu *menu) {

 	debug("%s(%p)",__FUNCTION__,menu);

 }

 static void popup_menu_deactivated(GtkMenu *menu, GtkWidget G_GNUC_UNUSED(*label)) {
 	gtk_menu_detach(menu);
 }

#ifdef DEBUG
 static void menu_destroy(GtkWidget *menu) {
 	debug("%s(%p)",__FUNCTION__,menu);
 }
#endif // DEBUG

 static gboolean tab_label_button_press(GtkWidget *label, GdkEventButton *event) {

	if (event->button == 3 && event->type == GDK_BUTTON_PRESS) {

		GtkWidget * menu = gtk_menu_new();

#ifdef DEBUG
		g_signal_connect(menu,"destroy",G_CALLBACK(menu_destroy),NULL);
#endif // DEBUG
		g_signal_connect(menu,"deactivate",G_CALLBACK(popup_menu_deactivated),label);

		debug("menu=%p",menu);

		gtk_menu_attach_to_widget(GTK_MENU(menu), GTK_WIDGET(label), popup_menu_detach);
		g_signal_emit_by_name(GTK_LABEL(label),"populate-popup",menu);

#if GTK_CHECK_VERSION(3,22,0)
		gtk_menu_popup_at_widget(GTK_MENU(menu),label,GDK_GRAVITY_SOUTH_WEST,GDK_GRAVITY_NORTH_WEST,(GdkEvent *) event);
#else
		gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, 0, 0);
#endif

		return TRUE;
	}

	return FALSE;
 }

 static void pw3270TabLabel_class_init(pw3270TabLabelClass *klass) {

	GTK_WIDGET_CLASS(klass)->button_press_event = tab_label_button_press;

 }

 static void pw3270TabLabel_init(pw3270TabLabel G_GNUC_UNUSED(*widget)) {

 }

//----------------------------------------------------------------------------------------------------------------------------------

 static gboolean on_terminal_focus(GtkWidget *terminal, GdkEvent G_GNUC_UNUSED(*event), GtkWidget * window);
 static void session_changed(GtkWidget *terminal, GtkWidget *label);
 static void disconnected(GtkWidget *terminal, GtkWindow * window);
 static void connected(GtkWidget *terminal, const gchar *host, GtkWindow * window);
 static void destroy(GtkWidget *terminal, GtkWindow * window);
 static void close_page(GtkButton *button, GtkWidget *terminal);
 static gboolean terminal_popup(GtkWidget *widget, gboolean selected, gboolean online, GdkEvent *event, pw3270ApplicationWindow * window);
 static gboolean oia_popup(GtkWidget *widget, guint field, GdkEvent *event, pw3270ApplicationWindow * window);
 static void label_populate_popup(GtkLabel *label, GtkMenu *menu, GtkWidget *terminal);
 static void label_disconnect(GtkWidget *label, GtkWidget *terminal);

 gint pw3270_application_window_append_page(GtkWidget * window, GtkWidget * terminal) {

	// Setup label
 	GtkWidget * label =
		GTK_WIDGET(
			g_object_new(
			pw3270TabLabel_get_type(),
			"label", v3270_get_session_name(terminal),
			"selectable", TRUE,
			NULL)
		);

	g_signal_connect(G_OBJECT(terminal), "session_changed", G_CALLBACK(session_changed),label);
	g_signal_connect(G_OBJECT(label), "destroy", G_CALLBACK(label_disconnect),terminal);

	// Setup tab

 	GtkWidget * tab			= gtk_box_new(GTK_ORIENTATION_HORIZONTAL,2);
 	GtkWidget * button		= gtk_button_new_from_icon_name("window-close-symbolic",GTK_ICON_SIZE_MENU);
 	GtkNotebook	* notebook	= PW3270_APPLICATION_WINDOW(window)->notebook;

 	gtk_button_set_relief(GTK_BUTTON(button),GTK_RELIEF_NONE);

	g_signal_connect(G_OBJECT(label), "populate-popup", G_CALLBACK(label_populate_popup), terminal);

	g_signal_connect(G_OBJECT(terminal), "focus-in-event", G_CALLBACK(on_terminal_focus), window);
	g_signal_connect(G_OBJECT(terminal), "disconnected", G_CALLBACK(disconnected),window);
	g_signal_connect(G_OBJECT(terminal), "connected", G_CALLBACK(connected),window);
	g_signal_connect(G_OBJECT(terminal), "destroy", G_CALLBACK(destroy),window);

	g_signal_connect(G_OBJECT(terminal), "popup", G_CALLBACK(terminal_popup), window);
	g_signal_connect(G_OBJECT(terminal), "oia-popup", G_CALLBACK(oia_popup), window);

	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(close_page), terminal);


 	gtk_box_pack_start(GTK_BOX(tab),label,FALSE,FALSE,0);
 	gtk_box_pack_end(GTK_BOX(tab),button,FALSE,FALSE,0);

	gtk_widget_show_all(terminal);
	gtk_widget_show_all(tab);

	gint page = gtk_notebook_append_page(notebook,terminal,tab);

	gtk_notebook_set_tab_detachable(notebook,terminal,TRUE);
	gtk_notebook_set_tab_reorderable(notebook,terminal,TRUE);

	return page;

 }

 static gboolean on_terminal_focus(GtkWidget *terminal, GdkEvent G_GNUC_UNUSED(*event), GtkWidget * window) {

	gtk_widget_grab_default(terminal);
	pw3270_application_window_set_active_terminal(window,terminal);
 	return FALSE;
 }

 static void session_changed(GtkWidget *terminal, GtkWidget *label) {

	gtk_label_set_text(GTK_LABEL(label),v3270_get_session_name(terminal));

	// Do I have to change the window title?
	GtkWidget * toplevel = gtk_widget_get_toplevel(terminal);
	if(PW3270_IS_APPLICATION_WINDOW(toplevel)) {

		pw3270ApplicationWindow * window = PW3270_APPLICATION_WINDOW(toplevel);

		if(gtk_widget_has_default(terminal)) {
			g_autofree gchar * title = v3270_get_session_title(terminal);
			gtk_window_set_title(GTK_WINDOW(window), title);
		}

	}

 }

 static void disconnected(GtkWidget *terminal, GtkWindow * window) {

 	debug("%s",__FUNCTION__);

 	if(terminal != gtk_window_get_default_widget(window))
		return;

	pw3270_window_set_subtitle(GTK_WIDGET(window), _("Disconnected from host"));

 }

 static void connected(GtkWidget *terminal, const gchar *host, GtkWindow * window) {

 	debug("%s(%s)",__FUNCTION__,host);

 	if(terminal != gtk_window_get_default_widget(window))
		return;

	pw3270_window_set_subtitle(GTK_WIDGET(window), _("Connected to host"));

 }

 static void destroy(GtkWidget *terminal, GtkWindow * window) {

	if(pw3270_application_window_get_active_terminal(GTK_WIDGET(window)) != terminal) {
		debug("Terminal %p was destroyed (Default one is %p)",__FUNCTION__,pw3270_application_window_get_active_terminal(GTK_WIDGET(window)));
		return;
	}

	debug("Default terminal %p was destroyed (toplevel=%p)",__FUNCTION__,gtk_widget_get_toplevel(terminal));

	pw3270_application_window_set_active_terminal(GTK_WIDGET(window),NULL);

 }

  static void close_page(GtkButton G_GNUC_UNUSED(*button), GtkWidget *terminal) {

 	debug("%s",__FUNCTION__);

	GtkNotebook * notebook = GTK_NOTEBOOK(gtk_widget_get_parent(terminal));
	gtk_notebook_remove_page(notebook,gtk_notebook_page_num(notebook, terminal));


 }

 static void on_rename_session_response(GtkDialog *dialog, gint response_id, GtkWidget *terminal) {

	if(response_id == GTK_RESPONSE_APPLY) {

		v3270_set_session_name(terminal, gtk_entry_get_text(GTK_ENTRY(g_object_get_data(G_OBJECT(dialog),"entry"))));
		g_signal_emit_by_name(terminal,"save-settings");

	}

 	gtk_widget_destroy(GTK_WIDGET(dialog));
 }

 static void rename_session(GtkWidget G_GNUC_UNUSED(*widget), GtkWidget *terminal) {

 	debug("%s",__FUNCTION__);

 	GtkWidget * dialog = v3270_dialog_new_with_buttons(
								_("Rename Session"),
								terminal,
								_("Apply"), GTK_RESPONSE_APPLY,
								_("Cancel"), GTK_RESPONSE_CANCEL,
								NULL
							);

	g_signal_connect(G_OBJECT(dialog),"response",G_CALLBACK(on_rename_session_response),terminal);

	// Create controls.
	GtkWidget * box = v3270_dialog_set_content_area(dialog,gtk_box_new(GTK_ORIENTATION_HORIZONTAL,12));

	// Create label.rename
	GtkWidget *label = gtk_label_new(_("New session name"));
	gtk_label_set_xalign(GTK_LABEL(label),1);
	gtk_box_pack_start(GTK_BOX(box),label,FALSE,TRUE,0);

	// Create entry
	GtkWidget * entry = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(entry),10);
	gtk_entry_set_activates_default(GTK_ENTRY(entry),TRUE);
	gtk_entry_set_width_chars(GTK_ENTRY(entry),12);
	gtk_entry_set_placeholder_text(GTK_ENTRY(entry),G_STRINGIFY(PRODUCT_NAME));
	gtk_entry_set_input_purpose(GTK_ENTRY(entry),GTK_INPUT_PURPOSE_ALPHA);

	g_object_set_data(G_OBJECT(dialog),"entry",entry);

	{
		g_autofree gchar * session_name = g_strdup(v3270_get_session_name(terminal));

		gchar *ptr = strrchr(session_name,':');
		if(ptr)
			*ptr = 0;

		gtk_entry_set_text(GTK_ENTRY(entry),session_name);

	}

	gtk_box_pack_start(GTK_BOX(box),entry,FALSE,TRUE,0);


	// Show dialog.
	gtk_widget_show_all(dialog);



 	/*
	GtkWidget * dialog = pw3270_settings_dialog_new(
								_("Rename session"),
								GTK_WINDOW(gtk_widget_get_toplevel(terminal))
							);

	// https://developer.gnome.org/hig/stable/visual-layout.html.en
	gtk_container_set_border_width(GTK_CONTAINER(content),18);


	gtk_widget_show_all(dialog);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_APPLY) {

		v3270_set_session_name(terminal, gtk_entry_get_text(GTK_ENTRY(entry)));
		g_signal_emit_by_name(terminal,"save-settings");

	}

	gtk_widget_destroy(dialog);
	*/

 }

 static gboolean terminal_popup(GtkWidget *widget, gboolean selected, gboolean online, GdkEvent *event, pw3270ApplicationWindow * window) {

	GtkWidget * popup = window->popups[PW3270_APP_WINDOW_POPUP_OVER_UNSELECTED_AREA];

	if(!online && window->popups[PW3270_APP_WINDOW_POPUP_WHEN_OFFLINE])
		popup = window->popups[PW3270_APP_WINDOW_POPUP_WHEN_OFFLINE];
	else if(selected && window->popups[PW3270_APP_WINDOW_POPUP_OVER_SELECTED_AREA])
		popup = window->popups[PW3270_APP_WINDOW_POPUP_OVER_SELECTED_AREA];
	else
		popup = window->popups[PW3270_APP_WINDOW_POPUP_DEFAULT];

	if(!popup)
		return FALSE;

	gtk_widget_show_all(popup);
	gtk_menu_set_screen(GTK_MENU(popup), gtk_widget_get_screen(widget));

#if GTK_CHECK_VERSION(3,22,0)
	gtk_menu_popup_at_pointer(GTK_MENU(popup), event);
#else
	gtk_menu_popup(GTK_MENU(popup), NULL, NULL, NULL, NULL, 0, 0);
#endif

	return TRUE;

 }

 static gboolean oia_popup(GtkWidget *widget, guint field, GdkEvent *event, pw3270ApplicationWindow * window) {

	debug("%s(%u)",__FUNCTION__,field);

	GtkWidget *popup = window->popups[PW3270_APP_WINDOW_POPUP_OVER_OIA];

	if(!popup)
		return FALSE;

	gtk_widget_show_all(popup);
	gtk_menu_set_screen(GTK_MENU(popup), gtk_widget_get_screen(widget));

#if GTK_CHECK_VERSION(3,22,0)
	gtk_menu_popup_at_pointer(GTK_MENU(popup), event);
#else
	gtk_menu_popup(GTK_MENU(popup), NULL, NULL, NULL, NULL, 0, 0);
#endif

	return TRUE;
 }

 static void label_populate_popup(GtkLabel G_GNUC_UNUSED(*label), GtkMenu *menu, GtkWidget *terminal) {

	static const struct Item {
		const gchar * label;
		GCallback	  callback;
		gboolean (*check_permission)(GtkWidget *widget);
	} items[] = {

		{
			.label = N_("_Rename session"),
			.callback = G_CALLBACK(rename_session),
			.check_permission = v3270_allow_custom_settings
		},

		{
			.label = N_("_Close session"),
			.callback = G_CALLBACK(close_page)
		}

	};

	size_t ix;

	debug("%s",__FUNCTION__);

	for(ix = 0; ix < G_N_ELEMENTS(items); ix++) {
		GtkWidget *item = gtk_menu_item_new_with_mnemonic(gettext(items[ix].label));

		if(items[ix].check_permission)
			gtk_widget_set_sensitive(item,items[ix].check_permission(terminal));

		g_signal_connect(G_OBJECT(item),"activate",items[ix].callback,terminal);
		gtk_widget_show_all(item);
		gtk_menu_shell_append(GTK_MENU_SHELL(menu),item);
	}

 }

 static void label_disconnect(GtkWidget *label, GtkWidget *terminal) {
 	debug("%s(%p)",__FUNCTION__,label);
 	g_signal_handlers_disconnect_by_data(G_OBJECT(terminal),label);
 }


