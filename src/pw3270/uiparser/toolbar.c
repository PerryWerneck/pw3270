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
 * Este programa está nomeado como toolbar.c e possui - linhas de código.
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

 GObject * ui_create_toolbar(GtkAction *action,struct parser *info,const gchar **names, const gchar **values, GError **error)
 {
 	GtkWidget	* widget	= NULL;

 	if(info->element)
	{
		*error = g_error_new(ERROR_DOMAIN,EINVAL, _( "<%s> should be on toplevel"), "toolbar");
		return NULL;
	}

	if(action)
	{
		*error = g_error_new(ERROR_DOMAIN,EINVAL, _( "Unexpected action attribute in <%s>"), "toolbar");
		return NULL;
	}

	widget = gtk_toolbar_new();
	gtk_widget_set_can_focus(widget,FALSE);

	if(ui_get_attribute("label",names,values))
	{
		// Toolbar has label, create and setup an action
		const gchar *name = ui_get_attribute("name",names,values);
		if(name)
		{
			GtkToggleAction *action = gtk_toggle_action_new(name,NULL,NULL,NULL);
			ui_action_set_options(GTK_ACTION(action),info,names,values,error);
			g_object_set_data_full(G_OBJECT(widget),"view_action",action,g_object_unref);
			g_signal_connect(action,"toggled",G_CALLBACK(toggled),widget);
		}
	}


	return G_OBJECT(ui_insert_element(info, action, UI_ELEMENT_TOOLBAR, names, values, G_OBJECT(widget), error));

 }

 void ui_end_toolbar(GObject *widget,struct parser *info,GError **error)
 {
 }
