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
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como xml2pot.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <string.h>

 static const gchar	*filename = NULL;
 static FILE		 	*out;
 static GHashTable	 	*hash = NULL;

 struct record
 {
 	const gchar	*filename;
 	const gchar	*label;
 	gint			line_number;
 	gint			char_number;

 	gchar			text[1];
 };

/*---[ Implement ]----------------------------------------------------------------------------------------*/

 static void element_start(GMarkupParseContext *context,const gchar *element_name,const gchar **names,const gchar **values, gpointer user_data, GError **error)
 {
 	int f;

 	for(f=0;names[f];f++)
 	{
 		if(!strcmp(names[f],"label") && values[f])
 		{
 			struct record *rec = g_hash_table_lookup(hash,values[f]);

 			if(!rec)
 			{
				struct record *rec = g_malloc0(sizeof(struct record)+strlen(values[f])+strlen(filename)+3);
				char 	*ptr = rec->text;

				g_markup_parse_context_get_position(context,&rec->line_number,&rec->char_number);

				strcpy(ptr,filename);
				rec->filename = ptr;
				ptr += (strlen(ptr)+1);

				strcpy(ptr,values[f]);
				rec->label = ptr;

				g_hash_table_insert(hash,(gpointer) rec->label, rec);
 			}
 		}
 	}

 }

 static void element_end(GMarkupParseContext *context, const gchar *element_name, gpointer user_data, GError **error)
 {
 }

 static void element_text(GMarkupParseContext *context,const gchar *text,gsize text_len, gpointer user_data, GError **error)
 {
 }

 static void element_passthrough(GMarkupParseContext *context,const gchar *passthrough_text, gsize text_len,  gpointer user_data,GError **error)
 {
 }

 static void element_error(GMarkupParseContext *context,GError *error,gpointer user_data)
 {
 }

 static const GMarkupParser parser =
 {
	element_start,
	element_end,
	element_text,
	element_passthrough,
	element_error,
 };

 static int parsefile(GMarkupParseContext *context)
 {
 	GError	*error	= NULL;
 	gchar	*contents = NULL;

	if(!g_file_get_contents(filename,&contents,NULL,&error))
	{
		fprintf(stderr,"%s\n",error->message);
		g_error_free(error);
		return -1;
	}

	if(!g_markup_parse_context_parse(context,contents,strlen(contents),&error))
	{
		fprintf(stderr,"%s\n",error->message);
		g_error_free(error);
		g_free(contents);
		return -1;
	}

	g_free(contents);
	return 0;
 }

 static void write_file(gpointer key,struct record *rec, FILE *out)
 {
	fprintf(out,"#: %s:%d\n",rec->filename,(int) rec->line_number);
	fprintf(out,"msgid \"%s\"\n",rec->label);
	fprintf(out,"msgstr \"\"\n\n");
 }

 int main (int argc, char *argv[])
 {
	static const char * header=	"# SOME DESCRIPTIVE TITLE.\n"
					"# Copyright (C) YEAR THE PACKAGE'S COPYRIGHT HOLDER\n"
					"# This file is distributed under the same license as the PACKAGE package.\n"
					"# FIRST AUTHOR <EMAIL@ADDRESS>, YEAR.\n"
					"#\n"
					"#, fuzzy\n"
					"msgid \"\"\n"
					"msgstr \"\"\n"
					"\"Project-Id-Version: PACKAGE VERSION\\n\"\n"
					"\"Report-Msgid-Bugs-To: \\n\"\n"
					"\"POT-Creation-Date: 2010-01-18 17:12-0200\\n\"\n"
					"\"PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE\\n\"\n"
					"\"Last-Translator: FULL NAME <EMAIL@ADDRESS>\\n\"\n"
					"\"Language-Team: LANGUAGE <LL@li.org>\\n\"\n"
					"\"Language: \\n\"\n"
					"\"MIME-Version: 1.0\\n\"\n"
					"\"Content-Type: text/plain; charset=CHARSET\\n\"\n"
					"\"Content-Transfer-Encoding: 8bit\\n\"\n\n";

 	int rc = 0;
 	int f;

	GMarkupParseContext *context = g_markup_parse_context_new(&parser,G_MARKUP_TREAT_CDATA_AS_TEXT,NULL,NULL);

	out = stdout;

	fprintf(out,"%s",header);

	hash = g_hash_table_new(g_str_hash, g_str_equal);

	for(f=1;f<argc;f++)
	{
		filename = argv[f];
		rc = parsefile(context);
	}

	g_hash_table_foreach(hash,(GHFunc) write_file, out);


	return rc;
 }

