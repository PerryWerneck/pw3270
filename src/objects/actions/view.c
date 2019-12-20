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
 * Este programa está nomeado como - e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 /**
  * @brief Implement PW3270 Action view widget and related tools.
  *
  */

 #include <config.h>
 #include <pw3270.h>
 #include <pw3270/actions.h>
 #include <lib3270/log.h>

 struct ListElement {
 	GAction		* action;
 	GtkImage	* image;
 	gchar		  name[1];
 };

 GtkWidget * pw3270_action_view_new() {

	GtkWidget * view = GTK_WIDGET(gtk_tree_view_new_with_model(GTK_TREE_MODEL(gtk_list_store_new(1,G_TYPE_OBJECT))));

	gtk_widget_set_hexpand(view,TRUE);
	gtk_widget_set_vexpand(view,TRUE);
	gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW(view),FALSE);

	// Create Renderers
	GtkCellRenderer * text_renderer = gtk_cell_renderer_text_new();

	gtk_tree_view_insert_column_with_attributes(
		GTK_TREE_VIEW(view),
		-1,
		_("Label"),
		text_renderer,
			"text",0,
			NULL
	);

	return view;
 }

 /*
 void pw3270_action_view_append_application_action(GtkWidget *widget, GAction *action) {

	g_return_if_fail(PW3270_IS_ACTION(action));

	GtkListStore * store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));
	GtkTreeIter iter;

	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter, 0, action);

 }
 */

 static GSList * append_action(GSList * list, const gchar *type, GAction *action) {

	if(!action)
		return list;

	GParamSpec *spec = g_object_class_find_property(G_OBJECT_GET_CLASS(action),"toolbar-icon");

	if(!spec)
		return list;

	const gchar *name = g_action_get_name(action);
//	debug("%s=%p",name,action);

	GValue value = G_VALUE_INIT;
	g_value_init(&value, GTK_TYPE_IMAGE);

	g_object_get_property(G_OBJECT(action),"toolbar-icon",&value);
	GObject * image = g_value_get_object(&value);
	g_value_unset (&value);

	if(!image)
		return list;

	struct ListElement * element = (struct ListElement *) g_malloc0(sizeof(struct ListElement) + strlen(type) + strlen(name));

	strcpy(element->name,type);
	strcat(element->name,name);

	element->action = action;

	element->image = GTK_IMAGE(image);
	g_object_ref_sink(G_OBJECT(element->image));

	return g_slist_prepend(list,element);

 }

 Pw3270ActionList * pw3270_action_list_new(GtkApplication *application) {

 	GSList * list = NULL;

 	gchar ** action_names;
 	size_t ix;

 	// Get application actions.

	action_names = g_action_group_list_actions(G_ACTION_GROUP(application));
	for(ix = 0; action_names[ix];ix++) {
		list = append_action(list,"app.",g_action_map_lookup_action(G_ACTION_MAP(application),action_names[ix]));
	}
	g_strfreev(action_names);

	// Get Windows actions.
	GtkWindow * window = gtk_application_get_active_window(application);

	if(window && G_IS_ACTION_GROUP(window)) {

		// Get window actions.
		action_names = g_action_group_list_actions(G_ACTION_GROUP(window));
		for(ix = 0; action_names[ix];ix++) {
			list = append_action(list,"win.",g_action_map_lookup_action(G_ACTION_MAP(window),action_names[ix]));
		}
		g_strfreev(action_names);

	}

 	return (Pw3270ActionList *) list;
 }

 static void free_element(struct ListElement *element) {

 	if(element->image) {
		g_object_unref(element->image);
		element->image = NULL;
 	}

 	g_free(element);

 }

 void pw3270_action_list_free(Pw3270ActionList *action_list) {
	g_slist_free_full((GSList *) action_list, (GDestroyNotify) free_element);
 }

