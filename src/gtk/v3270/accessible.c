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
 * Este programa está nomeado como accessible.c e possui - linhas de código.
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
 #include "accessible.h"

// References:
//
//	http://git.gnome.org/browse/gtk+/tree/gtk/a11y/gtkwidgetaccessible.c
//	http://git.gnome.org/browse/gtk+/tree/gtk/a11y/gtkentryaccessible.c
//

/*--[ Prototipes ]-----------------------------------------------------------------------------------*/

static void atk_editable_text_interface_init	(AtkEditableTextIface	*iface);
static void v3270_accessible_class_init			(v3270AccessibleClass	*klass);
static void v3270_accessible_init				(v3270Accessible		*widget);

/*--[ Widget definition ]----------------------------------------------------------------------------*/

G_DEFINE_TYPE_WITH_CODE (v3270Accessible, v3270_accessible, GTK_TYPE_V3270_ACCESSIBLE,
                         G_IMPLEMENT_INTERFACE (ATK_TYPE_EDITABLE_TEXT, atk_editable_text_interface_init) )

//                         G_IMPLEMENT_INTERFACE (ATK_TYPE_ACTION, atk_action_interface_init)
//                         G_IMPLEMENT_INTERFACE (ATK_TYPE_EDITABLE_TEXT, atk_editable_text_interface_init)
//                         G_IMPLEMENT_INTERFACE (ATK_TYPE_TEXT, atk_text_interface_init)

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void v3270_accessible_class_init(v3270AccessibleClass *klass)
{
	trace("******************************* %s",__FUNCTION__);
/*
  AtkObjectClass *class = ATK_OBJECT_CLASS (klass);

  klass->notify_gtk = gtk_widget_accessible_notify_gtk;

  class->get_description = gtk_widget_accessible_get_description;
  class->get_parent = gtk_widget_accessible_get_parent;
  class->ref_relation_set = gtk_widget_accessible_ref_relation_set;
  class->ref_state_set = gtk_widget_accessible_ref_state_set;
  class->get_index_in_parent = gtk_widget_accessible_get_index_in_parent;
  class->initialize = gtk_widget_accessible_initialize;
  class->get_attributes = gtk_widget_accessible_get_attributes;
  class->focus_event = gtk_widget_accessible_focus_event;
*/
}

static void v3270_accessible_init(v3270Accessible *widget)
{
	trace("*********************************** %s",__FUNCTION__);

}

static void atk_editable_text_interface_init(AtkEditableTextIface *iface)
{
	trace("********************************** %s",__FUNCTION__);
/*
	iface->set_text_contents = gtk_entry_accessible_set_text_contents;
	iface->insert_text = gtk_entry_accessible_insert_text;
	iface->copy_text = gtk_entry_accessible_copy_text;
	iface->cut_text = gtk_entry_accessible_cut_text;
	iface->delete_text = gtk_entry_accessible_delete_text;
	iface->paste_text = gtk_entry_accessible_paste_text;
	iface->set_run_attributes = NULL;
*/
}

