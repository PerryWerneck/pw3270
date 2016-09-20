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

using namespace PW3270_NAMESPACE;

JNIEXPORT jint JNICALL Java_pw3270_terminal_popup_1dialog(JNIEnv *env, jobject obj, jint id, jstring j_title, jstring j_message, jstring j_secondary) {

	const char	* title		= env->GetStringUTFChars(j_title, 0);
	const char	* message	= env->GetStringUTFChars(j_message, 0);
	const char	* secondary	= env->GetStringUTFChars(j_secondary, 0);
	jint		  rc		= -1;

	try {

		rc = (jint) java::getHandle(env,obj)->popup_dialog((LIB3270_NOTIFY) id, title, message, secondary);

	} catch(std::exception &e) {

		env->ReleaseStringUTFChars( j_title, title);
		env->ReleaseStringUTFChars( j_message, message);
		env->ReleaseStringUTFChars( j_secondary, secondary);
		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());

	}

	env->ReleaseStringUTFChars( j_title, title);
	env->ReleaseStringUTFChars( j_message, message);
	env->ReleaseStringUTFChars( j_secondary, secondary);

	return rc;

}

JNIEXPORT jstring JNICALL Java_pw3270_terminal_file_1chooser_1dialog(JNIEnv *env, jobject obj, jint action, jstring j_title, jstring j_extension, jstring j_filename) {

	string		  str;
	const char	* title		= env->GetStringUTFChars(j_title, 0);
	const char	* extension	= env->GetStringUTFChars(j_extension, 0);
	const char	* filename	= env->GetStringUTFChars(j_filename, 0);

	try {

		str = java::getHandle(env,obj)->file_chooser_dialog((int) action, title, extension, filename);

	} catch(std::exception &e) {

		env->ReleaseStringUTFChars( j_title, title);
		env->ReleaseStringUTFChars( j_extension, extension);
		env->ReleaseStringUTFChars( j_filename, filename);
		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());

	}

	env->ReleaseStringUTFChars( j_title, title);
	env->ReleaseStringUTFChars( j_extension, extension);
	env->ReleaseStringUTFChars( j_filename, filename);

	return env->NewStringUTF(str.c_str());


}
