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

static jbyteArray retString(const char *txt)
{
	size_t len = strlen(txt);
	jbyteArray ret = PW3270_JNI_ENV->NewByteArray(len);
	PW3270_JNI_ENV->SetByteArrayRegion(ret, 0, len, (jbyte*) txt);
	return ret;
}

JNIEXPORT jbyteArray JNICALL Java_br_com_bb_pw3270_lib3270_getHTML(JNIEnv *env, jobject obj)
{
	jbyteArray ret;

	PW3270_JNI_BEGIN

	trace("%s starts, session=%p",__FUNCTION__,PW3270_SESSION);

	char *text = lib3270_get_as_html(PW3270_SESSION,(LIB3270_HTML_OPTION) (LIB3270_HTML_OPTION_ALL|LIB3270_HTML_OPTION_FORM));

	if(text)
	{
		ret = retString(text);
		lib3270_free(text);
	}
	else
	{
		ret = retString("<b>Empty session</b>");
	}

	trace("%s ends",__FUNCTION__);

	PW3270_JNI_END

	return ret;
}


/*
JNIEXPORT jbyteArray JNICALL Java_br_com_bb_pw3270_lib3270_getText(JNIEnv *env, jobject obj)
{
	jbyteArray ret;

	PW3270_JNI_BEGIN

	trace("%s starts",__FUNCTION__);

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

	PW3270_JNI_END

	return ret;
}
*/

JNIEXPORT void JNICALL Java_br_com_bb_pw3270_lib3270_setTextAt(JNIEnv *env, jobject obj, jint pos, jbyteArray inText, jint szText)
{
	PW3270_JNI_BEGIN

	if(lib3270_connected(PW3270_SESSION))
	{
		unsigned char 	  str[szText+1];
		int 		 	  f;
		jbyte			* bt	= env->GetByteArrayElements(inText,0);

		for(int f=0;f<szText;f++)
			str[f] = (char) bt[f];
		str[szText] = 0;

//		trace("Buffer(%d/%d)=\"%s\"",(int) pos, lib3270_field_addr(PW3270_SESSION, (int) pos), str);

		if( ((int) pos) == lib3270_field_addr(PW3270_SESSION, (int) pos))
		{
			// Begin of field, clear it first
			int 			  sz = lib3270_field_length(PW3270_SESSION,pos);
			unsigned char	* buffer = (unsigned char *) lib3270_malloc(sz+1);

			memset(buffer,' ',sz);

			lib3270_clear_operator_error(PW3270_SESSION);
			lib3270_set_cursor_address(PW3270_SESSION,(int) pos);
			lib3270_set_string(PW3270_SESSION,buffer);

			lib3270_free(buffer);
		}

		lib3270_clear_operator_error(PW3270_SESSION);
		lib3270_set_cursor_address(PW3270_SESSION,(int) pos);
		lib3270_set_string(PW3270_SESSION,str);

		lib3270_clear_operator_error(PW3270_SESSION);

		env->ReleaseByteArrayElements(inText,bt,JNI_ABORT);
	}

	PW3270_JNI_END
}
