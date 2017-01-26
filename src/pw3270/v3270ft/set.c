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
 * Este programa está nomeado como set.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "private.h"

/*--[ Implement ]------------------------------------------------------------------------------------*/

void v3270ft_set_active(v3270ft *dialog, GList * active) {

	int f;
	gboolean sensitive = (active != NULL);

	dialog->active = active;

	// Habilita/Desabilita widgets
	gtk_widget_set_sensitive(GTK_WIDGET(dialog->type),sensitive);
	gtk_widget_set_sensitive(GTK_WIDGET(dialog->local),sensitive);
	gtk_widget_set_sensitive(GTK_WIDGET(dialog->remote),sensitive);

	for(f=0;f<NUM_OPTIONS_WIDGETS;f++) {
		gtk_widget_set_sensitive(dialog->opt[f],sensitive);
	}

	for(f=0;f<FT_BUTTON_COUNT;f++) {
		gtk_widget_set_sensitive(dialog->button[f],sensitive);
	}

	for(f = 0; f < LIB3270_FT_VALUE_COUNT; f++) {
		gtk_widget_set_sensitive(dialog->value[f],sensitive);
	}

	// Preenche dialogo
	if(sensitive) {

		struct v3270ft_entry * info = (struct v3270ft_entry *) active->data;

		gtk_combo_box_set_active(dialog->type,info->type);

		gtk_entry_set_text(dialog->local,info->local);
		gtk_entry_set_text(dialog->remote,info->remote);

		v3270ft_set_options(GTK_WIDGET(dialog),info->options);

		for(f = 0; f < LIB3270_FT_VALUE_COUNT; f++) {
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dialog->value[f]),info->value[f]);
		}


	} else {

		gtk_entry_set_text(dialog->local,"");
		gtk_entry_set_text(dialog->remote,"");

		v3270ft_set_options(GTK_WIDGET(dialog),0);

		for(f = 0; f < LIB3270_FT_VALUE_COUNT; f++) {
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(dialog->value[f]),0);
		}

	}


}


/**
 * v3270ft_set_options:
 * @widget: a #v3270ft
 * @options: The lib3270 file transfer options.
 *
 * Since: 5.1
 **/
void v3270ft_set_options(GtkWidget *widget, LIB3270_FT_OPTION opt) {

	int		  id;
	v3270ft	* ft	= GTK_V3270FT(widget);

	if(opt & LIB3270_FT_OPTION_RECEIVE) {

		// Receber arquivo, reseto flags específicas do envio
		opt &= ~(LIB3270_FT_RECORD_FORMAT_DEFAULT|LIB3270_FT_RECORD_FORMAT_FIXED|LIB3270_FT_RECORD_FORMAT_VARIABLE|LIB3270_FT_RECORD_FORMAT_UNDEFINED|LIB3270_FT_ALLOCATION_UNITS_DEFAULT|LIB3270_FT_ALLOCATION_UNITS_TRACKS|LIB3270_FT_ALLOCATION_UNITS_CYLINDERS|LIB3270_FT_ALLOCATION_UNITS_AVBLOCK);

		gtk_widget_set_sensitive(ft->radio[0],FALSE);
		gtk_widget_set_sensitive(ft->radio[1],FALSE);

		for(id = 0; id < 4; id++) {
			gtk_widget_set_sensitive(ft->value[id],FALSE);
			gtk_spin_button_set_value(GTK_SPIN_BUTTON(ft->value[id]),0);
		}

	} else {

		gtk_widget_set_sensitive(ft->radio[0],TRUE);
		gtk_widget_set_sensitive(ft->radio[1],TRUE);

		for(id = 0; id < 4; id++) {
			gtk_widget_set_sensitive(ft->value[id],TRUE);
		}
	}

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ft->opt[4]),TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ft->opt[8]),TRUE);

	for(id=0;id<NUM_OPTIONS_WIDGETS;id++) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ft->opt[id]),(opt & ft_option[id].opt) == ft_option[id].opt);
	}

	struct v3270ft_entry *entry	= v3270ft_get_selected(ft);

	if(entry) {
		entry->options = opt;
		v3270ft_update_actions(ft);
	}

}

void v3270ftprogress_set_header(GtkWidget *widget, const gchar *status) {

#ifdef HAVE_GTK_HEADER_BAR
	gtk_header_bar_set_subtitle(GTK_HEADER_BAR(gtk_dialog_get_header_bar(GTK_DIALOG(widget))),status);
#endif

}

void v3270ftprogress_set_transfer(GtkWidget *widget, struct v3270ft_entry *entry) {

	v3270ftprogress * dialog = GTK_V3270FTPROGRESS(widget);

	/*
	if(dialog->session) {
		lib3270_ft_set_user_data(dialog->session,NULL);
		lib3270_ft_destroy(dialog->session);
	}
	*/

	dialog->active = entry;

	debug("%s\n%s <-> %s",__FUNCTION__,entry->local,entry->remote);

	gtk_entry_set_text(dialog->info[PROGRESS_FIELD_LOCAL],entry->local);
	gtk_entry_set_text(dialog->info[PROGRESS_FIELD_REMOTE],entry->remote);

	gtk_entry_set_text(dialog->info[PROGRESS_FIELD_TOTAL],"");
	gtk_entry_set_text(dialog->info[PROGRESS_FIELD_CURRENT],"");
	gtk_entry_set_text(dialog->info[PROGRESS_FIELD_SPEED],"");
	gtk_entry_set_text(dialog->info[PROGRESS_FIELD_ETA],"");

	gtk_progress_bar_set_fraction(dialog->progress,0);
	v3270ftprogress_set_header(widget,_("Preparing"));

}

void v3270ftprogress_set_session(GtkWidget *widget, H3270 *session) {

	v3270ftprogress * dialog = GTK_V3270FTPROGRESS(widget);

	if(dialog->session) {
		lib3270_ft_destroy(dialog->session);
	}

	dialog->session = session;
}

guint v3270ft_append_selection(GtkWidget *widget, GtkSelectionData *data) {

	gchar		**uri 		= g_strsplit((const gchar *) gtk_selection_data_get_text(data),"\n",-1);
	size_t		  f;
	guint		  qtd		= 0;

	for(f=0;uri[f];f++) {

		if(!g_ascii_strncasecmp("file:///",uri[f],8)) {

			const gchar *filename = uri[f]+7;

			if(g_file_test(filename,G_FILE_TEST_IS_REGULAR)) {
				v3270ft_append_file(widget,filename,TRUE);
				qtd++;
			}

		}
	}

	g_strfreev(uri);

	return qtd;
}


