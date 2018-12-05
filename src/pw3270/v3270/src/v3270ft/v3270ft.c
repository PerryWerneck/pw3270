/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob
 * o nome G3270.
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
 * Este programa está nomeado como v3270ft.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

/**
 * SECTION:v3270ft
 * @Short_description: A 3270 file transfer dialog
 * @Title: v3270ft
 *
 */

 #include <limits.h>
 #include "private.h"

/*--[ GTK Requires ]---------------------------------------------------------------------------------*/

 G_DEFINE_TYPE(v3270ft, v3270ft, GTK_TYPE_DIALOG);

/*--[ Implement ]------------------------------------------------------------------------------------*/

void v3270ft_remove_all(GtkWidget *widget) {

	v3270ft *dialog = GTK_V3270FT(widget);

	if(dialog->files) {
		g_list_free_full(dialog->files,g_free);
		dialog->files	= NULL;
		dialog->active	= NULL;
	}

	v3270ft_set_active(dialog,NULL);

}

static void v3270ft_finalize(GObject *object) {

	v3270ft *dialog = GTK_V3270FT(object);

	debug("%s",__FUNCTION__);

	if(dialog->files) {
		g_list_free_full(dialog->files,g_free);
		dialog->files	= NULL;
		dialog->active	= NULL;
	}

	G_OBJECT_CLASS(v3270ft_parent_class)->finalize(object);

}

static void v3270ft_class_init(v3270ftClass *klass) {

	debug("%s",__FUNCTION__);

	G_OBJECT_CLASS(klass)->finalize = v3270ft_finalize;
}

GtkGrid * v3270ft_new_grid(void) {
	GtkGrid * grid = GTK_GRID(gtk_grid_new());
    gtk_grid_set_column_spacing(grid,4);
	gtk_grid_set_row_spacing(grid,4);
	gtk_container_set_border_width(GTK_CONTAINER(grid),3);
	return grid;
}

static void transfer_type_changed(GtkComboBox *widget, v3270ft *dialog) {

	gint selected = gtk_combo_box_get_active(widget);

	if(selected >= 0) {

		// Tem opção seleciona

		struct v3270ft_entry *entry = v3270ft_get_selected(dialog);
		LIB3270_FT_OPTION opt = entry->options & ~(LIB3270_FT_OPTION_SEND|LIB3270_FT_OPTION_RECEIVE|LIB3270_FT_OPTION_ASCII|LIB3270_FT_OPTION_CRLF|LIB3270_FT_OPTION_REMAP);

		opt |= ft_type[selected].opt;

		entry->type = selected;
		debug("Transfer type=%d opt=%08x last=%08x",selected,opt,entry->options);

		if(entry->options != opt) {
			debug("Transfer type=%d opt=%08x",selected,opt);
			v3270ft_set_options(GTK_WIDGET(dialog),opt);
		}

	}

	v3270ft_update_actions(dialog);

}

static void local_file_changed(GtkEntry *widget, v3270ft *dialog) {

	struct v3270ft_entry *entry = v3270ft_get_selected(dialog);

	if(entry) {
		strncpy(entry->local,gtk_entry_get_text(widget),sizeof(entry->local));
		v3270ft_update_actions(dialog);
	}

}

static void remote_file_changed(GtkEntry *widget, v3270ft *dialog) {

	struct v3270ft_entry *entry = v3270ft_get_selected(dialog);

	if(entry) {
		strncpy(entry->remote,gtk_entry_get_text(widget),sizeof(entry->remote));
		v3270ft_update_actions(dialog);
	}

}

static void start_transfer(GtkButton *button, v3270ft *dialog) {
	gtk_dialog_response(GTK_DIALOG(dialog),GTK_RESPONSE_APPLY);
}

static void select_first(GtkButton *button, v3270ft *dialog) {
	v3270ft_select_first(GTK_WIDGET(dialog));
}

static void select_last(GtkButton *button, v3270ft *dialog) {
	v3270ft_select_last(GTK_WIDGET(dialog));
}

static void select_previous(GtkButton *button, v3270ft *dialog) {
	v3270ft_select_previous(GTK_WIDGET(dialog));
}

static void select_next(GtkButton *button, v3270ft *dialog) {
	v3270ft_select_next(GTK_WIDGET(dialog));
}

static void insert_file(GtkButton *button, v3270ft *dialog) {
	dialog->files = g_list_append(dialog->files,v3270ft_create_entry());
	v3270ft_select_last(GTK_WIDGET(dialog));
}

static void remove_file(GtkButton *button, v3270ft *dialog) {

	GList *next = dialog->active->next;

	debug("%s",__FUNCTION__);

	dialog->files = g_list_delete_link(dialog->files,dialog->active);

	if(next) {

		v3270ft_set_active(dialog,next);

	} else {

		if(!dialog->files) {
			dialog->files = g_list_append(dialog->files,v3270ft_create_entry());
		}

		v3270ft_select_last(GTK_WIDGET(dialog));
	}

}

static void load_file(GtkButton *button, v3270ft *dialog) {

	gchar * filename = v3270ft_select_file(
								dialog,
								_("Load queue from file"),
								_("Load"), GTK_FILE_CHOOSER_ACTION_OPEN,
								"",
								N_("XML file"), "*.xml",
								NULL );

	if(filename) {
		v3270ft_load(GTK_WIDGET(dialog),filename);
		g_free(filename);
		v3270ft_select_last(GTK_WIDGET(dialog));
	}


}

static void save_file(GtkButton *button, v3270ft *dialog) {

	gchar * filename = v3270ft_select_file(
								dialog,
								_("Save queue to file"),
								_("Save"),
								GTK_FILE_CHOOSER_ACTION_SAVE,
								"",
								N_("XML file"), "*.xml",
								NULL );

	if(filename) {
		v3270ft_save(GTK_WIDGET(dialog),filename);
		g_free(filename);
	}

}

static void option_toggled(GtkToggleButton *togglebutton, v3270ft *ft) {

	struct v3270ft_entry * entry = v3270ft_get_selected(ft);

	if(entry) {

		LIB3270_FT_OPTION	newoption 	= entry->options;
		int 				id;

		// Reseta todas as opções
		for(id=0;id<NUM_OPTIONS_WIDGETS;id++) {
			newoption &= ~ft_option[id].opt;
		}

		// Ativo apenas as opções selecionadas
		for(id=0;id<NUM_OPTIONS_WIDGETS;id++) {

			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ft->opt[id]))) {
				newoption |= ft_option[id].opt;
			}

		}

		if(entry->options != newoption) {
			debug("Options: %08lx -> %08lx",(unsigned long) entry->options, (unsigned long) newoption);
			entry->options = newoption;
		}

	}

}

static void spin_changed(GtkWidget *button, v3270ft *ft) {

	int f;
	struct v3270ft_entry * entry = v3270ft_get_selected(ft);

	if(entry) {
		for(f=0;f < LIB3270_FT_VALUE_COUNT;f++) {

			if(button == ft->value[f]) {
				v3270ft_get_selected(ft)->value[f] = (guint) gtk_spin_button_get_value(GTK_SPIN_BUTTON(button));
				return;
			}

		}
	}

}

static gboolean spin_format(GtkSpinButton *spin, gpointer data) {

	GtkAdjustment	* adjustment = gtk_spin_button_get_adjustment (spin);
	int				  value = (int)gtk_adjustment_get_value(adjustment);

	if(value < 1) {
		gtk_entry_set_text(GTK_ENTRY(spin), "");
	} else {
		gchar * text = g_strdup_printf ("%d", value);
		gtk_entry_set_text (GTK_ENTRY(spin), text);
		g_free (text);
	}

	return TRUE;
}

#ifdef WIN32
static void select_local_file(GtkButton *button, v3270ft *dialog) {
#else
static void icon_press(GtkEntry *entry, GtkEntryIconPosition icon_pos, GdkEvent *event, v3270ft *dialog) {
#endif // WIN32

	gchar *filename = v3270ft_select_file(
								dialog,
								_("Select local file"),
								_("Select"),
								GTK_FILE_CHOOSER_ACTION_OPEN,
								gtk_entry_get_text(dialog->local),
								N_("All files"), "*.*",
								N_("Text files"), "*.txt",
								NULL );

	if(filename) {

		const gchar *remote = gtk_entry_get_text(dialog->remote);

		gtk_entry_set_text(dialog->local,filename);

		if(!*remote) {
			gchar * text = g_path_get_basename(filename);
			gtk_entry_set_text(dialog->remote,text);
			g_free(text);
		}

		g_free(filename);
	}

}

static void drag_data_received(GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *data, guint info, guint time) {

	gtk_drag_finish(context, v3270ft_append_selection(widget, data) > 0, FALSE, time);

}

static void v3270ft_init(v3270ft *dialog) {

	static const gchar * label[] = {
		N_( "_Operation:" ),
		N_( "_Local file:"),
		N_( "_Remote file:")
	};

	static const gchar * frame[] = {
		N_("Transfer options"),
		N_("Record format"),
        N_("Space allocation units")
	};

	int				  f;
	GtkGrid 		* grid;
	GtkTreeModel    * model;
	GtkCellRenderer * renderer;
	GtkWidget		* widget;
	GtkBox			* box = GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog)));
	GtkWidget		* entry[G_N_ELEMENTS(label)];

	// Initialize
	gtk_window_set_title(GTK_WINDOW(dialog),_( "3270 File transfer"));
	gtk_window_set_resizable(GTK_WINDOW(dialog),FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(box),3);
	dialog->files = dialog->active = g_list_append(NULL,v3270ft_create_entry());

	// Buttons
	// https://developer.gnome.org/icon-naming-spec/
	static const struct _action {
		FT_BUTTON	  id;
		gboolean	  start;
		const gchar	* name;
		const gchar	* tooltip;
		GCallback 	  callback;
	} action[FT_BUTTON_COUNT] = {

		{
			FT_BUTTON_LOAD_LIST,
			TRUE,
			"document-open",
			N_("Get transfer queue from an external XML file"),
			G_CALLBACK(load_file)
		},

		{
			FT_BUTTON_SAVE_LIST,
			TRUE,
			"document-save",
			N_("Save transfer queue to an external XML file"),
			G_CALLBACK(save_file)
		},

		{
			FT_BUTTON_INSERT_FILE,
			TRUE,
			"list-add",
			N_("Insert new file in the transfer queue"),
			G_CALLBACK(insert_file)
		},

		{
			FT_BUTTON_REMOVE_FILE,
			TRUE,
			"list-remove",
			N_("Remove selected file from the transfer queue"),
			G_CALLBACK(remove_file)
		},

		{
			FT_BUTTON_START_TRANSFER,
			FALSE,
			"network-server",
			N_("Start file transfer"),
			G_CALLBACK(start_transfer)
		},

#if HAVE_GTK_HEADER_BAR
		{
			FT_BUTTON_GO_LAST,
			FALSE,
			"go-last",
			N_("Select last file"),
			G_CALLBACK(select_last)
		},

		{
			FT_BUTTON_GO_NEXT,
			FALSE,
			"go-next",
			N_("Select next file"),
			G_CALLBACK(select_next)
		},

		{
			FT_BUTTON_GO_PREVIOUS,
			FALSE,
			"go-previous",
			N_("Select previous file"),
			G_CALLBACK(select_previous)
		},

		{
			FT_BUTTON_GO_FIRST,
			FALSE,
			"go-first",
			N_("Select first file"),
			G_CALLBACK(select_first)
		},

#else
		{
			FT_BUTTON_GO_FIRST,
			FALSE,
			"go-first",
			N_("Select first file"),
			G_CALLBACK(select_first)
		},

		{
			FT_BUTTON_GO_PREVIOUS,
			FALSE,
			"go-previous",
			N_("Select previous file"),
			G_CALLBACK(select_previous)
		},

		{
			FT_BUTTON_GO_NEXT,
			FALSE,
			"go-next",
			N_("Select next file"),
			G_CALLBACK(select_next)
		},

		{
			FT_BUTTON_GO_LAST,
			FALSE,
			"go-last",
			N_("Select last file"),
			G_CALLBACK(select_last)
		},

#endif // HAVE_GTK_HEADER_BAR

	};

#if HAVE_GTK_HEADER_BAR
	widget = gtk_dialog_get_header_bar(GTK_DIALOG(dialog));

	for(f=0;f<G_N_ELEMENTS(action);f++) {

		GtkWidget *button = gtk_button_new_from_icon_name(action[f].name,GTK_ICON_SIZE_BUTTON);

		gtk_widget_set_tooltip_markup(button,gettext(action[f].tooltip));

		if(action[f].start) {
			gtk_header_bar_pack_start(GTK_HEADER_BAR(widget),button);
		} else {
			gtk_header_bar_pack_end(GTK_HEADER_BAR(widget),button);
		}

		g_signal_connect(button,"clicked",action[f].callback,dialog);

		dialog->button[action[f].id] = button;
		gtk_widget_set_sensitive(button,FALSE);

	}

#else

	widget = gtk_toolbar_new();
	gtk_toolbar_set_icon_size(GTK_TOOLBAR(widget),GTK_ICON_SIZE_SMALL_TOOLBAR);

	gtk_box_pack_start(box,GTK_WIDGET(widget),FALSE,FALSE,2);

	for(f=0;f<G_N_ELEMENTS(action);f++) {

		GtkWidget *button = GTK_WIDGET(gtk_tool_button_new(gtk_image_new_from_icon_name(action[f].name,GTK_ICON_SIZE_SMALL_TOOLBAR),NULL));

		gtk_widget_set_tooltip_markup(button,gettext(action[f].tooltip));

		gtk_toolbar_insert(GTK_TOOLBAR(widget),GTK_TOOL_ITEM(button),-1);

		g_signal_connect(button,"clicked",action[f].callback,dialog);

		dialog->button[action[f].id] = button;
		gtk_widget_set_sensitive(button,FALSE);

	}

#endif

	gtk_widget_set_sensitive(dialog->button[FT_BUTTON_LOAD_LIST],TRUE);
	gtk_widget_set_sensitive(dialog->button[FT_BUTTON_REMOVE_FILE],TRUE);

	// Top level
	grid = v3270ft_new_grid();
	gtk_box_pack_start(box,GTK_WIDGET(grid),TRUE,TRUE,2);

	// Transfer type combo
	model           = (GtkTreeModel *) gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_ULONG);
	renderer        = gtk_cell_renderer_text_new();

	widget = gtk_combo_box_new_with_model(model);
	g_signal_connect(G_OBJECT(widget),"changed",G_CALLBACK(transfer_type_changed),dialog);
	entry[0] = GTK_WIDGET(widget);

	dialog->type = GTK_COMBO_BOX(widget);
	gtk_grid_attach(grid,GTK_WIDGET(widget),1,0,1,1);
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget), renderer, TRUE);
	gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), renderer, "text", 0, NULL);

	for(f=0;f < NUM_TYPES;f++) {

			GtkTreeIter iter;

			gtk_list_store_append((GtkListStore *) model,&iter);
			gtk_list_store_set((GtkListStore *) model, &iter, 0, gettext(ft_type[f].label),-1);

			if(!f) {
				gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&iter);
			}
	}

	// Local file entry
	dialog->local   = GTK_ENTRY(gtk_entry_new());
	entry[1] 		= GTK_WIDGET(dialog->local);

	gtk_widget_set_hexpand(GTK_WIDGET(dialog->local),TRUE);

#ifdef WIN32
	widget = gtk_button_new_from_icon_name("document-open",GTK_ICON_SIZE_BUTTON);
	g_signal_connect(G_OBJECT(widget),"clicked",G_CALLBACK(select_local_file),dialog);
	gtk_grid_attach(grid,widget,2,1,1,1);
#else
	gtk_entry_set_icon_from_icon_name(dialog->local,GTK_ENTRY_ICON_SECONDARY,"document-open");
	gtk_entry_set_icon_activatable(dialog->local,GTK_ENTRY_ICON_SECONDARY,TRUE);
	gtk_entry_set_icon_tooltip_text(dialog->local,GTK_ENTRY_ICON_SECONDARY,_("Select file"));
	g_signal_connect(G_OBJECT(dialog->local),"icon-press",G_CALLBACK(icon_press),dialog);
#endif // WIN32

	g_signal_connect(G_OBJECT(dialog->local),"changed",G_CALLBACK(local_file_changed),dialog);

	gtk_entry_set_width_chars(dialog->local,60);
	gtk_entry_set_max_length(dialog->local,PATH_MAX);
	gtk_grid_attach(grid,GTK_WIDGET(dialog->local),1,1,1,1);

	// Remote file entry
	dialog->remote  = GTK_ENTRY(gtk_entry_new());
	entry[2] 		= GTK_WIDGET(dialog->remote);

	gtk_widget_set_hexpand(GTK_WIDGET(dialog->remote),TRUE);

	g_signal_connect(G_OBJECT(dialog->remote),"changed",G_CALLBACK(remote_file_changed),dialog);

	gtk_entry_set_width_chars(dialog->remote,60);
	gtk_entry_set_max_length(dialog->remote,PATH_MAX);
	gtk_grid_attach(grid,GTK_WIDGET(dialog->remote),1,2,1,1);

	for(f=0;f<G_N_ELEMENTS(label);f++) {
		GtkWidget * widget = gtk_label_new_with_mnemonic(gettext(label[f]));
        gtk_widget_set_halign(widget,GTK_ALIGN_START);
        gtk_widget_set_valign(widget,GTK_ALIGN_CENTER);
		gtk_grid_attach(grid,GTK_WIDGET(widget),0,f,1,1);
		gtk_label_set_mnemonic_widget(GTK_LABEL(widget),entry[f]);
	}

	// Transfer options
	widget = gtk_frame_new(gettext(frame[0]));
	grid = v3270ft_new_grid();
	gtk_grid_set_column_homogeneous(grid,TRUE);
	gtk_container_add(GTK_CONTAINER(widget),GTK_WIDGET(grid));
	gtk_box_pack_start(box,widget,FALSE,TRUE,2);

	for(f=0;f<4;f++) {

		dialog->opt[f] = widget = gtk_check_button_new_with_mnemonic(gettext(ft_option[f].label));
        gtk_widget_set_tooltip_markup(widget,gettext(ft_option[f].tooltip));
		gtk_grid_attach(grid,widget,f&1,f/2,1,1);
		g_signal_connect(G_OBJECT(widget),"toggled",G_CALLBACK(option_toggled),dialog);

	}

	// Formats
	grid = v3270ft_new_grid();
	gtk_grid_set_column_homogeneous(grid,TRUE);
	gtk_box_pack_start(box,GTK_WIDGET(grid),FALSE,TRUE,2);

	// Record & Space allocation
	for(f=0;f<2;f++) {
		int i;
		GtkBox * box   = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL,2));
		GSList * group = NULL;

		dialog->radio[f] = gtk_frame_new(gettext(frame[f+1]));
		gtk_container_add(GTK_CONTAINER(dialog->radio[f]),GTK_WIDGET(box));
		gtk_container_add(GTK_CONTAINER(grid),dialog->radio[f]);

		for(i=0;i<4;i++) {

			int ix = ((f+1)*4)+i;

			dialog->opt[ix] = widget = gtk_radio_button_new_with_label(group,gettext(ft_option[ix].label));

			g_signal_connect(G_OBJECT(widget),"toggled",G_CALLBACK(option_toggled),dialog);
			gtk_widget_set_tooltip_markup(widget,gettext(ft_option[ix].tooltip));
			gtk_box_pack_start(box,widget,FALSE,FALSE,2);
			group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(widget));

		}

	}

	// Values
	grid = v3270ft_new_grid();
	gtk_grid_set_column_homogeneous(grid,TRUE);
	gtk_box_pack_start(box,GTK_WIDGET(grid),FALSE,TRUE,2);

	for(f=0;f < LIB3270_FT_VALUE_COUNT;f++) {

		GtkWidget * label = gtk_label_new_with_mnemonic(gettext(ft_value[f].label));
        gtk_widget_set_halign(label,GTK_ALIGN_START);
        gtk_widget_set_valign(label,GTK_ALIGN_CENTER);

		widget = gtk_spin_button_new_with_range(ft_value[f].minval,ft_value[f].maxval,1);
		g_signal_connect(G_OBJECT(widget),"value-changed",G_CALLBACK(spin_changed),dialog);
		g_signal_connect(G_OBJECT(widget),"output",G_CALLBACK(spin_format),dialog);

		gtk_widget_set_tooltip_markup(widget,gettext(ft_value[f].tooltip));
		gtk_widget_set_tooltip_markup(label,gettext(ft_value[f].tooltip));

		gtk_label_set_mnemonic_widget(GTK_LABEL(label),widget);

		gtk_grid_attach(grid,label,(f&1)*2,f/2,1,1);
		gtk_grid_attach(grid,widget,((f&1)*2)+1,f/2,1,1);

		dialog->value[f] = widget;

	}

	// Setup drag & drop
	// http://ftp.math.utah.edu/u/ma/hohn/linux/gnome/developer.gnome.org/doc/tutorials/gnome-libs/x1003.html
	static const GtkTargetEntry targets[] = {
		{ "text/plain", 					GTK_TARGET_OTHER_APP, 0 }
	};

	gtk_drag_dest_set(GTK_WIDGET(dialog),GTK_DEST_DEFAULT_ALL,targets,G_N_ELEMENTS(targets),GDK_ACTION_COPY);
	// g_signal_connect(dialog,"drag-drop",G_CALLBACK(drag_drop),dialog);
	g_signal_connect(dialog,"drag-data-received",G_CALLBACK(drag_data_received),dialog);


	// Finish setup
	v3270ft_select_last(GTK_WIDGET(dialog));


}

/**
 * v3270ft_dialog_new:
 *
 * Creates a new #v3270ft.
 *
 * Returns: a new #v3270ft.
 */
GtkWidget * v3270ft_new(void) {
	return GTK_WIDGET(g_object_new(GTK_TYPE_V3270FT, "use-header-bar", (gint) 1, NULL));
}

