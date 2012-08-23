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
 * Este programa está nomeado como tools.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "globals.h"

#if defined WIN32
	BOOL WINAPI DllMain(HANDLE hinst, DWORD dwcallpurpose, LPVOID lpvResvd);

	static int libpw3270_loaded(void);
	static int libpw3270_unloaded(void);

#else
	int libpw3270_loaded(void) __attribute__((constructor));
	int libpw3270_unloaded(void) __attribute__((destructor));
#endif

/*--[ Implement ]------------------------------------------------------------------------------------*/

#if defined WIN32

BOOL WINAPI DllMain(HANDLE hinst, DWORD dwcallpurpose, LPVOID lpvResvd)
{
//	Trace("%s - Library %s",__FUNCTION__,(dwcallpurpose == DLL_PROCESS_ATTACH) ? "Loaded" : "Unloaded");

    switch(dwcallpurpose)
    {
	case DLL_PROCESS_ATTACH:
			libpw3270_loaded();
			break;

	case DLL_PROCESS_DETACH:
			libpw3270_unloaded();
			break;
    }

    return TRUE;
}

#endif // WIN32

int libpw3270_loaded(void)
{
	trace("%s",__FUNCTION__);
	return 0;
}

int libpw3270_unloaded(void)
{
	trace("%s",__FUNCTION__);
	configuration_deinit();
	return 0;
}


LIB3270_EXPORT gchar * pw3270_build_filename(GtkWidget *widget, const gchar *first_element, ...)
{
	va_list args;
	gchar	*path;

	va_start(args, first_element);
	path = filename_from_va(first_element,args);
	va_end(args);
	return path;
}

LIB3270_EXPORT void pw3270_save_window_state(GtkWidget *widget, const gchar *name)
{
	save_window_to_config("window", name, widget);
}

LIB3270_EXPORT void pw3270_restore_window_state(GtkWidget *widget, const gchar *name)
{
	restore_window_from_config("window", name, widget);
}
