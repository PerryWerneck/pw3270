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

 #include "private.h"
 #include <v3270.h>

 static void pw3270_action_iface_init(GActionInterface *iface);
 static void pw3270Action_class_init(pw3270ActionClass *klass);
 static void pw3270Action_init(pw3270Action *action);
 static void pw3270_action_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
 static void pw3270_action_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
 static void pw3270_action_set_state(GAction *action, GVariant *value);
 static gboolean pw3270_action_get_enabled(GAction *action);
 static void pw3270_action_activate(GAction *action, GVariant *parameter);

 static gboolean get_enabled(GAction *action, GtkWidget *terminal);
 static void activate(GAction *action, GVariant *parameter, GtkWidget *terminal);
 static void change_widget(GAction *action, GtkWidget *from, GtkWidget *to);
 static const gchar *get_null(GAction *action);

 static void finalize(GObject *object);

 static	const GVariantType	* get_state_type(GAction *action);
 static	GVariant			* get_state_property(GAction *action);

 static	GVariant 			* internal_get_state_property(GAction *action, GtkWidget *terminal);

 static	const GVariantType	* get_parameter_type(GAction *action);
 static GVariant			* pw3270_action_get_state_hint(GAction *action);
 static void				  pw3270_action_change_state(GAction *action, GVariant *value);

 enum {
	PROP_NONE,
	PROP_NAME,
	PROP_PARAMETER_TYPE,
	PROP_ENABLED,
	PROP_STATE_TYPE,
	PROP_STATE
 };

 enum {
  SIGNAL_CHANGE_STATE,
  NR_SIGNALS
 };

 static guint action_signals[NR_SIGNALS];

 G_DEFINE_TYPE_WITH_CODE(pw3270Action, pw3270Action, G_TYPE_OBJECT, G_IMPLEMENT_INTERFACE(G_TYPE_ACTION, pw3270_action_iface_init))

 void pw3270_action_iface_init(GActionInterface *iface) {
	iface->get_name = pw3270_action_get_name;
	iface->get_parameter_type = get_parameter_type;
	iface->get_state_type = get_state_type;
	iface->get_state_hint = pw3270_action_get_state_hint;
	iface->get_enabled = pw3270_action_get_enabled;
	iface->get_state = get_state_property;
	iface->change_state = pw3270_action_change_state;
	iface->activate = pw3270_action_activate;
 }

 void pw3270Action_class_init(pw3270ActionClass *klass) {

	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	debug("%s",__FUNCTION__);

	klass->change_widget		= change_widget;
	klass->get_enabled			= get_enabled;
	klass->get_icon_name		= get_null;
	klass->get_label			= get_null;
	klass->get_tooltip			= get_null;

 	object_class->finalize		= finalize;
	object_class->set_property	= pw3270_action_set_property;
	object_class->get_property	= pw3270_action_get_property;

	// Install properties
	g_object_class_install_property(object_class, PROP_NAME,
		g_param_spec_string ("name",
			N_("Action Name"),
			N_("The name used to invoke the action"),
			NULL,
			G_PARAM_READWRITE |
			G_PARAM_CONSTRUCT_ONLY |
			G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class, PROP_PARAMETER_TYPE,
		g_param_spec_boxed ("parameter-type",
			N_("Parameter Type"),
			N_("The type of GVariant passed to activate()"),
			G_TYPE_VARIANT_TYPE,
			G_PARAM_READWRITE |
			G_PARAM_CONSTRUCT_ONLY |
			G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class, PROP_STATE_TYPE,
		g_param_spec_boxed ("state-type",
			N_("State Type"),
			N_("The type of the state kept by the action"),
			G_TYPE_VARIANT_TYPE,
			G_PARAM_READABLE |
			G_PARAM_STATIC_STRINGS));

	// Enabled property
	klass->properties.enabled =
			g_param_spec_boolean(
				"enabled",
				N_("Enabled"),
				N_("If the action can be activated"),
				TRUE,
				G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
			);

	g_object_class_install_property(object_class, PROP_ENABLED, klass->properties.enabled);

	// State property
	klass->properties.state =
		g_param_spec_variant(
			"state",
			N_("State"),
			N_("The state the action is in"),
			G_VARIANT_TYPE_ANY,
			NULL,
			G_PARAM_READWRITE | G_PARAM_CONSTRUCT |
			G_PARAM_STATIC_STRINGS
		);

	g_object_class_install_property (object_class, PROP_STATE, klass->properties.state);

	// Install signals
	action_signals[SIGNAL_CHANGE_STATE] =
		g_signal_new(
			I_("change_state"),
			G_TYPE_ACTION,
			G_SIGNAL_RUN_LAST | G_SIGNAL_MUST_COLLECT,
			0, NULL, NULL,
			NULL,
			G_TYPE_NONE, 1,
			G_TYPE_VARIANT
		);
 }

 void pw3270Action_init(pw3270Action *action) {

	action->terminal			= NULL;
	action->types.parameter		= NULL;

	action->activate			= activate;
	action->get_state_property	= internal_get_state_property;

 }

 void finalize(GObject *object) {

	pw3270Action * action = PW3270_ACTION(object);

	if(action->terminal) {
		pw3270_action_set_terminal_widget(G_ACTION(object),NULL);
		action->terminal = NULL;
	}

	G_OBJECT_CLASS(pw3270Action_parent_class)->finalize(object);

 }

 void pw3270_action_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec G_GNUC_UNUSED(*pspec)) {

	GAction *action = G_ACTION(object);

	debug("%s(%d)",__FUNCTION__,prop_id);

	switch (prop_id) {
    case PROP_NAME:
		g_value_set_string(value, pw3270_action_get_name(action));
		break;

	case PROP_PARAMETER_TYPE:
		g_value_set_boxed(value, get_parameter_type(action));
		break;

	case PROP_ENABLED:
		g_value_set_boolean(value, pw3270_action_get_enabled(action));
		break;

	case PROP_STATE_TYPE:
		g_value_set_boxed(value, get_state_type(action));
		break;

	case PROP_STATE:
		g_value_take_variant(value, get_state_property(action));
		break;

	default:
		g_assert_not_reached ();
	}

 }

 void pw3270_action_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec G_GNUC_UNUSED(*pspec)) {

//	debug("%s(%d)",__FUNCTION__,prop_id);

	GAction *action = G_ACTION(object);

	switch (prop_id)
	{
	case PROP_NAME:
		pw3270_action_set_name(action, g_value_get_string(value));
		break;

	case PROP_PARAMETER_TYPE:
//		action->parameter_type = g_value_dup_boxed (value);
		break;

	case PROP_ENABLED:
//		action->enabled = g_value_get_boolean (value);
		break;

	case PROP_STATE:
		pw3270_action_set_state(action, g_value_get_variant(value));
		break;

	default:
		g_assert_not_reached ();
	}

 }

 const gchar * pw3270_action_get_name(GAction *action) {
 	return PW3270_ACTION(action)->name;
 }

 void pw3270_action_set_name(GAction *object, const gchar *name) {

 	if(name)
		g_warning("Invalid call to %s on action %s with value \"%s\"",__FUNCTION__,g_action_get_name(object),name);

 }

 GVariant * internal_get_state_property(GAction *object, GtkWidget G_GNUC_UNUSED(*terminal)) {

	pw3270Action * action = PW3270_ACTION(object);

	if(action->types.state == G_VARIANT_TYPE_BOOLEAN)
		return g_variant_new_boolean(TRUE);

	return NULL;
 }

 GVariant * get_state_property(GAction *object) {

	pw3270Action * action = PW3270_ACTION(object);
	GVariant * state;

	if(action->terminal)
		state = action->get_state_property(object,action->terminal);
	else
		state = internal_get_state_property(object,NULL);

 	if(state)
		g_variant_ref(state);

	return state;
 }

 const GVariantType * get_parameter_type(GAction *action) {
 	return PW3270_ACTION(action)->types.parameter;
 }

 const GVariantType * get_state_type(GAction *object) {
	return PW3270_ACTION(object)->types.state;
 }

 GVariant * pw3270_action_get_state_hint(GAction G_GNUC_UNUSED(*action)) {
	return NULL;
 }

 void pw3270_action_change_state(GAction *object, GVariant *value) {
	pw3270_action_set_state(object, value);
 }

 void pw3270_action_change_state_boolean(GAction *action, gboolean state) {

 	g_return_if_fail(PW3270_IS_ACTION(action));
	pw3270_action_set_state(action,g_variant_new_boolean(state));

 }

 void pw3270_action_set_state(GAction *object, GVariant *value) {

	/*
	if(value) {

		pw3270Action * action = PW3270_ACTION(object);

		g_variant_ref_sink(value);

		if (!action->state || !g_variant_equal(action->state, value)) {

			if(action->state)
				g_variant_unref(action->state);

			action->state = g_variant_ref(value);

			if (g_signal_has_handler_pending(object, action_signals[SIGNAL_CHANGE_STATE], 0, TRUE)) {
				g_signal_emit(object, action_signals[SIGNAL_CHANGE_STATE], 0, value);
			}

			g_object_notify_by_pspec(G_OBJECT(object), PW3270_ACTION_GET_CLASS(object)->properties.state);

		}

		g_variant_unref(value);

	}
	*/

 }

 void pw3270_action_notify_enabled(GAction *object) {

	// g_object_notify_by_pspec(G_OBJECT(object), PW3270_ACTION_GET_CLASS(object)->properties.enabled);
	g_object_notify(G_OBJECT (object), "enabled");

 }

 static void change_widget(GAction *action, GtkWidget *from, GtkWidget *to) {

 	if(from != to) {
		PW3270_ACTION(action)->terminal = to;
		pw3270_action_notify_enabled(action);
 	}

 }

 void pw3270_action_set_terminal_widget(GAction *object, GtkWidget *widget) {

	g_return_if_fail(PW3270_IS_ACTION(object));

	if(widget) {
		 g_return_if_fail(GTK_IS_V3270(widget));
	}

 	pw3270Action * action = PW3270_ACTION(object);

 	if(action->terminal != widget) {
		PW3270_ACTION_GET_CLASS(object)->change_widget(object,action->terminal,widget);
		action->terminal = widget;
 	}

 }

 gboolean pw3270_action_get_enabled(GAction *object) {

	gboolean enabled = FALSE;

 	pw3270Action * action = PW3270_ACTION(object);

 	if(action && action->terminal) {
		enabled = PW3270_ACTION_GET_CLASS(object)->get_enabled(object,action->terminal);
//		debug("Action %s is %s",g_action_get_name(object),enabled ? "enabled" : "disabled");
 	}

	return enabled;

 }

 void pw3270_action_activate(GAction *object, GVariant *parameter) {

 	pw3270Action * action = PW3270_ACTION(object);

 	debug("%s: terminal=%p",__FUNCTION__,action->terminal);

 	if(action && action->terminal) {
		action->activate(object,parameter,action->terminal);
 	}

 }

 gboolean get_enabled(GAction G_GNUC_UNUSED(*object), GtkWidget *terminal) {
 	return terminal != NULL;
 }

 void activate(GAction *action, GVariant G_GNUC_UNUSED(*parameter), GtkWidget G_GNUC_UNUSED(*terminal)) {
	g_message("Action %s can't be activated",pw3270_action_get_name(action));
 }

 const gchar * get_null(GAction G_GNUC_UNUSED(*action)) {
	return NULL;
 }

 const gchar * pw3270_action_get_icon_name(GAction *action) {
	return PW3270_ACTION_GET_CLASS(action)->get_icon_name(action);
 }

 const gchar * pw3270_action_get_label(GAction *action) {
	const gchar * label = PW3270_ACTION_GET_CLASS(action)->get_label(action);

	if(label)
		return gettext(label);

	return NULL;
 }

 const gchar * pw3270_action_get_tooltip(GAction *action) {
	const gchar * tooltip = PW3270_ACTION_GET_CLASS(action)->get_tooltip(action);

	if(tooltip)
		return gettext(tooltip);

	return NULL;
 }

 H3270 * pw3270_action_get_session(GAction *action) {
 	return v3270_get_session(PW3270_ACTION(action)->terminal);
 }

 GAction * pw3270_action_new() {
	return G_ACTION(g_object_new(PW3270_TYPE_ACTION, NULL));
 }
