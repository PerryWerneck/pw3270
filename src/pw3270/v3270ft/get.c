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
 * Este programa está nomeado como get.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "private.h"

/*--[ Globals ]--------------------------------------------------------------------------------------*/


/*--[ Implement ]------------------------------------------------------------------------------------*/

/**
 * v3270ft_get_options:
 * @widget: a #v3270ft
 * @return: The lib3270 file transfer options for the current dialog.
 *
 * Since: 5.1
 **/
LIB3270_FT_OPTION v3270ft_get_options(GtkWidget *widget) {
	return v3270ft_get_selected(GTK_V3270FT(widget))->options;
}

static void entry_is_valid(struct v3270ft_entry *entry, gboolean *rc) {

	*rc = (*rc && entry->valid);

}

const gchar * v3270ft_get_local_filename(GtkWidget *widget) {
	return v3270ft_get_selected(GTK_V3270FT(widget))->local;
}

const gchar * v3270ft_get_remote_filename(GtkWidget *widget) {
	return v3270ft_get_selected(GTK_V3270FT(widget))->remote;
}

gboolean v3270ft_has_selected(GtkWidget *widget) {
	return GTK_V3270FT(widget)->active != NULL;
}

gboolean v3270ft_has_next(GtkWidget *widget) {

	g_return_val_if_fail(GTK_IS_V3270FT(widget),FALSE);

	v3270ft * dialog = GTK_V3270FT(widget);

	return dialog->active && dialog->active->next;

}

gboolean v3270ft_is_valid(GtkWidget *widget) {

	gboolean rc = FALSE;

	struct v3270ft_entry *entry = v3270ft_get_selected(GTK_V3270FT(widget));

	if(entry) {

		// Marco como TRUE
		rc = TRUE;

		// Primeiro testo o corrente que é o que tem mais chance de ser "FALSE"
		entry_is_valid(entry,&rc);

		// Verifico toda a lista
		if(rc) {
			g_list_foreach(GTK_V3270FT(widget)->files,(GFunc) entry_is_valid,&rc);
		}

	}

	return rc;

}

struct v3270ft_entry * v3270ft_get_selected(v3270ft *dialog) {

	if(dialog->active)
		return (struct v3270ft_entry *) dialog->active->data;

	return NULL;
}


