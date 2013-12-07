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
 #include <exception>
 #include <pw3270/class.h>

 using namespace std;
 using namespace PW3270_NAMESPACE;

/*--[ Implement ]------------------------------------------------------------------------------------*/

RexxRoutine0(CSTRING, rx3270version)
{
	try
	{
		return session::get_default()->get_version().c_str();
	}
	catch(std::exception& e)
	{
		context->RaiseException1(Rexx_Error_Application_error,context->NewStringFromAsciiz(e.what()));
	}

	return NULL;
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
 	LIB3270_CSTATE	state = session::get_default()->get_cstate();

	for(f=0;f < (sizeof(xlat_state)/sizeof(struct _xlat_state)); f++)
	{
		if(state == xlat_state[f].state)
			return xlat_state[f].ret;
	}

	return "UNEXPECTED";
}

RexxRoutine0(int, rx3270Disconnect)
{
	return session::get_default()->disconnect();
}

RexxRoutine2(int, rx3270Connect, CSTRING, hostname, int, wait)
{
	return session::get_default()->connect(hostname,wait);
}

RexxRoutine0(int, rx3270isConnected)
{
	return session::get_default()->is_connected();
}

RexxRoutine0(int, rx3270WaitForEvents)
{
	return session::get_default()->iterate();
}

RexxRoutine1(int, rx3270Sleep, int, seconds)
{
	return session::get_default()->wait(seconds);
}

RexxRoutine0(int, rx3270SendENTERKey)
{
	return session::get_default()->enter();
}

RexxRoutine1(int, rx3270SendPFKey, int, key)
{
	return session::get_default()->pfkey(key);
}

RexxRoutine1(int, rx3270SendPAKey, int, key)
{
	return session::get_default()->pakey(key);
}

RexxRoutine1(int, rx3270WaitForTerminalReady, int, seconds)
{
	return session::get_default()->wait_for_ready(seconds);
}

RexxRoutine4(int, rx3270WaitForStringAt, int, row, int, col, CSTRING, key, int, timeout)
{
	try
	{
		return session::get_default()->wait_for_string_at(row,col,key,timeout);
	}
	catch(std::exception &e)
	{
		context->RaiseException1(Rexx_Error_Application_error,context->NewStringFromAsciiz(e.what()));
	}

	return ETIMEDOUT;

}

RexxRoutine3(RexxStringObject, rx3270GetStringAt, int, row, int, col, int, sz)
{
	try
	{
        string *str = session::get_default()->get_string_at(row,col,(int) sz);

        if(str)
		{
			RexxStringObject ret = context->String((CSTRING) str->c_str());
			delete str;
			return ret;
		}
	}
	catch(std::exception &e)
	{
		context->RaiseException1(Rexx_Error_Application_error,context->NewStringFromAsciiz(e.what()));
	}

	return context->String("");
}

RexxRoutine0(int, rx3270IsTerminalReady)
{
	return session::get_default()->is_ready();
}

RexxRoutine3(int, rx3270queryStringAt, int, row, int, col, CSTRING, key)
{
	try
	{
		return session::get_default()->cmp_string_at(row,col,key);
	}
	catch(std::exception &e)
	{
		context->RaiseException1(Rexx_Error_Application_error,context->NewStringFromAsciiz(e.what()));
	}

	return -1;
}

RexxRoutine2(int, rx3270SetCursorPosition, int, row, int, col)
{
	try
	{
		return session::get_default()->set_cursor_position(row,col);
	}
	catch(std::exception &e)
	{
		context->RaiseException1(Rexx_Error_Application_error,context->NewStringFromAsciiz(e.what()));
	}

	return -1;
}

RexxRoutine3(int, rx3270SetStringAt, int, row, int, col, CSTRING, text)
{
	try
	{
		return session::get_default()->set_string_at(row,col,text);
	}
	catch(std::exception &e)
	{
		context->RaiseException1(Rexx_Error_Application_error,context->NewStringFromAsciiz(e.what()));
	}
	return -1;
}

RexxRoutine0(int, rx3270CloseApplication)
{
	return session::get_default()->quit();
}


RexxRoutine2(RexxStringObject, asc2ebc, CSTRING, str, OPTIONAL_int, sz)
{
	try
	{
		if(sz < 1)
			sz = strlen(str);

		if(sz)
		{
			char buffer[sz+1];
			memcpy(buffer,str,sz);
			buffer[sz] = 0;
			return context->String((CSTRING) session::get_default()->asc2ebc((unsigned char *)buffer,sz));
		}
	}
	catch(std::exception &e)
	{
		context->RaiseException1(Rexx_Error_Application_error,context->NewStringFromAsciiz(e.what()));
	}

	return context->String("");
}

RexxRoutine2(RexxStringObject, ebc2asc, CSTRING, str, OPTIONAL_int, sz)
{
	try
	{
		if(sz < 1)
			sz = strlen(str);

		if(sz)
		{
			char buffer[sz+1];
			memcpy(buffer,str,sz);
			buffer[sz] = 0;
			return context->String((CSTRING) session::get_default()->ebc2asc((unsigned char *)buffer,sz));
		}
	}
	catch(std::exception &e)
	{
		context->RaiseException1(Rexx_Error_Application_error,context->NewStringFromAsciiz(e.what()));
	}

	return context->String("");
}

