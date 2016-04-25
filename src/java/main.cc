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
 * Este programa está nomeado como main.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "jni3270.h"
 #include "private.h"

/*---[ Implement ]----------------------------------------------------------------------------------*/

namespace PW3270_NAMESPACE {

	jfieldID java::getHandleField(JNIEnv *env, jobject obj) {
		jclass c = env->GetObjectClass(obj);
		// J is the type signature for long:
		return env->GetFieldID(c, "nativeHandle", "J");
	}

	session * java::getHandle(JNIEnv *env, jobject obj) {
		jlong handle = env->GetLongField(obj, getHandleField(env, obj));
		return reinterpret_cast<PW3270_NAMESPACE::session *>(handle);
	}

}

using namespace PW3270_NAMESPACE;
using namespace PW3270_NAMESPACE::java;

JNIEXPORT jint JNICALL Java_pw3270_terminal_init__(JNIEnv *env, jobject obj) {

	trace("%s",__FUNCTION__);

	try {

		jlong handle = reinterpret_cast<jlong>(session::create());
		env->SetLongField(obj, getHandleField(env, obj), handle);

	} catch(std::exception &e) {

		trace("%s: %s",__FUNCTION__,e.what());
		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());

	}

	return 0;
}


JNIEXPORT jint JNICALL Java_pw3270_terminal_init__Ljava_lang_String_2(JNIEnv *env, jobject obj, jstring j_id) {

	const char * id = env->GetStringUTFChars(j_id, 0);

	try {

		jlong handle = reinterpret_cast<jlong>(session::create(id));
		env->SetLongField(obj, getHandleField(env, obj), handle);
		env->ReleaseStringUTFChars( j_id, id);

	} catch(std::exception &e) {

		env->ReleaseStringUTFChars( j_id, id);
		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());

	}


	return 0;
}

JNIEXPORT jint JNICALL Java_pw3270_terminal_deinit(JNIEnv *env, jobject obj) {

	try {

		session *s = getHandle(env,obj);

		if(s) {
			delete s;
		}

		env->SetLongField(obj, getHandleField(env, obj), 0);


	} catch(std::exception &e) {

		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());

	}

	return 0;
}

JNIEXPORT jint JNICALL Java_pw3270_terminal_wait_1for_1ready(JNIEnv *env, jobject obj, jint seconds) {

	try {

		return getHandle(env,obj)->wait_for_ready((int) seconds);

	} catch(std::exception &e) {

		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());

	}

	return 0;

}

JNIEXPORT jint JNICALL Java_pw3270_terminal_wait(JNIEnv *env, jobject obj, jint seconds) {

	try {

		return getHandle(env,obj)->wait((int) seconds);

	} catch(std::exception &e) {

		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());

	}

	return 0;

}


JNIEXPORT void JNICALL Java_pw3270_terminal_log(JNIEnv *env, jobject obj, jstring j_str) {

	const char	* str = env->GetStringUTFChars(j_str, 0);

	try {

		getHandle(env,obj)->log("%s", str);

	} catch(std::exception &e) {

		env->ReleaseStringUTFChars( j_str, str);
		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());
		return;

	}

	env->ReleaseStringUTFChars( j_str, str);

}
