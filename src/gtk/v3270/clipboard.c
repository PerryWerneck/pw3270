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
//	v3270 *widget = GTK_V3270(obj);

	trace("%s: widget=%p target=\"%s\"",__FUNCTION__,obj,targets[target].target);

	switch(target)
	{
	case CLIPBOARD_TYPE_TEXT:
		gtk_selection_data_set_text(selection,"teste de clipboard",-1);
		break;

	default:
		g_warning("Unexpected clipboard type %d\n",target);
	}
}

void v3270_copy_clipboard(v3270 *widget)
{
	GtkClipboard * clipboard = gtk_widget_get_clipboard(GTK_WIDGET(widget),GDK_SELECTION_CLIPBOARD);

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

