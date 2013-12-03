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
 * Este programa está nomeado como filetransfer.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 * Agradecimento:
 *
 * Roberto Soares 			(a_r_soares@hotmail.com)
 *
 */

#include <stdlib.h>
#include "globals.h"
#include "ft/v3270ft.h"


/*--[ Constants ]------------------------------------------------------------------------------------*/

	static const struct _ftoptions
	{
		LIB3270_FT_OPTION	  val;
		const gchar			* name;
	}
	ftoptions[] =
	{	{	LIB3270_FT_OPTION_ASCII,		"text"		},
		{	LIB3270_FT_OPTION_CRLF,			"cr"		},
		{	LIB3270_FT_OPTION_APPEND,		"append"	},
		{   LIB3270_FT_OPTION_REMAP,		"remap"		}
	};

	static const struct _recfm
	{
		LIB3270_FT_OPTION	  val;
		const gchar			* name;
	} recfm[] =
	{
		{ LIB3270_FT_RECORD_FORMAT_DEFAULT,			"default" 		},
		{ LIB3270_FT_RECORD_FORMAT_FIXED,			"fixed"			},
		{ LIB3270_FT_RECORD_FORMAT_VARIABLE,		"variable"		},
		{ LIB3270_FT_RECORD_FORMAT_UNDEFINED,		"undefined"		}
	};

	static const struct _ftunits
	{
		LIB3270_FT_OPTION	  val;
		const gchar			* name;
	} units[] =
	{
		{ LIB3270_FT_ALLOCATION_UNITS_DEFAULT,		"default"		},
		{ LIB3270_FT_ALLOCATION_UNITS_TRACKS,		"tracks"		},
		{ LIB3270_FT_ALLOCATION_UNITS_CYLINDERS,	"cylinders"		},
		{ LIB3270_FT_ALLOCATION_UNITS_AVBLOCK,		"avblock"		}
	};


/*--[ Implement ]------------------------------------------------------------------------------------*/

static LIB3270_FT_OPTION get_options_from_config(const gchar *group)
{
	int					  f;
	LIB3270_FT_OPTION	  rc	= 0;
	gchar				* ptr;

	// Get base FT options
	for(f=0;f<G_N_ELEMENTS(ftoptions);f++)
	{
		if(get_boolean_from_config(group,ftoptions[f].name,FALSE))
			rc |= ftoptions[f].val;
	}

	// Get Record format
	ptr = get_string_from_config(group, "units", "default");
	for(f=0;f<G_N_ELEMENTS(units);f++)
	{
		if(!g_ascii_strcasecmp(ptr,units[f].name))
			rc |= units[f].val;
	}
	g_free(ptr);

	// Get allocation units
	ptr = get_string_from_config(group, "recfm", "default");
	for(f=0;f<G_N_ELEMENTS(recfm);f++)
	{
		if(!g_ascii_strcasecmp(ptr,recfm[f].name))
			rc |= recfm[f].val;
	}
	g_free(ptr);


	return rc;
}

static void ft_dialog_load(GtkWidget *widget, const gchar *name)
{
	gchar *ptr;

	ptr = get_string_from_config(name, "remote", "");
	v3270_ft_dialog_set_host_filename(widget,ptr);
	g_free(ptr);

	ptr = get_string_from_config(name, "local", "");
	v3270_ft_dialog_set_local_filename(widget,ptr);
	g_free(ptr);

	v3270_ft_dialog_set_dft_buffer_size(widget,get_integer_from_config(name,"dft",4096));
	v3270_ft_dialog_set_record_length(widget,get_integer_from_config(name,"reclen",0));
	v3270_ft_dialog_set_block_size(widget,get_integer_from_config(name,"blksize",0));
	v3270_ft_dialog_set_primary_space(widget,get_integer_from_config(name,"primspace",0));
	v3270_ft_dialog_set_secondary_space(widget,get_integer_from_config(name,"secspace",0));

}

static void ft_dialog_save(GtkWidget *widget, const gchar *name)
{
	LIB3270_FT_OPTION	  opt		= v3270_ft_dialog_get_options(widget);
	const gchar			* filename	= v3270_ft_dialog_get_local_filename(widget);
	int 				  f;

	for(f=0;f<G_N_ELEMENTS(ftoptions);f++)
	{
		trace("%s=%s",ftoptions[f].name,((opt & ftoptions[f].val) != 0) ? "ON" : "OFF");
		set_boolean_to_config(name, ftoptions[f].name, ((opt & ftoptions[f].val) != 0));
	}

	for(f=0;f<G_N_ELEMENTS(recfm);f++)
	{
		if((opt & LIB3270_FT_RECORD_FORMAT_MASK) == recfm[f].val)
			set_string_to_config(name,"recfm","%s",recfm[f].name);
	}

	for(f=0;f<G_N_ELEMENTS(units);f++)
	{
		if( (opt & LIB3270_FT_ALLOCATION_UNITS_MASK) == units[f].val)
			set_string_to_config(name,"units","%s",units[f].name);
	}

	set_integer_to_config(name,"dft",v3270_ft_dialog_get_dft_buffer_size(widget));
	set_integer_to_config(name,"reclen",v3270_ft_dialog_get_record_length(widget));
	set_integer_to_config(name,"blksize",v3270_ft_dialog_get_block_size(widget));
	set_integer_to_config(name,"primspace",v3270_ft_dialog_get_primary_space(widget));
	set_integer_to_config(name,"secspace",v3270_ft_dialog_get_secondary_space(widget));

	set_string_to_config(name,"local","%s",filename);
	set_string_to_config(name,"remote","%s",v3270_ft_dialog_get_host_filename(widget));

#ifdef HAVE_WIN_REGISTRY
	gchar *ext = strrchr(filename,'.');

	if(ext)
	{
		// Save extension based file settings
		HKEY	  hKey;
		DWORD	  disp;
		gchar	* path = g_strdup_printf("%s\\%s\\%s\\%s","SOFTWARE",g_get_application_name(),name,ext+1);

		if(RegCreateKeyEx(HKEY_CURRENT_USER,path,0,NULL,REG_OPTION_NON_VOLATILE,KEY_SET_VALUE,NULL,&hKey,&disp) == ERROR_SUCCESS)
		{
			DWORD value;

			value = (DWORD) v3270_ft_dialog_get_options(widget);
			RegSetValueEx(hKey, "options", 0, REG_DWORD,(const BYTE *) &value,sizeof(value));

			value = (DWORD) v3270_ft_dialog_get_dft_buffer_size(widget);
			RegSetValueEx(hKey, "dft", 0, REG_DWORD,(const BYTE *) &value,sizeof(value));

			value = (DWORD) v3270_ft_dialog_get_record_length(widget);
			RegSetValueEx(hKey, "reclen", 0, REG_DWORD,(const BYTE *) &value,sizeof(value));

			value = (DWORD) v3270_ft_dialog_get_block_size(widget);
			RegSetValueEx(hKey, "blksize", 0, REG_DWORD,(const BYTE *) &value,sizeof(value));

			value = (DWORD) v3270_ft_dialog_get_primary_space(widget);
			RegSetValueEx(hKey, "primspace", 0, REG_DWORD,(const BYTE *) &value,sizeof(value));

			value = (DWORD) v3270_ft_dialog_get_secondary_space(widget);
			RegSetValueEx(hKey, "secspace", 0, REG_DWORD,(const BYTE *) &value,sizeof(value));

			RegCloseKey(hKey);
		}

		g_free(path);
	}

#endif // HAVE_WIN_REGISTRY

}

static void ft_complete(H3270FT *ft, unsigned long length,double kbytes_sec)
{
	v3270_ft_progress_complete(GTK_WIDGET(ft->widget),length,kbytes_sec);
}

static void ft_message(struct _h3270ft *ft, const char *text)
{
	v3270_ft_progress_set_message(GTK_WIDGET(ft->widget),gettext(text));
}

static void ft_update(H3270FT *ft, unsigned long current, unsigned long length, double kbytes_sec)
{
	v3270_ft_progress_update(GTK_WIDGET(ft->widget), current, length, kbytes_sec);
}

static void ft_running(H3270FT *ft, int is_cut)
{
}

static void ft_aborting(H3270FT *ft)
{
}

static void ft_state_changed(H3270FT *ft, LIB3270_FT_STATE state)
{
}

gint v3270_transfer_file(GtkWidget *widget, LIB3270_FT_OPTION options, const gchar *local, const gchar *remote, int lrecl, int blksize, int primspace, int secspace, int dft)
{
	g_return_val_if_fail(GTK_IS_V3270(widget),NULL);

	H3270FT		* ft		= lib3270_ft_new(v3270_get_session(widget),options,local,remote,lrecl,blksize,primspace,secspace,dft);

	if(!ft)
		return -1;

	GtkWidget	* dialog	= gtk_dialog_new_with_buttons(
											(options & LIB3270_FT_OPTION_RECEIVE) ? _( "Receiving file" ) : _( "Sending file" ),
											GTK_WINDOW(gtk_widget_get_toplevel(widget)),
											GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
											_( "_Cancel" ), GTK_RESPONSE_CANCEL,NULL );


	// Create FT progress dialog
	GtkWidget	* progress	= v3270_ft_progress_new();

	ft->widget 			= progress;
	ft->complete 		= ft_complete;
	ft->update			= ft_update;
	ft->running			= ft_running;
	ft->aborting		= ft_aborting;
	ft->state_changed	= ft_state_changed;
	ft->message			= ft_message;

	v3270_ft_progress_set_host_filename(progress,remote);
	v3270_ft_progress_set_local_filename(progress,local);
	v3270_ft_progress_set_message(progress,_("Starting transfer"));

	gtk_widget_show_all(progress);
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),GTK_WIDGET(progress),FALSE,TRUE,2);

	gtk_widget_show_all(dialog);
	lib3270_ft_start(v3270_get_session(widget));
	int rc = gtk_dialog_run(GTK_DIALOG(dialog));
	lib3270_ft_destroy(v3270_get_session(widget));

	gtk_widget_destroy(dialog);

	return rc;

}

void download_action(GtkAction *action, GtkWidget *widget)
{
	const gchar *name = g_object_get_data(G_OBJECT(action),"configuration");

	if(!name)
		name = "download";

	if(lib3270_get_ft_state(v3270_get_session(widget)) != LIB3270_FT_STATE_NONE)
	{
		lib3270_popup_dialog(	v3270_get_session(widget),
								LIB3270_NOTIFY_ERROR,
								_( "Request failed" ),
								_( "Can't start download." ),
								"%s",
								_( "File transfer is already active." ));

		return;
	}

	GtkWidget *dialog 	= v3270_ft_dialog_new(widget,LIB3270_FT_OPTION_RECEIVE|get_options_from_config(name));

	v3270_ft_dialog_set_tso(dialog,lib3270_is_tso(v3270_get_session(widget)));

	ft_dialog_load(dialog,name);

    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
		ft_dialog_save(dialog,name);
		gtk_widget_hide(dialog);

		v3270_transfer_file(	widget,
								v3270_ft_dialog_get_options(dialog),
								v3270_ft_dialog_get_local_filename(dialog),
								v3270_ft_dialog_get_host_filename(dialog),
								v3270_ft_dialog_get_record_length(dialog),
								v3270_ft_dialog_get_block_size(dialog),
								v3270_ft_dialog_get_primary_space(dialog),
								v3270_ft_dialog_get_secondary_space(dialog),
								v3270_ft_dialog_get_dft_buffer_size(dialog));
    }

    gtk_widget_destroy(dialog);

}

void upload_action(GtkAction *action, GtkWidget *widget)
{
	const gchar *name = g_object_get_data(G_OBJECT(action),"configuration");

	if(!name)
		name = "upload";

	if(lib3270_get_ft_state(v3270_get_session(widget)) != LIB3270_FT_STATE_NONE)
	{
		lib3270_popup_dialog(	v3270_get_session(widget),
								LIB3270_NOTIFY_ERROR,
								_( "Request failed" ),
								_( "Can't start upload." ),
								"%s",
								_( "File transfer is already active." ));

		return;
	}

	GtkWidget *dialog 	= v3270_ft_dialog_new(widget,LIB3270_FT_OPTION_SEND|get_options_from_config(name));

	v3270_ft_dialog_set_tso(dialog,lib3270_is_tso(v3270_get_session(widget)));
	ft_dialog_load(dialog,name);

    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
    {
		ft_dialog_save(dialog,name);
		gtk_widget_hide(dialog);

		v3270_transfer_file(	widget,
								v3270_ft_dialog_get_options(dialog),
								v3270_ft_dialog_get_local_filename(dialog),
								v3270_ft_dialog_get_host_filename(dialog),
								v3270_ft_dialog_get_record_length(dialog),
								v3270_ft_dialog_get_block_size(dialog),
								v3270_ft_dialog_get_primary_space(dialog),
								v3270_ft_dialog_get_secondary_space(dialog),
								v3270_ft_dialog_get_dft_buffer_size(dialog));
    }

    gtk_widget_destroy(dialog);

}


