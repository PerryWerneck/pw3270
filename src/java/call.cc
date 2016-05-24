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
 #include <v3270.h>
 #include <lib3270/trace.h>
 #include <lib3270/log.h>


/*---[ Implement ]----------------------------------------------------------------------------------*/

namespace PW3270_NAMESPACE {


	void java::call(GtkWidget *widget, const char *classname) {

		debug("%s(%s)",__FUNCTION__,classname);

		if(!trylock()) {

			failed(widget, _( "Can't access java virtual machine" ), "%s", strerror(EBUSY));
			return;

		}

		if(jvm || load_jvm(widget)) {

			v3270_set_script(widget,'J',TRUE);

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
				jthrowable exc = env->ExceptionOccurred();
				env->ExceptionClear();

				if (exc) {
					jclass throwable_class = env->FindClass("java/lang/Throwable");

					jmethodID jni_getMessage = env->GetMethodID(throwable_class,"getMessage","()Ljava/lang/String;");
					jstring j_msg = (jstring) env->CallObjectMethod(exc,jni_getMessage);

					GtkWidget *dialog = gtk_message_dialog_new(	GTK_WINDOW(gtk_widget_get_toplevel(widget)),
																GTK_DIALOG_DESTROY_WITH_PARENT,
																GTK_MESSAGE_ERROR,
																GTK_BUTTONS_OK_CANCEL,
																_(  "Java application \"%s\" has failed." ), classname );

					gtk_window_set_title(GTK_WINDOW(dialog), _( "Java error" ));

					if(!env->IsSameObject(j_msg,NULL)) {

						const char	* msg = env->GetStringUTFChars(j_msg, 0);

						gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s",msg);

						env->ReleaseStringUTFChars( j_msg, msg);
					}

					if(gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_CANCEL)
						gtk_main_quit();
					gtk_widget_destroy(dialog);


				}

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
