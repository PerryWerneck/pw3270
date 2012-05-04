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
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como menubar.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

 #include "private.h"

/*--[ Implement ]------------------------------------------------------------------------------------*/

 GObject * ui_create_separator(GtkAction *action,struct parser *info,const gchar **names, const gchar **values, GError **error)
 {
 	GtkWidget *widget = NULL;

/*
 	trace("%s label=%s parent=%p menuitem=%s menu=%s menushell=%s",
				__FUNCTION__,
				ui_get_attribute("label",names,values),
				info->element,
				GTK_IS_MENU_ITEM(info->element) ? "Yes" : "No",
				GTK_IS_MENU(info->element) ? "Yes" : "No",
				GTK_IS_MENU_SHELL(info->element) ? "Yes" : "No"
			 );
*/

	if(!info->element)
	{
		*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s", _( "<separator> should be inside a <menu> or <toolbar>"));
		return NULL;
	}

	if(GTK_IS_TOOLBAR(info->element))
	{
		widget = GTK_WIDGET(gtk_separator_tool_item_new());

		gtk_separator_tool_item_set_draw(
					GTK_SEPARATOR_TOOL_ITEM(widget),
					ui_get_bool_attribute("draw",names,values,TRUE));

		gtk_toolbar_insert(GTK_TOOLBAR(info->element),GTK_TOOL_ITEM(widget),-1);
	}
	else if(GTK_IS_MENU_ITEM(info->element))
	{
		GtkWidget *menu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(info->element));
		if(!menu)
		{
			menu = gtk_menu_new();
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(GTK_MENU_ITEM(info->element)),menu);
		}

		widget = gtk_separator_menu_item_new();

		gtk_menu_shell_append((GtkMenuShell *) menu, widget);

	}
	else if(GTK_IS_MENU_ITEM(info->element))
	{
		widget = gtk_separator_menu_item_new();
		gtk_menu_shell_append((GtkMenuShell *) info->element, widget);
	}
	else if(GTK_IS_MENU(info->element))
	{
		widget = gtk_separator_menu_item_new();
		gtk_menu_shell_append((GtkMenuShell *) info->element, widget);
	}
	else
	{
		*error = g_error_new(ERROR_DOMAIN,EINVAL,_( "<%s> is invalid at this context"),"separator");
		return NULL;
	}

	g_object_set_data(G_OBJECT(widget),"parent",info->element);

	return G_OBJECT(widget);
 }

 void ui_end_separator(GObject *widget,struct parser *info,GError **error)
 {
 }


