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

/*---[ Implement ]----------------------------------------------------------------------------------*/

 static void create_child(const KeypadElement *element, GtkGrid *grid) {

 	GtkWidget * button;

 	if(element->icon_name) {
		button = gtk_button_new_from_icon_name(element->icon_name,GTK_ICON_SIZE_SMALL_TOOLBAR);
 	} else if(element->label) {
 		g_autofree gchar * label = g_strcompress(element->label);
		button = gtk_button_new_with_label(label);
	} else {
		button = gtk_button_new();
 	}

 	gtk_button_set_relief(GTK_BUTTON(button),GTK_RELIEF_NORMAL);
	gtk_widget_set_can_focus(button,FALSE);
	gtk_widget_set_can_default(button,FALSE);
	gtk_widget_set_focus_on_click(button,FALSE);

  	gtk_grid_attach(
		grid,
		button,
		element->col,element->row,
		element->width,element->height
	);

 }

 GtkWidget * pw3270_keypad_get_from_model(GObject *model) {

	g_return_val_if_fail(PW_IS_KEYPAD_MODEL(model),NULL);

	GtkWidget * grid = gtk_grid_new();
	gtk_grid_set_column_homogeneous(GTK_GRID(grid),TRUE);
	gtk_grid_set_row_homogeneous(GTK_GRID(grid),TRUE);

	g_list_foreach(PW_KEYPAD_MODEL(model)->elements,(GFunc) create_child, grid);

	return grid;
 }
