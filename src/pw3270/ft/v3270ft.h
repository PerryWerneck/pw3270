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
 * Este programa está nomeado como v3270ft.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

#ifndef V3270FT_H_INCLUDED

 #define ENABLE_NLS

 #ifndef GETTEXT_PACKAGE
        #define GETTEXT_PACKAGE PACKAGE_NAME
 #endif

 #include <libintl.h>
 #include <glib/gi18n.h>
 #include <gtk/gtk.h>
 #include <lib3270.h>
 #include <lib3270/filetransfer.h>

 #define V3270FT_H_INCLUDED 1

 G_BEGIN_DECLS

/*--[ Select file dialog ]---------------------------------------------------------------------------*/
 #define GTK_TYPE_V3270FTD				(v3270FTD_get_type ())
 #define GTK_V3270FTD(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_V3270FTD, v3270FTD))
 #define GTK_V3270FTD_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_V3270FTD, v3270FTDClass))
 #define GTK_IS_V3270FTD(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_V3270FTD))
 #define GTK_IS_V3270FTD_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_V3270FTD))
 #define GTK_V3270FTD_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_V3270FTD, v3270FTDClass))

 typedef struct _v3270FTD		v3270FTD;
 typedef struct _v3270FTDClass	v3270FTDClass;

/*--[ Progress widget ]------------------------------------------------------------------------------*/

 #define GTK_TYPE_V3270FTProgress				(v3270FTProgress_get_type ())
 #define GTK_V3270FTProcess(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_V3270FTProgress, v3270FTProgress))
 #define GTK_V3270FTProgress_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_V3270FTProgress, v3270FTProgressClass))
 #define GTK_IS_V3270FTProgress(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_V3270FTProgress))
 #define GTK_IS_V3270FTProgress_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_V3270FTProgress))
 #define GTK_V3270FTProgress_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_V3270FTProgress, v3270FTProgressClass))

 typedef struct _v3270FTProgress		v3270FTProgress;
 typedef struct _v3270FTProgressClass	v3270FTProgressClass;

/*--[ Prototipes ]-----------------------------------------------------------------------------------*/
 LIB3270_EXPORT	GtkWidget			* v3270_ft_dialog_new(GtkWidget *parent, LIB3270_FT_OPTION options);
 LIB3270_EXPORT	void				  v3270_ft_dialog_set_host_filename(GtkWidget *widget, const gchar *name);
 LIB3270_EXPORT	void				  v3270_ft_dialog_set_local_filename(GtkWidget *widget, const gchar *name);
 LIB3270_EXPORT const gchar			* v3270_ft_dialog_get_host_filename(GtkWidget *widget);
 LIB3270_EXPORT const gchar			* v3270_ft_dialog_get_local_filename(GtkWidget *widget);
 LIB3270_EXPORT void				  v3270_ft_dialog_set_options(GtkWidget *widget,LIB3270_FT_OPTION options);
 LIB3270_EXPORT LIB3270_FT_OPTION	  v3270_ft_dialog_get_options(GtkWidget *widget);
 LIB3270_EXPORT void				  v3270_ft_dialog_set_tso(GtkWidget *widget,gboolean flag);
 LIB3270_EXPORT void				  v3270_ft_dialog_set_dft_buffer_size(GtkWidget *widget, gint value);
 LIB3270_EXPORT void 				  v3270_ft_dialog_set_record_length(GtkWidget *widget, gint value);
 LIB3270_EXPORT void 				  v3270_ft_dialog_set_block_size(GtkWidget *widget, gint value);
 LIB3270_EXPORT void 				  v3270_ft_dialog_set_primary_space(GtkWidget *widget, gint value);
 LIB3270_EXPORT void 				  v3270_ft_dialog_set_secondary_space(GtkWidget *widget, gint value);

 LIB3270_EXPORT gint				  v3270_ft_dialog_get_dft_buffer_size(GtkWidget *widget);
 LIB3270_EXPORT gint 				  v3270_ft_dialog_get_record_length(GtkWidget *widget);
 LIB3270_EXPORT gint 				  v3270_ft_dialog_get_block_size(GtkWidget *widget);
 LIB3270_EXPORT gint 				  v3270_ft_dialog_get_primary_space(GtkWidget *widget);
 LIB3270_EXPORT gint 				  v3270_ft_dialog_get_secondary_space(GtkWidget *widget);

 LIB3270_EXPORT GtkWidget			* v3270_ft_progress_new(void);
 LIB3270_EXPORT void				  v3270_ft_progress_update(GtkWidget *widget, unsigned long current, unsigned long total, double kbytes_sec);
 LIB3270_EXPORT void				  v3270_ft_progress_set_message(GtkWidget *widget, const gchar *msg);
 LIB3270_EXPORT	void				  v3270_ft_progress_set_host_filename(GtkWidget *widget, const gchar *name);
 LIB3270_EXPORT	void				  v3270_ft_progress_set_local_filename(GtkWidget *widget, const gchar *name);
 LIB3270_EXPORT void				  v3270_ft_progress_complete(GtkWidget *widget,unsigned long length,double kbytes_sec);



 G_END_DECLS

#endif // V3270_H_INCLUDED
