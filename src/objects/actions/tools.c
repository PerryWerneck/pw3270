/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes paul.mattes@case.edu), de emulação de terminal 3270 para acesso a
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
#include <v3270.h>

gchar * g_action_get_text(GAction *action, const gchar * property_name) {
	gchar *rc = NULL;

	GValue value = G_VALUE_INIT;
	g_value_init(&value, G_TYPE_STRING);
	g_object_get_property(G_OBJECT(action),property_name,&value);

	const gchar * text = g_value_get_string(&value);
	if(text)
		rc = g_strdup(text);

	g_value_unset(&value);

	return rc;

}

gchar * g_action_get_tooltip(GAction *action) {
	return g_action_get_text(action, "tooltip");
}

gchar * g_action_get_label(GAction *action) {
	return g_action_get_text(action, "label");
}

gchar * g_action_get_icon_name(GAction *action) {
	return g_action_get_text(action, "icon-name");
}

static GdkPixbuf * pixbuf_from_icon_name(GValue *value, gint width, gint G_GNUC_UNUSED(height), GtkIconLookupFlags flags) {

	const gchar * icon_name = g_value_get_string(value);

	if(!icon_name)
		return NULL;

	return gtk_icon_theme_load_icon(
	           gtk_icon_theme_get_default(),
	           icon_name,
	           width,
	           flags, // GTK_ICON_LOOKUP_GENERIC_FALLBACK,
	           NULL
	       );

}

GdkPixbuf * g_action_get_pixbuf(GAction *action, GtkIconSize icon_size, GtkIconLookupFlags flags) {

	struct Properties {
		const gchar * name;
		GType value_type;
		GdkPixbuf * (*translate)(GValue *value, gint width, gint height, GtkIconLookupFlags flags);
	} properties[] = {
		{
			.name = "icon-name",
			.value_type = G_TYPE_STRING,
			.translate = pixbuf_from_icon_name
		}
	};

	size_t ix;
	GdkPixbuf * pixbuf = NULL;
	gint width, height;

	gtk_icon_size_lookup(icon_size,&width,&height);

	for(ix = 0; ix < G_N_ELEMENTS(properties) && !pixbuf; ix++) {

		GParamSpec *spec = g_object_class_find_property(G_OBJECT_GET_CLASS(action),properties[ix].name);
		if(spec && spec->value_type == properties[ix].value_type && (spec->flags & G_PARAM_READABLE) != 0) {

			GValue value = G_VALUE_INIT;
			g_value_init(&value, properties[ix].value_type);

			g_object_get_property(G_OBJECT(action),properties[ix].name,&value);

			pixbuf = properties[ix].translate(&value,width,height,flags);

			g_value_unset(&value);

		}

	}

	return pixbuf;
}
