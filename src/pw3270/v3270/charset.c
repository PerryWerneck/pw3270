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
 #include <lib3270/X11keysym.h>

 #define ERROR_DOMAIN g_quark_from_static_string(PACKAGE_NAME)

/*--[ Implement ]------------------------------------------------------------------------------------*/

 struct parse
 {
	char			* host;
	char	 		* display;
	unsigned long	  cgcsgid;
	size_t			  len;

	struct {
		unsigned short		ebc;
		unsigned short		iso;
		lib3270_remap_scope	scope;
		unsigned char		oneway;
	} map[256];

 };

 static unsigned short getChar(const gchar *id, GError **error) {

	static struct
	{
		const char		* name;
		unsigned short	  keysym;
	} latin[] =
	{
		{ "space", XK_space },
		{ "exclam", XK_exclam },
		{ "quotedbl", XK_quotedbl },
		{ "numbersign", XK_numbersign },
		{ "dollar", XK_dollar },
		{ "percent", XK_percent },
		{ "ampersand", XK_ampersand },
		{ "apostrophe", XK_apostrophe },
		{ "quoteright", XK_quoteright },
		{ "parenleft", XK_parenleft },
		{ "parenright", XK_parenright },
		{ "asterisk", XK_asterisk },
		{ "plus", XK_plus },
		{ "comma", XK_comma },
		{ "minus", XK_minus },
		{ "period", XK_period },
		{ "slash", XK_slash },
		{ "0", XK_0 },
		{ "1", XK_1 },
		{ "2", XK_2 },
		{ "3", XK_3 },
		{ "4", XK_4 },
		{ "5", XK_5 },
		{ "6", XK_6 },
		{ "7", XK_7 },
		{ "8", XK_8 },
		{ "9", XK_9 },
		{ "colon", XK_colon },
		{ "semicolon", XK_semicolon },
		{ "less", XK_less },
		{ "equal", XK_equal },
		{ "greater", XK_greater },
		{ "question", XK_question },
		{ "at", XK_at },
		{ "A", XK_A },
		{ "B", XK_B },
		{ "C", XK_C },
		{ "D", XK_D },
		{ "E", XK_E },
		{ "F", XK_F },
		{ "G", XK_G },
		{ "H", XK_H },
		{ "I", XK_I },
		{ "J", XK_J },
		{ "K", XK_K },
		{ "L", XK_L },
		{ "M", XK_M },
		{ "N", XK_N },
		{ "O", XK_O },
		{ "P", XK_P },
		{ "Q", XK_Q },
		{ "R", XK_R },
		{ "S", XK_S },
		{ "T", XK_T },
		{ "U", XK_U },
		{ "V", XK_V },
		{ "W", XK_W },
		{ "X", XK_X },
		{ "Y", XK_Y },
		{ "Z", XK_Z },
		{ "bracketleft", XK_bracketleft },
		{ "backslash", XK_backslash },
		{ "bracketright", XK_bracketright },
		{ "asciicircum", XK_asciicircum },
		{ "underscore", XK_underscore },
		{ "grave", XK_grave },
		{ "quoteleft", XK_quoteleft },
		{ "a", XK_a },
		{ "b", XK_b },
		{ "c", XK_c },
		{ "d", XK_d },
		{ "e", XK_e },
		{ "f", XK_f },
		{ "g", XK_g },
		{ "h", XK_h },
		{ "i", XK_i },
		{ "j", XK_j },
		{ "k", XK_k },
		{ "l", XK_l },
		{ "m", XK_m },
		{ "n", XK_n },
		{ "o", XK_o },
		{ "p", XK_p },
		{ "q", XK_q },
		{ "r", XK_r },
		{ "s", XK_s },
		{ "t", XK_t },
		{ "u", XK_u },
		{ "v", XK_v },
		{ "w", XK_w },
		{ "x", XK_x },
		{ "y", XK_y },
		{ "z", XK_z },
		{ "braceleft", XK_braceleft },
		{ "bar", XK_bar },
		{ "braceright", XK_braceright },
		{ "asciitilde", XK_asciitilde },
		{ "nobreakspace", XK_nobreakspace },
		{ "exclamdown", XK_exclamdown },
		{ "cent", XK_cent },
		{ "sterling", XK_sterling },
		{ "currency", XK_currency },
		{ "yen", XK_yen },
		{ "brokenbar", XK_brokenbar },
		{ "section", XK_section },
		{ "diaeresis", XK_diaeresis },
		{ "copyright", XK_copyright },
		{ "ordfeminine", XK_ordfeminine },
		{ "guillemotleft", XK_guillemotleft },
		{ "notsign", XK_notsign },
		{ "hyphen", XK_hyphen },
		{ "registered", XK_registered },
		{ "macron", XK_macron },
		{ "degree", XK_degree },
		{ "plusminus", XK_plusminus },
		{ "twosuperior", XK_twosuperior },
		{ "threesuperior", XK_threesuperior },
		{ "acute", XK_acute },
		{ "mu", XK_mu },
		{ "paragraph", XK_paragraph },
		{ "periodcentered", XK_periodcentered },
		{ "cedilla", XK_cedilla },
		{ "onesuperior", XK_onesuperior },
		{ "masculine", XK_masculine },
		{ "guillemotright", XK_guillemotright },
		{ "onequarter", XK_onequarter },
		{ "onehalf", XK_onehalf },
		{ "threequarters", XK_threequarters },
		{ "questiondown", XK_questiondown },
		{ "Agrave", XK_Agrave },
		{ "Aacute", XK_Aacute },
		{ "Acircumflex", XK_Acircumflex },
		{ "Atilde", XK_Atilde },
		{ "Adiaeresis", XK_Adiaeresis },
		{ "Aring", XK_Aring },
		{ "AE", XK_AE },
		{ "Ccedilla", XK_Ccedilla },
		{ "Egrave", XK_Egrave },
		{ "Eacute", XK_Eacute },
		{ "Ecircumflex", XK_Ecircumflex },
		{ "Ediaeresis", XK_Ediaeresis },
		{ "Igrave", XK_Igrave },
		{ "Iacute", XK_Iacute },
		{ "Icircumflex", XK_Icircumflex },
		{ "Idiaeresis", XK_Idiaeresis },
		{ "ETH", XK_ETH },
		{ "Eth", XK_Eth },
		{ "Ntilde", XK_Ntilde },
		{ "Ograve", XK_Ograve },
		{ "Oacute", XK_Oacute },
		{ "Ocircumflex", XK_Ocircumflex },
		{ "Otilde", XK_Otilde },
		{ "Odiaeresis", XK_Odiaeresis },
		{ "multiply", XK_multiply },
		{ "Ooblique", XK_Ooblique },
		{ "Ugrave", XK_Ugrave },
		{ "Uacute", XK_Uacute },
		{ "Ucircumflex", XK_Ucircumflex },
		{ "Udiaeresis", XK_Udiaeresis },
		{ "Yacute", XK_Yacute },
		{ "THORN", XK_THORN },
		{ "Thorn", XK_Thorn },
		{ "ssharp", XK_ssharp },
		{ "agrave", XK_agrave },
		{ "aacute", XK_aacute },
		{ "acircumflex", XK_acircumflex },
		{ "atilde", XK_atilde },
		{ "adiaeresis", XK_adiaeresis },
		{ "aring", XK_aring },
		{ "ae", XK_ae },
		{ "ccedilla", XK_ccedilla },
		{ "egrave", XK_egrave },
		{ "eacute", XK_eacute },
		{ "ecircumflex", XK_ecircumflex },
		{ "ediaeresis", XK_ediaeresis },
		{ "igrave", XK_igrave },
		{ "iacute", XK_iacute },
		{ "icircumflex", XK_icircumflex },
		{ "idiaeresis", XK_idiaeresis },
		{ "eth", XK_eth },
		{ "ntilde", XK_ntilde },
		{ "ograve", XK_ograve },
		{ "oacute", XK_oacute },
		{ "ocircumflex", XK_ocircumflex },
		{ "otilde", XK_otilde },
		{ "odiaeresis", XK_odiaeresis },
		{ "division", XK_division },
		{ "oslash", XK_oslash },
		{ "ugrave", XK_ugrave },
		{ "uacute", XK_uacute },
		{ "ucircumflex", XK_ucircumflex },
		{ "udiaeresis", XK_udiaeresis },
		{ "yacute", XK_yacute },
		{ "thorn", XK_thorn },
		{ "ydiaeresis", XK_ydiaeresis },

		// The following are, umm, hacks to allow symbolic names for
		// control codes.
	#if !defined(_WIN32)
		{ "BackSpace", 0x08 },
		{ "Tab", 0x09 },
		{ "Linefeed", 0x0a },
		{ "Return", 0x0d },
		{ "Escape", 0x1b },
		{ "Delete", 0x7f },
	#endif
	};

	size_t ix;

	if(*error) {
		return 0;
	}

 	if(g_str_has_prefix(id,"0x")) {

        unsigned int rc = 0;

		if(sscanf(id + 2, "%x", &rc) != 1)
		{
			*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s",_( "Can't parse character value" ));
			return 0;
		}

		return (unsigned short) rc;

 	}

 	for(ix=0;ix < G_N_ELEMENTS(latin);ix++) {
		if(!g_ascii_strcasecmp(id,latin[ix].name))
			return latin[ix].keysym;
 	}

 	if(strlen(id) != 1)
	{
		*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s",_( "Invalid character value" ));
		return 0;
	}

 	return (unsigned short) *id;
 }

 static lib3270_remap_scope getRemapScope(const gchar *str, GError **error) {

 	static const char *text[] = { "CS_ONLY","FT_ONLY", "BOTH" };
 	int i;

 	if(!error)
	{
		for(i=0;i < G_N_ELEMENTS(text);i++)
		{
			if(!g_ascii_strcasecmp(str,text[i]))
				return (lib3270_remap_scope) i;
		}

		*error = g_error_new(ERROR_DOMAIN,EINVAL,"%s",_( "Invalid remap scope" ));
	}

	return BOTH;
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

		info->map[info->len].ebc 	= getChar(ebc,error);
		info->map[info->len].iso 	= getChar(iso,error);
		info->map[info->len].scope	= getRemapScope(scope,error);

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

	} else {

		H3270 * hSession = v3270_get_session(widget);


		if(hSession)
		{
			unsigned int i;

			trace("cgcsgid = %lx",cfg.cgcsgid);
			trace("display = %s",cfg.display);
			trace("host = %s",cfg.host);
			trace("length = %u",(unsigned int) cfg.len);

			lib3270_reset_charset(hSession, cfg.host, cfg.display, cfg.cgcsgid);

			for(i=0;i < cfg.len; i++)
			{
				lib3270_remap_char(hSession,cfg.map[i].ebc,cfg.map[i].iso, BOTH, 0);
			}

		}


	}


	g_free(text);
	g_free(cfg.host);
	g_free(cfg.display);

 }

