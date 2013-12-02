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
 * Este programa está nomeado como ftprogress.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "v3270ft.h"

/*--[ Widget definition ]----------------------------------------------------------------------------*/

 enum _filename
 {
 	TEXT_LOCALFILE,
 	TEXT_HOSTFILE,
 	TEXT_STATUS,

	TEXT_COUNT
 };

 enum _value
 {
	VALUE_TOTAL,
	VALUE_CURRENT,
	VALUE_SPEED,
	VALUE_ETA,

	VALUE_COUNT
 };

 struct _v3270FTProgress
 {
 	GtkBin			  parent;
 	GtkLabel		* text[TEXT_COUNT];
 	GtkLabel		* value[VALUE_COUNT];
 	GtkProgressBar	* progress;
 };

 struct _v3270FTProgressClass
 {
	GtkBinClass parent_class;

 };

 G_DEFINE_TYPE(v3270FTProgress, v3270FTProgress, GTK_TYPE_BIN);


/*--[ Implement ]------------------------------------------------------------------------------------*/

static void v3270FTProgress_class_init(v3270FTProgressClass *klass)
{
//	GtkDialogClass	* widget_class	= GTK_DIALOG_CLASS(klass);

#if GTK_CHECK_VERSION(3,0,0)

#else

	#error Implementar

#endif // GTK_CHECK_VERSION

}

static void v3270FTProgress_init(v3270FTProgress *widget)
{
	GtkWidget	* frame;
	GtkGrid 	* grid;
	int			  f;
	GtkWidget 	* box	= gtk_box_new(GTK_ORIENTATION_VERTICAL,3);

	gtk_container_set_border_width(GTK_CONTAINER(box),3);

	// Create From/to fields
	static const gchar * label[TEXT_COUNT] = { N_("From"), N_("To"), N_("Status") };

	frame = gtk_frame_new( _( "Informations" ) );
	grid = GTK_GRID(gtk_grid_new());

	gtk_container_set_border_width(GTK_CONTAINER(grid),3);
	gtk_grid_set_column_spacing(grid,5);
	gtk_grid_set_row_spacing(grid,5);
	gtk_widget_set_hexpand(GTK_WIDGET(grid),TRUE);


	for(f=0;f<TEXT_COUNT;f++)
	{
		GtkWidget	* l		= gtk_label_new("");
		gchar 		* ptr	= g_strdup_printf("<b>%s:</b>",gettext(label[f]));

		gtk_label_set_markup(GTK_LABEL(l),ptr);
		gtk_misc_set_alignment(GTK_MISC(l),0,0);
		g_free(ptr);

		gtk_grid_attach(grid,l,0,f,1,1);

		widget->text[f] = GTK_LABEL(gtk_label_new("-"));
		gtk_label_set_ellipsize(widget->text[f],PANGO_ELLIPSIZE_START);
		gtk_label_set_width_chars(widget->text[f],50);
		gtk_misc_set_alignment(GTK_MISC(widget->text[f]),0,0);
		gtk_widget_set_hexpand(GTK_WIDGET(widget->text[f]),TRUE);

		gtk_grid_attach(grid,GTK_WIDGET(widget->text[f]),1,f,1,1);


	}
	gtk_container_add(GTK_CONTAINER(frame),GTK_WIDGET(grid));
	gtk_container_add(GTK_CONTAINER(box),GTK_WIDGET(frame));

	// Progress info
	static const gchar *progressLabel[VALUE_COUNT] = { N_( "Total" ), N_( "Current" ), N_( "Speed" ), N_( "ETA" ) };

	frame = gtk_frame_new( _( "Progress" ) );
	grid	= GTK_GRID(gtk_grid_new());

	gtk_container_set_border_width(GTK_CONTAINER(grid),3);
	gtk_grid_set_column_spacing(grid,5);
	gtk_grid_set_row_spacing(grid,5);
	gtk_widget_set_hexpand(GTK_WIDGET(grid),TRUE);
	gtk_grid_set_column_homogeneous(grid,TRUE);

	for(f=0;f<VALUE_COUNT;f++)
	{
		GtkWidget	* l		= gtk_label_new(_("N/A"));
		int 		  r		= f/2;
		int			  c		= (f&1)*2;
		gchar 		* ptr	= g_strdup_printf("<b>%s:</b>",gettext(progressLabel[f]));

		gtk_label_set_markup(GTK_LABEL(l),ptr);
		gtk_misc_set_alignment(GTK_MISC(l),0,0);
		g_free(ptr);

		gtk_grid_attach(grid,l,c,r,1,1);

		widget->value[f] = GTK_LABEL(gtk_label_new(_("N/A")));
		gtk_misc_set_alignment(GTK_MISC(widget->value[f]),1,0);
		gtk_widget_set_hexpand(GTK_WIDGET(widget->value[f]),TRUE);

		gtk_grid_attach(grid,GTK_WIDGET(widget->value[f]),c+1,r,1,1);


	}

 	widget->progress = GTK_PROGRESS_BAR(gtk_progress_bar_new());
	gtk_widget_set_hexpand(GTK_WIDGET(widget->progress),TRUE);
	gtk_grid_attach(grid,GTK_WIDGET(widget->progress),0,2,4,1);

	gtk_container_add(GTK_CONTAINER(frame),GTK_WIDGET(grid));
	gtk_container_add(GTK_CONTAINER(box),GTK_WIDGET(frame));

	// Add box on parent widget
	gtk_container_add(GTK_CONTAINER(widget),GTK_WIDGET(box));

}

GtkWidget * v3270_ft_progress_new(void)
{
	return g_object_new(GTK_TYPE_V3270FTProgress, NULL);
}

void v3270_ft_progress_update(GtkWidget *widget, unsigned long current, unsigned long total, double kbytes_sec)
{
	g_return_if_fail(GTK_IS_V3270FTProgress(widget));

	v3270FTProgress *obj = GTK_V3270FTProcess(widget);

	if(current)
	{
		gchar *str = g_strdup_printf("%ld",current);
		gtk_label_set_text(obj->value[VALUE_CURRENT],str);
		g_free(str);
	}

	if(total)
	{
		gchar *str = g_strdup_printf("%ld",total);
		gtk_label_set_text(obj->value[VALUE_TOTAL],str);
		g_free(str);
	}

	if(kbytes_sec)
	{
		gchar *str = g_strdup_printf("%ld KB/s",(unsigned long) kbytes_sec);
		gtk_label_set_text(obj->value[VALUE_SPEED],str);
		g_free(str);
	}

	if(total && current)
		gtk_progress_bar_set_fraction(obj->progress,((gdouble) current) / ((gdouble) total));
	else
		gtk_progress_bar_pulse(obj->progress);

}

void v3270_ft_progress_set_message(GtkWidget *widget, const gchar *msg)
{
	g_return_if_fail(GTK_IS_V3270FTProgress(widget));
	gtk_label_set_text(GTK_V3270FTProcess(widget)->text[TEXT_STATUS],msg);
}

void v3270_ft_progress_set_host_filename(GtkWidget *widget, const gchar *name)
{
	g_return_if_fail(GTK_IS_V3270FTProgress(widget));
	gtk_label_set_text(GTK_V3270FTProcess(widget)->text[TEXT_HOSTFILE],name);
}

void v3270_ft_progress_set_local_filename(GtkWidget *widget, const gchar *name)
{
	g_return_if_fail(GTK_IS_V3270FTProgress(widget));
	gtk_label_set_text(GTK_V3270FTProcess(widget)->text[TEXT_LOCALFILE],name);
}
