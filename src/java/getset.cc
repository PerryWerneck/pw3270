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
 * Este programa está nomeado como info.cc e possui - linhas de código.
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

using namespace std;
using namespace PW3270_NAMESPACE;

JNIEXPORT jstring JNICALL Java_pw3270_terminal_toString(JNIEnv *env, jobject obj) {

	string str;

	try {

		str = java::getHandle(env,obj)->get_string();

	} catch(std::exception &e) {

		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());

	}

	return env->NewStringUTF(str.c_str());

}

JNIEXPORT jstring JNICALL Java_pw3270_terminal_get_1string(JNIEnv *env, jobject obj, jint baddr, jint len) {

	string str;

	try {

		str = java::getHandle(env,obj)->get_string((int) baddr, (int) len);


	} catch(std::exception &e) {

		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());

	}

	return env->NewStringUTF(str.c_str());

}

JNIEXPORT jstring JNICALL Java_pw3270_terminal_get_1string_1at(JNIEnv *env, jobject obj, jint row, jint col, jint sz) {

	string str;

	try {

		str = java::getHandle(env,obj)->get_string_at((int) row, (int) col, (int) sz);


	} catch(std::exception &e) {

		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());

	}

	return env->NewStringUTF(str.c_str());

}

JNIEXPORT jint JNICALL Java_pw3270_terminal_set_1string_1at(JNIEnv *env, jobject obj, jint row, jint col, jstring j_str) {

	const char	* str = env->GetStringUTFChars(j_str, 0);
	jint 		  rc	= -1;

	try {

		rc = java::getHandle(env,obj)->set_string_at((int) row, (int) col, str);

	} catch(std::exception &e) {

		env->ReleaseStringUTFChars( j_str, str);
		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());
		return -1;

	}

	env->ReleaseStringUTFChars( j_str, str);
	return rc;


}

JNIEXPORT jint JNICALL Java_pw3270_terminal_cmp_1string_1at(JNIEnv *env, jobject obj, jint row, jint col, jstring j_str) {

	const char	* str = env->GetStringUTFChars(j_str, 0);
	jint 		  rc	= -1;

	try {

		rc = java::getHandle(env,obj)->cmp_string_at((int) row, (int) col, str);

	} catch(std::exception &e) {

		env->ReleaseStringUTFChars( j_str, str);
		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());
		return -1;

	}

	trace("cmp_string_at(%d,%d,\"%s\") = %d",(int) row, (int) col, str, (int) rc);

	env->ReleaseStringUTFChars( j_str, str);
	return rc;

}

JNIEXPORT jint JNICALL Java_pw3270_terminal_wait_1for_1string_1at(JNIEnv *env, jobject obj, jint row, jint col, jstring j_str, jint timeout) {

	const char	* str = env->GetStringUTFChars(j_str, 0);
	jint 		  rc	= -1;

	try {

		rc = java::getHandle(env,obj)->wait_for_string_at((int) row, (int) col, str, timeout);

	} catch(std::exception &e) {

		env->ReleaseStringUTFChars( j_str, str);
		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());
		return -1;

	}

	env->ReleaseStringUTFChars( j_str, str);
	return rc;


}

JNIEXPORT jint JNICALL Java_pw3270_terminal_input_1string(JNIEnv *env, jobject obj, jstring j_str) {

	const char	* str = env->GetStringUTFChars(j_str, 0);
	jint 		  rc	= -1;

	try {

		rc = java::getHandle(env,obj)->input_string(str);

	} catch(std::exception &e) {

		env->ReleaseStringUTFChars( j_str, str);
		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());
		return -1;

	}

	env->ReleaseStringUTFChars( j_str, str);
	return rc;

}

JNIEXPORT jboolean JNICALL Java_pw3270_terminal_is_1connected(JNIEnv *env, jobject obj) {

	jboolean rc = false;

	try {

		rc = java::getHandle(env,obj)->is_connected();

	} catch(std::exception &e) {

		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());
		return rc;

	}


	return rc;

}

JNIEXPORT jboolean JNICALL Java_pw3270_terminal_is_1ready(JNIEnv *env, jobject obj) {

	jboolean rc = false;

	try {

		rc = java::getHandle(env,obj)->is_ready();

	} catch(std::exception &e) {

		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());
		return rc;

	}

	return rc;

}

JNIEXPORT void JNICALL Java_pw3270_terminal_set_1unlock_1delay(JNIEnv *env, jobject obj, jint ms) {

	try {

		java::getHandle(env,obj)->set_unlock_delay((unsigned short) ms);

	} catch(std::exception &e) {

		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());

	}

}
