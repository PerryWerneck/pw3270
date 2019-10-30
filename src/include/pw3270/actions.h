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

	//
	// Abstract action
	//
	#define PW3270_TYPE_ACTION				(pw3270Action_get_type())
	#define PW3270_ACTION(inst)				(G_TYPE_CHECK_INSTANCE_CAST ((inst), PW3270_TYPE_ACTION, pw3270Action))
	#define PW3270_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), PW3270_TYPE_ACTION, pw3270ActionClass))
	#define PW3270_IS_ACTION(inst)			(G_TYPE_CHECK_INSTANCE_TYPE ((inst), PW3270_TYPE_ACTION))
	#define PW3270_IS_ACTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PW3270_TYPE_ACTION))
	#define PW3270_ACTION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), PW3270_TYPE_ACTION, pw3270ActionClass))

	typedef struct _pw3270Action {

		GObject parent;

		const gchar		* name;				/// @brief Action name (const string).
		GVariantType	* parameter_type;	/// @brief Parameter type.
		GVariant     	* state;			/// @brief Action state.
		GtkWidget		* terminal;			/// @brief The active terminal widget.

		/// @brief Activation method.
		void (*activate)(GAction *action, GVariant *parameter, GtkWidget *terminal);

	} pw3270Action;

	typedef struct _pw3270ActionClass {

		GObjectClass parent_class;

		struct {
			GParamSpec * state;
			GParamSpec * enabled;
		} properties;

		void (*change_widget)(GAction *action, GtkWidget *from, GtkWidget *to);
		gboolean (*get_enabled)(GAction *action, GtkWidget *terminal);
		const GVariantType * (*get_parameter_type)(GAction *action);
		const gchar * (*get_icon_name)(GAction *action);
		const gchar	* (*get_label)(GAction *action);
		const gchar	* (*get_tooltip)(GAction *action);

	} pw3270ActionClass;

	GType pw3270Action_get_type(void) G_GNUC_CONST;

	/// @brief New generica acion.
	GAction * pw3270_action_new();

	/// @brief New action from LIB3270's control data.
	GAction * pw3270_action_new_from_lib3270(const LIB3270_ACTION * definition);

	/// @brief Get action name.
	const gchar			* pw3270_action_get_name(GAction *action);

	/// @brief Get the action icon name.
	const gchar 		* pw3270_action_get_icon_name(GAction *action);

	/// @brief Get the action label.
	const gchar			* pw3270_action_get_label(GAction *action);

	/// @brief Get the action tooltip.
	const gchar			* pw3270_action_get_tooltip(GAction *action);

	/// @brief Associate action with the terminal widget.
	void				  pw3270_action_set_terminal_widget(GAction *action, GtkWidget *terminal);

	/// @brief Get lib3270 session handle.
	H3270				* pw3270_action_get_session(GAction *action);

	/// @brief Add lib3270 actions to an application window.
	void				  pw3270_window_add_actions(GtkWidget * appwindow);

	//
	// "Simple" action
	//
	#define PW3270_TYPE_SIMPLE_ACTION				(pw3270SimpleAction_get_type())
	#define PW3270_SIMPLE_ACTION(inst)				(G_TYPE_CHECK_INSTANCE_CAST ((inst), PW3270_TYPE_SIMPLE_ACTION, pw3270SimpleAction))
	#define PW3270_SIMPLE_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), PW3270_TYPE_SIMPLE_ACTION, pw3270SimpleActionClass))
	#define PW3270_IS_SIMPLE_ACTION(inst)			(G_TYPE_CHECK_INSTANCE_TYPE ((inst), PW3270_TYPE_SIMPLE_ACTION))
	#define PW3270_IS_SIMPLE_ACTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PW3270_TYPE_SIMPLE_ACTION))
	#define PW3270_SIMPLE_ACTION_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), PW3270_TYPE_SIMPLE_ACTION, pw3270SimpleActionClass))

	typedef struct _pw3270SimpleAction {

		pw3270Action parent;

		// Fixed data
		const gchar * icon_name;
		const gchar	* label;
		const gchar	* tooltip;

		// Lib3270 Action group
		struct {
			LIB3270_ACTION_GROUP id;
			const void * listener;
		} group;

		/// @brief Activation method.
		void (*activate)(GAction *action, GVariant *parameter, GtkWidget *terminal);

	} pw3270SimpleAction;

	typedef struct _pw3270SimpleActionClass {

		pw3270ActionClass parent_class;

	} pw3270SimpleActionClass;

	GType pw3270SimpleAction_get_type(void) G_GNUC_CONST;

	/// @brief Create an empty simple action.
	pw3270SimpleAction * pw3270_simple_action_new();

	/// @brief New simple action from LIB3270's control data.
	pw3270SimpleAction * pw3270_simple_action_new_from_lib3270(const LIB3270_ACTION * definition, const gchar *name);

	/// @brief New simple action from LIB3270's action name.
	pw3270SimpleAction * pw3270_simple_action_new_from_name(const gchar *source_name, const gchar *name);

	G_END_DECLS

#endif // PW3270_ACTIONS_H_INCLUDED
