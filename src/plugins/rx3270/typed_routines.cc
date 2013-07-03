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
 * Este programa está nomeado como typed_routines.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "rx3270.h"
 #include <time.h>
 #include <string.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

RexxRoutine0(CSTRING, rx3270version)
{
	return rx3270::get_default()->get_version();
}

RexxRoutine0(CSTRING, rx3270QueryCState)
{
 	#define DECLARE_XLAT_STATE( x ) { x, #x }

 	static const struct _xlat_state
 	{
 		LIB3270_CSTATE	  state;
 		const char		* ret;
 	} xlat_state[] =
 	{
			{ LIB3270_NOT_CONNECTED,		"NOT_CONNECTED" 		},
			{ LIB3270_RESOLVING, 			"RESOLVING" 			},
			{ LIB3270_PENDING, 				"PENDING" 				},
			{ LIB3270_CONNECTED_INITIAL,	"CONNECTED_INITIAL" 	},
			{ LIB3270_CONNECTED_ANSI,		"CONNECTED_ANSI" 		},
			{ LIB3270_CONNECTED_3270,		"CONNECTED_3270" 		},
			{ LIB3270_CONNECTED_INITIAL_E,	"CONNECTED_INITIAL_E" 	},
			{ LIB3270_CONNECTED_NVT,		"CONNECTED_NVT" 		},
			{ LIB3270_CONNECTED_SSCP,		"CONNECTED_SSCP" 		},
			{ LIB3270_CONNECTED_TN3270E,	"CONNECTED_TN3270E" 	},
 	};

 	size_t			f;
 	LIB3270_CSTATE	state = rx3270::get_default()->get_cstate();

	for(f=0;f < (sizeof(xlat_state)/sizeof(struct _xlat_state)); f++)
	{
		if(state == xlat_state[f].state)
			return xlat_state[f].ret;
	}

	return "UNEXPECTED";
}

RexxRoutine0(int, rx3270Disconnect)
{
	return rx3270::get_default()->disconnect();
}

RexxRoutine2(int, rx3270Connect, CSTRING, hostname, int, wait)
{
	return rx3270::get_default()->connect(hostname,wait);
}

RexxRoutine0(int, rx3270isConnected)
{
	return rx3270::get_default()->is_connected();
}

RexxRoutine0(int, rx3270WaitForEvents)
{
	return rx3270::get_default()->iterate();
}

RexxRoutine1(int, rx3270Sleep, int, seconds)
{
	return rx3270::get_default()->wait(seconds);
}

RexxRoutine0(int, rx3270SendENTERKey)
{
	return rx3270::get_default()->enter();
}

RexxRoutine1(int, rx3270SendPFKey, int, key)
{
	return rx3270::get_default()->pfkey(key);
}

RexxRoutine1(int, rx3270SendPAKey, int, key)
{
	return rx3270::get_default()->pakey(key);
}

RexxRoutine1(int, rx3270WaitForTerminalReady, int, seconds)
{
	return rx3270::get_default()->wait_for_ready(seconds);
}

RexxRoutine4(int, rx3270WaitForStringAt, int, row, int, col, CSTRING, key, int, timeout)
{
	rx3270	* session 	= rx3270::get_default();
	time_t	  end		= time(0) + timeout;
	char	* text		= session->get_3270_string(key);

	while(time(0) < end)
	{
		if(!session->is_connected())
		{
			return ENOTCONN;
		}
		else if( !(session->wait_for_ready(1) || session->cmp_text_at(row,col,text)) )
		{
			free(text);
			return 0;
		}
	}

	free(text);

	return ETIMEDOUT;

}

RexxRoutine3(RexxStringObject, rx3270GetStringAt, int, row, int, col, int, sz)
{
	rx3270	* session 	= rx3270::get_default();
	char	* str		= session->get_text_at(row,col,sz);

	if(str)
	{
		char				* text	= session->get_local_string(str);
		RexxStringObject	  ret	= context->String((CSTRING) text);
		free(str);
		free(text);
		return ret;
	}

	return context->String("");
}

RexxRoutine0(int, rx3270IsTerminalReady)
{
	return rx3270::get_default()->is_ready();
}

RexxRoutine3(int, rx3270queryStringAt, int, row, int, col, CSTRING, key)
{
	int		  rc		= 0;
	rx3270	* session 	= rx3270::get_default();
	char	* str		= session->get_text_at(row,col,strlen(key));

	if(str)
	{
		char * text	= session->get_3270_string(key);
		rc = strcasecmp(str,text);
		free(text);
	}

	free(str);

	return rc;
}

RexxRoutine2(int, rx3270SetCursorPosition, int, row, int, col)
{
	return rx3270::get_default()->set_cursor_position(row,col);
}

RexxRoutine3(int, rx3270SetStringAt, int, row, int, col, CSTRING, text)
{
	rx3270	* session 	= rx3270::get_default();
	char	* str		= session->get_3270_string(text);
	int		  rc;

	rc = session->set_text_at(row,col,str);

	free(str);

	return rc;
}

RexxRoutine0(int, rx3270CloseApplication)
{
	rx3270	* session 	= rx3270::get_default();
	return session->quit();
}
