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
 #include <v3270.h>
 #include <v3270/settings.h>

 static void pw3270DialogAction_class_init(pw3270DialogActionClass *klass);
 static void pw3270DialogAction_init(pw3270DialogAction *action);
 static void activate(GAction G_GNUC_UNUSED(*action), GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal);

 struct _pw3270DialogAction {

	pw3270SimpleAction parent;

	GtkWidget * dialog;
	GtkWidget * (*factory)(pw3270SimpleAction *, GtkWidget *);

 };

 struct _pw3270DialogActionClass {

	pw3270SimpleActionClass parent_class;

 };

 G_DEFINE_TYPE(pw3270DialogAction, pw3270DialogAction, PW3270_TYPE_SIMPLE_ACTION);

 static gboolean get_enabled(GAction *action, GtkWidget *terminal) {

 	if((PW3270_DIALOG_ACTION(action)->dialog)) {
		return FALSE;
 	}

 	if(terminal) {
		return lib3270_action_group_get_activatable(v3270_get_session(terminal),PW3270_SIMPLE_ACTION(action)->group.id);
 	}

 	return FALSE;

 }

 static void pw3270DialogAction_class_init(pw3270DialogActionClass *klass) {
 	klass->parent_class.parent_class.get_enabled = get_enabled;
 }

 static void pw3270DialogAction_init(pw3270DialogAction *action) {

 	action->dialog = NULL;
	action->parent.parent.activate = activate;

 }

 pw3270SimpleAction * pw3270_dialog_action_new(GtkWidget * (*factory)(pw3270SimpleAction *, GtkWidget *)) {

  	pw3270DialogAction * action = (pw3270DialogAction *) g_object_new(PW3270_TYPE_DIALOG_ACTION, NULL);
  	action->factory = factory;
  	return PW3270_SIMPLE_ACTION(action);

 }

 static void on_destroy(GtkWidget *dialog, pw3270DialogAction *action) {

 	if(action->dialog == dialog) {
		action->dialog = NULL;
		pw3270_action_notify_enabled(G_ACTION(action));
 	}

 }

 void activate(GAction *object, GVariant G_GNUC_UNUSED(*parameter), GtkWidget *terminal) {

	if(!GTK_IS_V3270(terminal))
		return;

	pw3270DialogAction * action = PW3270_DIALOG_ACTION(object);

	if(action->dialog || !action->factory)
		return;

	action->dialog = action->factory((pw3270SimpleAction *) object, terminal);
	pw3270_action_notify_enabled(G_ACTION(action));

	if(action->dialog) {

		g_signal_connect(action->dialog,"destroy",G_CALLBACK(on_destroy),action);
		g_signal_connect(action->dialog,"close",G_CALLBACK(gtk_widget_destroy),NULL);
		gtk_widget_show_all(GTK_WIDGET(action->dialog));

	}

 }

