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
 * Este programa está nomeado como popup.c e possui - linhas de código.
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

 GObject * ui_create_popup(GtkAction *action,struct parser *info,const gchar **names, const gchar **values, GError **error)
 {
 	GtkWidget	* widget 	= NULL;
 	const gchar * id;
 	int 		  pos		= -1;
 	int			  f;

 	if(info->element)
	{
		*error = g_error_new(ERROR_DOMAIN,EINVAL, _( "<%s> should be on toplevel"), "popup");
		return NULL;
	}

	if(action)
	{
		*error = g_error_new(ERROR_DOMAIN,EINVAL, _( "Unexpected action attribute in <%s>"), "popup");
		return NULL;
	}

	id = ui_get_attribute("group",names,values);
	if(!id)
		id = ui_get_attribute("type",names,values);

	if(!id)
	{
		*error = g_error_new(ERROR_DOMAIN,ENOENT, _( "<%s> requires %s"),"accelerator",_( "a type or group attribute" ) );
		return NULL;
	}

	for(f=0;info->popupname[f] && pos < 0;f++)
	{
		if(!g_strcasecmp(info->popupname[f],id))
		{
			pos = f;
			break;
		}
	}

	if(pos < 0)
	{
		*error = g_error_new(ERROR_DOMAIN,EINVAL,_( "Unknown popup type \"%s\""),id);
		return NULL;
	}
	else
	{
		widget = info->popup[pos];
	}

	if(!widget)
	{
		info->popup[pos] = widget = gtk_menu_new();
		g_object_ref(widget);
		gtk_widget_show_all(widget);
	}

	trace("popup(%s): %p id=\"%s\" (%d)",id,widget,info->popupname[pos],pos);

	return G_OBJECT(ui_insert_element(info, action, UI_ELEMENT_POPUP, names, values, G_OBJECT(widget), error));
 }

 void ui_end_popup(GObject *widget,struct parser *info,GError **error)
 {

 }
