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
 #include <lib3270/log.h>

 #undef trace
 #define trace( fmt, ... )	{ FILE *out = fopen("c:\\Users\\Perry\\hllapi.log","a"); if(out) { fprintf(out, "%s(%d) " fmt "\n", __FILE__, __LINE__, __VA_ARGS__ ); fclose(out); } }

/*--[ Prototipes ]-----------------------------------------------------------------------------------*/

 static int connect_ps(char *buffer, unsigned short *length, unsigned short *rc);
 static int disconnect_ps(char *buffer, unsigned short *length, unsigned short *rc);
 static int get_library_revision(char *buffer, unsigned short *length, unsigned short *rc);
 static int copy_ps_to_str(char *buffer, unsigned short *length, unsigned short *rc);

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
	{ HLLAPI_CMD_CONNECTPS,			connect_ps				},
	{ HLLAPI_CMD_DISCONNECTPS,		disconnect_ps			},
	{ HLLAPI_CMD_GETREVISION,		get_library_revision	},
	{ HLLAPI_CMD_QUERYCURSOR,		get_cursor_position		},
	{ HLLAPI_CMD_SETCURSOR,			set_cursor_position		},
	{ HLLAPI_CMD_COPYPSTOSTR,		copy_ps_to_str			},
	{ HLLAPI_CMD_INPUTSTRING,		input_string			},
	{ HLLAPI_CMD_WAIT,				invalid_request			},
	{ HLLAPI_CMD_COPYPS,			invalid_request			},
	{ HLLAPI_CMD_SEARCHPS,			invalid_request			},
	{ HLLAPI_CMD_COPYSTRTOPS,		invalid_request			},
	{ HLLAPI_CMD_SENDFILE,			invalid_request			},
	{ HLLAPI_CMD_RECEIVEFILE,		invalid_request			},

 };

 static const char control_char = '@';

/*--[ Implement ]------------------------------------------------------------------------------------*/

#ifdef _WIN32
 __declspec (dllexport) int __stdcall hllapi(LPWORD func, LPSTR buffer, LPWORD length, LPWORD rc)
#else
 LIB3270_EXPORT int hllapi(const unsigned long *func, char *buffer, unsigned short *length, unsigned short *rc)
#endif // _WIN32
{
	int f;

	trace("%s(%d)",__FUNCTION__,*func);

	for(f=0;f< (sizeof (hllapi_call) / sizeof ((hllapi_call)[0]));f++)
	{
		if(hllapi_call[f].func == *func)
			return hllapi_call[f].exec(buffer,length,rc);
	}

	return invalid_request(buffer, length, rc);
}

static int invalid_request(char *buffer, unsigned short *length, unsigned short *rc)
{
	*rc = HLLAPI_STATUS_BAD_PARAMETER;
	return *rc;
}

static int connect_ps(char *buffer, unsigned short *length, unsigned short *rc)
{
	char *tempbuffer = NULL;

	trace("%s: len=%d buflen=%d",__FUNCTION__,*length,strlen(buffer));

	if(strlen(buffer) > *length)
		buffer[*length] = 0;

	if(!strrchr(buffer,':'))
	{
		int sz = strlen(buffer);

		tempbuffer = malloc(sz+2);
		strcpy(tempbuffer,buffer);
		tempbuffer[sz-1] = ':';
		tempbuffer[sz]   = buffer[sz-1];
		tempbuffer[sz+1] = 0;
		buffer = tempbuffer;
	}

	if(hllapi_init(buffer) == 0)
		*rc = HLLAPI_STATUS_SUCESS;
	else
		*rc = HLLAPI_STATUS_UNAVAILABLE;

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
	return hllapi_get_screen(*rc,buffer,*length);
}

static int input_string(char *input, unsigned short *length, unsigned short *rc)
{
	size_t	  szText;
	char 	* text;

	if(!input)
	{
		*rc = HLLAPI_STATUS_BAD_PARAMETER;
		return HLLAPI_STATUS_BAD_PARAMETER;
	}

	szText = strlen(input);

	if(*length > 0 && *length < szText)
		szText = *length;

	text = malloc(szText+2);
	memcpy(text,input,szText);
	text[szText] = 0;

	*rc = 0;

	if(strchr(text,control_char))
	{
		// Convert control char
		char	* buffer = text;
		char	* ptr;

		for(ptr = strchr(text,control_char);ptr;ptr = strchr(buffer,control_char))
		{
			*(ptr++) = 0;

			hllapi_emulate_input(buffer,-1,0);

			switch(*(ptr++))
			{
			case 'P':	// Print
				*rc = hllapi_print();
				break;

			case 'E':	// Enter
				hllapi_enter();
				break;

			case 'F':	// Erase EOF
				hllapi_erase_eof();
				break;

			case '1':	// PF1
				hllapi_pfkey(1);
				break;

			case '2':	// PF2
				hllapi_pfkey(2);
				break;

			case '3':	// PF3
				hllapi_pfkey(3);
				break;

			case '4':	// PF4
				hllapi_pfkey(4);
				break;

			case '5':	// PF5
				hllapi_pfkey(5);
				break;

			case '6':	// PF6
				hllapi_pfkey(6);
				break;

			case '7':	// PF7
				hllapi_pfkey(7);
				break;

			case '8':	// PF8
				hllapi_pfkey(8);
				break;

			case '9':	// PF9
				hllapi_pfkey(9);
				break;

			case 'a':	// PF10
				hllapi_pfkey(10);
				break;

			case 'b':	// PF11
				hllapi_pfkey(11);
				break;

			case 'c':	// PF12
				hllapi_pfkey(12);
				break;
			}

			buffer = ptr;

		}

		hllapi_emulate_input(buffer,-1,0);

	}
	else
	{
		hllapi_emulate_input(text,szText,0);
	}

	free(text);

	return 0;
}

