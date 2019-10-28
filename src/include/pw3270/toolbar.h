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
 * @brief Declares the pw3270 Toolbar widget.
 *
 */

#ifndef PW3270_TOOLBAR_H_INCLUDED

	#define PW3270_TOOLBAR_H_INCLUDED

	#include <gtk/gtk.h>
	#include <lib3270.h>
	#include <lib3270/actions.h>

	G_BEGIN_DECLS

	#define PW3270_TYPE_TOOLBAR				(pw3270ToolBar_get_type())
	#define PW3270_TOOLBAR(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), PW3270_TYPE_TOOLBAR, pw3270ToolBar))
	#define PW3270_TOOLBAR_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), PW3270_TYPE_TOOLBAR, pw3270ToolBarClass))
	#define PW3270_IS_TOOLBAR(obj)			(G_TYPE_CHECK_INSTANCE_TYPE ((obj), PW3270_TYPE_TOOLBAR))
	#define PW3270_IS_TOOLBAR_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PW3270_TYPE_TOOLBAR))
	#define PW3270_TOOLBAR_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), PW3270_TYPE_TOOLBAR, pw3270ToolBarClass))

	typedef struct _pw3270ToolBar			pw3270ToolBar;
	typedef struct _pw3270ToolBarClass		pw3270ToolBarClass;

	GType pw3270ToolBar_get_type(void) G_GNUC_CONST;

	GtkWidget * pw3270_toolbar_new(void);

	GtkWidget * pw3270_toolbar_insert_lib3270_action(GtkWidget *toolbar, const LIB3270_ACTION *action, gint pos);
	GtkWidget * pw3270_toolbar_insert_action(GtkWidget *toolbar, GAction *action, gint pos);
	GtkWidget * pw3270_toolbar_insert_action_by_name(GtkWidget *toolbar, const gchar *name, gint pos);

	void pw3270_toolbar_toolbar_set_style(GtkToolbar *toolbar, GtkToolbarStyle style);
//	GtkToolbarStyle pw3270_toolbar_toolbar_get_style(GtkToolbar *toolbar);

	void pw3270_toolbar_set_icon_size(GtkToolbar *toolbar, GtkIconSize icon_size);
//	GtkIconSize pw3270_toolbar_get_icon_size(GtkToolbar *toolbar);

	G_END_DECLS

#endif // PW3270_TOOLBAR_H_INCLUDED
