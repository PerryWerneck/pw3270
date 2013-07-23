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
 * Este programa está nomeado como keypad.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "keypad.h"

/*--[ Implement ]------------------------------------------------------------------------------------*/

 void keypad_row_start(GMarkupParseContext *context, const gchar **names,const gchar **values, GError **error, struct keypad *keypad)
 {
	keypad->row = g_malloc0(sizeof(struct row));

	keypad->row->pos = ++keypad->num_rows;
	keypad->col	= 0;

	keypad->rows = g_list_append(keypad->rows,keypad->row);
 }

 static void element_start(GMarkupParseContext *context, const gchar *element_name, const gchar **names,const gchar **values, struct keypad *keypad, GError **error)
 {
 	static const struct _cmd
 	{
		const gchar *element_name;
		void (*start)(GMarkupParseContext *, const gchar **,const gchar **, GError **, struct keypad *);
 	} cmd[] =
 	{
 		{ "row",	keypad_row_start	},
 		{ "button",	keypad_button_start	},
 	};

 	int f;

 	for(f = 0; f < G_N_ELEMENTS(cmd); f++)
	{
		if(!g_ascii_strcasecmp(cmd[f].element_name,element_name))
		{
			cmd[f].start(context,names,values,error,keypad);
			return;
		}
	}

	*error = g_error_new(ERROR_DOMAIN,EINVAL, _( "Unexpected element <%s>"), element_name);
 }

 static void element_end(GMarkupParseContext *context, const gchar *element_name, struct keypad *keypad, GError **error)
 {
 	keypad->widget = NULL;
// 	trace("%s: %s",__FUNCTION__,element_name);
 }

 static void toggled(GtkToggleAction *action, GtkWidget *widget)
 {
 	gboolean active = gtk_toggle_action_get_active(action);
	set_boolean_to_config("view",gtk_action_get_name(GTK_ACTION(action)),active);
#if GTK_CHECK_VERSION(2,18,0)
 	gtk_widget_set_visible(widget,active);
#else
	if(active)
		gtk_widget_show(widget);
	else
		gtk_widget_hide(widget);
#endif // GTK(2,18,0)

 }

 UI_ATTR_DIRECTION ui_get_position_attribute(const gchar **names, const gchar **values)
 {
 	static const gchar	* posname[]	= { "top", "bottom", "left", "right" };
	const gchar			* dir		= ui_get_attribute("position",names,values);
	int					  f;

	if(dir)
	{
		for(f=0;f<G_N_ELEMENTS(posname);f++)
		{
			if(!g_ascii_strcasecmp(dir,posname[f]))
				return f;
		}
	}

	return UI_ATTR_DIRECTION_NONE;
 }

 static void element_text(GMarkupParseContext *context, const gchar *text, gsize sz, struct keypad *keypad, GError **error)
 {
		if(keypad->widget)
		{
			gchar *base = g_strstrip(g_strdup(text));
			gchar *text = g_strdup(base);
			g_free(base);

			if(*text)
			{
				gtk_widget_set_sensitive(keypad->widget,TRUE);
				g_object_set_data_full(G_OBJECT(keypad->widget),"script_text",text,g_free);
			}
			else
			{
				g_free(text);
			}

		}

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
				element_text,

//		(void (*)(GMarkupParseContext *, GError *, gpointer))
		NULL

	};

	struct keypad	* keypad;
 	const gchar		* label		= NULL;

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
	keypad->pos			= ui_get_position_attribute(names,values);
	keypad->relief		= ui_get_relief(names, values, GTK_RELIEF_NORMAL);

	switch(keypad->pos)
	{
	case UI_ATTR_UP:
#if GTK_CHECK_VERSION(3,0,0)
        keypad->box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
#else
		keypad->box = gtk_vbox_new(FALSE,0);
#endif // GTK(3,0,0)
		gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(keypad->handle),GTK_POS_BOTTOM);
		break;

	case UI_ATTR_DOWN:
#if GTK_CHECK_VERSION(3,0,0)
        keypad->box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
#else
		keypad->box = gtk_vbox_new(FALSE,0);
#endif // GTK(3,0,0)
		gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(keypad->handle),GTK_POS_TOP);
		break;

	case UI_ATTR_LEFT:
#if GTK_CHECK_VERSION(3,0,0)
        keypad->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
#else
		keypad->box = gtk_hbox_new(FALSE,0);
#endif // GTK(3,0,0)
		gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(keypad->handle),GTK_POS_RIGHT);
		break;

	default:
#if GTK_CHECK_VERSION(3,0,0)
        keypad->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
#else
		keypad->box = gtk_hbox_new(FALSE,0);
#endif // GTK(3,0,0)
		keypad->pos = UI_ATTR_RIGHT;
		gtk_handle_box_set_handle_position(GTK_HANDLE_BOX(keypad->handle),GTK_POS_LEFT);

	}

	label = ui_get_attribute("label",names,values);
	if(label)
	{
		// Keypad has label, create and setup an action
		const gchar *name = ui_get_attribute("name",names,values);
		if(name)
		{
			GtkToggleAction *action = gtk_toggle_action_new(name,gettext(label),NULL,NULL);
			ui_action_set_options(GTK_ACTION(action),info,names,values,error);
			g_object_set_data_full(G_OBJECT(keypad->handle),"view_action",action,g_object_unref);
			g_signal_connect(action,"toggled",G_CALLBACK(toggled),keypad->handle);
		}
	}

	gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(keypad->handle),GTK_SHADOW_ETCHED_IN);
    gtk_container_add(GTK_CONTAINER(keypad->handle),keypad->box);

	g_markup_parse_context_push(context,(GMarkupParser *) &parser,keypad);

	return G_OBJECT(ui_insert_element(info, action, UI_ELEMENT_KEYPAD, names, values, G_OBJECT(keypad->handle), error));
 }

 static void create_col(GtkWidget *widget, struct keypad *keypad)
 {
	if(widget)
	{
		gtk_table_attach(	GTK_TABLE(keypad->table),
							widget,
							keypad->col,keypad->col+keypad->button_width,
							keypad->num_rows,keypad->num_rows+1,
							GTK_EXPAND|GTK_FILL,GTK_EXPAND|GTK_FILL,0,0 );

	}
	keypad->col += keypad->button_width;

 }

 static void create_row(struct row *info, struct keypad *keypad)
 {
 	if(info->cols)
	{
		keypad->col = 0;
		keypad->button_width = keypad->num_cols / info->num_cols;

//		trace("Max cols=%d row cols=%d width=%d",keypad->num_cols,info->num_cols,keypad->button_width);

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

		keypad->num_rows = 0;
		g_list_foreach(keypad->rows,(GFunc) create_row,keypad);
#if GTK_CHECK_VERSION(2,28,0)
		g_list_free_full(keypad->rows,g_free);
#else
		g_list_foreach(keypad->rows,(GFunc) g_free,NULL);
		g_list_free(keypad->rows);
#endif // GTK(2,28)
		gtk_box_pack_start(GTK_BOX(keypad->box),keypad->table,FALSE,FALSE,0);

		gtk_widget_show_all(keypad->box);
	}

	g_free(keypad);
 	g_markup_parse_context_pop(context);
 }
