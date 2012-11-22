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

 #include "globals.h"
 #include <pw3270/v3270.h>



/*--[ Implement ]------------------------------------------------------------------------------------*/

 static int pw3270_macro_copy(GtkWidget *widget, int argc, const char **argv)
 {
 	trace("%s",__FUNCTION__);
	v3270_copy(widget, V3270_SELECT_TEXT, FALSE);
 	return 0;
 }

 static int pw3270_macro_append(GtkWidget *widget, int argc, const char **argv)
 {
 	trace("%s",__FUNCTION__);
	v3270_copy_append(widget);
 	return 0;
 }

 LIB3270_EXPORT int pw3270_run_macro(GtkWidget *widget, int argc, const char **argv)
 {
	#define PW3270_MACRO( name )  				{ #name, pw3270_macro_ ## name			}

	static const struct _list
	{
		const char *name;
		int (*exec)(GtkWidget *widget, int argc, const char **argv);
	} list[] =
	{
		PW3270_MACRO( copy ),
		PW3270_MACRO( append ),
	};
	int f;

			trace("<%s>",argv[0]);

	for(f=0;f<G_N_ELEMENTS(list);f++)
	{
		if(!g_strcasecmp(argv[0],list[f].name))
			return list[f].exec(widget,argc,argv);
	}

			trace("<%s>",argv[0]);

	return -1;
 }
