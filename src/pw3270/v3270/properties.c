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
 * Este programa está nomeado como properties.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <gtk/gtk.h>
 #include <lib3270.h>
 #include <lib3270/session.h>
 #include <lib3270/actions.h>
 #include <lib3270/log.h>
 #include <lib3270/macros.h>
 #include <errno.h>
 #include <pw3270/v3270.h>
 #include "private.h"

/*--[ Globals ]--------------------------------------------------------------------------------------*/

  GParamSpec * v3270_properties[PROP_LAST]		= { 0 };

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void v3270_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
 {
	v3270  *window = GTK_V3270(object);

	switch (prop_id)
	{
	case PROP_MODEL:
		lib3270_set_model(window->host,g_value_get_string(value));
		break;

	case PROP_AUTO_DISCONNECT:
		v3270_set_auto_disconnect(GTK_WIDGET(object),g_value_get_int(value));
		break;

	default:
		if(prop_id < (PROP_TOGGLE + LIB3270_TOGGLE_COUNT))
		{
			lib3270_set_toggle(window->host,prop_id - PROP_TOGGLE, (int) g_value_get_boolean (value));
			return;
		}
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}

 }

 static void v3270_get_property(GObject *object,guint prop_id, GValue *value, GParamSpec *pspec)
 {
	v3270  *window = GTK_V3270(object);

	switch (prop_id)
	{
	case PROP_MODEL:
		g_value_set_string(value,lib3270_get_model(window->host));
		break;

	case PROP_AUTO_DISCONNECT:
		g_value_set_int(value,v3270_get_auto_disconnect(GTK_WIDGET(object)));
		break;

	case PROP_LUNAME:
		g_value_set_string(value,lib3270_get_luname(window->host));
		break;

	case PROP_ONLINE:
		g_value_set_boolean(value,lib3270_is_connected(window->host) ? TRUE : FALSE );
		break;

	case PROP_SELECTION:
		g_value_set_boolean(value,lib3270_has_selection(window->host) ? TRUE : FALSE );
		break;

	default:
		if(prop_id < (PROP_TOGGLE + LIB3270_TOGGLE_COUNT))
		{
			g_value_set_boolean(value,lib3270_get_toggle(window->host,prop_id - PROP_TOGGLE) ? TRUE : FALSE );
			return;
		}
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
	}
 }

 void v3270_init_properties(GObjectClass * gobject_class)
 {
 	memset(v3270_properties,0,sizeof(v3270_properties));

	gobject_class->set_property = v3270_set_property;
	gobject_class->get_property = v3270_get_property;

	v3270_properties[PROP_ONLINE] = g_param_spec_boolean(
					"online",
					"online",
					"True if is online",
					FALSE,G_PARAM_READABLE);
	g_object_class_install_property(gobject_class,PROP_ONLINE,v3270_properties[PROP_ONLINE]);

	v3270_properties[PROP_SELECTION] = g_param_spec_boolean(
					"selection",
					"selection",
					"True on selected area",
					FALSE,G_PARAM_READABLE);
	g_object_class_install_property(gobject_class,PROP_SELECTION,v3270_properties[PROP_SELECTION]);

	v3270_properties[PROP_MODEL] = g_param_spec_string(
					"model",
					"model",
					"The model of 3270 display to be emulated",
					FALSE,G_PARAM_READABLE|G_PARAM_WRITABLE);
	g_object_class_install_property(gobject_class,PROP_MODEL,v3270_properties[PROP_MODEL]);

	v3270_properties[PROP_LUNAME] = g_param_spec_string(
					"luname",
					"luname",
					"The logical Unit (LU) name",
					FALSE,G_PARAM_READABLE|G_PARAM_WRITABLE);
	g_object_class_install_property(gobject_class,PROP_LUNAME,v3270_properties[PROP_LUNAME]);


	v3270_properties[PROP_AUTO_DISCONNECT] = g_param_spec_string(
					"auto_disconnect",
					"auto_disconnect",
					"Minutes to disconnect when idle",
					FALSE,G_PARAM_READABLE|G_PARAM_WRITABLE);
	g_object_class_install_property(gobject_class,PROP_AUTO_DISCONNECT,v3270_properties[PROP_AUTO_DISCONNECT]);

	// Toggle properties
	int f;

	for(f=0;f<LIB3270_TOGGLE_COUNT;f++)
	{
		v3270_properties[PROP_TOGGLE+f] = g_param_spec_boolean(lib3270_get_toggle_name(f),lib3270_get_toggle_name(f),lib3270_get_toggle_description(f),FALSE,G_PARAM_WRITABLE|G_PARAM_READABLE);
		g_object_class_install_property(gobject_class,PROP_TOGGLE+f,v3270_properties[PROP_TOGGLE+f]);
	}
	debug("%s",__FUNCTION__);
 }

 LIB3270_EXPORT	void v3270_set_auto_disconnect(GtkWidget *widget, guint minutes)
 {
	g_return_if_fail(GTK_IS_V3270(widget));
 	GTK_V3270(widget)->activity.disconnect = minutes;
 }

 LIB3270_EXPORT guint v3270_get_auto_disconnect(GtkWidget *widget)
 {
	g_return_val_if_fail(GTK_IS_V3270(widget),0);
 	return GTK_V3270(widget)->activity.disconnect;
 }
