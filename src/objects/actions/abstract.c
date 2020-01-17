/*
 * "Software v3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
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
 #include <pw3270/actions.h>

 static void				  PW3270_action_iface_init(GActionInterface *iface);
 static void				  PW3270Action_class_init(PW3270ActionClass *klass);
 static void				  PW3270Action_init(PW3270Action *action);

 static void				  get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
 static void				  set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);

 static const gchar			* get_icon_name(GAction *action);
 static const gchar			* get_label(GAction *action);
 static const gchar			* get_tooltip(GAction *action);
 static const gchar			* get_name(GAction *action);
 static const GVariantType	* get_state_type(GAction *action);
 static	const GVariantType	* get_parameter_type(GAction *object);

 static void				  finalize(GObject *object);

 static gboolean			  get_enabled(GAction *action);
 static void 				  activate(GAction *action, GVariant *parameter, GtkApplication *application);
 static	GVariant 			* get_state(GAction *action);

 static const gchar			* iface_get_name(GAction *action);
 static	const GVariantType	* iface_get_parameter_type(GAction *action);
 static GVariant			* iface_get_state_hint(GAction *action);
 static	const GVariantType	* iface_get_state_type(GAction *action);
 static	GVariant			* iface_get_state(GAction *action);
 static gboolean			  iface_get_enabled(GAction *action);
 static GVariant			* iface_get_state(GAction *object);
 static void				  iface_change_state(GAction *object, GVariant *value);
 static void				  iface_activate(GAction *object, GVariant *parameter);

 enum {
	PROP_NONE,
	PROP_NAME,
	PROP_PARAMETER_TYPE,
	PROP_ENABLED,
	PROP_STATE_TYPE,
	PROP_STATE,
	PROP_ICON_NAME,
	PROP_LABEL,
	PROP_TOOLTIP
 };

 G_DEFINE_TYPE_WITH_CODE(PW3270Action, PW3270Action, G_TYPE_OBJECT, G_IMPLEMENT_INTERFACE(G_TYPE_ACTION, PW3270_action_iface_init))

 void PW3270Action_class_init(PW3270ActionClass *klass) {

	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	debug("%s",__FUNCTION__);

	klass->get_enabled 			= get_enabled;

 	object_class->finalize		= finalize;
	object_class->get_property	= get_property;
	object_class->set_property	= set_property;

	// Install properties
	g_object_class_install_property(object_class, PROP_NAME,
		g_param_spec_string (
			"name",
			N_("Action Name"),
			N_("The name used to invoke the action"),
			NULL,
			G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_ICON_NAME,
		g_param_spec_string (
			"icon-name",
			N_("Icon Name"),
			N_("The name of the icon associated with the action"),
			NULL,
			G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_LABEL,
		g_param_spec_string (
			"label",
			N_("The action label"),
			N_("The label for the action"),
			NULL,
			G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(object_class, PROP_TOOLTIP,
		g_param_spec_string (
			"tooltip",
			N_("The action tooltip"),
			N_("The tooltip for the action"),
			NULL,
			G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class, PROP_PARAMETER_TYPE,
		g_param_spec_boxed ("parameter-type",
			N_("Parameter Type"),
			N_("The type of GVariant passed to activate()"),
			G_TYPE_VARIANT_TYPE,
			G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property (object_class, PROP_STATE_TYPE,
		g_param_spec_boxed ("state-type",
			N_("State Type"),
			N_("The type of the state kept by the action"),
			G_TYPE_VARIANT_TYPE,
			G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READABLE | G_PARAM_STATIC_STRINGS));

	// Enabled property
	klass->properties.enabled =
			g_param_spec_boolean(
				"enabled",
				N_("Enabled"),
				N_("If the action can be activated"),
				TRUE,
				G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS
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
			G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK | G_PARAM_STATIC_BLURB | G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS
		);

	g_object_class_install_property (object_class, PROP_STATE, klass->properties.state);

 }

 void PW3270Action_init(PW3270Action *action) {
	action->activate = activate;
 }

 void finalize(GObject *object) {

	// PW3270Action * action = PW3270_ACTION(object);
	G_OBJECT_CLASS(PW3270Action_parent_class)->finalize(object);

 }

 void get_property(GObject *object, guint prop_id, GValue *value, GParamSpec G_GNUC_UNUSED(*pspec)) {

	GAction *action = G_ACTION(object);

	switch (prop_id) {
    case PROP_NAME:
		g_value_set_string(value, g_action_get_name(action));
		break;

	case PROP_ICON_NAME:
		g_value_set_string(value, get_icon_name(action));
		break;

	case PROP_LABEL:
		g_value_set_string(value, get_label(action));
		break;

	case PROP_TOOLTIP:
		g_value_set_string(value, get_tooltip(action));
		break;

	case PROP_PARAMETER_TYPE:
		g_value_set_boxed(value, get_parameter_type(action));
		break;

	case PROP_ENABLED:
		g_value_set_boolean(value, get_enabled(action));
		break;

	case PROP_STATE_TYPE:
		g_value_set_boxed(value, get_state_type(action));
		break;

	case PROP_STATE:
		g_value_take_variant(value, get_state(action));
		break;

	default:
		g_assert_not_reached ();
	}

 }

 void set_property(GObject G_GNUC_UNUSED(*object), guint G_GNUC_UNUSED(prop_id), const GValue G_GNUC_UNUSED(*value), GParamSpec *pspec) {
// 	g_message("Action %s property %s is read-only",g_action_get_name(G_ACTION(object)),pspec->name);
 }

 static gboolean bg_notify_enabled(GObject *action) {
 	debug("%s(%s,%s)",__FUNCTION__,g_action_get_name(G_ACTION(action)),(g_action_get_enabled(G_ACTION(action)) ? "enabled" : "disabled"));
	g_object_notify(action, "enabled");
	return FALSE;
 }

 static gboolean bg_notify_state(GObject *action) {
	g_object_notify(action, "state");
	return FALSE;
 }

 void pw3270_action_notify_enabled(GAction *action) {
	g_idle_add((GSourceFunc) bg_notify_enabled, G_OBJECT(action));
 }

 void pw3270_action_notify_state(GAction *action) {
 	if(g_action_get_state_type(action))
		g_idle_add((GSourceFunc) bg_notify_state, G_OBJECT(action));
 }

 gboolean get_enabled(GAction G_GNUC_UNUSED(*object)) {
 	return TRUE;
 }

 void activate(GAction *action, GVariant G_GNUC_UNUSED(*parameter), GtkApplication G_GNUC_UNUSED(*application)) {
	g_message("Action %s can't be activated",g_action_get_name(action));
 }

 //
 // Action methods.
 //
 PW3270Action * pw3270_action_new() {
	return PW3270_ACTION(g_object_new(PW3270_TYPE_ACTION, NULL));
 }

 GdkPixbuf * pw3270_action_get_pixbuf(GAction *action, GtkIconSize icon_size, GtkIconLookupFlags flags) {

	const gchar * icon_name = v3270_action_get_icon_name(action);

	if(!icon_name)
		return NULL;

	return gtk_icon_theme_load_icon(
					gtk_icon_theme_get_default(),
					icon_name,
					icon_size,
					flags,
					NULL
			);

 }

//
// Default methods.
//
 GVariant * get_state(GAction G_GNUC_UNUSED(*object)) {
	return g_variant_new_boolean(TRUE);
 }

//
// Interface Methods.
//
 void PW3270_action_iface_init(GActionInterface *iface) {
	iface->get_name				= iface_get_name;
	iface->get_parameter_type	= iface_get_parameter_type;
	iface->get_state_type		= iface_get_state_type;
	iface->get_state_hint		= iface_get_state_hint;
	iface->get_enabled			= iface_get_enabled;
	iface->get_state			= iface_get_state;
	iface->change_state			= iface_change_state;
	iface->activate				= iface_activate;
 }

 const gchar * iface_get_name(GAction *action) {
 	return get_name(action);
 }

 GVariant * iface_get_state(GAction *object) {

 	GVariant * state = NULL;

 	if(g_action_get_state_type(object)) {

		state = get_state(object);

		if(state)
			g_variant_ref(state);

 	}

	return state;

 }

 const GVariantType * iface_get_parameter_type(GAction *object) {
	return get_parameter_type(object);
 }

 const GVariantType * iface_get_state_type(GAction *object) {
	return get_state_type(object);
 }

 GVariant * iface_get_state_hint(GAction G_GNUC_UNUSED(*object)) {
	return NULL;
 }

 void iface_change_state(GAction G_GNUC_UNUSED(*object), GVariant G_GNUC_UNUSED(*value)) {
 	debug("%s",__FUNCTION__);
 }

 gboolean iface_get_enabled(GAction *object) {
	return PW3270_ACTION_GET_CLASS(object)->get_enabled(object);
 }

 void iface_activate(GAction *object, GVariant *parameter) {
	PW3270_ACTION(object)->activate(object,parameter,GTK_APPLICATION(g_application_get_default()));
 }

 const gchar * get_icon_name(GAction *action) {
 	return PW3270_ACTION(action)->icon_name;
 }

 const gchar * get_label(GAction *action) {
 	return PW3270_ACTION(action)->label;
 }

 const gchar * get_tooltip(GAction *action) {
 	return PW3270_ACTION(action)->tooltip;
 }

 const gchar * get_name(GAction *action) {
  	return PW3270_ACTION(action)->name;
 }

 const GVariantType	* get_state_type(GAction G_GNUC_UNUSED(*object)) {
 	return NULL;
 }

 const GVariantType	* get_parameter_type(GAction G_GNUC_UNUSED(*object)) {
 	return NULL;
 }
