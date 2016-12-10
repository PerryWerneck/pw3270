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
 * Este programa está nomeado como plugin.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#if defined WIN32

	// http://msdn.microsoft.com/en-us/library/windows/desktop/ms684179(v=vs.85).aspx
	#ifndef LOAD_LIBRARY_SEARCH_DEFAULT_DIRS
		#define LOAD_LIBRARY_SEARCH_DEFAULT_DIRS	0x00001000
	#endif // LOAD_LIBRARY_SEARCH_DEFAULT_DIRS

	#ifndef LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR
		#define LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR 	0x00000100
	#endif // LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR

	#include <windows.h>

#endif

 #include "private.h"

 #include <malloc.h>
 #include <libintl.h>
 #include <glib/gi18n.h>
 #include <gtk/gtk.h>

 #include <pw3270.h>
 #include <pw3270/plugin.h>
 #include <v3270.h>
 #include <lib3270/actions.h>
 #include <lib3270/log.h>
 #include <lib3270/trace.h>
 #include <lib3270/charset.h>
 #include <pw3270/class.h>
 #include <pw3270/trace.h>


/*--[ Globals ]--------------------------------------------------------------------------------------*/

#if GTK_CHECK_VERSION(2,32,0)
 static GMutex            mutex;
#else
 static GStaticMutex	  mutex = G_STATIC_MUTEX_INIT;
#endif // GTK_CHECK_VERSION

 using namespace std;


/*---[ Implement ]----------------------------------------------------------------------------------*/

namespace PW3270_NAMESPACE {

	void java::lock() {
#if GTK_CHECK_VERSION(2,32,0)
		g_mutex_lock(&mutex);
#else
		g_static_mutex_lock(&mutex);
#endif // GTK_CHECK_VERSION
	}

	void java::unlock() {
#if GTK_CHECK_VERSION(2,32,0)
		g_mutex_unlock(&mutex);
#else
		g_static_mutex_unlock(&mutex);
#endif // GTK_CHECK_VERSION
	}

	bool java::trylock() {
#if GTK_CHECK_VERSION(2,32,0)
		return g_mutex_trylock(&mutex);
#else
		return g_static_mutex_trylock(&mutex);
#endif // GTK_CHECK_VERSION
	}

}

using namespace PW3270_NAMESPACE;

extern "C" {

	static PW3270_NAMESPACE::session * factory(const char *name) {
		return session::create_local(lib3270_get_default_session_handle());
	}

	LIB3270_EXPORT int pw3270_plugin_start(GtkWidget *window, GtkWidget *terminal) {

		trace("JAVA: %s",__FUNCTION__);

		#if GTK_CHECK_VERSION(2,32,0)
			g_mutex_init(&mutex);
		#endif // GTK_CHECK_VERSION

		session::set_plugin(factory);

		return 0;
	}

	LIB3270_EXPORT int pw3270_plugin_stop(GtkWidget *window, GtkWidget *terminal) {

		java::lock();

		if(java::jvm) {
			java::jvm->DestroyJavaVM();
			java::jvm = NULL;
		}

		java::unlock();

		#if GTK_CHECK_VERSION(2,32,0)
			g_mutex_clear(&mutex);
		#endif // GTK_CHECK_VERSION

		trace("JAVA: %s",__FUNCTION__);

		return 0;
	}


}

