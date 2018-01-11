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
 * Este programa está nomeado como charset.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <v3270.h>
 #include "private.h"
 #include <lib3270/charset.h>
 #include <lib3270/log.h>

 #define ERROR_DOMAIN g_quark_from_static_string(PACKAGE_NAME)

/*--[ Implement ]------------------------------------------------------------------------------------*/

 struct parse
 {
	char			* host;
	char	 		* display;
	unsigned long	  cgcsgid;
	size_t			  len;

	struct {
		unsigned short	ebc;
		unsigned short	iso;
		unsigned char	scope;
		unsigned char	oneway;
	} map[256];

 };

 static unsigned short getChar(const gchar *id, GError **error) {

	if(*error) {
		return 0;
	}

 	if(g_str_has_prefix(id,"0x")) {

        unsigned int rc = 0;

		if(sscanf(id + 2, "%x", &rc) != 1) {
			*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s",_( "Can't parse character value" ));
			return;
		}

		return (unsigned short) rc;

 	}


 	return (unsigned short) *id;
 }

 static void element_start(GMarkupParseContext *context, const gchar *element_name, const gchar **names,const gchar **values, struct parse *info, GError **error)
 {
	trace("%s(%s)",__FUNCTION__,element_name);

 	if(!g_ascii_strcasecmp(element_name,"pw3270-remap"))
	{
		const gchar *host		= NULL;
		const gchar *cgcsgid	= NULL;
		const gchar *display	= NULL;

		g_markup_collect_attributes(element_name,names,values,error,
								G_MARKUP_COLLECT_STRING, "host", &host,
								G_MARKUP_COLLECT_STRING, "cgcsgid", &cgcsgid,
								G_MARKUP_COLLECT_STRING, "display", &display,
								G_MARKUP_COLLECT_INVALID);
		if(*error)
		{
			return;
		}

		if(host)
		{
			g_free(info->host);
			info->host = g_strdup(host);
		}

		if(display)
		{
			g_free(info->display);
			info->display = g_strdup(display);
		}

		if(cgcsgid) {

			if(!g_str_has_prefix(cgcsgid,"0x"))
			{
				*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s",_( "Invalid cgcsgid value" ));
				return;
			}

			if(sscanf(cgcsgid + 2, "%lx", &info->cgcsgid) != 1) {
				*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s",_( "Can't parse cgcsgid value" ));
				return;
			}

		}

	}
 	else if(!g_ascii_strcasecmp(element_name,"char"))
 	{
 		if(info->len >= G_N_ELEMENTS(info->map)) {
			*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s",_( "Too many remaps" ));
			return;
 		}

		const gchar *ebc		= NULL;
		const gchar *iso		= NULL;
		const gchar *scope		= NULL;
		const gchar *oneway		= NULL;

		g_markup_collect_attributes(element_name,names,values,error,
								G_MARKUP_COLLECT_STRING, "ebc", &ebc,
								G_MARKUP_COLLECT_STRING, "iso", &iso,
								G_MARKUP_COLLECT_STRING|G_MARKUP_COLLECT_OPTIONAL, "scope", &scope,
								G_MARKUP_COLLECT_STRING|G_MARKUP_COLLECT_OPTIONAL, "one-way", &oneway,
								G_MARKUP_COLLECT_INVALID);

		if(*error)
		{
			return;
		}

		if(!scope)
		{
			scope = "both";
		}

		if(!oneway)
		{
			oneway = "no";
		}

		info->map[info->len].ebc = getChar(ebc,error);
		info->map[info->len].iso = getChar(iso,error);

		trace("%u: ebc=%04x iso=%04x %c",(unsigned int) info->len,info->map[info->len].ebc,info->map[info->len].iso,info->map[info->len].iso);

 		info->len++;

 	}
 }


 static void element_end(GMarkupParseContext *context, const gchar *element_name, struct parse *info, GError **error)
 {
	// trace("%s(%s)",__FUNCTION__,element_name);
 }


 LIB3270_EXPORT	void v3270_remap_from_xml(GtkWidget *widget, const gchar *path)
 {
	static const GMarkupParser parser =
	{
		(void (*)(GMarkupParseContext *, const gchar *, const gchar **, const gchar **, gpointer, GError **))
				element_start,
		(void (*)(GMarkupParseContext *, const gchar *, gpointer, GError **))
				element_end,
//		(void (*)(GMarkupParseContext *, const gchar *, gsize, gpointer, GError **))
		NULL,

//		(void (*)(GMarkupParseContext *context, const gchar *passthrough_text, gsize text_len,  gpointer user_data,GError **error))
		NULL,

//		(void (*)(GMarkupParseContext *, GError *, gpointer))
		NULL

	};

 	GError				* error		= NULL;
 	gchar				* text 		= NULL;
	struct parse		  cfg;
	v3270				* terminal	= GTK_V3270(widget);

	memset(&cfg,0,sizeof(cfg));

	if(g_file_get_contents(path,&text,NULL,&error))
	{

		GMarkupParseContext	* context = g_markup_parse_context_new(&parser,G_MARKUP_TREAT_CDATA_AS_TEXT|G_MARKUP_PREFIX_ERROR_POSITION,&cfg,NULL);
		g_markup_parse_context_parse(context,text,strlen(text),&error);
		g_markup_parse_context_free(context);

	}

	debug("error=%p",error);

	if(error)
	{
		GtkWidget	* dialog;
		gchar		* name = g_path_get_basename(path);

		dialog = gtk_message_dialog_new(	NULL,
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_WARNING,
											GTK_BUTTONS_OK,
											_(  "Can't parse %s" ), name);

		g_free(name);

		gtk_window_set_title(GTK_WINDOW(dialog), _( "Remap Failed" ) );

		if(error->message)
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", error->message);

		g_error_free(error);

		gtk_dialog_run(GTK_DIALOG (dialog));
		gtk_widget_destroy(dialog);

	}

	trace("cgcsgid = %lx",cfg.cgcsgid);
	trace("display = %s",cfg.display);
	trace("host = %s",cfg.host);
	trace("length = %u",(unsigned int) cfg.len);

	g_free(text);
	g_free(cfg.host);
	g_free(cfg.display);

 }

