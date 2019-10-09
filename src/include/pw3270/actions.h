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
 * @brief Declares the pw3270 Action objects.
 *
 */

#ifndef PW3270_ACTIONS_H_INCLUDED

	#define PW3270_ACTIONS_H_INCLUDED

	#include <gio/gio.h>
	#include <lib3270.h>
	#include <lib3270/actions.h>

	G_BEGIN_DECLS

	#define PW3270_TYPE_ACTION				(pw3270Action_get_type())
	#define PW3270_ACTION(inst)				(G_TYPE_CHECK_INSTANCE_CAST ((inst), PW3270_TYPE_ACTION, pw3270Action))
	#define PW3270_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), PW3270_TYPE_ACTION, pw3270ActionClass))
	#define PW3270_IS_ACTION(inst)			(G_TYPE_CHECK_INSTANCE_TYPE ((inst), PW3270_TYPE_ACTION))
	#define PW3270_IS_ACTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PW3270_TYPE_ACTION))
	#define PW3270_ACTION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), PW3270_TYPE_ACTION, pw3270ActionClass))

	typedef struct _pw3270Action			pw3270Action;
	typedef struct _pw3270ActionClass		pw3270ActionClass;

	GType pw3270Action_get_type(void) G_GNUC_CONST;

	GAction				* pw3270_action_get_from_lib3270(const LIB3270_ACTION * definition);
	const gchar			* pw3270_action_get_name(GAction *action);
	gboolean			  pw3270_action_get_enabled(GAction *action);
	void				  pw3270_action_activate(GAction *action, GVariant *parameter);

	G_END_DECLS

#endif // PW3270_ACTIONS_H_INCLUDED
