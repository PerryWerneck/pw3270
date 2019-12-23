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

 enum {
	COLUMN_PIXBUF,
	COLUMN_LABEL,
	COLUMN_ACTION_NAME
 };

 struct ListElement {
 	GAction		* action;
 	GdkPixbuf	* pixbuf;
 	gchar		  name[1];
 };

 static void list_element_free(struct ListElement *element);

 static gint view_sort(GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer G_GNUC_UNUSED(user_data)) {

	gint rc = 0;
	GValue value[] = { G_VALUE_INIT, G_VALUE_INIT };

	gtk_tree_model_get_value(model, a, COLUMN_LABEL, &value[0]);
	gtk_tree_model_get_value(model, b, COLUMN_LABEL, &value[1]);

	rc = g_ascii_strcasecmp(g_value_get_string(&value[0]),g_value_get_string(&value[1]));

	g_value_unset(&value[0]);
	g_value_unset(&value[1]);

	return rc;

 }

 GtkWidget * pw3270_action_view_new() {

	GtkTreeModel * model = GTK_TREE_MODEL(gtk_list_store_new(3,G_TYPE_OBJECT,G_TYPE_STRING,G_TYPE_STRING));
	GtkWidget * view = GTK_WIDGET(gtk_tree_view_new_with_model(model));

	gtk_widget_set_hexpand(view,TRUE);
	gtk_widget_set_vexpand(view,TRUE);
	gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW(view),FALSE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view),FALSE);

	// Create Renderers
	GtkCellRenderer * text_renderer = gtk_cell_renderer_text_new();
	GtkCellRenderer * pixbuf_renderer = gtk_cell_renderer_pixbuf_new();

	gtk_tree_view_insert_column_with_attributes(
		GTK_TREE_VIEW(view),
		-1,
		_("Icon"),
		pixbuf_renderer,
			"pixbuf",COLUMN_PIXBUF,
			NULL
	);

	gtk_tree_view_insert_column_with_attributes(
		GTK_TREE_VIEW(view),
		-1,
		_("Label"),
		text_renderer,
			"text",COLUMN_LABEL,
			NULL
	);

	gtk_tree_sortable_set_default_sort_func(GTK_TREE_SORTABLE(model),view_sort,NULL,NULL);

	return view;
 }

 static void pw3270_action_view_append_element(GtkListStore * store, struct ListElement * element) {

	size_t ix;

	struct Properties {
		const gchar * name;
		GType g_type;
		GValue value;
	} properties[] = {
		{
			.name = "label",
			.g_type = G_TYPE_STRING,
			.value = G_VALUE_INIT
		}
	};

	for(ix = 0; ix < G_N_ELEMENTS(properties); ix++) {

		g_value_init(&properties[ix].value, properties[ix].g_type);
		g_object_get_property(G_OBJECT(element->action), properties[ix].name, &properties[ix].value);

	}

	// Remove "_"
	g_autofree gchar * label = g_strdup(g_value_get_string(&properties[0].value));

	if(label) {

		gchar *from, *to;

		for(from=to=label;*from;from++) {
			if(*from != '_') {
				*(to++) = *from;
			}
		}
		*to = 0;

	}

	GtkTreeIter iter;
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(
		store,
		&iter,
		COLUMN_PIXBUF,	element->pixbuf,
		COLUMN_LABEL, 	(label ? label : g_action_get_name(element->action)),
		-1
	);

	for(ix = 0; ix < G_N_ELEMENTS(properties); ix++) {
		g_value_unset(&properties[ix].value);
	}

 }

 Pw3270ActionList * pw3270_action_list_move_action(Pw3270ActionList *action_list, const gchar *action_name, GtkWidget *view) {

	GSList * item = (GSList *) action_list;
	GtkListStore * store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(view)));

	while(item) {

		struct ListElement * element = (struct ListElement *) item->data;

		if(!g_ascii_strcasecmp(action_name,element->name)) {

			pw3270_action_view_append_element(store, element);
			list_element_free(element);
			return (Pw3270ActionList *) g_slist_remove_link(action_list,item);
		}

		item = g_slist_next(item);

	}

	return action_list;

 }

 void pw3270_action_view_set_actions(GtkWidget *view, Pw3270ActionList *list) {

	GSList *item;
	GtkListStore * store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(view)));

	for(item = (GSList *) list; item; item = g_slist_next(item)) {

		pw3270_action_view_append_element(store, (struct ListElement *) item->data);

	}

 }

 static GSList * append_action(GSList * list, const gchar *type, GAction *action) {

	if(!action)
		return list;

	if(!g_object_class_find_property(G_OBJECT_GET_CLASS(action),"label"))
		return list;

	GParamSpec *spec = g_object_class_find_property(G_OBJECT_GET_CLASS(action),"toolbar-icon");
	if(!spec)
		return list;

	const gchar *name = g_action_get_name(action);

	GdkPixbuf * pixbuf = g_action_get_pixbuf(action, GTK_ICON_SIZE_MENU);
	if(!pixbuf)
		return list;

	struct ListElement * element = (struct ListElement *) g_malloc0(sizeof(struct ListElement) + strlen(type) + strlen(name));

	strcpy(element->name,type);
	strcat(element->name,name);

	element->action = action;

	element->pixbuf = pixbuf;
	g_object_ref_sink(G_OBJECT(element->pixbuf));

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

 void list_element_free(struct ListElement *element) {

 	if(element->pixbuf) {
		g_object_unref(element->pixbuf);
		element->pixbuf = NULL;
 	}

 	g_free(element);

 }

 void pw3270_action_list_free(Pw3270ActionList *action_list) {
	g_slist_free_full((GSList *) action_list, (GDestroyNotify) list_element_free);
 }

