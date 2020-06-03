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

/**
 * @brief Declares the pw3270 Keypad objects.
 *
 */

#ifndef PW3270_KEYPAD_H_INCLUDED

	#define PW3270_KEYPAD_H_INCLUDED

	#include <gtk/gtk.h>

	G_BEGIN_DECLS

	#define PW_TYPE_KEYPAD_MODEL			(KeypadModel_get_type())
	#define PW_KEYPAD_MODEL(obj)			(G_TYPE_CHECK_INSTANCE_CAST ((obj), PW_TYPE_KEYPAD_MODEL, KeypadModel))
	#define PW_KEYPAD_MODEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PW_TYPE_KEYPAD_MODEL, KeypadModelClass))
	#define PW_IS_KEYPAD_MODEL(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PW_TYPE_KEYPAD_MODEL))
	#define PW_IS_KEYPAD_MODEL_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PW_TYPE_KEYPAD_MODEL))
	#define PW_KEYPAD_MODEL_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), PW_TYPE_KEYPAD_MODEL, KeypadModelClass))

	typedef struct _KeypadModel			KeypadModel;
	typedef struct _KeypadModelClass	KeypadModelClass;

	GType KeypadModel_get_type(void) G_GNUC_CONST;

	GList		* pw3270_keypad_model_new_from_xml(GList *keypads, const gchar *filename);
	GtkWidget	* pw3270_keypad_get_from_model(GObject *model);
	const gchar	* pw3270_keypad_model_get_name(GObject *model);
	const gchar * pw3270_keypad_model_get_label(GObject *model);

	typedef enum _keypad_position {
        KEYPAD_POSITION_TOP,
        KEYPAD_POSITION_LEFT,
        KEYPAD_POSITION_BOTTOM,
        KEYPAD_POSITION_RIGHT
	} 	KEYPAD_POSITION;

	KEYPAD_POSITION pw3270_keypad_get_position(GObject *model);

	G_END_DECLS

#endif // PW3270_KEYPAD_H_INCLUDED
