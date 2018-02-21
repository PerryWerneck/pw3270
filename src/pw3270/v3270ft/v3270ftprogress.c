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
 * SECTION:v3270ftprogress
 * @Short_description: A 3270 file transfer progress dialog
 * @Title: v3270ftprogress
 *
 */

 #include "private.h"
 #include "marshal.h"
 #include <lib3270/filetransfer.h>

/*--[ GTK Requires ]---------------------------------------------------------------------------------*/

 G_DEFINE_TYPE(v3270ftprogress, v3270ftprogress, GTK_TYPE_DIALOG);


/*--[ Globals ]--------------------------------------------------------------------------------------*/

guint v3270ftprogress_signal[V3270FTPROGRESS_SIGNAL_COUNT] = { 0 };

static const struct _field {

	const gchar *label;
	const gchar *tooltip;

} field[PROGRESS_FIELD_COUNT] = {

	{
		N_("Local:"),
		N_("Path and name of the local file")
	},

	{
		N_("Remote:"),
		N_("The name of the file in the host")
	},

	{
		N_("Total:"),
		N_("Total bytes to transfer")
	},

	{
		N_("Current:"),
		N_("Current transfer position")
	},

	{
		N_("Speed:"),
		N_("Transfer speed")
	},

	{
		N_("ETA:"),
		N_("Estimated transfer arrival")
	},

};

// http://www3.rocketsoftware.com/bluezone/help/v42/en/bz/DISPLAY/IND$FILE/IND$FILE_Technical_Reference.htm

static const struct _ftmsg {

	const char		* id;
	unsigned char	  failed;
	const char 		* text;

} ftmsg[] = {

	{
		"TRANS00",
		TRUE,	// - Error in file transfer: file transfer canceled
		N_("An error occurred in the file transfer, which may be an error in the data being transferred, or an unidentified system error.")
	},

	{
		"TRANS03",
		FALSE,	// - File transfer complete
		N_("The file transfer operation has been successfully completed.")
	},

	{
		"TRANS04",
		FALSE,	// - File transfer complete with records segmented
		N_("The file transfer operation has been completed, and any record greater than the logical record length (LRECL) of the file being appended has been divided and becomes multiple records.")
	},

	{
		"TRANS05",
		TRUE,	// - Personal computer filespec incorrect: file transfer canceled
		N_("An error exists in the PC's file name.")
	},

	{
		"TRANS06",
		TRUE,	// - Command incomplete: file transfer canceled
		N_("You did not enter the required parameters after a SEND or RECEIVE command.")
	},

	{
		"TRANS13",
		TRUE,	// - Error writing file to host: file transfer canceled
		N_("The host program has detected an error in the file data during a RECEIVE operation.")
	},

	{
		"TRANS14",
		TRUE,	// - Error reading file from host: file transfer canceled
		N_("The host program has detected an error in the file data during a RECEIVE operation.")
	},

	{
		"TRANS15",
		TRUE,	// - Required host storage unavailable: file transfer canceled
		N_("You need 30 KB of main storage (not disk space) on the host for the file transfer, in addition to the host requirement.")
	},

	{
		"TRANS16",
		TRUE,	// - Incorrect request code: file transfer canceled
		N_("An invalid SEND or RECEIVE parameter was sent to the host.")
	},

	{
		"TRANS17",
		TRUE,
		NULL
	},

	/*
	{
		"TRANS17",
		TRUE,	// - Missing or incorrect TSO data set name: file transfer canceled
		N_("You did not specify the TSO data set name, or the specified TSO data set name is not a sequential or partitioned data set.")
	},

	{
		"TRANS17",
		TRUE,	// - Invalid file name: file transfer canceled
		N_("The command that started the file transfer specified a file name that is not a valid file name for the host. Correct CICS file names must be 1 to 8 characters, composed of letters and numbers.")
	},

	{
		"TRANS17",
		TRUE,	// - Missing or incorrect CMS data set name: file transfer canceled
		N_("You did not specify the CMS data set name, or the specified CMS data set name is incorrect.")
	},
*/

	{
		"TRANS18",
		TRUE,	// - Incorrect option specified: file transfer canceled
		N_("You specified an option that is invalid.")
	},

	{
		"TRANS19",
		TRUE,	// - Error while reading or writing to host disk: file transfer canceled
		N_("There is not enough space available for data on the host.")
	},

	{
		"TRANS28",
		TRUE,	// - Invalid option xxxxxxxx: file transfer canceled
		N_("You selected an option that is either not recognized, is specified as a positional keyword, or has an associated value that is incorrect.")
	},

	{
		"TRANS29",
		TRUE,	// - Invalid option xxxxxxxx with RECEIVE: file transfer canceled
		N_("You selected an option that is not valid with RECEIVE, but can be used with SEND.")
	},

	{
		"TRANS30",
		TRUE,	// - Invalid option xxxxxxxx with APPEND: file transfer canceled
		N_("You selected an option that is not valid with APPEND, but otherwise may be used.")
	},

	{
		"TRANS31",
		TRUE,	// - Invalid option xxxxxxxx without SPACE: file transfer canceled
		N_("You selected an option that can only be used if SPACE is also specified.")
	},

	{
		"TRANS32",
		TRUE,	// - Invalid option xxxxxxxx with PDS: file transfer canceled
		N_("You selected an option that is invalid with a host-partitioned data set.")
	},

	{
		"TRANS33",
		TRUE,	// - Only one of TRACKS, CYLINDERS, AVBLOCK allowed: file transfer canceled
		N_("SPACE can be specified in units of TRACKS, CYLINDERS, or AVBLOCK, and only one option can be used.")
	},

	{
		"TRANS34",
		TRUE,	// - CMS file not found: file transfer canceled
		N_("You did not specify an existing CMS file for RECEIVE.")
	},

	{
		"TRANS35",
		TRUE,	// - CMS disk is read-only: file transfer canceled
		N_("You specified a CMS file mode for the SEND key that does not allow write access.")
	},

	{
		"TRANS36",
		TRUE,	// - CMS disk is not accessed: file transfer canceled
		N_("You specified a CMS file mode that is not in the CMS search order.")
	},

	{
		"TRANS37",
		TRUE,	// - CMS disk is full: file transfer canceled
		N_("The CMS disk is full, or the maximum number of files on the minidisk (3400) has been reached, or the maximum number of data blocks per file (16060) has been reached.")
	},

	{
		"TRANS99",
		TRUE,	// - Host program error code xxxxxxxxxx: file transfer canceled
		N_("This is a host program error.")
	},

};

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void stop_pulse(void *widget) {

	v3270ftprogress * dialog = GTK_V3270FTPROGRESS(widget);

	if(dialog->pulse) {
		g_source_destroy(dialog->pulse);
		dialog->pulse = NULL;
	}

}

static void stop_timer(void *widget) {

	v3270ftprogress * dialog = GTK_V3270FTPROGRESS(widget);

	if(dialog->timer) {
		g_source_destroy(dialog->timer);
		dialog->timer = NULL;
	}

}

gboolean v3270ftprogress_cleanup(v3270ftprogress * dialog) {

	if(dialog->session) {
		debug("%s: FT session was destroyed",__FUNCTION__);
		lib3270_ft_set_user_data(dialog->session,NULL);
		lib3270_ft_destroy(dialog->session);
	}

	return FALSE;

}

static void v3270ftprogress_finalize(GObject *object) {

	v3270ftprogress * dialog = GTK_V3270FTPROGRESS(object);

	debug("%s",__FUNCTION__);

	stop_pulse(object);
	stop_timer(object);

	// Tem sessão, removo do objeto para evitar a geração de sinais e destruo.
	v3270ftprogress_cleanup(dialog);
	dialog->session = NULL;

	G_OBJECT_CLASS(v3270ftprogress_parent_class)->finalize(object);

}

static void dialog_response(GtkDialog *widget, gint response_id) {

	v3270ftprogress * dialog 	= GTK_V3270FTPROGRESS(widget);
	GtkDialogClass	* klass		= GTK_DIALOG_CLASS(v3270ftprogress_parent_class);

	debug("%s(%d)",__FUNCTION__,(int) response_id);

	if(dialog->session) {

		// Removo do objeto para evitar a geração de sinais e destruo.
		lib3270_ft_set_user_data(dialog->session,NULL);
		lib3270_ft_destroy(dialog->session);

	}

	if(klass->response)
		klass->response(widget,response_id);

}

static void dialog_close(GtkDialog *object) {

	v3270ftprogress * dialog = GTK_V3270FTPROGRESS(object);

	debug("%s",__FUNCTION__);

	// Se tem sessão e conseguiu cancelar.
	if(dialog->session && lib3270_ft_cancel(dialog->session,0) == 0)
		return;

	GTK_DIALOG_CLASS(v3270ftprogress_parent_class)->close(object);

}

static void v3270ftprogress_class_init(v3270ftprogressClass *klass) {

	GObjectClass 	* gobject_class = G_OBJECT_CLASS(klass);
	GtkDialogClass	* dialog_class	= GTK_DIALOG_CLASS(klass);
//	GtkWidgetClass	* widget_class	= GTK_WIDGET_CLASS(klass);

	gobject_class->finalize	= v3270ftprogress_finalize;
	dialog_class->response	= dialog_response;
	dialog_class->close		= dialog_close;

	v3270ftprogress_signal[V3270FTPROGRESS_SIGNAL_SUCCESS] =
		g_signal_new(	"success",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
						0,
						NULL, NULL,
						v3270ft_VOID__POINTER_POINTER,
						G_TYPE_NONE, 2, G_TYPE_POINTER, G_TYPE_POINTER);

	v3270ftprogress_signal[V3270FTPROGRESS_SIGNAL_FAILED] =
		g_signal_new(	"failed",
						G_OBJECT_CLASS_TYPE (gobject_class),
						G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
						0,
						NULL, NULL,
						v3270ft_VOID__POINTER_POINTER,
						G_TYPE_NONE, 2, G_TYPE_POINTER, G_TYPE_POINTER);


}

static GtkWidget * create_text(guint width) {

	GtkWidget * widget = gtk_entry_new();

	gtk_entry_set_width_chars(GTK_ENTRY(widget),width);
	gtk_editable_set_editable(GTK_EDITABLE(widget),FALSE);

	gtk_widget_set_halign(GTK_WIDGET(widget),GTK_ALIGN_START);
	gtk_widget_set_valign(GTK_WIDGET(widget),GTK_ALIGN_CENTER);

	return GTK_WIDGET(widget);
}

static GtkWidget * create_label(int id, GtkWidget *entry) {

	GtkWidget * widget = gtk_label_new(gettext(field[id].label));

	gtk_widget_set_tooltip_markup(widget,field[id].tooltip);
	gtk_widget_set_tooltip_markup(entry,field[id].tooltip);

	gtk_widget_set_halign(widget,GTK_ALIGN_START);
	gtk_widget_set_valign(widget,GTK_ALIGN_CENTER);

	return widget;
}

static void cancel_clicked(GtkButton *button,v3270ftprogress *dialog) {

	debug("%s",__FUNCTION__);

	if(dialog->session) {
		lib3270_ft_cancel(dialog->session,1);
	}
}


static void v3270ftprogress_init(v3270ftprogress *dialog) {

	int			  f;
	GtkWidget	* frame;
	GtkGrid 	* grid;
	GtkWidget	* widget;
	GtkBox 		* box	= GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog)));

	// Initialize
	gtk_container_set_border_width(GTK_CONTAINER(box),3);
	gtk_window_set_title(GTK_WINDOW(dialog),_( "3270 File transfer"));
	gtk_window_set_resizable(GTK_WINDOW(dialog),FALSE);

	// Information box
	frame = gtk_frame_new( _( "Current file" ) );
	gtk_box_pack_start(box,GTK_WIDGET(frame),FALSE,TRUE,2);
	grid = v3270ft_new_grid();
	gtk_container_add(GTK_CONTAINER(frame),GTK_WIDGET(grid));

	for(f=0;f<2;f++) {

		dialog->info[f] = GTK_ENTRY(widget = create_text(50));
		gtk_widget_set_hexpand(widget,TRUE);

		gtk_grid_attach(grid,create_label(f,widget),0,f,1,1);

		gtk_grid_attach(grid,widget,1,f,1,1);

	}

	// Progress box
	frame = gtk_frame_new( _( "Progress" ) );
	gtk_box_pack_start(box,GTK_WIDGET(frame),FALSE,TRUE,2);
	grid = v3270ft_new_grid();
	gtk_container_add(GTK_CONTAINER(frame),GTK_WIDGET(grid));

	for(f=0;f<4;f++) {

		dialog->info[f+2] = GTK_ENTRY(widget = create_text(15));
		gtk_entry_set_alignment(GTK_ENTRY(widget),1);
		gtk_widget_set_hexpand(widget,TRUE);

		gtk_grid_attach(grid,create_label(f+2,widget),(f&1)*2,f/2,1,1);

		gtk_grid_attach(grid,widget,((f&1)*2)+1,f/2,1,1);

	}

 	dialog->progress = GTK_PROGRESS_BAR(widget = gtk_progress_bar_new());
	gtk_widget_set_hexpand(widget,TRUE);
	gtk_grid_attach(grid,widget,0,2,4,1);

	// Set defaults.
	v3270ftprogress_set_header(GTK_WIDGET(dialog),_("No active transfer"));


	// Buttons
	widget = gtk_button_new_with_label(_("Cancel"));
	g_signal_connect(widget,"clicked",G_CALLBACK(cancel_clicked),dialog);

#ifdef HAVE_GTK_HEADER_BAR
	frame = gtk_dialog_get_header_bar(GTK_DIALOG(dialog));
	gtk_header_bar_pack_start(GTK_HEADER_BAR(frame),widget);
#else
	frame = gtk_dialog_get_action_area(GTK_DIALOG(dialog));
	gtk_box_pack_end(GTK_BOX(frame),widget,FALSE,TRUE,2);
#endif

}

/**
 * v3270ftprogress_dialog_new:
 *
 * Creates a new #v3270ftprogress.
 *
 * Returns: a new #v3270ftprogress.
 */
GtkWidget * v3270ftprogress_new(void) {
	return GTK_WIDGET(g_object_new(GTK_TYPE_V3270FTPROGRESS, "use-header-bar", (gint) 1, NULL));
}

void v3270ftprogress_update(GtkWidget *widget, unsigned long current, unsigned long total, double kbytes_sec) {

	gchar			* text;
	v3270ftprogress	* dialog = GTK_V3270FTPROGRESS(widget);

	debug("%s(current=%lu total=%lu kbytes/sec=%u)",__FUNCTION__,current,total,(unsigned int) kbytes_sec);

	if(current) {

		if(current != dialog->current) {

			// Recebi um bloco de dados, cancelo timeout por 10 segundos.
			dialog->timeout = time(NULL) + 10;
			dialog->current	= current;

		}

		if(total) {

			if(dialog->pulse) {
				g_source_destroy(dialog->pulse);
				dialog->pulse = NULL;
			}

			double remaining = ((double) (total - current))/1024.0;

			if(remaining > 0 && kbytes_sec > 0) {

				char buffer[40];
				double	seconds = ((double) remaining) / kbytes_sec;
				time_t 	eta		= time(0) + ((time_t) seconds);
				strftime(buffer, 39, "%H:%M:%S", localtime(&eta));

				gtk_entry_set_text(dialog->info[PROGRESS_FIELD_ETA],buffer);

			} else {

				gtk_entry_set_text(dialog->info[PROGRESS_FIELD_ETA],"-");

			}

			gtk_progress_bar_set_fraction(dialog->progress, ((gdouble) current) / ((gdouble) total));

			text = g_strdup_printf("%lu",total);
			gtk_entry_set_text(dialog->info[PROGRESS_FIELD_TOTAL],text);
			g_free(text);

		} else {

			gtk_entry_set_text(dialog->info[PROGRESS_FIELD_TOTAL],"-");

		}

		text = g_strdup_printf("%lu",current);
		gtk_entry_set_text(dialog->info[PROGRESS_FIELD_CURRENT],text);
		g_free(text);

	} else {

		// Não tem posicao de arquivo
		gtk_entry_set_text(dialog->info[PROGRESS_FIELD_CURRENT],"-");

	}

	if(kbytes_sec > 0) {
		text = g_strdup_printf("%ld KB/s",(unsigned long) kbytes_sec);
		gtk_entry_set_text(dialog->info[PROGRESS_FIELD_SPEED],text);
		g_free(text);
	} else {
		gtk_entry_set_text(dialog->info[PROGRESS_FIELD_SPEED],"");
	}


}

struct delayed_signal {
	H3270	* hSession;
	int		  signal;
	gchar	* msg;
	gchar	* text;
};

gboolean send_delayed_signal(struct delayed_signal *sig) {

	void * userdata = lib3270_ft_get_user_data(sig->hSession);

	lib3270_ft_set_user_data(sig->hSession,NULL);
	lib3270_ft_destroy(sig->hSession);

	if(userdata) {
		g_signal_emit(GTK_WIDGET(userdata),v3270ftprogress_signal[sig->signal], 0, sig->msg, sig->text);
	}

	return FALSE;

}


static void delay_signal(H3270 *hSession, int signal, const gchar *msg, const gchar *text) {

	size_t sz[] = { strlen(msg),strlen(text ? text : "") };

	struct delayed_signal *sig = g_malloc0(sizeof(struct delayed_signal)+sz[0]+sz[1]+3);

	sig->hSession	= hSession;
	sig->signal		= signal;

	sig->msg		= (char *) (sig+1);
	if(msg)
		strcpy(sig->msg,msg);

	sig->text		= sig->msg + strlen(sig->msg) + 1;
	if(text)
		strcpy(sig->text,text);

	gdk_threads_add_idle_full(G_PRIORITY_LOW,(GSourceFunc) send_delayed_signal,sig,g_free);

}

static void ft_failed(H3270 *hSession, unsigned long length,double kbytes_sec,const char *msg, void *userdata) {

	debug("%s (user_data=%p length=%lu msg=%s)",__FUNCTION__, userdata, length, msg);

	if(userdata) {

		size_t f;

		GtkWidget 	* widget	= GTK_WIDGET(userdata);
		const gchar * text		= gettext(msg);

		stop_pulse(widget);
		stop_timer(widget);

		v3270ftprogress_update(widget, length, length, kbytes_sec);
		v3270ftprogress_set_header(widget,text);

		for(f=0;f < G_N_ELEMENTS(ftmsg); f++) {

			if(!g_ascii_strncasecmp(ftmsg[f].id,msg,7)) {

				// Known message, get translated one

				while(*text && !g_ascii_isspace(*text))
					text++;

				while(*text && g_ascii_isspace(*text))
					text++;

				delay_signal(hSession, V3270FTPROGRESS_SIGNAL_FAILED, text, gettext(ftmsg[f].text));

				return;

			}

		}

		delay_signal(hSession, V3270FTPROGRESS_SIGNAL_FAILED, text, NULL);

	}

}

static void ft_complete(H3270 *hSession, unsigned long length,double kbytes_sec,const char *msg, void *userdata) {

	debug("%s (user_data=%p length=%lu msg=%s)",__FUNCTION__, userdata, length, msg);

	if(userdata) {

		size_t f;

		GtkWidget 	* widget	= GTK_WIDGET(userdata);
		const gchar * text		= gettext(msg);

		stop_pulse(widget);
		stop_timer(widget);

		v3270ftprogress_update(widget, length, length, kbytes_sec);
		v3270ftprogress_set_header(widget,text);

		for(f=0;f < G_N_ELEMENTS(ftmsg); f++) {

			if(!g_ascii_strncasecmp(ftmsg[f].id,msg,7)) {

				// Known message, get translated message

				while(*text && !g_ascii_isspace(*text))
					text++;

				while(*text && g_ascii_isspace(*text))
					text++;

				delay_signal(hSession, ftmsg[f].failed ? V3270FTPROGRESS_SIGNAL_FAILED : V3270FTPROGRESS_SIGNAL_SUCCESS, text,  gettext(ftmsg[f].text));

				return;

			}

		}

		delay_signal(hSession, V3270FTPROGRESS_SIGNAL_SUCCESS, text, NULL);

	}

}

static void ft_message(H3270 *hSession, const char *text, void *widget){

	debug("%s(%s,%p)",__FUNCTION__,text,widget);

	if(widget) {
		v3270ftprogress_set_header(GTK_WIDGET(widget),gettext(text));
	}
}

static void ft_update(H3270 *hSession, unsigned long current, unsigned long total, double kbytes_sec, void *widget) {

	if(widget) {
		v3270ftprogress_update(GTK_WIDGET(widget), current, total, kbytes_sec);
	}
}

static void ft_running(H3270 *hSession, int is_cut, void *widget) {
	debug("%s",__FUNCTION__);
	GTK_V3270FTPROGRESS(widget)->timeout = time(NULL)+10;
}

static void ft_aborting(H3270 *hSession, void *widget) {

	if(widget) {
		v3270ftprogress_set_header(GTK_WIDGET(widget),_("Aborting transfer"));
	}

}

static void ft_state_changed(H3270 *hSession, LIB3270_FT_STATE state, const char *text, void *widget) {

	if(widget) {
		v3270ftprogress_set_header(GTK_WIDGET(widget),gettext(text));
	}

}


static gboolean do_pulse(v3270ftprogress *dialog) {
	gtk_progress_bar_pulse(dialog->progress);
	return TRUE;
}

static gboolean do_timer(v3270ftprogress *dialog) {
	debug("%d",(int) (dialog->timeout - time(NULL)));

	if(time(NULL) > dialog->timeout) {

		debug("%s: Dialog timeout",__FUNCTION__);
		v3270ftprogress_set_header(GTK_WIDGET(dialog),strerror(ETIMEDOUT));

		stop_pulse(dialog);
		stop_timer(dialog);

		if(dialog->session) {
			lib3270_ft_set_user_data(dialog->session,NULL);
			lib3270_ft_destroy(dialog->session);
		}

		g_signal_emit(GTK_WIDGET(dialog),v3270ftprogress_signal[V3270FTPROGRESS_SIGNAL_FAILED], 0, _( "Transfer failed" ), strerror(ETIMEDOUT));
	}

	return TRUE;
}

void v3270ftprogress_start_transfer(GtkWidget *widget) {

	v3270ftprogress * dialog = GTK_V3270FTPROGRESS(widget);
	H3270FT  		* ft;
	const char		* message = NULL;

	if(!(dialog->session && dialog->active)) {
		g_warning("Unexpected call to %s without session or file",__FUNCTION__);
		gtk_dialog_response(GTK_DIALOG(widget),GTK_RESPONSE_REJECT);
		return;
	}

	gtk_window_set_title(GTK_WINDOW(widget),(dialog->active->options & LIB3270_FT_OPTION_RECEIVE) ? _("Receiving file") : _("Sending file"));

	ft = lib3270_ft_new(	dialog->session,
							dialog->active->options,
							dialog->active->local,
							dialog->active->remote,
							dialog->active->value[LIB3270_FT_VALUE_LRECL],
							dialog->active->value[LIB3270_FT_VALUE_BLKSIZE],
							dialog->active->value[LIB3270_FT_VALUE_PRIMSPACE],
							dialog->active->value[LIB3270_FT_VALUE_SECSPACE],
							dialog->active->value[LIB3270_FT_VALUE_DFT],
							&message);

	if(!ft) {

        if(message) {

			g_signal_emit(GTK_WIDGET(widget),v3270ftprogress_signal[V3270FTPROGRESS_SIGNAL_FAILED], 0, gettext(message), NULL);

        } else {

			g_signal_emit(GTK_WIDGET(widget),v3270ftprogress_signal[V3270FTPROGRESS_SIGNAL_FAILED], 0, _( "Can't start file transfer session" ), NULL);

        }

        return;
	}

	// Instala callbacks
	struct lib3270_ft_callbacks *cbk = lib3270_get_ft_callbacks(dialog->session, sizeof(struct lib3270_ft_callbacks));

	if(!cbk) {

		lib3270_ft_destroy(dialog->session);
		g_signal_emit(GTK_WIDGET(widget),v3270ftprogress_signal[V3270FTPROGRESS_SIGNAL_FAILED], 0, _( "Can't set callback table" ), NULL);

		return;
	}

	lib3270_ft_set_user_data(dialog->session,widget);

	cbk->complete 		= ft_complete;
	cbk->failed 		= ft_failed;
	cbk->update			= ft_update;
	cbk->running		= ft_running;
	cbk->aborting		= ft_aborting;
	cbk->state_changed	= ft_state_changed;
	cbk->message		= ft_message;

	dialog->timeout = time(NULL)+10;

	v3270ftprogress_set_header(widget,_("Starting"));

	// Pulse timer
	dialog->pulse = g_timeout_source_new(100);
	g_source_set_callback(dialog->pulse,(GSourceFunc) do_pulse,dialog,NULL);
	g_source_attach(dialog->pulse,NULL);

	// Timeout timer
	dialog->timer = g_timeout_source_new_seconds(1);
	g_source_set_callback(dialog->timer,(GSourceFunc) do_timer,dialog,NULL);
	g_source_attach(dialog->timer,NULL);

	lib3270_ft_start(dialog->session);

}
