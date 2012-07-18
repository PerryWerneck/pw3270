/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob o nome G3270.
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
 * Este programa está nomeado como main.cpp e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 *
 */

 #include "globals.h"
 #include <stdio.h>
 #include <string.h>
 #include <lib3270/config.h>
 #include <lib3270/popup.h>
 #include <lib3270/internals.h>

/*--[ Structs ]--------------------------------------------------------------------------------------*/

 typedef struct _timer
 {
 	size_t	  sz;
 	bool	  enabled;
 	H3270	* session;
 	void	  (*proc)(H3270 *session);
 } TIMER;

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 const char *java_class_name = "br/com/bb/pw3270/lib3270";

/*--[ Implement ]------------------------------------------------------------------------------------*/

static void post_message(H3270 *session, int msgid, int arg1 = 0, int arg2 = 0)
{
	if(session->widget)
	{
		JNIEnv		* env			= ((INFO *) session->widget)->env;
		jobject		  obj 			= ((INFO *) session->widget)->obj;
		jclass 		  cls 			= env->GetObjectClass(obj);
		jmethodID	  mid			= env->GetMethodID(cls, "postMessage", "(III)V");;
		env->CallVoidMethod(obj,mid,(jint) msgid, (jint) arg1, (jint) arg2);
	}
}

static void changed(H3270 *session, int offset, int len)
{
	post_message(session,2,offset,len);
}

static void erase(H3270 *session)
{
	post_message(session,4);
}

static int popuphandler(H3270 *session, void *terminal, LIB3270_NOTIFY type, const char *title, const char *msg, const char *fmt, va_list args)
{
	if(session->widget)
	{
		JNIEnv		* env			= ((INFO *) session->widget)->env;
		jobject		  obj 			= ((INFO *) session->widget)->obj;
		jclass 		  cls 			= env->GetObjectClass(obj);
		jmethodID	  mid			= env->GetMethodID(cls, "popupMessage", "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
		char		* descr;

        descr = lib3270_vsprintf(fmt, args);

		env->CallVoidMethod(obj,mid,	(jint) type,
										env->NewStringUTF(title),
										env->NewStringUTF(msg),
										env->NewStringUTF(descr) );
        lib3270_free(descr);
	}
	else
	{
		__android_log_print(ANDROID_LOG_VERBOSE, PACKAGE_NAME, "Can't open popup \"%s\", no jni env for active session",title);
	}
}

static void ctlr_done(H3270 *session)
{
	post_message(session,4);
}

static int write_buffer(H3270 *session, unsigned const char *buf, int len)
{
	int rc = -1;

	if(session->widget)
	{
		JNIEnv		* env			= ((INFO *) session->widget)->env;
		jobject		  obj 			= ((INFO *) session->widget)->obj;
		jclass 		  cls 			= env->GetObjectClass(obj);
		jmethodID	  mid			= env->GetMethodID(cls, "send_data", "([BI)I");
		jbyteArray	  buffer		= env->NewByteArray(len);

		env->SetByteArrayRegion(buffer, 0, len, (jbyte*) buf);

		rc = env->CallIntMethod(obj, mid, buffer, (jint) len );
	}
	else
	{
		__android_log_print(ANDROID_LOG_VERBOSE, PACKAGE_NAME, "Can't send %d bytes, no jni env for active session",len);
	}

	return rc;
}

static void * add_timer(unsigned long interval_ms, H3270 *session, void (*proc)(H3270 *session))
{
	TIMER * timer = NULL;

	if(session->widget)
	{
		JNIEnv		* env			= ((INFO *) session->widget)->env;
		jobject		  obj 			= ((INFO *) session->widget)->obj;
		jclass 		  cls 			= env->GetObjectClass(obj);
		jmethodID	  mid			= env->GetMethodID(cls, "newTimer", "(JI)V");

		timer			= (TIMER *) lib3270_malloc(sizeof(TIMER));
		timer->sz		= sizeof(timer);
		timer->enabled	= true;
		timer->session  = session;
		timer->proc		= proc;

		trace("Timer %08lx created",(unsigned long) timer);

		env->CallVoidMethod(obj,mid, (jlong) timer, (jint) interval_ms);

	}
	else
	{
		__android_log_print(ANDROID_LOG_VERBOSE, PACKAGE_NAME, "Can set timer, no jni env for active session");
	}

	return timer;
}

static void remove_timer(void *id)
{
	TIMER *timer = (TIMER *) id;

	if(timer == NULL)
	{
//		__android_log_print(ANDROID_LOG_VERBOSE, PACKAGE_NAME, "Invalid timer ID %08lx",(unsigned long) timer);
		return;
	}

	__android_log_print(ANDROID_LOG_VERBOSE, PACKAGE_NAME, "Disabling timer %08lx",(unsigned long) timer);

	timer->enabled = false;

}

JNIEXPORT void JNICALL Java_br_com_bb_pw3270_lib3270_timerFinish(JNIEnv *env, jobject obj, jlong id)
{
	TIMER *timer = (TIMER *) id;

	if(timer == NULL)
	{
//		__android_log_print(ANDROID_LOG_VERBOSE, PACKAGE_NAME, "Unexpected call to %s: No timer ID",__FUNCTION__);
		return;
	}

	if(timer->enabled)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, PACKAGE_NAME, "Running timer %08lx",(unsigned long) timer);
		timer->proc(timer->session);
	}

	lib3270_free(timer);
}

#ifdef X3270_TRACE
static void tracehandler(H3270 *session, const char *fmt, va_list args)
{
	static char		* buffer 	= NULL;
	static int		  szBuffer	= 0;

	char		  	  temp[4096];
	size_t			  sz;
	char			* src;
	char			* dst;

	if(!szBuffer)
	{
		buffer = (char *) lib3270_malloc(szBuffer = 4096);
		*buffer = 0;
	}

	sz = vsnprintf(temp,4095,fmt,args);

	if( (sz+strlen(buffer)) > szBuffer)
	{
		szBuffer += (sz+strlen(buffer)+1);
		buffer = (char *) lib3270_realloc(buffer,szBuffer);
	}

	dst = buffer+strlen(buffer);
	for(src = temp;*src;src++)
	{
		if(*src == '\n')
		{
			*dst = 0;
			__android_log_print(ANDROID_LOG_DEBUG, PACKAGE_NAME, "%s", buffer);
			dst = buffer;
		}
		else
		{
			*(dst++) = *src;
		}
	}

	*dst = 0;
}
#endif // X3270_TRACE

JNIEXPORT jint JNICALL Java_br_com_bb_pw3270_lib3270_init(JNIEnv *env, jclass obj)
{
	H3270	* session	= lib3270_session_new("");

	__android_log_print(ANDROID_LOG_DEBUG, PACKAGE_NAME, "Initializing session %p",session);

#ifdef X3270_TRACE
	lib3270_set_trace_handler(tracehandler);
#endif // X3270_TRACE

	lib3270_set_popup_handler(popuphandler);
	lib3270_register_time_handlers(add_timer,remove_timer);

	session->write			= write_buffer;
	session->changed 		= changed;
	session->erase			= erase;
	session->ctlr_done		= ctlr_done;

	return 0;
}

JNIEXPORT jint JNICALL Java_br_com_bb_pw3270_lib3270_processEvents(JNIEnv *env, jobject obj)
{
	session_request(env,obj);

	lib3270_main_iterate(session,1);

	session_release();

	return 0;
}

JNIEXPORT jboolean JNICALL Java_br_com_bb_pw3270_lib3270_isConnected(JNIEnv *env, jobject obj)
{
	return (lib3270_connected(lib3270_get_default_session_handle())) ? JNI_TRUE : JNI_FALSE;;
}

JNIEXPORT jboolean JNICALL Java_br_com_bb_pw3270_lib3270_isTerminalReady(JNIEnv *env, jobject obj)
{
	return JNI_FALSE;
}

JNIEXPORT void JNICALL Java_br_com_bb_pw3270_lib3270_setHost(JNIEnv *env, jobject obj, jstring hostname)
{
	session_request(env,obj);
	lib3270_set_host(session,env->GetStringUTFChars(hostname, 0));
	session_release();
}

JNIEXPORT jstring JNICALL Java_br_com_bb_pw3270_lib3270_getHost(JNIEnv *env, jobject obj)
{
	return env->NewStringUTF(lib3270_get_host(lib3270_get_default_session_handle()));
}

JNIEXPORT void JNICALL Java_br_com_bb_pw3270_lib3270_set_1connection_1status(JNIEnv *env, jobject obj, jboolean connected)
{
	session_request(env,obj);
	if(connected)
		lib3270_set_connected(session);
	else
		lib3270_set_disconnected(session);
	session_release();
}

JNIEXPORT void JNICALL Java_br_com_bb_pw3270_lib3270_procRecvdata(JNIEnv *env, jobject obj, jbyteArray buffer, jint sz)
{
	unsigned char *netrbuf = (unsigned char *) env->GetByteArrayElements(buffer,NULL);

	session_request(env,obj);

	trace("Processando %d bytes",(size_t) sz);

	lib3270_data_recv(session, (size_t) sz, netrbuf);

	trace("Liberando %d bytes",(size_t) sz);

	env->ReleaseByteArrayElements(buffer, (signed char *) netrbuf, 0);

	session_release();

}
