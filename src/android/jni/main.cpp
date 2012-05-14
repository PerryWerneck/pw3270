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
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como main.cpp e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 *
 */

 #include "globals.h"
 #include <lib3270/session.h>

/*--[ Defines ]--------------------------------------------------------------------------------------*/

 typedef struct _info
 {
 	JNIEnv	 * env;
 	jobject	   obj;

 } INFO;

/*--[ Implement ]------------------------------------------------------------------------------------*/

JNIEXPORT jint JNICALL Java_br_com_bb_pw3270_lib3270_init(JNIEnv *env, jclass obj)
{
	H3270	* session	= lib3270_session_new("");

	return 0;
}

JNIEXPORT jint JNICALL Java_br_com_bb_pw3270_lib3270_processEvents(JNIEnv *env, jobject obj)
{
	/*
	INFO	  data		= { env, obj };
	H3270	* session 	= lib3270_get_default_session_handle();

	session->widget = &data;
	lib3270_main_iterate(session,1);
	session->widget = 0;
	*/
	return 0;
}

JNIEXPORT jboolean JNICALL Java_br_com_bb_pw3270_lib3270_isConnected(JNIEnv *env, jobject obj)
{
//	return (lib3270_connected(lib3270_get_default_session_handle())) ? JNI_TRUE : JNI_FALSE;;
	return JNI_FALSE;
}

JNIEXPORT jboolean JNICALL Java_br_com_bb_pw3270_lib3270_isTerminalReady(JNIEnv *env, jobject obj)
{
	return JNI_FALSE;
}

JNIEXPORT void JNICALL Java_br_com_bb_pw3270_lib3270_setHost(JNIEnv *env, jobject obj, jstring hostname)
{

}

JNIEXPORT jstring JNICALL Java_br_com_bb_pw3270_lib3270_getHost(JNIEnv *env, jobject obj)
{
	return env->NewStringUTF("");
}

