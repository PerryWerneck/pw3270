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
 * Este programa está nomeado como globals.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 *
 */

 #include "lib3270jni.h"

 #include <lib3270.h>
 #include <lib3270/session.h>
 #include <lib3270/log.h>

/*--[ Defines ]--------------------------------------------------------------------------------------*/

 typedef struct _info
 {
 	JNIEnv	 * env;
 	jobject	   obj;

 } INFO;

 #define session_request(env, obj)	INFO	  jni_data	= { env, obj }; \
									H3270	* session 	= lib3270_get_default_session_handle(); \
									session->widget		= &jni_data;

 #define session_release()			session->widget		= 0;

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 extern const char *java_class_name;



