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
 * Este programa está nomeado como toolitem.c e possui - linhas de código.
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

 GObject * ui_create_toolitem(GtkAction *action,struct parser *info,const gchar **names, const gchar **values, GError **error)
 {
 	GtkWidget *widget = NULL;

	if(!(info->element && (GTK_IS_TOOLBAR(info->element))))
	{
		*error = g_error_new(ERROR_DOMAIN,EINVAL,_( "<%s> is invalid at this context"),"toolitem");
		return NULL;
	}

	if(action)
	{
		widget = gtk_action_create_tool_item(action);
	}
	else
	{
		const gchar *icon = ui_get_attribute("icon",names,values);
		if(icon)
		{
			gchar * stock = g_strconcat("gtk-",icon,NULL);
			widget = GTK_WIDGET(gtk_tool_button_new_from_stock(stock));
			g_free(stock);
		}
		else
		{
			const gchar *label = ui_get_attribute("label",names,values);
			widget = GTK_WIDGET(gtk_tool_button_new(NULL,label));
		}
	}

	gtk_widget_set_can_focus(widget,FALSE);

	gtk_toolbar_insert(GTK_TOOLBAR(info->element),GTK_TOOL_ITEM(widget),-1);

	return G_OBJECT(ui_insert_element(info, action, UI_ELEMENT_TOOLITEM, names, values, G_OBJECT(widget), error));
 }

 void ui_end_toolitem(GObject *widget,struct parser *info,GError **error)
 {
 }

