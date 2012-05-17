/*
 * "Software G3270, desenvolvido com base nos códigos fontes do WC3270  e  X3270
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
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como html.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 *
 */

 #include <string.h>
 #include <lib3270.h>
 #include <lib3270/session.h>
 #include <lib3270/html.h>

 #include "globals.h"
 #include "utilc.h"

/*--[ Defines ]--------------------------------------------------------------------------------------*/

 enum html_element
 {
	HTML_ELEMENT_LINE_BREAK,
	HTML_ELEMENT_BEGIN_COLOR,
	HTML_ELEMENT_END_COLOR,

	HTML_ELEMENT_COUNT
 };

 static const char * element_text[HTML_ELEMENT_COUNT] =
 {
#ifdef DEBUG
	"<br />\n",
#else
	"<br />",
#endif // Debug
	"<span style=\"color:%s;background-color:%s\">",
	"</span>",
 };

 static const char * html_color[] =
 {
		"black",
		"deepSkyBlue",
		"red",
		"pink",
		"green",
		"turquoise",
		"yellow",
		"white",
		"black",
		"blue",
		"orange",
		"purple",
		"paleGreen",
		"paleTurquoise",
		"grey",
		"white"
 };

 struct html_info
 {
	int				  szText;
	char			* text;
	unsigned short	  fg;
	unsigned short	  bg;
 };

 /*--[ Implement ]------------------------------------------------------------------------------------*/

 static void append_string(struct html_info *info, const char *text)
 {
 	int sz = strlen(info->text)+strlen(text);

	if(strlen(info->text)+sz <= info->szText)
	{
		info->szText	+= (100+sz);
		info->text		 = lib3270_realloc(info->text,info->szText);
	}

	strcat(info->text,text);

 }

 static void append_element(struct html_info *info, enum html_element id)
 {
	append_string(info,element_text[id]);
 }

 static update_colors(struct html_info *info, unsigned short attr)
 {
	unsigned short	  fg;
	unsigned short	  bg	= ((attr & 0x00F0) >> 4);
	char 			* txt;

	#warning Fix field colors
	if(attr & LIB3270_ATTR_FIELD)
		fg = (attr & 0x0003);
	else
		fg = (attr & 0x000F);

	if(fg == info->fg && bg == info->bg)
		return;

	if(info->fg != 0xFF)
		append_string(info,element_text[HTML_ELEMENT_END_COLOR]);

	txt = xs_buffer(element_text[HTML_ELEMENT_BEGIN_COLOR],html_color[fg],html_color[bg]);
	append_string(info,txt);
	lib3270_free(txt);

	info->fg = fg;
	info->bg = bg;
 }

 LIB3270_EXPORT char * lib3270_get_as_html(H3270 *session, unsigned char all)
 {
	int	row, col, baddr;
	struct html_info info;

 	memset(&info,0,sizeof(info));
 	info.szText = session->rows * (session->cols + strlen(element_text[HTML_ELEMENT_LINE_BREAK])+1);
 	info.text	= lib3270_malloc(info.szText+1);
 	info.fg		= 0xFF;
 	info.bg		= 0xFF;

	baddr = 0;
	for(row=0;row < session->rows;row++)
	{
		int cr = 0;

		for(col = 0; col < session->cols;col++)
		{
			if(all || session->text[baddr].attr & LIB3270_ATTR_SELECTED)
			{
				char txt[] = { session->text[baddr].chr, 0 };
				cr++;
				update_colors(&info,session->text[baddr].attr);
				append_string(&info,txt);
			}
			baddr++;
		}

		if(cr)
			append_element(&info,HTML_ELEMENT_LINE_BREAK);
	}

	if(info.fg != 0xFF)
		append_string(&info,element_text[HTML_ELEMENT_END_COLOR]);

	return lib3270_realloc(info.text,strlen(info.text)+2);
 }


