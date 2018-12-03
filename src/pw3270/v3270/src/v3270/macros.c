/*
 * "Software pw3270, desenvolvido com base nos c�digos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emula��o de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270.
 *
 * Copyright (C) <2008> <Banco do Brasil S.A.>
 *
 * Este programa � software livre. Voc� pode redistribu�-lo e/ou modific�-lo sob
 * os termos da GPL v.2 - Licen�a P�blica Geral  GNU,  conforme  publicado  pela
 * Free Software Foundation.
 *
 * Este programa � distribu�do na expectativa de  ser  �til,  mas  SEM  QUALQUER
 * GARANTIA; sem mesmo a garantia impl�cita de COMERCIALIZA��O ou  de  ADEQUA��O
 * A QUALQUER PROP�SITO EM PARTICULAR. Consulte a Licen�a P�blica Geral GNU para
 * obter mais detalhes.
 *
 * Voc� deve ter recebido uma c�pia da Licen�a P�blica Geral GNU junto com este
 * programa; se n�o, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa est� nomeado como macros.c e possui - linhas de c�digo.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendon�a)
 * licinio@bb.com.br		(Lic�nio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aur�lio Caldas Miranda)
 *
 */

 #include "private.h"
 #include <lib3270/macros.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 static int v3270_macro_copy(GtkWidget *widget, int argc, const char **argv)
 {
	v3270_copy(widget, V3270_SELECT_TEXT, FALSE);
 	return 0;
 }

 static int v3270_macro_append(GtkWidget *widget, int argc, const char **argv)
 {
	v3270_copy_append(widget);
 	return 0;
 }

 static int v3270_macro_clearsel(GtkWidget *widget, int argc, const char **argv)
 {
	v3270_clear_clipboard(GTK_V3270(widget));
 	return 0;
 }

 static int run_macro(GtkWidget *widget, int argc, const char **argv)
 {
	#define V3270_MACRO( name )  				{ #name, v3270_macro_ ## name			}

	static const struct _list
	{
		const char *name;
		int (*exec)(GtkWidget *widget, int argc, const char **argv);
	} list[] =
	{
		V3270_MACRO( copy ),
		V3270_MACRO( append ),
		V3270_MACRO( clearsel ),
	};

	int f;
	gchar *rsp;

	for(f=0;f<G_N_ELEMENTS(list);f++)
	{
		if(!g_ascii_strcasecmp(argv[0],list[f].name))
			return list[f].exec(widget,argc,argv);
	}

	rsp = lib3270_run_macro(GTK_V3270(widget)->host,argv);
	if(rsp)
	{
		g_free(rsp);
		return 0;
	}

	return -1;
 }

 int v3270_run_script(GtkWidget *widget, const gchar *script)
{
 	gchar **ln;
 	int 	f;

	if(!script)
		return 0;

	g_return_val_if_fail(GTK_IS_V3270(widget),EINVAL);

 	ln = g_strsplit(script,"\n",-1);

 	for(f=0;ln[f];f++)
	{
		GError	* error	= NULL;
		gint	  argc	= 0;
		gchar	**argv	= NULL;

		if(g_shell_parse_argv(g_strstrip(ln[f]),&argc,&argv,&error))
		{
			run_macro(widget, argc, (const char **) argv);
		}
		else
		{
			g_warning("Error parsing \"%s\": %s",g_strstrip(ln[f]),error->message);
			g_error_free(error);
		}

		if(argv)
			g_strfreev(argv);

	}

	g_strfreev(ln);

	return 0;
}

