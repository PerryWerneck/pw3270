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
 * Este programa está nomeado como clipboard.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <gtk/gtk.h>
 #include <pw3270.h>
 #include <malloc.h>
 #include "v3270.h"
 #include "private.h"

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 enum
 {
 	CLIPBOARD_TYPE_TEXT,
 };

 static const GtkTargetEntry targets[] =
 {
	{ "COMPOUND_TEXT", 	0, CLIPBOARD_TYPE_TEXT },
	{ "UTF8_STRING", 	0, CLIPBOARD_TYPE_TEXT },
 };

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void clipboard_clear(GtkClipboard *clipboard, GObject *obj)
{
	trace("%s widget=%p",__FUNCTION__,obj);

}

static void clipboard_get(GtkClipboard *clipboard, GtkSelectionData *selection, guint target, GObject *obj)
{
	v3270 * widget = GTK_V3270(obj);

	trace("%s: widget=%p target=\"%s\"",__FUNCTION__,obj,targets[target].target);

	switch(target)
	{
	case CLIPBOARD_TYPE_TEXT:
		if(!widget->clipboard)
			lib3270_ring_bell(widget->host);
		else
			gtk_selection_data_set_text(selection,widget->clipboard,-1);
		break;

	default:
		g_warning("Unexpected clipboard type %d\n",target);
	}
}

void v3270_copy_clipboard(v3270 *widget)
{
	char *text;
	GtkClipboard * clipboard = gtk_widget_get_clipboard(GTK_WIDGET(widget),GDK_SELECTION_CLIPBOARD);

	if(widget->clipboard)
	{
		g_free(widget->clipboard);
		widget->clipboard = NULL;
	}

	text = lib3270_get_selected(widget->host);

	if(!text)
	{
		lib3270_ring_bell(widget->host);
		return;
	}

	widget->clipboard = g_convert(text, -1, "UTF-8", lib3270_get_charset(widget->host), NULL, NULL, NULL);

	free(text);

	if(gtk_clipboard_set_with_owner(	clipboard,
										targets,
										G_N_ELEMENTS(targets),
										(GtkClipboardGetFunc)	clipboard_get,
										(GtkClipboardClearFunc) clipboard_clear,
										G_OBJECT(widget)
										))
	{
		gtk_clipboard_set_can_store(clipboard,targets,1);
	}
}

static void paste_text(GtkWidget *widget, const gchar *text, const gchar *encoding)
{
 	gchar 	*buffer = NULL;
 	gchar 	*ptr;
 	GError	*error = NULL;
 	H3270	*session = v3270_get_session(widget);

 	if(!text)
		return;

	buffer = g_convert(text, -1, lib3270_get_charset(session), encoding, NULL, NULL, &error);

    if(!buffer)
    {
    	/* Falhou ao converter - Reajusta e tenta de novo */
    	int f;

    	static const struct _xlat
    	{
    		const gchar *from;
    		const gchar *to;
    	} xlat[] =
    	{
    		{ "–",		"-"		},
    		{ "→",		"->"	},
    		{ "←",		"<-" 	},
    		{ "©",		"(c)"	},
    		{ "↔",		"<->"	},
    		{ "™",		"(TM)"	},
    		{ "®",		"(R)"	},
    		{ "“",		"\""	},
    		{ "”",		"\""	},
    		{ "…",		"..."	},
    		{ "•",		"*"		},
    		{ "․",		"."		},
    		{ "·",		"*"		},

    	};

		gchar *string = g_strdup(text);

		if(error)
		{
			g_error_free(error);
			error = NULL;
		}

		// FIXME (perry#1#): Is there any better way for a "sed" here?
		for(f=0;f<G_N_ELEMENTS(xlat);f++)
		{
			gchar *ptr = g_strstr_len(string,-1,xlat[f].from);

			if(ptr)
			{
				gchar *old = string;
				gchar **tmp = g_strsplit(old,xlat[f].from,-1);
				string = g_strjoinv(xlat[f].to,tmp);
				g_strfreev(tmp);
				g_free(old);
			}
		}

		buffer = g_convert(string, -1, lib3270_get_charset(session), encoding, NULL, NULL, &error);

		if(!buffer)
		{
			gchar **ln = g_strsplit(string,"\n",-1);

			for(f=0;ln[f];f++)
			{
				gchar *str = g_convert(ln[f], -1, lib3270_get_charset(session), encoding, NULL, NULL, &error);

				if(error)
				{
					g_error_free(error);
					error = 0;
				}

				if(!str)
				{
					GtkWidget *dialog = gtk_message_dialog_new(	GTK_WINDOW( gtk_widget_get_toplevel(widget)),
																GTK_DIALOG_DESTROY_WITH_PARENT,
																GTK_MESSAGE_ERROR,
																GTK_BUTTONS_OK,
																_(  "Can't convert line %d from %s to %s" ),f+1, encoding, lib3270_get_charset(session));

					gtk_window_set_title(GTK_WINDOW(dialog), _( "Charset error" ) );
					if(error)
					{
						gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s\n%s", error->message ? error->message : N_( "Unexpected error" ), ln[f]);
						g_error_free(error);
						error = 0;
					}
					else
					{
						gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s", ln[f]);
					}
					gtk_dialog_run(GTK_DIALOG (dialog));
					gtk_widget_destroy(dialog);
					return;

				}
				else
				{
					g_free(str);
				}
			}

			g_strfreev(ln);
			g_free(string);
		}

		g_free(string);

		if(error)
		{
			g_error_free(error);
			error = 0;
		}


    	if(!buffer)
    	{
			GtkWidget *dialog = gtk_message_dialog_new(	GTK_WINDOW( gtk_widget_get_toplevel(widget)),
														GTK_DIALOG_DESTROY_WITH_PARENT,
														GTK_MESSAGE_ERROR,
														GTK_BUTTONS_OK,
														_(  "Can't convert text from %s to %s" ), encoding, lib3270_get_charset(session));

			gtk_window_set_title(GTK_WINDOW(dialog), _( "Charset error" ) );
			if(error)
			{
				gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s", error->message ? error->message : N_( "Unexpected error" ));
				g_error_free(error);
				error = 0;
			}
			gtk_dialog_run(GTK_DIALOG (dialog));
			gtk_widget_destroy(dialog);

			return;
    	}
    }

	if(error)
		g_error_free(error);

    /* Remove TABS */
    for(ptr = buffer;*ptr;ptr++)
    {
		if(*ptr == '\t')
			*ptr = ' ';
    }

	trace("Received text:%p (%d bytes)",buffer,(int) strlen(buffer));

//	paste_string(buffer);

	g_free(buffer);

}

static void text_received(GtkClipboard *clipboard, const gchar *text, GtkWidget *widget)
{
	paste_text(widget,text,"UTF-8");
}

void v3270_paste_clipboard(v3270 *widget)
{
	gtk_clipboard_request_text(gtk_widget_get_clipboard(GTK_WIDGET(widget),GDK_SELECTION_CLIPBOARD),(GtkClipboardTextReceivedFunc) text_received,(gpointer) widget);
}

