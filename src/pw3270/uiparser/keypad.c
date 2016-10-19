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
 #include <stdlib.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void element_start(GMarkupParseContext *context, const gchar *element_name, const gchar **names,const gchar **values, struct keypad *keypad, GError **error)
 {
	int			  width 	= 1;
	int			  height	= 1;
	const gchar	* tmp;

 	trace("%s(%s,%d,%d)",__FUNCTION__,element_name,(int) keypad->row, (int) keypad->col);

 	keypad->widget = NULL;

 	if(!strcasecmp(element_name,"button")) {
		keypad_button_start(context, names, values, error, keypad);
 	}

	tmp = ui_get_attribute("width", names, values);
	if(tmp) {
		width = atoi(tmp);
	}

	tmp = ui_get_attribute("height", names, values);
	if(tmp) {
		height = atoi(tmp);
	}

 	if(keypad->widget) {

		// Criou widget, incluir
		tmp = ui_get_attribute("column", names, values);
		if(tmp) {
			keypad->col = atoi(tmp);
		}

#if GTK_CHECK_VERSION(3,0,0)

		gtk_grid_attach(keypad->grid,keypad->widget,keypad->col,keypad->row,width,height);

#else
		guint r = keypad->rows, c = keypad->cols;

		if(r < keypad->row || c < (keypad->col+1)) {
			trace("Resize to %u,%u to %u,%u",r,c,keypad->row,keypad->col+1);
			gtk_table_resize(keypad->grid,keypad->rows = keypad->row,keypad->cols = (keypad->col+1));
		}

		r = keypad->row-1;
		c = keypad->col;

		gtk_table_attach(	keypad->grid,
							keypad->widget,
							c,c+width,
							r,r+height,
							GTK_EXPAND|GTK_FILL,
							GTK_EXPAND|GTK_FILL,
							1,1);
#endif

		keypad->widget = NULL;

 	}

 	if(!strcasecmp(element_name,"row")) {
		keypad->row += height;
		keypad->col = 0;
 	} else {
		keypad->col += width;
 	}

 }

 static void element_end(GMarkupParseContext *context, const gchar *element_name, struct keypad *keypad, GError **error)
 {

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

 GtkPositionType ui_get_position_attribute(const gchar **names, const gchar **values)
 {
	static const struct _pos {
		GtkPositionType	  type;
		const gchar		* name;
	} pos [] = {

		{	GTK_POS_LEFT,	"left"		},
		{	GTK_POS_RIGHT,	"right"		},
		{	GTK_POS_TOP,	"top"		},
		{	GTK_POS_BOTTOM,	"bottom"	},

	};

	const gchar	* dir		= ui_get_attribute("position",names,values);
	int			  f;

	if(dir)
	{
		for(f=0;f<G_N_ELEMENTS(pos);f++)
		{
			if(!g_ascii_strcasecmp(dir,pos[f].name))
				return pos[f].type;
		}
	}

	return GTK_POS_TOP;
 }

 static void element_text(GMarkupParseContext *context, const gchar *text, gsize sz, struct keypad *keypad, GError **error)
 {
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
	keypad->pos			= ui_get_position_attribute(names,values);
	keypad->relief		= ui_get_relief(names, values, GTK_RELIEF_NORMAL);

#if GTK_CHECK_VERSION(3,0,0)
	keypad->grid		= GTK_GRID(gtk_grid_new());
	gtk_grid_set_row_homogeneous(keypad->grid,TRUE);
	gtk_grid_set_column_homogeneous(keypad->grid,TRUE);
#else
	keypad->rows	= 1;
	keypad->cols	= 1;
	keypad->grid	= GTK_TABLE(gtk_table_new(keypad->rows,keypad->cols,TRUE));
#endif	// GTK3

	g_object_set_data(G_OBJECT(keypad->grid),"position",(gpointer) keypad->pos);

	label = ui_get_attribute("label",names,values);
	if(label)
	{
		// Keypad has label, create and setup an action
		const gchar *name = ui_get_attribute("name",names,values);

		trace("%s name=%s",__FUNCTION__,name);

		if(name)
		{
			GtkToggleAction *action = gtk_toggle_action_new(name,gettext(label),NULL,NULL);
			ui_action_set_options(GTK_ACTION(action),info,names,values,error);
			g_object_set_data_full(G_OBJECT(keypad->grid),"view_action",action,g_object_unref);
			g_signal_connect(action,"toggled",G_CALLBACK(toggled),keypad->grid);
			gtk_widget_set_name(GTK_WIDGET(keypad->grid),name);
		}
	}

	g_markup_parse_context_push(context,(GMarkupParser *) &parser,keypad);

	return G_OBJECT(ui_insert_element(info, action, UI_ELEMENT_KEYPAD, names, values, G_OBJECT(keypad->grid), error));
 }

 void ui_end_keypad(GMarkupParseContext *context,GObject *widget,struct parser *info,GError **error)
 {
	struct keypad *keypad  = (struct keypad *) info->block_data;
	info->block_data = NULL;

	gtk_widget_show_all(GTK_WIDGET(keypad->grid));

	g_free(keypad);
 	g_markup_parse_context_pop(context);
 }
