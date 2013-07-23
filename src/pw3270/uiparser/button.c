
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
 * Este programa está nomeado como button.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "keypad.h"
 #include <pw3270/v3270.h>

/*--[ Globals ]--------------------------------------------------------------------------------------*/


/*--[ Implement ]------------------------------------------------------------------------------------*/

 GtkReliefStyle ui_get_relief(const gchar **names, const gchar **values, GtkReliefStyle def)
 {

 	const gchar *name = ui_get_attribute("relief",names,values);
 	if(name)
	{
		static const struct _style
		{
			GtkReliefStyle	  val;
			const gchar		* name;
		} style[] =
		{
			{ GTK_RELIEF_NORMAL,	"normal" 	},
			{ GTK_RELIEF_HALF,		"half"		},
			{ GTK_RELIEF_NONE,		"none"		}
		};

		int f;

		for(f=0;f<G_N_ELEMENTS(style);f++)
		{
			if(!g_ascii_strcasecmp(style[f].name,name))
				return style[f].val;
		}
	}

 	return def;
 }

 static void button_clicked(GtkButton *button, GtkAction *action)
 {
	gtk_action_activate(action);
 }

 static void button_script(GtkButton *button, GtkWidget *widget)
 {
	v3270_run_script(widget,g_object_get_data(G_OBJECT(button),"script_text"));
 }

 void keypad_button_start(GMarkupParseContext *context, const gchar **names,const gchar **values, GError **error, struct keypad *keypad)
 {
 	const gchar		* label		= ui_get_attribute("label", names, values);
 	const gchar		* icon		= ui_get_attribute("icon", names, values);
 	const gchar		* name		= ui_get_attribute("action", names, values);
	struct parser	* info		= keypad->parser;
 	GtkAction		* action	= NULL;
 	GtkWidget		* widget	= NULL;

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

	if(!widget)
		return;

#if GTK_CHECK_VERSION(2,18,0)
	gtk_widget_set_can_focus(widget,FALSE);
	gtk_widget_set_can_default(widget,FALSE);
#else
	GTK_WIDGET_UNSET_FLAGS(widget,GTK_CAN_FOCUS);
	GTK_WIDGET_UNSET_FLAGS(widget,GTK_CAN_DEFAULT);
#endif // GTK(2,18)

	gtk_button_set_relief(GTK_BUTTON(widget),ui_get_relief(names, values, keypad->relief));
	gtk_button_set_alignment(GTK_BUTTON(widget),0.5,0.5);
	gtk_button_set_focus_on_click(GTK_BUTTON(widget),FALSE);

	if(name)
		action = ui_get_action(info->center_widget,name,info->actions,names,values,error);

	if(action)
	{
		ui_action_set_options(action,info,names,values,error);
		g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(button_clicked),action);
	}
	else
	{
		keypad->widget = widget;
		gtk_widget_set_sensitive(widget,FALSE);
		g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(button_script),info->center_widget);
	}
 }

