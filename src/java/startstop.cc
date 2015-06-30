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
 * Este programa está nomeado como startstop.cc e possui - linhas de código.
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

#else

	#include <dlfcn.h>

#endif

 #include "private.h"

 #include <pw3270.h>
 #include <pw3270/plugin.h>
 #include <pw3270/v3270.h>
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

/*---[ Implement ]----------------------------------------------------------------------------------*/

using namespace PW3270_NAMESPACE::java;

extern "C" {

	static void trace_cleanup(GtkWidget *widget, GtkWidget **window) {
		*window = NULL;
	}

	static jint JNICALL jni_vfprintf(FILE *fp, const char *fmt, va_list args) {

		char 				* msg	= NULL;
		static GtkWidget	* trace = NULL;

		if(vasprintf(&msg,fmt,args) < 1) {
			lib3270_write_log(lib3270_get_default_session_handle(),"java","vasprintf() error on \"%s\"",fmt);
			return 0;
		}

		fprintf(fp,"%s",msg);
		lib3270_write_log(lib3270_get_default_session_handle(),"java","%s",msg);

		if(!trace) {
			// Cria janela de trace.
			trace = pw3270_trace_new();
			g_signal_connect(G_OBJECT(trace), "destroy",G_CALLBACK(trace_cleanup), &trace);

			pw3270_trace_set_destroy_on_close(trace,TRUE);

			// gtk_window_set_transient_for(GTK_WINDOW(trace),GTK_WINDOW(gtk_widget_get_toplevel(widget)));
			gtk_window_set_destroy_with_parent(GTK_WINDOW(trace),TRUE);

			gtk_window_set_default_size(GTK_WINDOW(trace),590,430);
			gtk_widget_show_all(trace);

			pw3270_trace_printf(trace,"%s",msg);

			free(msg);
		}

		return 0;
	}

 }

 LIB3270_EXPORT int pw3270_plugin_start(GtkWidget *window)
 {
	trace("JAVA: %s",__FUNCTION__);
#if GTK_CHECK_VERSION(2,32,0)
	g_mutex_init(&mutex);
#endif // GTK_CHECK_VERSION
	set_java_session_factory(factory);
	return 0;
 }

 LIB3270_EXPORT int pw3270_plugin_stop(GtkWidget *window)
 {
 	lock();

 	if(jvm) {
		jvm->DestroyJavaVM();
		jvm = NULL;
 	}

 	unlock();

#if GTK_CHECK_VERSION(2,32,0)
	g_mutex_clear(&mutex);
#endif // GTK_CHECK_VERSION
    trace("JAVA: %s",__FUNCTION__);



	return 0;
 }

 LIB3270_EXPORT void pw3270_action_java_activated(GtkAction *action, GtkWidget *widget)
 {
	gchar *filename = (gchar *) g_object_get_data(G_OBJECT(action),"src");

	lib3270_trace_event(v3270_get_session(widget),"Action %s activated on widget %p",gtk_action_get_name(action),widget);

	if(filename)
	{
		// Has filename, call it directly
		call(widget,filename);
	}
	else
	{
		// No filename, ask user
		static const struct _list
		{
			const gchar *name;
			const gchar *pattern;
		} list[] =
		{
			{ N_( "Java class file" ),	"*.class" }
		};

		GtkFileFilter * filter[G_N_ELEMENTS(list)+1];
		unsigned int f;

		memset(filter,0,sizeof(filter));

		for(f=0;f<G_N_ELEMENTS(list);f++)
		{
			filter[f] = gtk_file_filter_new();
			gtk_file_filter_set_name(filter[f],gettext(list[f].name));
			gtk_file_filter_add_pattern(filter[f],list[f].pattern);
		}

		filename = pw3270_get_filename(widget,"java","script",filter,_( "Select script to run" ));

		if(filename)
		{
			call(widget,filename);
			g_free(filename);
		}


	}

 }

 namespace PW3270_NAMESPACE {

	JavaVM	* java::jvm	= NULL;
	JNIEnv	* java::env	= NULL;

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


