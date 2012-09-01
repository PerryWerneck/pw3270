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
 * Este programa está nomeado como action.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

 #include <stdlib.h>
 #include "private.h"

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static gboolean get_boolean(const gchar *value)
 {
 	if(!g_ascii_strcasecmp(value,"yes") || atoi(value))
		return TRUE;
	return FALSE;
 }

 static gboolean check_internal_attribute(const gchar *name)
 {
	static const gchar *ignored[] = { "target", "direction", "sysmenu" };
	int p;

	for(p=0;p< G_N_ELEMENTS(ignored);p++)
	{
		if(!g_ascii_strcasecmp(name,ignored[p]))
			return FALSE;
	}
	return TRUE;
 }

 void ui_action_set_options(GtkAction *action, struct parser *info, const gchar **name, const gchar **value, GError **error)
 {
	int f;

	for(f=0;name[f];f++)
	{
		if(!g_ascii_strcasecmp(name[f],"group"))
		{
			int id = -1;
			int p;

			for(p=0;info->group[p] && id == -1;p++)
			{
				if(!g_ascii_strcasecmp(value[f],info->group[p]))
					id = p;
			}

			if(id == -1)
			{
				*error = g_error_new(ERROR_DOMAIN,EINVAL,_( "Unexpected group \"%s\""),value[f]);
				return;
			}

			g_object_set_data(G_OBJECT(action),"id_group",GINT_TO_POINTER(id));

		}
		else if(!g_ascii_strcasecmp(name[f],"icon"))
		{
			gchar * stock = g_strconcat("gtk-",value[f],NULL);
			gtk_action_set_stock_id(action,stock);
			g_free(stock);
		}
		else if(!g_ascii_strcasecmp(name[f],"sensitive"))
		{
			gtk_action_set_sensitive(action,get_boolean(value[f]));
		}
		else if(!g_ascii_strcasecmp(name[f],"label"))
		{
			gtk_action_set_label(action,gettext(value[f]));
		}
		else if(!g_ascii_strcasecmp(name[f],"short-label"))
		{
			gtk_action_set_short_label(action,gettext(value[f]));
		}
		else if(!g_ascii_strcasecmp(name[f],"tooltip"))
		{
			gtk_action_set_tooltip(action,gettext(value[f]));
		}
		else if(!g_ascii_strcasecmp(name[f],"important"))
		{
			gtk_action_set_is_important(action,get_boolean(value[f]));
		}
		else if(!g_ascii_strcasecmp(name[f],"key"))
		{
			g_object_set_data_full(G_OBJECT(action),"accel_attr",g_strdup(value[f]),g_free);
		}
		else if(!g_ascii_strcasecmp(name[f],"id"))
		{
			g_object_set_data(G_OBJECT(action),"action_id",GINT_TO_POINTER(atoi(value[f])));
		}
		else if(check_internal_attribute(name[f]))
		{
			g_object_set_data(G_OBJECT(action),name[f],g_string_chunk_insert_const(info->strings,value[f]));
		}
	}
 }
