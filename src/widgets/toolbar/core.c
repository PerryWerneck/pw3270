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

 #include <config.h>
 #include <pw3270/toolbar.h>

 struct _pw3270ToolBar {
 	GtkToolbar parent;

 };

 struct _pw3270ToolBarClass {

	GtkToolbarClass parent_class;


 };

 G_DEFINE_TYPE(pw3270ToolBar, pw3270ToolBar, GTK_TYPE_TOOLBAR);

 static void pw3270ToolBar_class_init(pw3270ToolBarClass *klass) {

 }

 static void pw3270ToolBar_init(pw3270ToolBar *widget) {

 }

 GtkWidget * pw3270_toolbar_new(void) {
	return g_object_new(PW3270_TYPE_TOOLBAR, NULL);
 }

 GtkWidget * pw3270_toolbar_insert_lib3270_action(GtkWidget *toolbar, const LIB3270_ACTION *action, gint pos) {

	g_return_val_if_fail(GTK_IS_TOOLBAR(toolbar),NULL);

	if(!action) {
		g_message("Invalid action identifier");
		return NULL;
	}

	if(!action->icon) {
		g_message("Action \"%s\" doesn't have an icon", action->name);
		return NULL;
	}

	if(!action->label) {
		g_message("Action \"%s\" doesn't have a label", action->name);
		return NULL;
	}

	GtkToolItem * item = gtk_tool_button_new(gtk_image_new_from_icon_name(action->icon,GTK_ICON_SIZE_LARGE_TOOLBAR),action->label);

	if(action->summary)
		gtk_tool_item_set_tooltip_text(item,action->summary);

	gtk_toolbar_insert(GTK_TOOLBAR(toolbar), item, pos);

	return GTK_WIDGET(item);
 }

