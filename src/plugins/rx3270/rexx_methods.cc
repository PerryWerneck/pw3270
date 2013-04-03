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
 * Este programa está nomeado como rexx_methods.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <time.h>
 #include <string.h>

 #include "rx3270.h"


/*--[ Implement ]------------------------------------------------------------------------------------*/

RexxMethod1(int, rx3270_method_init, CSTRING, type)
{
	// Set session class in rexx object
    RexxPointerObject sessionPtr = context->NewPointer(rx3270::create(type));
    context->SetObjectVariable("CSELF", sessionPtr);
    return 0;
}

RexxMethod1(int, rx3270_method_uninit, CSELF, sessionPtr)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(hSession)
		delete hSession;
	return 0;
}

RexxMethod3(int, rx3270_method_connect, CSELF, sessionPtr, CSTRING, uri, OPTIONAL_int, wait)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->connect(uri,wait != 0);
}

RexxMethod1(int, rx3270_method_disconnect, CSELF, sessionPtr)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->disconnect();
}

RexxMethod2(int, rx3270_method_sleep, CSELF, sessionPtr, int, seconds)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->wait(seconds);
}

RexxMethod1(int, rx3270_method_is_connected, CSELF, sessionPtr)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->is_connected();
}

RexxMethod1(int, rx3270_method_is_ready, CSELF, sessionPtr)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->is_ready();
}

RexxMethod2(int, rx3270_method_wait_for_ready, CSELF, sessionPtr, int, seconds)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->wait_for_ready(seconds);
}

RexxMethod3(int, rx3270_method_set_cursor, CSELF, sessionPtr, int, row, int, col)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->set_cursor_position(row,col);
}

RexxMethod1(int, rx3270_method_enter, CSELF, sessionPtr)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->enter();
}

RexxMethod2(int, rx3270_method_pfkey, CSELF, sessionPtr, int, key)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->pfkey(key);
}

RexxMethod2(int, rx3270_method_pakey, CSELF, sessionPtr, int, key)
{
	rx3270 *hSession = (rx3270 *) sessionPtr;
	if(!hSession)
		return -1;
	return hSession->pakey(key);
}

RexxMethod4(RexxStringObject, rx3270_method_get_text_at, CSELF, sessionPtr, int, row, int, col, int, sz)
{
	rx3270	* session 	= (rx3270 *) sessionPtr;

	if(session)
	{
		char * str = session->get_text_at(row,col,sz);

		if(str)
		{
			char				* text	= session->get_local_string(str);
			RexxStringObject	  ret	= context->String((CSTRING) text);
			free(str);
			free(text);
			return ret;
		}
	}

	return context->String("");
}


RexxMethod4(int, rx3270_method_set_text_at, CSELF, sessionPtr, int, row, int, col, CSTRING, text)
{
	rx3270 * session = (rx3270 *) sessionPtr;

	if(session)
	{
		char	* str		= session->get_3270_string(text);
		int		  rc;
		rc = session->set_text_at(row,col,str);
		free(str);
		return rc;
	}
	return -1;
}

RexxMethod4(int, rx3270_method_cmp_text_at, CSELF, sessionPtr, int, row, int, col, CSTRING, key)
{
	int		  rc		= 0;
	rx3270	* session	= (rx3270 *) sessionPtr;

	if(session)
	{
		char * str = session->get_text_at(row,col,strlen(key));
		if(str)
		{
			char * text	= session->get_3270_string(key);
			rc = strcasecmp(str,text);
			free(text);
		}
		free(str);
	}

	return rc;
}

