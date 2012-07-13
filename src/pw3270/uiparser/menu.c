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
 * Este programa está nomeado como menu.c e possui - linhas de código.
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

 GObject * ui_create_menu(GMarkupParseContext *context,GtkAction *action,struct parser *info,const gchar **names, const gchar **values, GError **error)
 {
 	GtkWidget		* widget;
	GtkWidget 		* menu = GTK_WIDGET(info->element);

	if(!(info->element && (GTK_IS_MENU_ITEM(info->element) || GTK_IS_MENU_SHELL(info->element))))
	{
		*error = g_error_new(ERROR_DOMAIN,EINVAL,_( "<%s> is invalid at this context"),"menu");
		return NULL;
	}

	if(action)
	{
		widget = gtk_action_create_menu_item(action);
	}
	else
	{
		const gchar *label = ui_get_attribute("label",names,values);

		if(!label)
		{
			*error = g_error_new(ERROR_DOMAIN,ENOENT, _( "<%s> requires %s"),"menu", _( "a label attribute" ) );
			return NULL;
		}
		widget = gtk_menu_item_new_with_mnemonic(gettext(label));
	}

	if(GTK_IS_MENU_ITEM(menu))
	{
		menu = gtk_menu_item_get_submenu(GTK_MENU_ITEM(info->element));
		if(!menu)
		{
			menu = gtk_menu_new();
			gtk_menu_item_set_submenu(GTK_MENU_ITEM(GTK_MENU_ITEM(info->element)),menu);
		}
	}

	gtk_menu_shell_append((GtkMenuShell *) menu, widget);

//	trace("%s: %s=%p",__FUNCTION__,gtk_widget_get_name(widget),widget);

	return ui_insert_element(info, action, UI_ELEMENT_MENU, names, values, G_OBJECT(widget), error);
 }

 void ui_end_menu(GMarkupParseContext *context,GObject *widget,struct parser *info,GError **error)
 {
 }
