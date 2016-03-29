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

using namespace PW3270_NAMESPACE;

JNIEXPORT jstring JNICALL Java_pw3270_terminal_get_1version (JNIEnv *env, jobject obj) {

	session *s = java::getHandle(env,obj);
	return env->NewStringUTF(s->get_version().c_str());

}

JNIEXPORT jstring JNICALL Java_pw3270_terminal_get_1revision(JNIEnv *env, jobject obj) {

	session *s = java::getHandle(env,obj);
	return env->NewStringUTF(s->get_revision().c_str());

}

JNIEXPORT jstring JNICALL Java_pw3270_terminal_get_1encoding(JNIEnv *env, jobject obj) {

	session *s = java::getHandle(env,obj);
	return env->NewStringUTF(s->get_encoding());

}

JNIEXPORT jint JNICALL Java_pw3270_terminal_get_1secure(JNIEnv *env, jobject obj) {

	jint rc	= -1;

	try {

		rc = java::getHandle(env,obj)->get_secure();

	} catch(std::exception &e) {

		env->ThrowNew(env->FindClass("java/lang/Exception"), e.what());

	}

	return rc;

}
