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
 #include "v3270.h"
 #include "accessible.h"

// References:
//
//	http://git.gnome.org/browse/gtk+/tree/gtk/a11y/gtkwidgetaccessible.c
//	http://git.gnome.org/browse/gtk+/tree/gtk/a11y/gtkentryaccessible.c
//

/*--[ Prototipes ]-----------------------------------------------------------------------------------*/

static void atk_editable_text_interface_init	(AtkEditableTextIface	*iface);
static void atk_text_interface_init				(AtkTextIface			*iface);
static void atk_action_interface_init			(AtkActionIface			*iface);
static void v3270_accessible_class_init			(v3270AccessibleClass	*klass);
static void v3270_accessible_init				(v3270Accessible		*widget);

/*--[ Widget definition ]----------------------------------------------------------------------------*/

G_DEFINE_TYPE_WITH_CODE (v3270Accessible, v3270_accessible, GTK_TYPE_V3270_ACCESSIBLE,
                         G_IMPLEMENT_INTERFACE (ATK_TYPE_EDITABLE_TEXT, atk_editable_text_interface_init)
                         G_IMPLEMENT_INTERFACE (ATK_TYPE_TEXT, atk_text_interface_init)
                         G_IMPLEMENT_INTERFACE (ATK_TYPE_ACTION, atk_action_interface_init))

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void v3270_accessible_class_init(v3270AccessibleClass *klass)
{

}

static void v3270_accessible_init(v3270Accessible *widget)
{

}

static void atk_editable_text_interface_init(AtkEditableTextIface *iface)
{

}

static void atk_text_interface_init(AtkTextIface *iface)
{

}

static void atk_action_interface_init(AtkActionIface *iface)
{

}

