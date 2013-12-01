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

 struct _v3270FTProgress
 {
 	GtkBin		  parent;
 	GtkWidget	* text[TEXT_COUNT];
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
	int f;

	// Create From/to fields
	static const gchar * label[TEXT_COUNT] = { N_("From"), N_("To"), N_("Status") };
	GtkWidget	* frame = gtk_frame_new( _( "Informations" ) );
	GtkGrid 	* grid	= GTK_GRID(gtk_grid_new());

	gtk_grid_set_column_spacing(grid,5);
	gtk_grid_set_row_spacing(grid,5);

	for(f=0;f<TEXT_COUNT;f++)
	{
		GtkWidget	* l		= gtk_label_new("");
		gchar 		* ptr	= g_strdup_printf("<b>%s:</b>",gettext(label[f]));

		gtk_label_set_markup(GTK_LABEL(l),ptr);
		gtk_misc_set_alignment(GTK_MISC(l),0,0);
		g_free(ptr);

		gtk_grid_attach(grid,l,0,f,1,1);

	}
	gtk_container_add(GTK_CONTAINER(frame),GTK_WIDGET(grid));



	gtk_container_add(GTK_CONTAINER(widget),GTK_WIDGET(frame));

}

GtkWidget * v3270_ft_progress_new(void)
{
	return g_object_new(GTK_TYPE_V3270FTProgress, NULL);
}
