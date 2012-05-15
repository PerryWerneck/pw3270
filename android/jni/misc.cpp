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
 * Este programa está nomeado como misc.cpp e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 *
 */

 #include "globals.h"

/*--[ Implement ]------------------------------------------------------------------------------------*/

JNIEXPORT jstring JNICALL Java_br_com_bb_pw3270_lib3270_getVersion(JNIEnv *env, jobject obj)
{
	return env->NewStringUTF(lib3270_get_version());
}

JNIEXPORT jstring JNICALL Java_br_com_bb_pw3270_lib3270_getRevision(JNIEnv *env, jobject obj)
{
	return env->NewStringUTF(lib3270_get_revision());
}

JNIEXPORT jstring JNICALL Java_br_com_bb_pw3270_lib3270_getEncoding(JNIEnv *env, jobject obj)
{
	return env->NewStringUTF(lib3270_get_charset(lib3270_get_default_session_handle()));
}
