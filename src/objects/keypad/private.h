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
 * Este programa está nomeado como - e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#ifndef PRIVATE_H_INCLUDED

#define PRIVATE_H_INCLUDED

#include <config.h>

#ifndef GETTEXT_PACKAGE
#define GETTEXT_PACKAGE PACKAGE_NAME
#endif

#include <libintl.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>

#include <lib3270.h>
#include <lib3270/log.h>

#include <pw3270/keypad.h>

G_BEGIN_DECLS

#define I_(string) g_intern_static_string (string)

#define PW_TYPE_KEYPAD_ELEMENT				(KeypadElement_get_type())
#define PW_KEYPAD_ELEMENT(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), PW_TYPE_KEYPAD_ELEMENT, KeypadElement))
#define PW_KEYPAD_ELEMENT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), PW_TYPE_KEYPAD_ELEMENT, KeypadModelClass))
#define PW_IS_KEYPAD_ELEMENT(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PW_TYPE_KEYPAD_ELEMENT))
#define PW_IS_KEYPAD_ELEMENT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PW_TYPE_KEYPAD_ELEMENT))
#define PW_KEYPAD_ELEMENT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), PW_TYPE_KEYPAD_ELEMENT, KeypadElementClass))

typedef struct _KeypadElement {
	GObject parent;

	unsigned short row;
	unsigned short col;
	unsigned short width;
	unsigned short height;

	gchar * icon_name;
	gchar * label;
	gchar * action;

} KeypadElement;

typedef struct _KeypadElementClass {
	GObjectClass parent;

} KeypadElementClass;

GType KeypadElement_get_type(void) G_GNUC_CONST;

struct _KeypadModel {
	GObject parent;

	unsigned short width;
	unsigned short height;
	unsigned short position;

	struct {
		unsigned short row;
		unsigned short col;
	} current;

	gchar *name;
	gchar *label;

	GList *elements;
	GList *widgets;

};

struct _KeypadModelClass {
	GObjectClass parent;
	GQuark domain;

};

G_GNUC_INTERNAL void keypad_model_set_position(GObject *model, const gchar *position);
G_GNUC_INTERNAL void keypad_model_parse_context(GObject *model, GMarkupParseContext *context);

G_GNUC_INTERNAL const gchar * keypad_model_get_position(GObject *mode);

G_GNUC_INTERNAL void keypad_model_element_parse_context(GObject *element, GMarkupParseContext *context);

G_GNUC_INTERNAL void attribute_element_start(GMarkupParseContext *context,const gchar **names,const gchar **values, GObject *parent, GError **error);
G_GNUC_INTERNAL void attribute_element_end(GMarkupParseContext *context, GObject *parent, GError **error);

G_END_DECLS

#endif // PRIVATE_H_INCLUDED
