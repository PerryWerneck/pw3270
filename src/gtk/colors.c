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
 * Este programa está nomeado como colors.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <gtk/gtk.h>
 #include "globals.h"

/*--[ Implement ]------------------------------------------------------------------------------------*/

 void load_color_schemes(GtkWidget *widget, gchar *active)
 {
	gchar *filename = build_data_filename("colors.conf",NULL);

 	if(!g_file_test(filename,G_FILE_TEST_IS_REGULAR))
	{
		gtk_widget_set_sensitive(widget,FALSE);
		g_warning("Unable to load color schemes in \"%s\"",filename);
	}
	else
	{
		gchar 		** group;
		GKeyFile	*  conf 	= g_key_file_new();
		int			   f		= 0;
		gboolean	   found 	= FALSE;

#if !GTK_CHECK_VERSION(3,0,0)
		GtkTreeModel	* model		= (GtkTreeModel *) gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_STRING);
		GtkCellRenderer * renderer	= gtk_cell_renderer_text_new();
		GtkTreeIter		  iter;

		gtk_combo_box_set_model(GTK_COMBO_BOX(widget),model);

		gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget), renderer, TRUE);
		gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), renderer, "text", 0, NULL);

#endif // !GTK(3,0,0)

		g_key_file_load_from_file(conf,filename,G_KEY_FILE_NONE,NULL);

		group = g_key_file_get_groups(conf,NULL);

		for(f=0;group[f];f++)
		{
			gchar *str = g_strjoin( ",",	g_key_file_get_string(conf,group[f],"Terminal",NULL),
											g_key_file_get_string(conf,group[f],"BaseAttributes",NULL),
											g_key_file_get_string(conf,group[f],"SelectedText",NULL),
											g_key_file_get_string(conf,group[f],"Cursor",NULL),
											g_key_file_get_string(conf,group[f],"OIA",NULL),
											NULL
								);
#if GTK_CHECK_VERSION(3,0,0)

			gtk_combo_box_text_insert(		GTK_COMBO_BOX_TEXT(widget),
											f,
											str,
											g_key_file_get_locale_string(conf,group[f],"Label",NULL,NULL));


			if(active && !g_strcasecmp(active,str))
			{
				found = TRUE;
				gtk_combo_box_set_active(GTK_COMBO_BOX(widget),f);
			}
#else

			gtk_list_store_append((GtkListStore *) model,&iter);
			gtk_list_store_set((GtkListStore *) model, &iter,
												0, g_key_file_get_locale_string(conf,group[f],"Label",NULL,NULL),
												1, str,
												-1);

			if(active && !g_strcasecmp(active,str))
			{
				found = TRUE;
				gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&iter);
			}

#endif // GTK(3,0,0)

			g_free(str);
		}

		g_strfreev(group);
		g_key_file_free(conf);

		if(active && !found)
		{
#if GTK_CHECK_VERSION(3,0,0)

			gtk_combo_box_text_insert(		GTK_COMBO_BOX_TEXT(widget),
											0,
											active,
											_( "Custom colors") );
			gtk_combo_box_set_active(GTK_COMBO_BOX(widget),0);

#else

			gtk_list_store_append((GtkListStore *) model,&iter);
			gtk_list_store_set((GtkListStore *) model, &iter,
												0, _( "Custom colors" ),
												1, active,
												-1);

			gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget),&iter);
#endif
		}

		gtk_widget_set_sensitive(widget,TRUE);

	}

	g_free(filename);
 }


