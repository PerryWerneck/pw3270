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
 * Este programa está nomeado como accessible.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_V3270_ACCESSIBLE					(v3270_accessible_get_type ())
#define GTK_V3270_ACCESSIBLE(obj)					(G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_V3270_ACCESSIBLE, v3270Accessible))
#define GTK_V3270_ACCESSIBLE_CLASS(klass)			(G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_V3270_ACCESSIBLE, v3270AccessibleClass))
#define GTK_IS_V3270_ACCESSIBLE(obj)				(G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_V3270_ACCESSIBLE))
#define GTK_IS_V3270_ACCESSIBLE_CLASS(klass)		(G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_V3270_ACCESSIBLE))
#define GTK_V3270_ACCESSIBLE_GET_CLASS(obj)			(G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_V3270_ACCESSIBLE, v3270AccessibleClass))

typedef struct _v3270Accessible      v3270Accessible;
typedef struct _v3270AccessibleClass v3270AccessibleClass;

typedef enum _v3270_state
{
	V3270_STATE_NONE			= 0x0000,
	V3270_STATE_EDITABLE		= 0x0001,
	V3270_STATE_BUSY			= 0x0002,
	V3270_STATE_ENABLED			= 0x0004,
	V3270_STATE_INVALID_ENTRY	= 0x0008,

} V3270_STATE;

struct _v3270Accessible
{
	GtkAccessible	parent;
	V3270_STATE		state;

//	AtkLayer	layer;
};

struct _v3270AccessibleClass
{
  GtkAccessibleClass parent_class;


};

GType v3270_accessible_get_type(void);

void v3270_acessible_set_state(GtkAccessible *obj, LIB3270_MESSAGE id);

G_END_DECLS
