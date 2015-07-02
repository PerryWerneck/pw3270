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
 * Este programa está nomeado como call.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */


 #include "private.h"
 #include <pw3270/v3270.h>
 #include <lib3270/trace.h>


/*---[ Implement ]----------------------------------------------------------------------------------*/

namespace PW3270_NAMESPACE {


	void java::call(GtkWidget *widget, const char *classname) {

		if(!trylock()) {

			failed(widget, _( "Can't access java virtual machine" ), "%s", strerror(EBUSY));
			return;

		}

		if(jvm || load_jvm(widget)) {

			v3270_set_script(widget,'J',TRUE);
/*

			gchar * dirname		= g_path_get_dirname(filename);
			gchar * classname	= g_path_get_basename(filename);

			gchar * ptr			= strrchr(classname,'.');
			if(ptr) {
				*ptr = 0;
			}

			gchar	* classpath;

#ifdef _WIN32

			char	  buffer[1024];
			gchar	* exports;

			if(GetModuleFileName(NULL,buffer,sizeof(buffer)) < sizeof(buffer)) {

				gchar * ptr = strrchr(buffer,G_DIR_SEPARATOR);
				if(ptr) {
					*ptr = 0;
					exports = g_build_filename(buffer,"jvm-exports",NULL);
				} else {
					exports = g_build_filename(".","jvm-exports",NULL);
				}


			} else {

				exports = g_build_filename(".","jvm-exports",NULL);

			}

			debug("myDir=%s",myDir);

			g_mkdir_with_parents(exports,0777);

#ifdef DEBUG
			classpath = g_strdup_printf("%s;%s;.bin/java",dirname,exports);
#else
			classpath = g_strdup_printf("%s;%s",dirname,exports);
#endif

			g_free(exports);
#else

#ifdef DEBUG
			classpath = g_strdup_printf("%s:%s:.bin/java",dirname,JARDIR);
#else
			classpath = g_strdup_printf("%s:%s",dirname,JARDIR);
#endif

#endif // _WIN32

*/

			try {

				jclass		cls;
				jmethodID	mid;

				/*

				DONT WORK!!
				http://stackoverflow.com/questions/271506/why-cant-system-setproperty-change-the-classpath-at-runtime

				// Atualizar o classpath
				cls = env->FindClass("java/lang/System");
				if(!cls) {
					throw exception( _(  "Can't find class %s" ), "java/lang/System");
				}

				mid = env->GetStaticMethodID(cls, "setProperty", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
				if(!mid) {
					throw exception( _(  "Can't find method %s/%s" ), "java/lang/System","setProperty");
				}

				lib3270_trace_event(v3270_get_session(widget),"java.class.path=%s\n",classpath);

				jstring name = env->NewStringUTF("java.class.path");
				jstring path = env->NewStringUTF(classpath);

				jstring rc = (jstring) env->CallObjectMethod(cls,mid,name,path);

				env->DeleteLocalRef(name);
				env->DeleteLocalRef(path);
				env->DeleteLocalRef(rc);
				*/

				// Get application entry point.
				cls = env->FindClass(classname);
				if(!cls) {
					throw exception( _(  "Can't find class %s" ), classname);
				}

				mid = env->GetStaticMethodID(cls, "main", "([Ljava/lang/String;)V");
				if(!mid) {
					throw exception( _(  "Can't find method %s/%s" ), classname, "main");
				}

				// Build arguments
				jobjectArray args = env->NewObjectArray(0, env->FindClass("java/lang/String"), env->NewStringUTF(""));

				// Call main()
				env->CallStaticVoidMethod(cls, mid, args);

				// Check for exception


				// And finish
				env->DeleteLocalRef(args);

			} catch(std::exception &e) {

				failed(widget,_("Can't start java application"),"%s", e.what());

			}

/*
			g_free(dirname);
			g_free(classname);
			g_free(classpath);
*/

			v3270_set_script(widget,'J',FALSE);

		}

		unlock();


	}

}
