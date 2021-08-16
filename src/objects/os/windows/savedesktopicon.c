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
 * @brief Implement Windows version of the save desktop icon action.
 *
 * References:
 *
 * <https://stackoverflow.com/questions/3906974/how-to-programmatically-create-a-shortcut-using-win32>
 * <https://docs.microsoft.com/pt-br/windows/win32/shell/links?redirectedfrom=MSDN>
 *
 */

// #include <stdafx.h>
#include <winsock2.h>
#include <windows.h>
#include <winnls.h>
#include <shobjidl.h>
#include <objbase.h>
#include <objidl.h>
#include <shlguid.h>

#include <v3270.h>
#include <pw3270.h>
#include <pw3270/application.h>
#include <v3270/actions.h>
#include <lib3270.h>
#include <lib3270/log.h>
#include <v3270/tools.h>
#include <v3270/keyfile.h>
#include <v3270/settings.h>

static GtkWidget * factory(V3270SimpleAction *action, GtkWidget *terminal);
static void response(GtkWidget *dialog, gint response_id, GtkWidget *terminal);

static const struct _entry {

	const gchar * label;
	const gchar * tooltip;
	gint margin_top;
	gint width;

} entries[] = {

	// 0 - Shorcut file name
	{
		.label = N_("Shortcut file"),
		.tooltip = N_("Path for the new shortcut"),
		.width = 40,
	},

	// 1 - Shortcut description
	{
		.label = N_("Description"),
		.width = 20,
	},

	// 2 = Session name
	{
		.label = N_("Session name"),
		.margin_top = 12,
		.tooltip = N_("The session name used in the window/tab title (empty for default)"),
		.width = 15,
	},

	// 3 = Session file
	{
		.label = N_("Session file"),
		.tooltip = N_("The file with the session preferences for this shortcut"),
		.width = 40,
	}

};

GAction * pw3270_action_save_desktop_icon_new(void) {

	V3270SimpleAction * action = v3270_dialog_action_new(factory);

	action->name = "save.launcher";
	action->label = _("Save session shortcut");
	action->tooltip = _("Create shortcut for the current session");

	return G_ACTION(action);

}

GtkWidget * factory(V3270SimpleAction *action, GtkWidget *terminal) {

	size_t ix;

	gboolean use_header;
	g_object_get(gtk_settings_get_default(), "gtk-dialogs-use-header", &use_header, NULL);

	GtkWidget *	dialog =
	    GTK_WIDGET(g_object_new(
	                   GTK_TYPE_DIALOG,
	                   "use-header-bar", (use_header ? 1 : 0),
	                   NULL
	               ));

	gtk_window_set_modal(GTK_WINDOW(dialog),TRUE);
	gtk_window_set_title(GTK_WINDOW(dialog),action->label);

	gtk_dialog_add_buttons(
	    GTK_DIALOG(dialog),
	    _("_Cancel"), GTK_RESPONSE_CANCEL,
	    _("_Save"), GTK_RESPONSE_APPLY,
	    NULL
	);

	g_signal_connect(dialog,"response",G_CALLBACK(response),terminal);

	// Create entry fields
	GtkWidget ** inputs = g_new0(GtkWidget *,G_N_ELEMENTS(entries));
	g_object_set_data_full(G_OBJECT(dialog),"inputs",inputs,g_free);
	debug("Dialog=%p inputs=%p",dialog,inputs);

	GtkGrid * grid = GTK_GRID(gtk_grid_new());

	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),GTK_WIDGET(grid),TRUE,TRUE,0);

	// https://developer.gnome.org/hig/stable/visual-layout.html.en
	gtk_container_set_border_width(GTK_CONTAINER(grid),18);
	gtk_grid_set_row_spacing(GTK_GRID(grid),6);
	gtk_grid_set_column_spacing(GTK_GRID(grid),12);

	for(ix = 0; ix < G_N_ELEMENTS(entries); ix++) {

		GtkWidget * label = gtk_label_new(gettext(entries[ix].label));
		gtk_label_set_xalign(GTK_LABEL(label),1);
		gtk_grid_attach(grid,label,0,ix,1,1);

		inputs[ix] = gtk_entry_new();
		debug("inputs[%u]=%p",(unsigned int) ix, inputs[ix]);

		gtk_entry_set_width_chars(GTK_ENTRY(inputs[ix]),entries[ix].width);
		gtk_widget_set_hexpand(inputs[ix],FALSE);
		gtk_widget_set_vexpand(inputs[ix],FALSE);

		if(entries[ix].tooltip) {
			gtk_widget_set_tooltip_markup(GTK_WIDGET(inputs[ix]),gettext(entries[ix].tooltip));
		}

		if(entries[ix].margin_top) {
			gtk_widget_set_margin_top(label,entries[ix].margin_top);
			gtk_widget_set_margin_top(inputs[ix],entries[ix].margin_top);
		}

		gtk_grid_attach(grid,inputs[ix],1,ix,entries[ix].width,1);

	}

	// Setup short-cut name entry.
	{
		gtk_entry_bind_to_filechooser(
		    inputs[0],
		    GTK_FILE_CHOOSER_ACTION_SAVE,
		    _("Save to windows shortcut"),
		    NULL,
		    "*.lnk",
		    _("Windows shortcuts")
		);

		gchar * filename = g_strdup_printf(
		                       "%s\\" G_STRINGIFY(PRODUCT_NAME) ".lnk",
		                       g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP)
		                   );

		size_t ix = 0;

		while(g_file_test(filename,G_FILE_TEST_EXISTS)) {

			g_free(filename);
			filename = g_strdup_printf(
			               "%s\\" G_STRINGIFY(PRODUCT_NAME) "%u.lnk",
			               g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP),
			               (unsigned int) ++ix
			           );

		}

		gtk_entry_set_text(GTK_ENTRY(inputs[0]),filename);
		g_free(filename);
	}

	{
		g_autofree gchar * session_filename = v3270_key_file_build_filename(terminal);
		gtk_entry_set_text(GTK_ENTRY(inputs[3]),session_filename);

		gtk_entry_bind_to_filechooser(
		    inputs[3],
		    GTK_FILE_CHOOSER_ACTION_SAVE,
		    _("File for session preferences"),
		    NULL,
		    "*.3270",
		    _("3270 session files")
		);

	}

	gtk_widget_show_all(GTK_WIDGET(grid));
	return dialog;
}

static HRESULT CreateShortCut(const char * pszTargetfile, const char * pszTargetargs, const char * pszLinkfile, const char * pszDescription, int iShowmode, const char * pszCurdir, LPSTR pszIconfile, int iIconindex) {

	// https://www.codeproject.com/Articles/11467/How-to-create-short-cuts-link-files
	IShellLink*   pShellLink;            // IShellLink object pointer
	IPersistFile* pPersistFile;          // IPersistFile object pointer
	WORD          wszLinkfile[MAX_PATH]; // pszLinkfile as Unicode string
	int           iWideCharsWritten;     // Number of wide characters written

	HRESULT hRes =
	    CoCreateInstance(
	        &CLSID_ShellLink,			// predefined CLSID of the IShellLink object
	        NULL,						// pointer to parent interface if part of aggregate
	        CLSCTX_INPROC_SERVER,		// caller and called code are in same process
	        &IID_IShellLink,			// predefined interface of the IShellLink object
	        (void **) &pShellLink);		// Returns a pointer to the IShellLink object

	if(!SUCCEEDED(hRes)) {
		return hRes;
	}

	if(pszTargetfile && strlen(pszTargetfile)) {
		hRes = pShellLink->lpVtbl->SetPath(pShellLink,pszTargetfile);
	} else {
		char filename[MAX_PATH+1];
		memset(filename,0,MAX_PATH+1);
		GetModuleFileName(NULL,filename,MAX_PATH);
		hRes = pShellLink->lpVtbl->SetPath(pShellLink,filename);
	}

	if(pszTargetargs) {
		hRes = pShellLink->lpVtbl->SetArguments(pShellLink,pszTargetargs);
	} else {
		hRes = pShellLink->lpVtbl->SetArguments(pShellLink,"");
	}

	if(pszDescription && strlen(pszDescription) > 0) {
		hRes = pShellLink->lpVtbl->SetDescription(pShellLink,pszDescription);
	} else {
		hRes = pShellLink->lpVtbl->SetDescription(pShellLink,_("IBM 3270 Terminal emulator"));
	}

	if(iShowmode > 0) {
		hRes = pShellLink->lpVtbl->SetShowCmd(pShellLink,iShowmode);
	}

	if(pszCurdir && strlen(pszCurdir) > 0) {
		hRes = pShellLink->lpVtbl->SetWorkingDirectory(pShellLink,pszCurdir);
	} else {
		g_autofree gchar * appdir = g_win32_get_package_installation_directory_of_module(NULL);
		hRes = pShellLink->lpVtbl->SetWorkingDirectory(pShellLink,appdir);
	}

	if(pszIconfile && strlen(pszIconfile) > 0 && iIconindex >= 0) {
		hRes = pShellLink->lpVtbl->SetIconLocation(pShellLink, pszIconfile, iIconindex);
	}

	// Use the IPersistFile object to save the shell link
	hRes = pShellLink->lpVtbl->QueryInterface(
	           pShellLink,					// existing IShellLink object
	           &IID_IPersistFile,			// pre-defined interface of the IPersistFile object
	           (void **) &pPersistFile);	// returns a pointer to the IPersistFile object


	if(SUCCEEDED(hRes)) {
		iWideCharsWritten = MultiByteToWideChar(CP_ACP, 0, pszLinkfile, -1, wszLinkfile, MAX_PATH);
		hRes = pPersistFile->lpVtbl->Save(pPersistFile,wszLinkfile, TRUE);
		pPersistFile->lpVtbl->Release(pPersistFile);
	}

	pShellLink->lpVtbl->Release(pShellLink);

	return hRes;
}

void response(GtkWidget *dialog, gint response_id, GtkWidget *terminal) {

	debug("%s(%d)",__FUNCTION__,response_id);

	if(response_id == GTK_RESPONSE_APPLY) {

		// Save desktop icon
		GtkWidget ** inputs = g_object_get_data(G_OBJECT(dialog),"inputs");

		// Save keyfile
		GError * error = NULL;
		v3270_key_file_save_to_file(
		    terminal,
		    gtk_entry_get_text(GTK_ENTRY(inputs[3])),
		    &error
		);

		if(!error) {

			HRESULT hRes = CreateShortCut(
			                   NULL,										// LPSTR pszTargetfile,
			                   gtk_entry_get_text(GTK_ENTRY(inputs[3])),	// LPSTR pszTargetargs,
			                   gtk_entry_get_text(GTK_ENTRY(inputs[0])),	// LPSTR pszLinkfile,
			                   gtk_entry_get_text(GTK_ENTRY(inputs[1])),	// LPSTR pszDescription,
			                   0,
			                   NULL,
			                   NULL,
			                   0
			               );

		}

		if(error) {

			g_message("%s",error->message);
			g_error_free(error);

		} else {

			// Set session name (after save to avoid changes on the old session file).
			v3270_set_session_name(terminal,gtk_entry_get_text(GTK_ENTRY(inputs[2])));
			v3270_emit_save_settings(terminal,NULL);

		}

	}

	gtk_widget_destroy(dialog);

}
