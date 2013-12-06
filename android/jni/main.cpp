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
 #include <pthread.h>
 #include <string.h>
 #include <lib3270/config.h>
 #include <lib3270/popup.h>
 #include <lib3270/internals.h>
 #include <lib3270/html.h>

/*--[ Structs ]--------------------------------------------------------------------------------------*/

 typedef struct _timer
 {
 	size_t	  sz;
 	bool	  enabled;
 	H3270	* session;
 	void	  (*proc)(H3270 *session);
 } TIMER;

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 const char 			* java_class_name	= "br/com/bb/pw3270/lib3270";
 PW3270_JNI				* pw3270_jni_active	= NULL;
 static pthread_mutex_t	  mutex;
 static char			* startup_script	= NULL;

/*--[ Implement ]------------------------------------------------------------------------------------*/


jmethodID lib3270_getmethodID(const char *name, const char *sig)
{
	if(!pw3270_jni_active)
	{
		__android_log_print(ANDROID_LOG_ERROR, PACKAGE_NAME, "%s(%s,%s) called outside jni environment",__FUNCTION__,name,sig);
		return NULL;
	}

	return PW3270_JNI_ENV->GetMethodID(PW3270_JNI_ENV->GetObjectClass(PW3270_JNI_OBJ), name, sig );
}

void pw3270_jni_post_message(int msgid, int arg1, int arg2)
{
	trace("%s: pw3270_env=%p pw3270_obj=%p msgid=%d",__FUNCTION__,PW3270_JNI_ENV,PW3270_JNI_OBJ,msgid);

	if(pw3270_jni_active)
		pw3270_jni_call_void("postMessage", "(III)V",(jint) msgid, (jint) arg1, (jint) arg2);
}

static void changed(H3270 *session, int offset, int len)
{
	trace("%s: offset=%d len=%d",__FUNCTION__,offset,len);

	{
		char *text = lib3270_get_text(PW3270_SESSION,0,-1);
		if(text)
		{
			char *strtok_r(char *str, const char *delim, char **saveptr);
			char *save;

/*
			__android_log_print(ANDROID_LOG_DEBUG, PACKAGE_NAME, "Contents:\n");
			for(char *ptr = strtok_r(text,"\n",&save);ptr;ptr = strtok_r(NULL,"\n",&save))
				__android_log_print(ANDROID_LOG_DEBUG, PACKAGE_NAME, "%s\n",ptr);
*/

			lib3270_free(text);
		}
	}

	pw3270_jni_post_message(2,offset,len);
}

static void erase(H3270 *session)
{
	pw3270_jni_post_message(4);
}

static int popuphandler(H3270 *session, void *terminal, LIB3270_NOTIFY type, const char *title, const char *msg, const char *fmt, va_list args)
{
	if(PW3270_JNI_ENV)
	{
		static const char *sig = "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V";
		char * descr = lib3270_vsprintf(fmt, args);

		if(msg)
		{
			pw3270_jni_call_void(	"postPopup",
									sig,
									(jint) type,
									pw3270_jni_new_string(title),
									pw3270_jni_new_string(msg),
									pw3270_jni_new_string(descr) );

		}
		else
		{
			pw3270_jni_call_void(	"postPopup",
									sig,
									(jint) type,
									pw3270_jni_new_string(title),
									pw3270_jni_new_string(descr),
									pw3270_jni_new_string("") );
		}

		lib3270_free(descr);
	}
	else
	{
		__android_log_print(ANDROID_LOG_VERBOSE, PACKAGE_NAME, "Can't open popup \"%s\", no jni env for active session",title);
	}
}

void update_status(H3270 *session, LIB3270_MESSAGE id)
{
	pw3270_jni_post_message(1,(int) id);
}

static int write_buffer(H3270 *session, unsigned const char *buf, int len)
{
	int rc = -1;

	__android_log_print(ANDROID_LOG_DEBUG, PACKAGE_NAME, "%s: Writing %d bytes",__FUNCTION__,len);

	if(PW3270_JNI_ENV)
	{
		jbyteArray buffer = pw3270_jni_new_byte_array(len);

		PW3270_JNI_ENV->SetByteArrayRegion(buffer, 0, len, (jbyte*) buf);

		rc = pw3270_jni_call_int("send_data", "([BI)I", buffer, (jint) len );
	}
	else
	{
		__android_log_print(ANDROID_LOG_ERROR, PACKAGE_NAME, "Can't send %d bytes, no jni env for active session",len);
	}

	trace("%s exits with rc=%d",__FUNCTION__,rc);

	return rc;
}

static void * add_timer(unsigned long interval_ms, H3270 *session, void (*proc)(H3270 *session))
{
	TIMER * timer = (TIMER *) lib3270_malloc(sizeof(TIMER));

	timer->sz		= sizeof(timer);
	timer->enabled	= true;
	timer->session  = session;
	timer->proc		= proc;

	trace("Timer %08lx created",(unsigned long) timer);

	pw3270_jni_call_void("newTimer", "(JI)V", (jlong) timer, (jint) interval_ms);

	return timer;
}

static void remove_timer(void *id)
{
	TIMER *timer = (TIMER *) id;

	if(timer == NULL)
		return;

	__android_log_print(ANDROID_LOG_VERBOSE, PACKAGE_NAME, "Disabling timer %08lx",(unsigned long) timer);

	timer->enabled = false;

}

JNIEXPORT void JNICALL Java_br_com_bb_pw3270_lib3270_timerFinish(JNIEnv *env, jobject obj, jlong id)
{
	TIMER *timer = (TIMER *) id;

	if(timer == NULL)
		return;

	PW3270_JNI_BEGIN

	if(timer->enabled)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, PACKAGE_NAME, "Running timer %08lx",(unsigned long) timer);
		timer->proc(timer->session);
	}

	lib3270_free(timer);

	PW3270_JNI_END

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

static void ctlr_done(H3270 *session)
{
	__android_log_print(ANDROID_LOG_DEBUG, PACKAGE_NAME, "%s",__FUNCTION__);
	pw3270_jni_post_message(5);
}

static void autostart(H3270 *session)
{
	if(startup_script)
	{
		// Input startup script contents
		lib3270_emulate_input(PW3270_SESSION,startup_script,-1,0);
		free(startup_script);
		startup_script = NULL;
	}
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
	H3270	* session	= lib3270_session_new("","bracket");

	__android_log_print(ANDROID_LOG_VERBOSE, PACKAGE_NAME, "Initializing %s",PACKAGE_NAME);

	pthread_mutex_init(&mutex,NULL);

#ifdef X3270_TRACE
	lib3270_set_trace_handler(tracehandler);
#endif // X3270_TRACE

	lib3270_set_popup_handler(popuphandler);
	lib3270_register_time_handlers(add_timer,remove_timer);

	session->write				= write_buffer;
	session->changed 			= changed;
	session->erase				= erase;
	session->ctlr_done			= ctlr_done;
	session->update_status		= update_status;
	session->autostart			= autostart;

	return JNI_VERSION_1_4;
}

void JNI_OnUnload(JavaVM *vm, void *reserved)
{
	__android_log_print(ANDROID_LOG_VERBOSE, PACKAGE_NAME, "Deinitializing %s",PACKAGE_NAME);
	pthread_mutex_destroy(&mutex);
}

JNIEXPORT jboolean JNICALL Java_br_com_bb_pw3270_lib3270_isConnected(JNIEnv *env, jobject obj)
{
	jboolean rc;

	PW3270_JNI_BEGIN

	rc = lib3270_connected(lib3270_get_default_session_handle()) ? JNI_TRUE : JNI_FALSE;

	PW3270_JNI_END

	return rc;
}

JNIEXPORT jboolean JNICALL Java_br_com_bb_pw3270_lib3270_isTerminalReady(JNIEnv *env, jobject obj)
{
	PW3270_JNI_BEGIN


	PW3270_JNI_END

	return JNI_FALSE;
}

JNIEXPORT void JNICALL Java_br_com_bb_pw3270_lib3270_setHost(JNIEnv *env, jobject obj, jstring hostname)
{
	PW3270_JNI_BEGIN

	lib3270_set_host(PW3270_SESSION,env->GetStringUTFChars(hostname, 0));

	PW3270_JNI_END
}

JNIEXPORT void JNICALL Java_br_com_bb_pw3270_lib3270_setStartupScript(JNIEnv *env, jobject obj, jstring str)
{
	const char *text;

	PW3270_JNI_BEGIN

	text = env->GetStringUTFChars(str, 0);

	if(startup_script)
		free(startup_script);

	if(text || strlen(text) > 0)
		startup_script = strdup(text);
	else
		startup_script = NULL;

	PW3270_JNI_END

}


JNIEXPORT jstring JNICALL Java_br_com_bb_pw3270_lib3270_getHost(JNIEnv *env, jobject obj)
{
	return env->NewStringUTF(lib3270_get_hostname(lib3270_get_default_session_handle()));
}

JNIEXPORT void JNICALL Java_br_com_bb_pw3270_lib3270_set_1connection_1status(JNIEnv *env, jobject obj, jboolean connected)
{
	PW3270_JNI_BEGIN

	trace("Host is %s",connected ? "connected" : "disconnected");

	if(connected)
	{
		lib3270_set_connected(PW3270_SESSION);
		lib3270_setup_session(PW3270_SESSION);
	}
	else
	{
		lib3270_set_disconnected(PW3270_SESSION);
	}

	PW3270_JNI_END
}

JNIEXPORT void JNICALL Java_br_com_bb_pw3270_lib3270_procRecvdata(JNIEnv *env, jobject obj, jbyteArray buffer, jint sz)
{
	unsigned char *netrbuf;

	PW3270_JNI_BEGIN

	netrbuf = (unsigned char *) env->GetByteArrayElements(buffer,NULL);
	lib3270_data_recv(PW3270_SESSION, (size_t) sz, netrbuf);
	PW3270_JNI_ENV->ReleaseByteArrayElements(buffer, (signed char *) netrbuf, 0);

	PW3270_JNI_END

}

int pw3270_jni_lock(JNIEnv *env, jobject obj)
{
	int status;

 	PW3270_JNI *datablock = (PW3270_JNI *) lib3270_malloc(sizeof(PW3270_JNI));

	datablock->parent		= pw3270_jni_active;
	datablock->env			= env;
	datablock->obj			= obj;

//	__android_log_print(ANDROID_LOG_DEBUG, PACKAGE_NAME, "%s",__FUNCTION__);

	status = pthread_mutex_trylock(&mutex);
	if(status)
	{
		__android_log_print(ANDROID_LOG_VERBOSE, PACKAGE_NAME, "Error %s when trying mutex semaphore (rc=%d)",strerror(status),status);
		status = pthread_mutex_lock(&mutex);
		if(status)
		{
			__android_log_print(ANDROID_LOG_VERBOSE, PACKAGE_NAME, "Error %s acquiring mutex semaphore (rc=%d)",strerror(status),status);
		}
	}

/*
	if(!pw3270_jni_active || pw3270_jni_active->env != env)
	{
		// Environment change, lock
		if(!pthread_mutex_trylock(&mutex))
		{
			__android_log_print(ANDROID_LOG_DEBUG, PACKAGE_NAME, "Recursive access");
		}

		trace("%s: Environment has changed",__FUNCTION__);
		pthread_mutex_lock(&mutex);
	}

	*/

	pw3270_jni_active = datablock;

	return status;

}

void pw3270_jni_unlock(void)
{
	PW3270_JNI *datablock 	= pw3270_jni_active;
	pw3270_jni_active		= datablock->parent;

	pthread_mutex_unlock(&mutex);

/*
	if(!pw3270_jni_active || pw3270_jni_active->env != datablock->env)
	{
		// Environment change, unlock
		trace("%s: Environment has changed",__FUNCTION__);
		pthread_mutex_unlock(&mutex);
	}
*/

	lib3270_free(datablock);
}

jstring	pw3270_jni_new_string(const char *str)
{
	return pw3270_jni_active->env->NewStringUTF(str ? str : "");
}
