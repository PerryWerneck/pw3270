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
  * @brief Implements PW3270 Dialog Action.
  *
  */

 #include "private.h"
 #include <pw3270/actions.h>

 typedef struct _PW3270DialogAction {

	PW3270Action parent;

	GtkWidget * dialog;
	GtkWidget * (*factory)(PW3270Action *, GtkApplication *);

 } PW3270DialogAction;

 typedef struct _PW3270DialogActionClass {

	PW3270ActionClass parent_class;

 } PW3270DialogActionClass;

 #define PW3270_TYPE_DIALOG_ACTION				(PW3270DialogAction_get_type())
 #define PW3270_DIALOG_ACTION(inst)				(G_TYPE_CHECK_INSTANCE_CAST ((inst), PW3270_TYPE_DIALOG_ACTION, PW3270DialogAction))
 #define PW3270_DIALOG_ACTION_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), PW3270_TYPE_DIALOG_ACTION, PW3270DialogActionClass))
 #define PW3270_IS_DIALOG_ACTION(inst)			(G_TYPE_CHECK_INSTANCE_TYPE ((inst), PW3270_TYPE_DIALOG_ACTION))
 #define PW3270_IS_DIALOG_ACTION_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PW3270_TYPE_DIALOG_ACTION))
 #define PW3270_DIALOG_ACTION_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), PW3270_TYPE_DIALOG_ACTION, PW3270DialogActionClass))

 static void		  PW3270DialogAction_class_init(PW3270DialogActionClass *klass);
 static void		  PW3270DialogAction_init(PW3270DialogAction *action);
 static void		  activate(GAction *action, GVariant *parameter, GtkApplication *application);
 static GtkWidget	* factory(PW3270Action *action, GtkApplication *application);
 static gboolean	  get_enabled(GAction *action);

 G_DEFINE_TYPE(PW3270DialogAction, PW3270DialogAction, PW3270_TYPE_ACTION);

 PW3270Action * pw3270_dialog_action_new(GtkWidget * (*factory)(PW3270Action *, GtkApplication *application)) {
 	 PW3270DialogAction *action = PW3270_DIALOG_ACTION(g_object_new(PW3270_TYPE_DIALOG_ACTION, NULL));
 	 action->parent.activate = activate;
 	 action->factory = factory;
 	 return PW3270_ACTION(action);
 }

 void PW3270DialogAction_class_init(PW3270DialogActionClass *klass) {
 	klass->parent_class.get_enabled = get_enabled;
 }

 void PW3270DialogAction_init(PW3270DialogAction *action) {
 	action->factory = factory;
 }

 GtkWidget * factory(PW3270Action *action, GtkApplication G_GNUC_UNUSED(*application)) {
 	g_warning("No widget factory for action \"%s\"",g_action_get_name(G_ACTION(action)));
 	return NULL;
 }

 gboolean get_enabled(GAction *action) {

	if((PW3270_DIALOG_ACTION(action)->dialog)) {
		return FALSE;
	}

	return PW3270_ACTION_CLASS(PW3270DialogAction_parent_class)->get_enabled(action);

 }

static void on_destroy(GtkWidget *dialog, PW3270DialogAction *action) {

	if(action->dialog == dialog) {
		action->dialog = NULL;
		pw3270_action_notify_enabled(G_ACTION(action));
	}

 }

 void activate(GAction *object, GVariant G_GNUC_UNUSED(*parameter), GtkApplication *application) {

	PW3270DialogAction * action = PW3270_DIALOG_ACTION(object);

	if(action->dialog)
		return;

	action->dialog = action->factory(PW3270_ACTION(action), application);

	if(action->dialog) {

		GtkWindow * window = gtk_application_get_active_window(application);
		if(window) {
			gtk_window_set_attached_to(GTK_WINDOW(action->dialog), GTK_WIDGET(window));
			gtk_window_set_transient_for(GTK_WINDOW(action->dialog),window);
		}

		pw3270_action_notify_enabled(G_ACTION(action));
		g_signal_connect(action->dialog,"destroy",G_CALLBACK(on_destroy),action);
		g_signal_connect(action->dialog,"close",G_CALLBACK(gtk_widget_destroy),NULL);
		gtk_widget_show(GTK_WIDGET(action->dialog));

	}

 }
