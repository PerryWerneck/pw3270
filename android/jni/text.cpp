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
 * Este programa está nomeado como text.cpp e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 *
 */

 #include "globals.h"
 #include <lib3270/html.h>
 #include <string.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

static jbyteArray retString(JNIEnv *env, const char *txt)
{
	size_t len = strlen(txt);
	jbyteArray ret = env->NewByteArray(len);
	env->SetByteArrayRegion(ret, 0, len, (jbyte*) txt);
	return ret;
}

JNIEXPORT jbyteArray JNICALL Java_br_com_bb_pw3270_lib3270_getHTML(JNIEnv *env, jobject obj)
{
	jbyteArray ret;

	session_request(env,obj);

	trace("%s starts, session=%p",__FUNCTION__,session);

	if(session)
	{
		char *text = lib3270_get_as_html(session,(LIB3270_HTML_OPTION) (LIB3270_HTML_OPTION_ALL|LIB3270_HTML_OPTION_FORM));

		if(text)
		{
			ret = retString(env,text);
			lib3270_free(text);
		}
		else
		{
			ret = retString(env, "<b>Empty session</b>");
		}
	}
	else
	{
		ret = retString(env, "<b>Invalid Session ID</b>");
	}

	trace("%s ends",__FUNCTION__);

	session_release();

	return ret;
}


JNIEXPORT jbyteArray JNICALL Java_br_com_bb_pw3270_lib3270_getText(JNIEnv *env, jobject obj)
{
	jbyteArray ret;

	session_request(env,obj);

	trace("%s starts, session=%p",__FUNCTION__,session);

	if(session)
	{
		char *text = lib3270_get_text(session,0,-1);

		trace("%s will return \"%s\"",__FUNCTION__,text ? text : "");

		if(text)
		{
			ret = retString(env,text);
			lib3270_free(text);
		}
		else
		{
			ret = retString(env, "");
		}
	}
	else
	{
		ret = retString(env, "<b>Invalid Session ID</b>");
	}

	trace("%s ends",__FUNCTION__);

	session_release();

	return ret;
}

JNIEXPORT void JNICALL Java_br_com_bb_pw3270_lib3270_setTextAt(JNIEnv *env, jobject obj, jint pos, jbyteArray inText, jint szText)
{
	char 	  str[szText+1];
	int  	  f;
	jbyte	* bt;


	session_request(env,obj);

	if(!session)
		return;

	bt = env->GetByteArrayElements(inText,0);

	for(int f=0;f<szText;f++)
		str[f] = (char) bt[f];
	str[szText] = 0;

	trace("Buffer(%d)=\"%s\"",(int) pos, str);

	env->ReleaseByteArrayElements(inText,bt,JNI_ABORT);
	session_release();
}
