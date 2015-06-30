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
 * Este programa está nomeado como dialog.cc e possui - linhas de código.
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
using namespace PW3270_NAMESPACE::java;

JNIEXPORT jint JNICALL Java_pw3270_terminal_set_1copy(JNIEnv *env, jobject obj, jstring j_str) {

	const char	* str = env->GetStringUTFChars(j_str, 0);
	jint 		  rc	= -1;

	try {

		rc = getHandle(env,obj)->set_copy(str);

	} catch(std::exception &e) {

		env->ReleaseStringUTFChars( j_str, str);
		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());
		return -1;

	}

	env->ReleaseStringUTFChars( j_str, str);
	return rc;

}

JNIEXPORT jstring JNICALL Java_pw3270_terminal_get_1copy(JNIEnv *env, jobject obj) {

	string str;

	try {

		str = getHandle(env,obj)->get_copy();


	} catch(std::exception &e) {

		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());

	}

	return env->NewStringUTF(str.c_str());


}

JNIEXPORT jstring JNICALL Java_pw3270_terminal_get_1clipboard(JNIEnv *env, jobject obj) {

	string str;

	try {

		str = getHandle(env,obj)->get_clipboard();


	} catch(std::exception &e) {

		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());

	}

	return env->NewStringUTF(str.c_str());

}

JNIEXPORT jint JNICALL Java_pw3270_terminal_set_1clipboard(JNIEnv *env, jobject obj, jstring j_str) {

	const char	* str = env->GetStringUTFChars(j_str, 0);
	jint 		  rc	= -1;

	try {

		rc = getHandle(env,obj)->set_clipboard(str);

	} catch(std::exception &e) {

		env->ReleaseStringUTFChars( j_str, str);
		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());
		return -1;

	}

	env->ReleaseStringUTFChars( j_str, str);
	return rc;


}
