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
 * Este programa está nomeado como actions.cpp e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 *
 */

 #include "globals.h"
 #include <lib3270/actions.h>
 #include <string.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

JNIEXPORT void JNICALL Java_br_com_bb_pw3270_lib3270_sendEnter(JNIEnv *env, jobject obj)
{
	PW3270_JNI_BEGIN

	lib3270_enter(PW3270_SESSION);

	PW3270_JNI_END
}

JNIEXPORT void JNICALL Java_br_com_bb_pw3270_lib3270_sendPFkey(JNIEnv *env, jobject obj, jint key)
{
	PW3270_JNI_BEGIN

	lib3270_pfkey(PW3270_SESSION,key);

	PW3270_JNI_END
}


