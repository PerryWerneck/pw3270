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
 #include <v3270.h>
 #include <lib3270/actions.h>
 #include <lib3270/log.h>
 #include <lib3270/trace.h>
 #include <lib3270/charset.h>
 #include <pw3270/class.h>
 #include <pw3270/trace.h>

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

	LIB3270_EXPORT void pw3270_action_java_activated(GtkAction *action, GtkWidget *widget) {

		gchar *classname = (gchar *) g_object_get_data(G_OBJECT(action),"src");

		lib3270_trace_event(v3270_get_session(widget),"Action %s activated on widget %p\n",gtk_action_get_name(action),widget);

		if(classname)
		{
			// Has filename, call it directly
			call(widget,classname);
		}
		else
		{
/*
			// No classname, ask user
			filename = pw3270_file_chooser(GTK_FILE_CHOOSER_ACTION_OPEN, "java", _( "Select script to run" ), "", "class");

			if(filename)
			{
				call(widget,filename);
				g_free(filename);
			}
*/
		}

	}
 }


 namespace PW3270_NAMESPACE {

	JavaVM	* java::jvm		= NULL;
	JNIEnv	* java::env		= NULL;
#ifdef _WIN32
	HMODULE	  java::hModule	= NULL;
#endif // _WIN32

	void java::failed(GtkWidget *widget, const char *msg, const char *format, ...) {

		GtkWidget *dialog = gtk_message_dialog_new(	GTK_WINDOW(gtk_widget_get_toplevel(widget)),
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_ERROR,
													GTK_BUTTONS_OK_CANCEL,
													"%s", msg );

		gtk_window_set_title(GTK_WINDOW(dialog), _( "Java error" ));

		if(format) {

			va_list arg_ptr;
			va_start(arg_ptr, format);
			gchar *msg = g_strdup_vprintf(format,arg_ptr);
			va_end(arg_ptr);
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s",msg);
			g_free(msg);

		}

		if(gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_CANCEL)
			gtk_main_quit();
		gtk_widget_destroy(dialog);

	}


#ifdef WIN32

	bool java::load_jvm(GtkWidget *widget) {

		if(jvm != NULL) {
			return true;
		}

		// Dynamically load jvm library to avoid naming and path problems.
		HMODULE		  kernel;
		HANDLE 		  WINAPI (*AddDllDirectory)(PCWSTR NewDirectory);
		BOOL 	 	  WINAPI (*RemoveDllDirectory)(HANDLE Cookie);

		struct _dlldir {
			const gchar	* env;
			const gchar	* path;
			HANDLE		  cookie;
		} dlldir[] = {
			{ "JRE_HOME", "bin\\client", 		0 },
			{ "JDK_HOME", "jre\\bin\\client",	0 }
		};

		kernel = LoadLibrary("kernel32.dll");

		if(kernel) {

			AddDllDirectory	= (HANDLE WINAPI (*)(PCWSTR)) GetProcAddress(kernel,"AddDllDirectory");
			if(AddDllDirectory) {

				// Acrescenta mais caminhos para achar a dll
				for(size_t f = 0; f < G_N_ELEMENTS(dlldir); f++) {

					const char *env = getenv(dlldir[f].env);

					debug("%s=\"%s\"",dlldir[f].env,env);

					if(env) {

						gchar *p = g_build_filename(env,dlldir[f].path,NULL);

						lib3270_trace_event(v3270_get_session(widget),"Adding \"%s\" to DLL search path",p);

						wchar_t	*path = (wchar_t *) malloc(4096*sizeof(wchar_t));
						mbstowcs(path, p, 4095);
						dlldir[f].cookie = AddDllDirectory(path);
						free(path);

						g_free(p);

					}
				}

			} else {

				lib3270_trace_event(v3270_get_session(widget),"Can't find %s: %s","AddDllDirectory",session::win32_strerror(GetLastError()).c_str());

			}

		} else {

			lib3270_trace_event(v3270_get_session(widget),"Can't load %s: %s\n","kernel32.dll",session::win32_strerror(GetLastError()).c_str());

		}

		hModule = LoadLibrary("jvm.dll");
		if(!hModule) {
			lib3270_trace_event(v3270_get_session(widget),"Can't load %s\n","jvm.dll",session::win32_strerror(GetLastError()).c_str());

			for(size_t f = 0; !hModule && f < G_N_ELEMENTS(dlldir); f++) {

				const char *env = getenv(dlldir[f].env);

				debug("%s=\"%s\"",dlldir[f].env,env);

				if(env) {

					gchar *p = g_build_filename(env,dlldir[f].path,"jvm.dll",NULL);
					hModule = LoadLibrary(p);
					if(!hModule) {
						lib3270_trace_event(v3270_get_session(widget),"Can't load %s: %s\n",p,session::win32_strerror(GetLastError()).c_str());
					}
					g_free(p);

				}
			}
		}

		if(!hModule) {
			failed(widget, _(  "Can't load java virtual machine" ), "%s", session::win32_strerror(GetLastError()).c_str());
		}

		if(kernel) {

			RemoveDllDirectory	= (BOOL WINAPI (*)(HANDLE)) GetProcAddress(kernel,"RemoveDllDirectory");
			if(RemoveDllDirectory) {

				for(size_t f = 0; f < G_N_ELEMENTS(dlldir); f++) {

					if(dlldir[f].cookie) {

						RemoveDllDirectory(dlldir[f].cookie);

					}
				}

			}

			FreeLibrary(kernel);

		}

		if(!hModule) {
			return false;
		}

		// Consegui carregar a JVM, obtenho o método de controle
		jint JNICALL (*CreateJavaVM)(JavaVM **, void **, void *) = (jint JNICALL (*)(JavaVM **, void **, void *)) GetProcAddress(hModule,"JNI_CreateJavaVM");

		if(!CreateJavaVM) {
			failed(widget, _(  "Can't load java virtual machine creation method" ), "%s", session::win32_strerror(GetLastError()).c_str());
			return false;
		}

		// Crio a JVM
		JavaVMInitArgs	  vm_args;
		JavaVMOption	  options[5];

		jint			  rc		= 0;

		memset(&vm_args,0,sizeof(vm_args));
		memset(options,0,sizeof(options));

		vm_args.version				= JNI_VERSION_1_4;
		vm_args.nOptions			= 0;
		vm_args.options 			= options;
		vm_args.ignoreUnrecognized	= JNI_FALSE;

		options[vm_args.nOptions].optionString = g_strdup("vfprintf");
		options[vm_args.nOptions].extraInfo = (void *) jni_vfprintf;
		vm_args.nOptions++;

		gchar	* exports = NULL;
		char	  buffer[1024];
		gchar 	* myDir;

		if(GetModuleFileName(NULL,buffer,sizeof(buffer)) < sizeof(buffer)) {

			gchar * ptr = strrchr(buffer,G_DIR_SEPARATOR);
			if(ptr) {
				*ptr = 0;
				myDir = g_strdup(buffer);
			} else {
				myDir = g_strdup(".");
			}


		} else {

			myDir = g_strdup(".");

		}

		debug("myDir=%s",myDir);

		exports = g_build_filename(myDir,"jvm-exports",NULL);
		g_mkdir_with_parents(exports,0777);

		lib3270_trace_event(v3270_get_session(widget),"java.class.path=%s",exports);
		lib3270_trace_event(v3270_get_session(widget),"java.library.path=%s",myDir);

		options[vm_args.nOptions++].optionString = g_strdup_printf("-Djava.class.path=%s",exports);
		options[vm_args.nOptions++].optionString = g_strdup_printf("-Djava.library.path=%s",myDir);

		g_free(myDir);
		g_free(exports);

		rc = CreateJavaVM(&jvm,(void **)&env,&vm_args);
		if(rc < 0) {
			failed(widget, _(  "Can't create java VM" ), _( "The error code was %d" ), (int) rc);
			jvm = NULL;
		}

		for(int f=0;f<vm_args.nOptions;f++) {
			trace("Releasing option %d: %s",f,options[f].optionString);
			g_free(options[f].optionString);
		}


		return jvm != NULL;

	}



#else

	bool java::load_jvm(GtkWidget *widget) {

		if(jvm != NULL) {
			return true;
		}

		// Start JNI
		JavaVMInitArgs	  vm_args;
		JavaVMOption	  options[5];
		jint			  rc		= 0;


		memset(&vm_args,0,sizeof(vm_args));
		memset(options,0,sizeof(options));

		vm_args.version				= JNI_VERSION_1_4;
		vm_args.nOptions			= 0;
		vm_args.options 			= options;
		vm_args.ignoreUnrecognized	= JNI_FALSE;

		options[vm_args.nOptions].optionString = g_strdup("vfprintf");
		options[vm_args.nOptions].extraInfo = (void *) jni_vfprintf;
		vm_args.nOptions++;

#if defined(DEBUG)

//		options[vm_args.nOptions++].optionString = g_strdup("-verbose");
		options[vm_args.nOptions++].optionString = g_strdup_printf("-Djava.library.path=.bin/Debug:.bin/Debug/lib:%s",JNIDIR);
		options[vm_args.nOptions++].optionString = g_strdup_printf("-Djava.class.path=./src/java:.bin/java:%s",JARDIR);

#else

		options[vm_args.nOptions++].optionString = g_strdup_printf("-Djava.library.path=%s",JNIDIR);
		options[vm_args.nOptions++].optionString = g_strdup_printf("-Djava.class.path=%s",JARDIR);

#endif // JNIDIR

		// Linux, just create JVM
		rc = JNI_CreateJavaVM(&jvm,(void **)&env,&vm_args);

		if(rc) {
			jvm = NULL;
			failed(widget, _( "Can't create java virtual machine" ), _( "The return code was %d" ), (int) rc);
		}

		return jvm != NULL;
	}


#endif // WIN32

 }


