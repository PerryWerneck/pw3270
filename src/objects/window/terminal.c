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

 #include <glib.h>
 #include <glib/gstdio.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <stdlib.h>

 #include <pw3270/actions.h>
 #include <lib3270/toggle.h>
 #include <v3270/settings.h>
 #include <v3270/actions.h>
 #include <v3270/keyfile.h>
 #include <v3270/print.h>
 #include <lib3270/os.h>

 static void destroy(GtkWidget *terminal, gpointer GNUC_UNUSED(dunno)) {
	v3270_key_file_close(terminal);
 }

 static void toggle_changed(GtkWidget *widget, G_GNUC_UNUSED LIB3270_TOGGLE_ID toggle_id, gboolean toggle_state, const gchar *toggle_name, gpointer GNUC_UNUSED(dunno)) {
	debug("%s(%s)=%s",__FUNCTION__,toggle_name,toggle_state ? "ON" : "OFF");
	v3270_key_file_set_boolean(widget,"terminal",toggle_name,toggle_state);
 }

 static void save_settings(GtkWidget *terminal, gpointer G_GNUC_UNUSED(dunno)) {
	v3270_key_file_save(terminal);
 }

 static void print_done(GtkWidget *widget, GtkPrintOperation *operation, GtkPrintOperationResult result, gpointer G_GNUC_UNUSED(dunno)) {
 	debug("%s(%u)",__FUNCTION__,(unsigned int) result);

 	if(result != GTK_PRINT_OPERATION_RESULT_APPLY)
		return;

	debug("%s: Saving print settings",__FUNCTION__);

	v3270_print_operation_to_key_file(operation,v3270_key_file_get(widget));
	v3270_emit_save_settings(widget,NULL);

 }

 static void print_setup(G_GNUC_UNUSED GtkWidget *widget, GtkPrintOperation *operation, gpointer G_GNUC_UNUSED(dunno) ) {

 	debug("%s(%p)",__FUNCTION__,operation);
	v3270_print_operation_load_key_file(operation,v3270_key_file_get(widget));

 }

 static GtkResponseType load_popup_response(GtkWidget *widget, const gchar *popup_name, gpointer G_GNUC_UNUSED(dunno)) {

	GKeyFile * key_file = v3270_key_file_get(widget);

	if(key_file && g_key_file_has_key(key_file,"dialogs",popup_name,NULL))
		return (GtkResponseType) g_key_file_get_integer(key_file,"dialogs",popup_name,NULL);

#ifdef _WIN32
	{
		// Windows - Check predefined responses on system registry.
		lib3270_auto_cleanup(HKEY) hKey;

		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,G_STRINGIFY(PRODUCT_NAME)"\\dialogs",0,KEY_READ,&hKey) == ERROR_SUCCESS) {

			DWORD val = 0;
			DWORD cbData = sizeof(DWORD);

			if(RegQueryValueEx(hKey, popup_name, NULL, NULL, (LPBYTE) &val, &cbData) == ERROR_SUCCESS) {
				return (GtkResponseType) val;
			}

		}
	}
#endif // _WIN32

	return key_file ? GTK_RESPONSE_NONE : 0;
 }

 static gboolean save_popup_response(GtkWidget *widget, const gchar *popup_name, GtkResponseType response, gpointer G_GNUC_UNUSED(dunno)) {

	GKeyFile * key_file = v3270_key_file_get(widget);

	debug("%s(%s)",__FUNCTION__,popup_name);

	if(!key_file)
		return FALSE;

	g_key_file_set_integer(key_file,"dialogs",popup_name,(gint) response);
	v3270_emit_save_settings(widget,NULL);

	return TRUE;
 }

 GtkWidget * pw3270_terminal_new(const gchar *session_file) {

 	GtkWidget	* terminal = v3270_new();
 	GError		* error = NULL;

 	gtk_widget_show_all(terminal);

 	if(session_file) {

		// Use the supplied session file
		v3270_key_file_open(terminal,session_file,&error);

 	} else {

		// No session file, use the default one.
		g_autofree gchar * compatible = g_build_filename(g_get_user_config_dir(),G_STRINGIFY(PRODUCT_NAME) ".conf",NULL);
		g_autofree gchar * filename = g_build_filename(g_get_user_config_dir(),"default.3270",NULL);

		if(g_file_test(compatible,G_FILE_TEST_IS_REGULAR))
		{
			g_rename(compatible,filename);
		}

		v3270_key_file_open(terminal,filename,&error);

 	}

 	// Setup signals.
 	g_signal_connect(G_OBJECT(terminal),"save-settings",G_CALLBACK(save_settings),NULL);
 	g_signal_connect(G_OBJECT(terminal),"toggle_changed",G_CALLBACK(toggle_changed),NULL);
 	g_signal_connect(G_OBJECT(terminal),"print-done",G_CALLBACK(print_done),NULL);
 	g_signal_connect(G_OBJECT(terminal),"print-setup",G_CALLBACK(print_setup),NULL);
	g_signal_connect(G_OBJECT(terminal),"destroy", G_CALLBACK(destroy),NULL);
	g_signal_connect(G_OBJECT(terminal),"load-popup-response",G_CALLBACK(load_popup_response),NULL);
 	g_signal_connect(G_OBJECT(terminal),"save-popup-response",G_CALLBACK(save_popup_response),NULL);

 	if(error) {

		GtkWidget * dialog = gtk_message_dialog_new_with_markup(
										GTK_WINDOW(gtk_widget_get_toplevel(terminal)),
										GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
										GTK_MESSAGE_ERROR,
										GTK_BUTTONS_CANCEL,
										_("Can't use \"%s\""),session_file
									);

		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s",error->message);

		gtk_window_set_title(GTK_WINDOW(dialog),_("Can't load session file"));

		gtk_widget_show_all(dialog);

		g_signal_connect(dialog,"close",G_CALLBACK(gtk_widget_destroy),NULL);
		g_signal_connect(dialog,"response",G_CALLBACK(gtk_widget_destroy),NULL);

		g_error_free(error);
	}

	return terminal;
 }

 GtkWidget * pw3270_application_window_new_tab(GtkWidget *widget, const gchar *session_file) {

	g_return_val_if_fail(PW3270_IS_APPLICATION_WINDOW(widget),NULL);

	GtkWidget * terminal = pw3270_terminal_new(session_file);

	pw3270_window_set_current_page(widget,pw3270_application_window_append_page(widget,terminal));

	return terminal;

 }


