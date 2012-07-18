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
 * Este programa está nomeado como menu.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

 #include "private.h"

 #ifdef X3270_TRACE
	#define trace_action(a,w) lib3270_trace_event(NULL,"Action %s activated on widget %p\n",gtk_action_get_name(a),w);
 #else
	#define trace_action(a,w) /* */
 #endif // X3270_TRACE

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static void element_start(GMarkupParseContext *context, const gchar *element_name, const gchar **names,const gchar **values, struct parser *info, GError **error)
 {
 	trace("%s: %s",__FUNCTION__,element_name);
 }

 static void element_end(GMarkupParseContext *context, const gchar *element_name, struct parser *info, GError **error)
 {
 	trace("%s: %s",__FUNCTION__,element_name);
 }

 static void script_text(GMarkupParseContext *context, const gchar *element_text, gsize text_len, struct parser *info, GError **error)
 {
	ui_connect_text_script(info->center_widget, info->script_action,element_text,error);
 }

 GObject * ui_create_script(GMarkupParseContext *context,GtkAction *action, struct parser *info, const gchar **names, const gchar **values, GError **error)
 {
	static const GMarkupParser parser =
	{
		(void (*)(GMarkupParseContext *, const gchar *, const gchar **, const gchar **, gpointer, GError **))
				element_start,
		(void (*)(GMarkupParseContext *, const gchar *, gpointer, GError **))
				element_end,
		(void (*)(GMarkupParseContext *, const gchar *, gsize, gpointer, GError **))
				script_text,

//		(void (*)(GMarkupParseContext *, GError *, gpointer))
		NULL

	};

	if(action)
	{
		*error = g_error_new(ERROR_DOMAIN,EINVAL,_( "action attribute is invalid for <%s>"),"script");
		return NULL;
	}

// 	trace("%s: info->element: %p action: %p",__FUNCTION__,info->element, action);

	if(!(info->element && info->actions))
	{
		*error = g_error_new(ERROR_DOMAIN,EINVAL,_( "<%s> is invalid at this context"),"script");
		return NULL;
	}

	info->script_action = info->action;
	g_markup_parse_context_push(context,&parser,info);

	return NULL;
 }

 void ui_end_script(GMarkupParseContext *context,GObject *widget,struct parser *info,GError **error)
 {
 	g_markup_parse_context_pop(context);
 }

