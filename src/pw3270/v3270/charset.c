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


 #define ERROR_DOMAIN g_quark_from_static_string(PACKAGE_NAME)

/*--[ Implement ]------------------------------------------------------------------------------------*/

 struct parse
 {
	char			* host;
	char	 		* display;
	unsigned long	  cgcsgid;
 };

 LIB3270_EXPORT	void v3270_remap_from_xml(GtkWidget *widget, const gchar *path)
 {
	struct parse	  cfg;
	v3270			* terminal = GTK_V3270(widget);

	memset(&cfg,0,sizeof(cfg));



	g_free(cfg.host);
	g_free(cfg.display);

 }

