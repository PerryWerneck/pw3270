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
 * Este programa está nomeado como accelerator.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <gtk/gtk.h>
 #include "private.h"

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 struct row
 {
 	unsigned short		  pos;
 	unsigned short		  num_cols;
	GList				* cols;
 };

 struct keypad
 {
	struct parser		* parser;
	unsigned short		  num_rows;
	unsigned short	  	  num_cols;
	unsigned short		  col;
	struct row			* row;
	GtkWidget			* box;
	GtkWidget			* handle;
	GtkWidget			* table;
	UI_ATTR_DIRECTION	  pos;
	GList				* rows;
 };

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void row_start(struct keypad *keypad, const gchar **names,const gchar **values, GError **error)
 {
	keypad->row = g_malloc0(sizeof(struct row));

	keypad->row->pos = ++keypad->num_rows;
	keypad->col	= 0;

	keypad->rows = g_list_append(keypad->rows,keypad->row);
 }

 static void button_start(struct keypad *keypad, const gchar **names,const gchar **values, GError **error)
 {
 	const gchar *label	= ui_get_attribute("label", names, values);
 	const gchar *icon	= ui_get_attribute("icon", names, values);
 	GtkWidget	*widget	= NULL;

	if(++keypad->col > keypad->num_cols)
		keypad->num_cols = keypad->col;

	keypad->row->num_cols++;

	if(label)
	{
		widget = gtk_button_new_with_label(gettext(g_strcompress(label)));
	}
	else if(icon)
	{
		gchar *text = g_strconcat("gtk-",icon,NULL);
		widget = gtk_button_new();
		gtk_container_add(GTK_CONTAINER(widget),gtk_image_new_from_stock(text,GTK_ICON_SIZE_SMALL_TOOLBAR));
		g_free(text);
	}

	keypad->row->cols = g_list_append(keypad->row->cols,widget);
 }

 static void element_start(GMarkupParseContext *context, const gchar *element_name, const gchar **names,const gchar **values, struct keypad *keypad, GError **error)
 {
 	static const struct _cmd
 	{
		const gchar *element_name;
		void (*start)(struct keypad *, const gchar **, const gchar **, GError **);
 	} cmd[] =
 	{
 		{ "row",	row_start		},
 		{ "button",	button_start	},
 	};

 	int f;

 	for(f = 0; f < G_N_ELEMENTS(cmd); f++)
	{
		if(!g_strcasecmp(cmd[f].element_name,element_name))
		{
			cmd[f].start(keypad,names,values,error);
			return;
		}
	}

	*error = g_error_new(ERROR_DOMAIN,EINVAL, _( "Unexpected element <%s>"), element_name);
 }

 static void element_end(GMarkupParseContext *context, const gchar *element_name, struct keypad *keypad, GError **error)
 {
// 	trace("%s: %s",__FUNCTION__,element_name);
 }

 GObject * ui_create_keypad(GMarkupParseContext *context,GtkAction *action,struct parser *info,const gchar **names, const gchar **values, GError **error)
 {
	static const GMarkupParser parser =
	{
		(void (*)(GMarkupParseContext *, const gchar *, const gchar **, const gchar **, gpointer, GError **))
				element_start,
		(void (*)(GMarkupParseContext *, const gchar *, gpointer, GError **))
				element_end,
		(void (*)(GMarkupParseContext *, const gchar *, gsize, gpointer, GError **))
				NULL,

//		(void (*)(GMarkupParseContext *, GError *, gpointer))
		NULL

	};

 	const gchar *label	= ui_get_attribute("label", names, values);
 	const gchar *name	= ui_get_attribute("name", names, values);

	struct keypad *keypad;

 	if(info->element)
	{
		*error = g_error_new(ERROR_DOMAIN,EINVAL, _( "<%s> should be on toplevel"), "keypad");
		return NULL;
	}

	if(action)
	{
		*error = g_error_new(ERROR_DOMAIN,EINVAL,_( "action attribute is invalid for <%s>"),"keypad");
		return NULL;
	}

	info->block_data = keypad = g_malloc0(sizeof(struct keypad));

	keypad->parser 		= info;
	keypad->handle		= gtk_handle_box_new();
	keypad->pos			= ui_get_dir_attribute(names,values);

	switch(keypad->pos)
	{
	case UI_ATTR_UP:
		keypad->box = gtk_vbox_new(FALSE,0);
		gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(keypad->handle),GTK_POS_BOTTOM);
		break;

	case UI_ATTR_DOWN:
		keypad->box = gtk_vbox_new(FALSE,0);
		gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(keypad->handle),GTK_POS_TOP);
		break;

	case UI_ATTR_LEFT:
		keypad->box = gtk_hbox_new(FALSE,0);
		gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(keypad->handle),GTK_POS_RIGHT);
		break;

	default:
		keypad->pos = UI_ATTR_RIGHT;
		keypad->box = gtk_hbox_new(FALSE,0);
		gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(keypad->handle),GTK_POS_LEFT);

	}

	if(name)
		gtk_widget_set_name(keypad->handle,name);

	if(label)
		g_object_set_data_full(G_OBJECT(keypad->handle),"keypad_label",g_strdup(label),g_free);

	gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(keypad->handle),GTK_SHADOW_ETCHED_IN);
    gtk_container_add(GTK_CONTAINER(keypad->handle),keypad->box);

	g_markup_parse_context_push(context,(GMarkupParser *) &parser,keypad);

	return G_OBJECT(ui_insert_element(info, action, UI_ELEMENT_KEYPAD, names, values, G_OBJECT(keypad->handle), error));
 }

 static void create_col(GtkWidget *widget, struct keypad *keypad)
 {
	if(widget)
	{
		gtk_widget_show_all(widget);
		gtk_table_attach(	GTK_TABLE(keypad->table),
							widget,
							keypad->num_cols,keypad->num_cols+1,
							keypad->num_rows,keypad->num_rows+1,
							GTK_EXPAND|GTK_FILL,GTK_EXPAND|GTK_FILL,0,0 );

	}
	keypad->num_cols++;

 }

 static void create_row(struct row *info, struct keypad *keypad)
 {
 	if(info->cols)
	{
		keypad->num_cols = 0;
		g_list_foreach(info->cols,(GFunc) create_col,keypad);
		g_list_free(info->cols);
	}
	keypad->num_rows++;
 }

 void ui_end_keypad(GMarkupParseContext *context,GObject *widget,struct parser *info,GError **error)
 {
	struct keypad *keypad  = (struct keypad *) info->block_data;
	info->block_data = NULL;

	keypad->num_cols *= 2;

	if(keypad->rows)
	{
		// Create Widgets & Release memory
		keypad->table = gtk_table_new(keypad->num_rows,keypad->num_cols,FALSE);

#if GTK_CHECK_VERSION(2,18,0)
		gtk_widget_set_can_focus(keypad->table,FALSE);
		gtk_widget_set_can_default(keypad->table,FALSE);
#else
		GTK_WIDGET_UNSET_FLAGS(keypad->table,GTK_CAN_FOCUS);
		GTK_WIDGET_UNSET_FLAGS(keypad->table,GTK_CAN_DEFAULT);
#endif // GTK(2,18)

		keypad->num_cols = keypad->num_rows = 0;
		g_list_foreach(keypad->rows,(GFunc) create_row,keypad);
		g_list_free_full(keypad->rows,g_free);
		gtk_box_pack_start(GTK_BOX(keypad->box),keypad->table,FALSE,FALSE,0);

		gtk_widget_show_all(keypad->box);
		gtk_widget_show_all(keypad->table);
		gtk_widget_show_all(keypad->handle);
	}

	g_free(keypad);
 	g_markup_parse_context_pop(context);
 }
