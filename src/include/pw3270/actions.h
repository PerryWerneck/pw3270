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
	#define PW3270_TYPE_ACTION				(PW3270Action_get_type())
	#define PW3270_ACTION(inst)				(G_TYPE_CHECK_INSTANCE_CAST ((inst), PW3270_TYPE_ACTION, PW3270Action))
	#define PW3270_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), PW3270_TYPE_ACTION, PW3270ActionClass))
	#define PW3270_IS_ACTION(inst)			(G_TYPE_CHECK_INSTANCE_TYPE ((inst), PW3270_TYPE_ACTION))
	#define PW3270_IS_ACTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PW3270_TYPE_ACTION))
	#define PW3270_ACTION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), PW3270_TYPE_ACTION, PW3270ActionClass))

	typedef struct _PW3270Action {

		GObject parent;

		const gchar * name;
		const gchar * icon_name;
		const gchar	* label;
		const gchar	* tooltip;

		/// @brief Activation method.
		void (*activate)(GAction *action, GVariant *parameter, GtkApplication *application);

	} PW3270Action;

	typedef struct _PW3270ActionClass {

		GObjectClass parent_class;

		gboolean (*get_enabled)(GAction *action);

		struct {
			GParamSpec * state;
			GParamSpec * enabled;
		} properties;

	} PW3270ActionClass;

	GType			  PW3270Action_get_type(void) G_GNUC_CONST;
	PW3270Action	* pw3270_action_new();
	PW3270Action	* pw3270_dialog_action_new(GtkWidget * (*factory)(PW3270Action *action, GtkApplication *application));

	//
	// Action view
	//
	typedef GSList Pw3270ActionList;

	typedef enum _pw3270ActionViewFlag {
		PW3270_ACTION_VIEW_FLAG_FIXED 		= 0,	///< @brief Don't move to other views.
		PW3270_ACTION_VIEW_FLAG_ALLOW_ADD	= 1,	///< @brief Allow add to target view.
		PW3270_ACTION_VIEW_ALLOW_REMOVE		= 2,	///< @brief Allow remove from source view.
		PW3270_ACTION_VIEW_ALLOW_MOVE		= 3		///< @brief Allow move from one view to another.
	} PW3270ActionViewFlag;


	GtkWidget 			* pw3270_action_view_new();
	void				  pw3270_action_view_set_actions(GtkWidget *view, Pw3270ActionList *list);
	void				  pw3270_action_view_order_by_label(GtkWidget *view);
	void				  pw3270_action_view_move_selected(GtkWidget *from, GtkWidget *to);
	void				  pw3270_action_view_append(GtkWidget *widget, const gchar *label, GdkPixbuf *pixbuf, const gchar *action_name, const PW3270ActionViewFlag flags);
	gchar				* pw3270_action_view_get_action_names(GtkWidget *widget);
	GtkWidget			* pw3270_action_view_extract_button_new(GtkWidget *widget, const gchar *icon_name);

	Pw3270ActionList	* pw3270_action_list_new(GtkApplication *application);
	Pw3270ActionList	* pw3270_action_list_append(Pw3270ActionList *action_list, const gchar *label, GdkPixbuf *pixbuf, const gchar *action_name, const PW3270ActionViewFlag flags);
	Pw3270ActionList	* pw3270_action_list_move_action(Pw3270ActionList *action_list, const gchar *action_name, GtkWidget *view);
	void				  pw3270_action_list_free(Pw3270ActionList *action_list);

	//
	// Tools
	//
	gchar				* g_action_get_icon_name(GAction *action);
	gchar 				* g_action_get_tooltip(GAction *action);
	gchar				* g_action_get_label(GAction *action);

	GdkPixbuf			* g_action_get_pixbuf(GAction *action, GtkIconSize icon_size, GtkIconLookupFlags flags);

	GtkWidget 			* gtk_button_new_from_action(GAction *action, GtkIconSize icon_size);
	GtkToolItem			* gtk_tool_button_new_from_action(GAction *action, GtkIconSize icon_size, gboolean symbolic);

	G_END_DECLS

#endif // PW3270_ACTIONS_H_INCLUDED
