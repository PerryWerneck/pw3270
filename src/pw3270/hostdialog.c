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
 * Este programa está nomeado como hostdialog.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "private.h"
 #include <v3270.h>

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 void hostname_action(GtkAction *action, GtkWidget *widget)
 {
 	H3270 * hSession = v3270_get_session(widget);
 	gchar * ptr;

 	lib3270_set_color_type(hSession,(unsigned short) get_integer_from_config("host","colortype",16));

	ptr = get_string_from_config("host","systype","s390");
	if(*ptr)
		lib3270_set_host_type_by_name(hSession,ptr);
	g_free(ptr);

	v3270_select_host(widget);

	/*

	No needed! The dialog fires a "save-settings" signal.

	set_string_to_config("terminal","oversize",pw3270_get_oversize(gtk_widget_get_toplevel(widget)));
	set_integer_to_config("terminal","model_number",lib3270_get_model_number(hSession));
 	set_string_to_config("terminal","model_name","%s",lib3270_get_model_name(hSession));
 	set_string_to_config("terminal","host_charset",lib3270_get_host_charset(hSession));
 	set_string_to_config("terminal","remap_file",v3270_get_remap_filename(widget));

 	{
 		g_autofree gchar * lunames = v3270_get_lunames(widget);
		set_string_to_config("terminal","lu_names",lunames);
 	}
 	*/

 }

