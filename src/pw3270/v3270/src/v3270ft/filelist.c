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
 * Este programa está nomeado como filelist.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "private.h"

/*--[ Implement ]------------------------------------------------------------------------------------*/

guint v3270ft_get_length(GtkWidget *widget) {
	return g_list_length(GTK_V3270FT(widget)->files);
}


void v3270ft_select_first(GtkWidget *widget) {

	v3270ft	* dialog = GTK_V3270FT(widget);
	v3270ft_set_active(dialog,g_list_first(dialog->files));

}

void v3270ft_select_last(GtkWidget *widget) {

	v3270ft	* dialog = GTK_V3270FT(widget);
	v3270ft_set_active(dialog,g_list_last(dialog->files));

}

void v3270ft_select_previous(GtkWidget *widget) {

	v3270ft	* dialog = GTK_V3270FT(widget);
	v3270ft_set_active(dialog,g_list_previous(dialog->active));

}

void v3270ft_select_next(GtkWidget *widget) {

	v3270ft	* dialog = GTK_V3270FT(widget);
	v3270ft_set_active(dialog,g_list_next(dialog->active));

}

void v3270ft_remove_selected(GtkWidget *widget) {

	v3270ft	* dialog = GTK_V3270FT(widget);

	if(dialog->active) {

		GList *next = dialog->active->next;

		dialog->files = g_list_delete_link(dialog->files,dialog->active);

		v3270ft_set_active(dialog,next);

	}

}

struct v3270ft_entry * v3270ft_create_entry(void) {
	 struct v3270ft_entry *rc = g_new0(struct v3270ft_entry,1);
	 rc->value[LIB3270_FT_VALUE_DFT] = 4096;
	 return rc;
}

void v3270ft_append_file(GtkWidget *widget, const gchar *filename, gboolean text) {

	v3270ft	* dialog = GTK_V3270FT(widget);

	debug("%s",filename);

	// Se a entrada atual não é valida remove.
	if(!v3270ft_is_valid(widget)) {
		v3270ft_remove_selected(widget);
	}

	// Inclui uma nova entrada
	struct v3270ft_entry	* entry = v3270ft_create_entry();
	gchar 					* base	= g_path_get_basename(filename);

	strncpy(entry->local,filename,FILENAME_MAX);
	strncpy(entry->remote,base,FILENAME_MAX);

	g_free(base);

	entry->type 	= text ? 3 : 1;
	entry->options	= ft_type[entry->type].opt;

	dialog->files = g_list_append(dialog->files,entry);
	v3270ft_select_last(widget);
}
