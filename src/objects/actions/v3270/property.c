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
  * @brief Implement GAction "wrapper" for v3270 properties.
  *
  * Reference:
  *
  * <https://github.com/GNOME/glib/blob/master/gio/gpropertyaction.c>
  *
  */

 #include "../private.h"
 #include <pw3270/window.h>
 #include <v3270.h>

 static void v3270PropertyAction_class_init(v3270PropertyActionClass *klass);
 static void v3270PropertyAction_init(v3270PropertyAction *action);

 G_DEFINE_TYPE(v3270PropertyAction, v3270PropertyAction, PW3270_TYPE_ACTION);

 void v3270PropertyAction_class_init(v3270PropertyActionClass *klass) {

 }

 static void v3270PropertyAction_init(v3270PropertyAction *action) {

 }

 v3270PropertyAction * v3270_property_action_new(GtkWidget *widget, const gchar *property_name) {

 	v3270PropertyAction * action = (v3270PropertyAction *) g_object_new(V3270_TYPE_PROPERTY_ACTION, NULL);

 	action->pspec = g_object_class_find_property(G_OBJECT_GET_CLASS(widget), property_name);

	if(~action->pspec->flags & G_PARAM_READABLE || ~action->pspec->flags & G_PARAM_WRITABLE || action->pspec->flags & G_PARAM_CONSTRUCT_ONLY) {

		g_critical(
			"Property '%s::%s' must be readable, writable, and not construct-only",
			G_OBJECT_TYPE_NAME(G_OBJECT(widget)),
			property_name
		);

		return NULL;
    }

 	return action;
 }
