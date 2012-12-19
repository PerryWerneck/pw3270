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
 * Este programa está nomeado como text.cc e possui - linhas de código.
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
 #include <lib3270/actions.h>


/*--[ Implement ]------------------------------------------------------------------------------------*/

RexxRoutine0(CSTRING, rx3270version)
{
	return lib3270_get_version();
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
 	LIB3270_CSTATE	state;

	state = lib3270_get_connection_state(RX3270SESSION);

	for(f=0;f < (sizeof(xlat_state)/sizeof(struct _xlat_state)); f++)
	{
		if(state == xlat_state[f].state)
			return xlat_state[f].ret;
	}

	return "UNEXPECTED";
}

RexxRoutine0(int, rx3270Disconnect)
{
	lib3270_disconnect(RX3270SESSION);
	return 0;
}

RexxRoutine2(int, rx3270Connect, CSTRING, hostname, int, wait)
{
	return lib3270_connect(RX3270SESSION, hostname, wait);
}

RexxRoutine0(int, rx3270isConnected)
{
	return lib3270_is_connected(RX3270SESSION) ? 1 : 0;
}

RexxRoutine0(int, rx3270WaitForEvents)
{
	H3270 * hSession = RX3270SESSION;

	if(!lib3270_is_connected(hSession))
		return ENOTCONN;

	lib3270_main_iterate(hSession,1);

	return 0;
}

RexxRoutine1(int, rx3270Sleep, int, seconds)
{
	return lib3270_wait(RX3270SESSION,seconds);
}

RexxRoutine0(int, rx3270SendENTERKey)
{
	return lib3270_enter(RX3270SESSION);
}

RexxRoutine1(int, rx3270SendPFKey, int, key)
{
	return lib3270_pfkey(RX3270SESSION,key);
}

RexxRoutine1(int, rx3270SendPAKey, int, key)
{
	return lib3270_pakey(RX3270SESSION,key);
}

RexxRoutine1(int, rx3270WaitForTerminalReady, int, seconds)
{
	return lib3270_wait_for_ready(RX3270SESSION,seconds);
}

RexxRoutine4(int, rx3270WaitForStringAt, int, row, int, col, CSTRING, key, int, timeout)
{
	H3270	* hSession	= RX3270SESSION;
	time_t	  end		= time(0) + timeout;
	int		  sz		= strlen(key);
	int		  rows;
	int		  cols;
	int		  start;

	lib3270_get_screen_size(hSession,&rows,&cols);

	if(row < 0 || row > rows || col < 0 || col > cols)
		return EINVAL;

	start = ((row) * cols) + col;

	while(time(0) < end)
	{
		if(!lib3270_is_connected(hSession))
		{
			return ENOTCONN;
		}
		else if(lib3270_is_ready(hSession))
		{
			char *buffer = get_contents(hSession, start, start+sz);
			if(buffer)
			{
				int rc = strncasecmp((const char *) buffer,key,sz);
				free(buffer);
				if(rc == 0)
					return 0;
			}
		}

		lib3270_main_iterate(hSession,1);

	}

	return ETIMEDOUT;

}

RexxRoutine3(RexxStringObject, rx3270GetStringAt, int, row, int, col, int, sz)
{
	H3270	* hSession	= RX3270SESSION;
	int		  rows;
	int		  cols;
	char	* text;

	lib3270_get_screen_size(hSession,&rows,&cols);

	text = get_contents(hSession, ((row) * cols) + col, sz);
	if(text)
	{
		RexxStringObject ret = context->String((CSTRING) text);
		free(text);
		return ret;
	}

	return context->String("");
}

RexxRoutine0(int, rx3270IsTerminalReady)
{
	return lib3270_is_ready(RX3270SESSION);
}

RexxRoutine3(RexxStringObject, rx3270ReadScreen, OPTIONAL_int, row, OPTIONAL_int, col, OPTIONAL_int, sz)
{
	H3270	* hSession	= RX3270SESSION;
	int		  rows;
	int		  cols;
	char	* text;
	int		  start;

	lib3270_get_screen_size(hSession,&rows,&cols);

	start = (row * cols) + col;

	if(sz == 0)
		sz = (rows*cols) - start;

	text = get_contents(hSession, start, sz);
	if(text)
	{
		RexxStringObject ret = context->String((CSTRING) text);
		free(text);
		return ret;
	}

	return context->String("");
}

RexxRoutine3(int, rx3270queryStringAt, int, row, int, col, CSTRING, key)
{
	H3270	* hSession	= RX3270SESSION;
	int		  rows;
	int		  cols;
	char	* text;
	size_t	  sz		= strlen(key);

	lib3270_get_screen_size(hSession,&rows,&cols);

	text = get_contents(hSession, (row * cols) + col, sz);
	if(text)
	{
		int rc = strncasecmp(text,key,sz);
		free(text);
		return rc;
	}

	return 0;
}

RexxRoutine2(int, rx3270SetCursorPosition, int, row, int, col)
{
	return lib3270_set_cursor_position(RX3270SESSION,row,col);
}

RexxRoutine3(int, rx3270SetStringAt, int, row, int, col, CSTRING, text)
{
	int		  rc;
	H3270	* hSession	= RX3270SESSION;
	char	* str		= set_contents(hSession,text);

	if(str)
		rc = lib3270_set_string_at(hSession,row,col,(const unsigned char *) str);
	else
		rc = -1;

	free(str);

	return rc;
}

/*
::method RunMode
return rx3270QueryRunMode()

::method 'encoding='
	use arg ptr
return rx3270SetCharset(ptr)

::method sendfile
	use arg from, tostrncasecmp

	status = rx3270BeginFileSend(from,to)
	if status <> 0
		then return status

return rx3270WaitForFTComplete()

::method recvfile
	use arg from, to

	status = rx3270BeginFileRecv(from,to)
	if status <> 0
		then return status

return rx3270WaitForFTComplete()
*/


