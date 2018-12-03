/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob
 * o nome G3270.
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
 * Este programa está nomeado como misc.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "private.h"

/*--[ Implement ]------------------------------------------------------------------------------------*/

void v3270ft_update_state(struct v3270ft_entry *entry) {

	// Reset
	entry->valid = FALSE;

	// Have filenames? If not just return
	if(!(*entry->local && *entry->remote))
		return;

	// Filename widgets has data, validade it.
	if(entry->options & LIB3270_FT_OPTION_RECEIVE) {

		// Receber arquivos, verifico se o diretório de destino é válido
		gchar *dir = g_path_get_dirname(entry->local);
		entry->valid = g_file_test(dir,G_FILE_TEST_IS_DIR);
		g_free(dir);

	} else {

		entry->valid = g_file_test(entry->local,G_FILE_TEST_IS_REGULAR);

	}

}

void v3270ft_update_actions(v3270ft *dialog) {

	struct v3270ft_entry *entry = v3270ft_get_selected(dialog);

	if(entry) {

		v3270ft_update_state(entry);

		// Atualizo botões
		gboolean is_valid = v3270ft_is_valid(GTK_WIDGET(dialog));

		gtk_widget_set_sensitive(dialog->button[FT_BUTTON_INSERT_FILE],is_valid);
		gtk_widget_set_sensitive(dialog->button[FT_BUTTON_START_TRANSFER],is_valid);
		gtk_widget_set_sensitive(dialog->button[FT_BUTTON_SAVE_LIST],is_valid);

		gtk_widget_set_sensitive(dialog->button[FT_BUTTON_GO_PREVIOUS],dialog->active->prev != NULL);
		gtk_widget_set_sensitive(dialog->button[FT_BUTTON_GO_FIRST],dialog->active->prev != NULL);

		gtk_widget_set_sensitive(dialog->button[FT_BUTTON_GO_NEXT],dialog->active->next != NULL);
		gtk_widget_set_sensitive(dialog->button[FT_BUTTON_GO_LAST],dialog->active->next != NULL);

	} else {

		gtk_widget_set_sensitive(dialog->button[FT_BUTTON_INSERT_FILE],FALSE);
		gtk_widget_set_sensitive(dialog->button[FT_BUTTON_START_TRANSFER],FALSE);
		gtk_widget_set_sensitive(dialog->button[FT_BUTTON_SAVE_LIST],FALSE);

		gtk_widget_set_sensitive(dialog->button[FT_BUTTON_GO_PREVIOUS],FALSE);
		gtk_widget_set_sensitive(dialog->button[FT_BUTTON_GO_FIRST],FALSE);

		gtk_widget_set_sensitive(dialog->button[FT_BUTTON_GO_NEXT],FALSE);
		gtk_widget_set_sensitive(dialog->button[FT_BUTTON_GO_LAST],FALSE);

	}

}

