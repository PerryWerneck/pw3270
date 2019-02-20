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
 * Este programa está nomeado como hllapi.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <lib3270.h>
 #include <malloc.h>
 #include <string.h>
 #include <errno.h>
 #include <pw3270/hllapi.h>
 #include <stdio.h>
 #include <time.h>
 #include <lib3270/log.h>
 #include "client.h"

 /*--[ Prototipes ]-----------------------------------------------------------------------------------*/

 static int connect_ps(char *buffer, unsigned short *length, unsigned short *rc);
 static int disconnect_ps(char *buffer, unsigned short *length, unsigned short *rc);
 static int get_library_revision(char *buffer, unsigned short *length, unsigned short *rc);
 static int copy_ps_to_str(char *buffer, unsigned short *length, unsigned short *rc);
 static int copy_str_to_ps(char *buffer, unsigned short *length, unsigned short *rc);
 static int search_ps(char *buffer, unsigned short *length, unsigned short *rc);
 static int copy_ps(char *buffer, unsigned short *length, unsigned short *rc);
 static int wait_system(char *buffer, unsigned short *length, unsigned short *rc);
 static int reset_system(char *buffer, unsigned short *length, unsigned short *rc);
 static int pause_system(char *buffer, unsigned short *length, unsigned short *rc);
 static int set_session_parameters(char *buffer, unsigned short *length, unsigned short *rc);

 static int get_cursor_position(char *buffer, unsigned short *length, unsigned short *rc);
 static int set_cursor_position(char *buffer, unsigned short *length, unsigned short *rc);
 static int input_string(char *buffer, unsigned short *length, unsigned short *rc);

 static int invalid_request(char *buffer, unsigned short *length, unsigned short *rc);

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 static const struct _hllapi_call
 {
	unsigned long func;
	int (*exec)(char *buffer, unsigned short *length, unsigned short *rc);
 } hllapi_call[] =
 {
	{ HLLAPI_CMD_CONNECTPS,				connect_ps				},
	{ HLLAPI_CMD_DISCONNECTPS,			disconnect_ps			},
	{ HLLAPI_CMD_GETREVISION,			get_library_revision	},
	{ HLLAPI_CMD_QUERYCURSOR,			get_cursor_position		},
	{ HLLAPI_CMD_SETCURSOR,				set_cursor_position		},
	{ HLLAPI_CMD_COPYPSTOSTR,			copy_ps_to_str			},
	{ HLLAPI_CMD_INPUTSTRING,			input_string			},
	{ HLLAPI_CMD_WAIT,					wait_system				},
	{ HLLAPI_CMD_COPYPS,				copy_ps					},
	{ HLLAPI_CMD_SEARCHPS,				search_ps				},
	{ HLLAPI_CMD_COPYSTRTOPS,			copy_str_to_ps			},
	{ HLLAPI_CMD_SENDFILE,				invalid_request			},
	{ HLLAPI_CMD_RECEIVEFILE,			invalid_request			},
	{ HLLAPI_RESET_SYSTEM,				reset_system			},
	{ HLLAPI_CMD_PAUSE,					pause_system			},
	{ HLLAPI_SET_SESSION_PARAMETERS,	set_session_parameters	}

 };

 static enum _pause_mode
 {
	PAUSE_MODE_IPAUSE,		///< @brief Interruptible pause. After the Start Host Notification (23) function is executed, a host event satisfies a pause.
	PAUSE_MODE_FPAUSE		///< @brief A full-duration pause lasts for however long you specified in the Pause (18) function.
 } pause_mode = PAUSE_MODE_IPAUSE;

/*--[ Implement ]------------------------------------------------------------------------------------*/

HLLAPI_API_CALL hllapi(const LPWORD func, LPSTR buffer, LPWORD length, LPWORD rc)
{
	unsigned int f;

	trace("%s(%d)",__FUNCTION__,*func);

	for(f=0;f< (sizeof (hllapi_call) / sizeof ((hllapi_call)[0]));f++)
	{
		if(hllapi_call[f].func == *func)
		{
			int status = hllapi_call[f].exec(buffer,length,rc);
			trace("hllapi(%d) exits with rc=%d",*func,status);
			return status;
		}
	}

	trace("hllapi(%d) failed",*func);

	*rc = HLLAPI_STATUS_UNSUPPORTED;
	return *rc;

}

static int invalid_request(char *buffer, unsigned short *length, unsigned short *rc)
{
	*rc = HLLAPI_STATUS_BAD_PARAMETER;
	return *rc;
}

static int connect_ps(char *buffer, unsigned short *length, unsigned short *rc)
{
	char *tempbuffer = NULL;

	trace("%s: len=%d buflen=%d",__FUNCTION__,*length,(int) strlen(buffer));

	if(strlen(buffer) > *length)
		buffer[*length] = 0;

	if(!strrchr(buffer,':'))
	{
		int sz = strlen(buffer);

		tempbuffer = (char *) malloc(sz+2);
		strcpy(tempbuffer,buffer);
		tempbuffer[sz-1] = ':';
		tempbuffer[sz]   = buffer[sz-1];
		tempbuffer[sz+1] = 0;
		buffer = tempbuffer;
	}

	*rc = hllapi_init(buffer);

	if(tempbuffer)
		free(tempbuffer);

	return 0;
}

static int disconnect_ps(char *buffer, unsigned short *length, unsigned short *rc)
{
	*rc = hllapi_deinit();
	return 0;
}

static int get_library_revision(char *buffer, unsigned short *length, unsigned short *rc)
{
	*rc = hllapi_get_revision();
	return 0;
}

static int get_cursor_position(char *buffer, unsigned short *length, unsigned short *rc)
{
	int pos = hllapi_getcursor();

	trace("%s(%d)",__FUNCTION__,pos);

	if(pos < 0)
		return -1;

	*rc = pos;
	return 0;
}

static int set_cursor_position(char *buffer, unsigned short *length, unsigned short *rc)
{
	trace("%s(%d)",__FUNCTION__,*rc);
	*rc = hllapi_setcursor(*rc);
	return 0;
}

static int copy_ps_to_str(char *buffer, unsigned short *length, unsigned short *rc)
{

	// Length		Length of the target data string.
	// PS Position	Position within the host presentation space of the first byte in your target data string.
	*rc = hllapi_get_screen(*rc,buffer,*length);

	return 0;
}

static int input_string(char *buffer, unsigned short *length, unsigned short *rc)
{
	*rc = hllapi_input_string(buffer,*length);
	return 0;
}

static int search_ps(char *buffer, unsigned short *length, unsigned short *ps)
{
	/*
	 * Data String	Target string for search.
	 * Length	Length of the target data string. Overridden in EOT mode.
	 * PS Position	Position within the host presentation space where the search is to begin (SRCHFRWD option) or to end
	 * (SRCHBKWD option). Overridden in SRCHALL (default) mode.
	 *
	 * Return in *ps:
	 *
	 * = 0	The string was not found.
	 * > 0	The string was found at the indicated host presentation space position.
	 *
	 * Return code:
	 *
	 * 0	The Search Presentation Space function was successful.
	 * 1	Your program is not connected to a host session.
	 * 2	An error was made in specifying parameters.
	 * 7	The host presentation space position is not valid.
	 * 9	A system error was encountered.
	 * 24	The search string was not found.
	 *
	 */
	size_t   szBuffer = strlen(buffer);
	char   * text;
	int		 rc = HLLAPI_STATUS_SYSTEM_ERROR;

	if(!hllapi_is_connected())
		return HLLAPI_STATUS_DISCONNECTED;

	if(*length < szBuffer)
		szBuffer = *length;


	text = hllapi_get_string(*ps,szBuffer);
	if(!text)
		return HLLAPI_STATUS_SYSTEM_ERROR;

	if(strncmp(text,buffer,szBuffer))
	{
		// String not found
		*ps = 0;
		rc = HLLAPI_STATUS_NOT_FOUND;
	}
	else
	{
		// String found
		*ps = 1;
		rc = HLLAPI_STATUS_SUCCESS;
	}

	hllapi_free(text);

	return rc;
}

static int copy_ps(char *buffer, unsigned short *length, unsigned short *rc)
{
	/*
	 * Data String	Preallocated target string the size of your host presentation space. This can vary depending on how your host presentation space is configured. When the Set Session Parameters (9) function with the EAB option is issued, the length of the data string must be at least twice the length of the presentation space.
	 *				DBCS Only: When the EAD option is specified, the length of the data string must be at least three times the length of the presentation space. When both the EAB and EAD options are specified, the length of the data string must be at least four times the length of the presentation space.
	 *
	 * Length		NA (the length of the host presentation space is implied).
	 * PS Position	NA.
	 *
	 * Return values:
	 *
	 * 0	The host presentation space contents were copied to the application program. The target presentation space was active, and the keyboard was unlocked.
	 * 1	Your program is not connected to a host session.
	 * 4	The host presentation space contents were copied. The connected host presentation space was waiting for host response.
	 * 5	The host presentation space was copied. The keyboard was locked.
	 * 9	A system error was encountered.
	 *
	 */
	size_t	  			  szBuffer	= strlen(buffer);
	char				* text;

	if(!hllapi_is_connected())
		return HLLAPI_STATUS_DISCONNECTED;

	text = hllapi_get_string(1, szBuffer);

	if(!text)
		return HLLAPI_STATUS_SYSTEM_ERROR;

	memcpy(buffer,text,szBuffer);

	hllapi_free(text);

	return hllapi_get_state();
}

static int wait_system(char *buffer, unsigned short *length, unsigned short *rc)
{
	/*
	 * Checks the status of the host-connected presentation space. If the session is
	 * waiting for a host response (indicated by XCLOCK (X []) or XSYSTEM), the Wait
	 * function causes HLLAPI to wait up to 1 minute to see if the condition clears.
	 *
	 */

	/*
	 * Return Code	Definition
	 *
	 * HLLAPI_STATUS_SUCCESS			0	The keyboard is unlocked and ready for input.
	 * HLLAPI_STATUS_DISCONNECTED		1	Your application program is not connected to a valid session.
	 * HLLAPI_STATUS_TIMEOUT			4	Timeout while still in XCLOCK (X []) or XSYSTEM.
	 * HLLAPI_STATUS_KEYBOARD_LOCKED	5	The keyboard is locked.
	 * HLLAPI_STATUS_SYSTEM_ERROR		9	A system error was encountered.
	 *
	 */

	 /*
	 time_t end = time(0) + 60;

	 while(time(0) < end)
	 {
		int state = hllapi_get_state();

		if(state != HLLAPI_STATUS_WAITING)
			return state;

		hllapi_wait(1);

	 }

	 return HLLAPI_STATUS_TIMEOUT;
	 */

	 int state = hllapi_wait_for_ready(60);
	 return (state == HLLAPI_STATUS_WAITING ? HLLAPI_STATUS_TIMEOUT : state);

}

static int copy_str_to_ps(char *text, unsigned short *length, unsigned short *ps)
{
	/*
	 * Call Parameters
	 *
	 * Data 	String of ASCII data to be copied into the host presentation space.
	 * Length	Length, in number of bytes, of the source data string. Overridden if in EOT mode.
	 * PS 		Position in the host presentation space to begin the copy, a value between 1 and the configured size of your host presentation space.
	 *
	 * Return Parameters
	 *
	 * HLLAPI_STATUS_SUCCESS			0	The Copy String to Presentation Space function was successful.
	 * HLLAPI_STATUS_DISCONNECTED		1	Your program is not connected to a host session.
	 * HLLAPI_STATUS_BAD_PARAMETER		2	Parameter error or zero length for copy.
	 * HLLAPI_STATUS_KEYBOARD_LOCKED	5	The target presentation space is protected or inhibited, or incorrect data was sent to the target presentation space (such as a field attribute byte).
	 * 									6	The copy was completed, but the data was truncated.
	 * 									7	The host presentation space position is not valid.
	 * HLLAPI_STATUS_SYSTEM_ERROR		9	A system error was encountered.
	 *
	 */
	size_t szText = strlen(text);

	if(*length < szText)
		szText = *length;

	if(!szText)
		return HLLAPI_STATUS_BAD_PARAMETER;

	switch(hllapi_get_message_id())
	{
		case LIB3270_MESSAGE_NONE:
			break;

		case LIB3270_MESSAGE_DISCONNECTED:
			return HLLAPI_STATUS_DISCONNECTED;

		case LIB3270_MESSAGE_MINUS:
		case LIB3270_MESSAGE_PROTECTED:
		case LIB3270_MESSAGE_NUMERIC:
		case LIB3270_MESSAGE_OVERFLOW:
		case LIB3270_MESSAGE_INHIBIT:
		case LIB3270_MESSAGE_KYBDLOCK:
			return HLLAPI_STATUS_KEYBOARD_LOCKED;

		default:
			return HLLAPI_STATUS_SYSTEM_ERROR;
	}

	return hllapi_emulate_input(text,szText,0);
}

static int reset_system(char *buffer, unsigned short *length, unsigned short *rc)
{
	return hllapi_reset();
}


static int pause_system(char *buffer, unsigned short *length, unsigned short *rc)
{
	if(!*length)
	{
		// If you use the IPAUSE option and the pause value is zero, then the function
		// waits up to 2400 half-second intervals, unless interrupted sooner. If you use the
		// FPAUSE option and the pause value is zero, then the function returns
		// immediately.

		if(pause_mode == PAUSE_MODE_FPAUSE)
		{
			return HLLAPI_STATUS_SUCCESS;
		}
		return hllapi_wait_for_ready(1200);
	}

	if(pause_mode == PAUSE_MODE_FPAUSE)
	{
		// Pause fixo - Aguarda pelo tempo informado, independente de eventos.
		return hllapi_wait( (*length) / 2);
	}

	// Pause "flexivel", aguarda mudança no conteúdo da tela!!!

	// #warning Mudar comportamento na lib!

	return hllapi_wait_for_ready((*length) / 2);
}

static int set_session_parameters(char *buffer, unsigned short *length, unsigned short *rc)
{
	if(!(*length && buffer && *buffer))
	{
		return HLLAPI_STATUS_BAD_PARAMETER;
	}

	*rc = hllapi_set_session_parameter(buffer, *length, *rc);

	return 0;
}

HLLAPI_API_CALL hllapi_set_session_parameter(LPSTR param, WORD len, WORD value)
{
 	if(!param)
	{
		return HLLAPI_STATUS_BAD_PARAMETER;
	}

	if(!len)
	{
		len = strlen(param);
	}

	if(!strncasecmp(param,"IPAUSE",len))
	{
		// IPAUSE
		pause_mode = PAUSE_MODE_IPAUSE;
	}
	else if(!strncasecmp(param,"FPAUSE",len))
	{
		// FPAUSE
		pause_mode = PAUSE_MODE_FPAUSE;
	}
	else if(!strncasecmp(param,"UNLOCKDELAY",len))
	{
		// UNLOCKDELAY
		hllapi_set_unlock_delay(value);
	}
	else
	{
		return HLLAPI_STATUS_BAD_PARAMETER;
	}

	return HLLAPI_STATUS_SUCCESS;
}
