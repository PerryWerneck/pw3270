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
 * Este programa está nomeado como private.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

 #include <gtk/gtk.h>
 #include <lib3270/config.h>
 #include "../common/common.h"
 #include "parser.h"

 #define ERROR_DOMAIN g_quark_from_static_string("uiparser")

 enum ui_element
 {
 	UI_ELEMENT_MENUBAR,
 	UI_ELEMENT_MENU,
 	UI_ELEMENT_TOOLBAR,
 	UI_ELEMENT_TOOLITEM,
 	UI_ELEMENT_POPUP,

 	UI_ELEMENT_COUNT
 };

 #define UI_ELEMENT_SEPARATOR	UI_ELEMENT_COUNT+1
 #define UI_ELEMENT_ACCELERATOR UI_ELEMENT_COUNT+2
 #define UI_ELEMENT_SCRIPT		UI_ELEMENT_COUNT+3
 #define UI_ELEMENT_MENUITEM	UI_ELEMENT_COUNT+4

 struct parser
 {
// 	int						   disabled;
 	GtkWidget				*  toplevel;
 	GObject					*  element;
 	GtkAction				*  action;
// 	GCallback				   callback;
 	GtkWidget				*  center_widget;
 	GtkWidget				** popup;			/**< Popup widgets */
 	GStringChunk			*  strings;
	const gchar 			** group;			/**< Action group list */
	const gchar 			** popupname;		/**< Popup names */
	GHashTable	 			*  actions;			/**< List of actions */
	GHashTable				*  element_list[UI_ELEMENT_COUNT];
	const UI_WIDGET_SETUP	*  setup;
 };

 int 			  ui_parse_file(struct parser *info, const gchar *filename);
 void 			  ui_action_set_options(GtkAction *action, struct parser *info, const gchar **names, const gchar **values, GError **error);

 GObject 		* ui_get_element(struct parser *info, GtkAction *action, enum ui_element id, const gchar **names, const gchar **values, GError **error);
 GObject		* ui_insert_element(struct parser *info, GtkAction *action, enum ui_element id, const gchar **names, const gchar **values, GObject *widget, GError **error);

 GObject		* ui_create_menubar(GMarkupParseContext *context,GtkAction *action,struct parser *info,const gchar **names, const gchar **values, GError **error);
 GObject		* ui_create_menu(GMarkupParseContext *context,GtkAction *action,struct parser *info,const gchar **names, const gchar **values, GError **error);
 GObject		* ui_create_menuitem(GMarkupParseContext *context,GtkAction *action,struct parser *info,const gchar **names, const gchar **values, GError **error);
 GObject		* ui_create_toolbar(GMarkupParseContext *context,GtkAction *action,struct parser *info,const gchar **names, const gchar **values, GError **error);
 GObject		* ui_create_toolitem(GMarkupParseContext *context,GtkAction *action,struct parser *info,const gchar **names, const gchar **values, GError **error);
 GObject		* ui_create_separator(GMarkupParseContext *context,GtkAction *action,struct parser *info,const gchar **names, const gchar **values, GError **error);
 GObject		* ui_create_accelerator(GMarkupParseContext *context,GtkAction *action,struct parser *info,const gchar **names, const gchar **values, GError **error);
 GObject		* ui_create_popup(GMarkupParseContext *context,GtkAction *action,struct parser *info,const gchar **names, const gchar **values, GError **error);
 GObject		* ui_create_script(GMarkupParseContext *context,GtkAction *action,struct parser *info,const gchar **names, const gchar **values, GError **error);

 void			  ui_end_menubar(GMarkupParseContext *context,GObject *widget,struct parser *info,GError **error);
 void			  ui_end_menu(GMarkupParseContext *context,GObject *widget,struct parser *info,GError **error);
 void			  ui_end_menuitem(GMarkupParseContext *context,GObject *widget,struct parser *info,GError **error);
 void			  ui_end_toolbar(GMarkupParseContext *context,GObject *widget,struct parser *info,GError **error);
 void			  ui_end_toolitem(GMarkupParseContext *context,GObject *widget,struct parser *info,GError **error);
 void			  ui_end_separator(GMarkupParseContext *context,GObject *widget,struct parser *info,GError **error);
 void			  ui_end_accelerator(GMarkupParseContext *context,GObject *widget,struct parser *info,GError **error);
 void			  ui_end_popup(GMarkupParseContext *context,GObject *widget,struct parser *info,GError **error);
 void			  ui_end_script(GMarkupParseContext *context,GObject *widget,struct parser *info,GError **error);

 #include "parser.h"
