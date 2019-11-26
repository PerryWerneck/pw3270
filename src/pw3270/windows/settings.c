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
 * Este programa está nomeado como - e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <config.h>
 #include "../private.h"
 #include <v3270/settings.h>
 #include <lib3270/log.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 void load_terminal_settings(GtkWidget *widget)
 {
 	static HKEY			  predefined[] = { HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER };
	g_autofree gchar	* path = g_strdup_printf("SOFTWARE\\%s",g_get_application_name());

 	size_t	ix;

 	for(ix=0;ix < G_N_ELEMENTS(predefined); ix++)
	{
		HKEY hKey;
		if(RegOpenKeyEx(predefined[ix],path,0,KEY_READ,&hKey) == ERROR_SUCCESS)
		{
			v3270_load_registry(widget,hKey,"terminal");
			RegCloseKey(hKey);
		}
	}


 }

