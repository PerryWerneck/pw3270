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
	#include <gtk/gtk.h>
	#include <lib3270.h>
	#include <lib3270/actions.h>
	#include <v3270/actions.h>

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

		const gchar			* name;			///> @brief Action name (const string).
		GtkWidget			* terminal;		///> @brief The active terminal widget.

		struct {
			const GVariantType	* state;		///> @brief State type type.
			const GVariantType	* parameter;	///> @brief Parameter type.
		} types;

		/// @brief Activation method.
		void (*activate)(GAction *action, GVariant *parameter, GtkWidget *terminal);

		/// @brief Get State method.
		GVariant * (*get_state_property)(GAction *action, GtkWidget *terminal);

		/// @brief Get state hint.
		GVariant * (*get_state_hint)(GAction *action, GtkWidget *terminal);

	} pw3270Action;

	typedef struct _pw3270ActionClass {

		GObjectClass parent_class;

		struct {
			GParamSpec * state;
			GParamSpec * enabled;
		} properties;

		void (*change_widget)(GAction *action, GtkWidget *from, GtkWidget *to);
		gboolean (*get_enabled)(GAction *action, GtkWidget *terminal);

		const gchar * (*get_icon_name)(GAction *action);
		const gchar	* (*get_label)(GAction *action);
		const gchar	* (*get_tooltip)(GAction *action);

	} pw3270ActionClass;

	GType pw3270Action_get_type(void) G_GNUC_CONST;

	/// @brief New generic action.
	GAction * pw3270_action_new();

	/// @brief New action from LIB3270's control data.
	GAction * g_action_new_from_lib3270(const LIB3270_ACTION * definition);

	/// @brief Get action name.
	const gchar			* pw3270_action_get_name(GAction *action);

	/// @brief Set action name.
	void pw3270_action_set_name(GAction *action, const gchar *name);

	/// @brief Get the action icon name.
	const gchar 		* pw3270_action_get_icon_name(GAction *action);

	/// @brief Get the action image icon.
	GtkImage			* pw3270_action_get_image(GAction *action, GtkIconSize icon_size);

	/// @brief Get the action label.
	const gchar			* pw3270_action_get_label(GAction *action);

	/// @brief Get the action tooltip.
	const gchar			* pw3270_action_get_tooltip(GAction *action);

	/// @brief Create a button associated with the action.
	//GtkWidget			* pw3270_action_button_new(GAction *action, const gchar *action_name);

	/// @brief Associate action with the terminal widget.
	void				  pw3270_action_set_terminal_widget(GAction *action, GtkWidget *terminal);

	/// @brief Get lib3270 session handle.
	H3270				* pw3270_action_get_session(GAction *action);

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

	/// @brief Update simple action from LIB3270's property description.
	void pw3270_simple_action_set_lib3270_property(pw3270SimpleAction *action, const LIB3270_PROPERTY * property);

	/*
	//
	// Dialog action
	//
	#define PW3270_TYPE_DIALOG_ACTION				(pw3270DialogAction_get_type())
	#define PW3270_DIALOG_ACTION(inst)				(G_TYPE_CHECK_INSTANCE_CAST ((inst), PW3270_TYPE_DIALOG_ACTION, pw3270DialogAction))
	#define PW3270_DIALOG_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), PW3270_TYPE_DIALOG_ACTION, pw3270DialogActionClass))
	#define PW3270_IS_DIALOG_ACTION(inst)			(G_TYPE_CHECK_INSTANCE_TYPE ((inst), PW3270_TYPE_DIALOG_ACTION))
	#define PW3270_IS_DIALOG_ACTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PW3270_TYPE_DIALOG_ACTION))
	#define PW3270_DIALOG_ACTION_GET_CLASS(obj)		(G_TYPE_INSTANCE_GET_CLASS ((obj), PW3270_TYPE_DIALOG_ACTION, pw3270DialogActionClass))

	typedef struct _pw3270DialogAction  pw3270DialogAction;
	typedef struct _pw3270DialogActionClass pw3270DialogActionClass;

	pw3270SimpleAction * pw3270_dialog_action_new(GtkWidget * (*factory)(pw3270SimpleAction *action, GtkWidget *));
	*/

	//
	// V3270 Property Action
	//
	#define V3270_TYPE_PROPERTY_ACTION				(v3270PropertyAction_get_type())
	#define V3270_PROPERTY_ACTION(inst)				(G_TYPE_CHECK_INSTANCE_CAST ((inst), V3270_TYPE_PROPERTY_ACTION, v3270PropertyAction))
	#define V3270_PROPERTY_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), V3270_TYPE_PROPERTY_ACTION, v3270PropertyActionClass))
	#define V3270_IS_PROPERTY_ACTION(inst)			(G_TYPE_CHECK_INSTANCE_TYPE ((inst), V3270_TYPE_PROPERTY_ACTION))
	#define V3270_IS_PROPERTY_ACTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), V3270_TYPE_PROPERTY_ACTION))
	#define V3270_PROPERTY_ACTION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), V3270_TYPE_PROPERTY_ACTION, v3270PropertyActionClass))

	typedef struct _v3270PropertyAction {

		pw3270SimpleAction parent;

		GParamSpec *pspec;

	} v3270PropertyAction;

	typedef struct _v3270PropertyActionClass {

		pw3270SimpleActionClass parent_class;

	} v3270PropertyActionClass;

	GType v3270PropertyAction_get_type(void) G_GNUC_CONST;

	GAction * v3270_property_action_new(GtkWidget *widget, const gchar *property_name);

	//
	// V3270 action who can be enable based on property
	//
	#define V3270_TYPE_CONDITIONAL_ACTION				(v3270ConditionalAction_get_type())
	#define V3270_CONDITIONAL_ACTION(inst)				(G_TYPE_CHECK_INSTANCE_CAST ((inst), V3270_TYPE_CONDITIONAL_ACTION, v3270ConditionalAction))
	#define V3270_CONDITIONAL_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), V3270_TYPE_CONDITIONAL_ACTION, v3270ConditionalActionClass))
	#define V3270_IS_CONDITIONAL_ACTION(inst)			(G_TYPE_CHECK_INSTANCE_TYPE ((inst), V3270_TYPE_CONDITIONAL_ACTION))
	#define V3270_IS_CONDITIONAL_ACTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), V3270_TYPE_CONDITIONAL_ACTION))
	#define V3270_CONDITIONAL_ACTION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), V3270_TYPE_CONDITIONAL_ACTION, v3270ConditionalActionClass))

	typedef struct _v3270ConditionalAction {

		pw3270SimpleAction parent;

		GParamSpec *pspec;

	} v3270ConditionalAction;

	typedef struct _v3270CopyActionClass {

		pw3270SimpleActionClass parent_class;

	} v3270ConditionalActionClass;

	GType v3270ConditionalAction_get_type(void) G_GNUC_CONST;

	GAction * v3270_conditional_action_new(GtkWidget *widget, const gchar *property_name);
	GAction * v3270_action_new(const V3270_ACTION *action_data);

	//
	// Action view
	//
	typedef GSList Pw3270ActionList;

	GtkWidget 			* pw3270_action_view_new();
	Pw3270ActionList	* pw3270_action_list_new(GtkApplication *application);
	void				  pw3270_action_list_free(Pw3270ActionList *action_list);
	void				  pw3270_action_view_set_actions(GtkWidget *view, Pw3270ActionList *list);
	void				  pw3270_action_view_move_selected(GtkWidget *from, GtkWidget *to);
	void				  pw3270_action_view_append(GtkWidget *widget, const gchar *label, GdkPixbuf *pixbuf, const gchar *action_name, gint flags);
	gchar				* pw3270_action_view_get_action_names(GtkWidget *widget);

	Pw3270ActionList	* pw3270_action_list_move_action(Pw3270ActionList *action_list, const gchar *action_name, GtkWidget *view);

	//
	// Tools
	//
	gchar				* g_action_get_icon_name(GAction *action);
	gchar 				* g_action_get_tooltip(GAction *action);
	gchar				* g_action_get_label(GAction *action);

	GdkPixbuf			* g_action_get_pixbuf(GAction *action, GtkIconSize icon_size, GtkIconLookupFlags flags);

	GtkWidget 			* gtk_button_new_from_action(GAction *action, GtkIconSize icon_size);
	GtkToolItem			* gtk_tool_button_new_from_action(GAction *action, GtkIconSize icon_size);

	G_END_DECLS

#endif // PW3270_ACTIONS_H_INCLUDED
