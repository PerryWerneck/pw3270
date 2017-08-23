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
 * Este programa está nomeado como calls.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <exception>
 #include <cstdlib>
 #include <cstring>

 #include <pw3270cpp.h>
 #include <pw3270/hllapi.h>
 #include "client.h"

 using namespace std;
 using namespace PW3270_NAMESPACE;

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 static session	* hSession 			= NULL;
 static time_t	  hllapi_timeout	= 120;

/*--[ Implement ]------------------------------------------------------------------------------------*/

 HLLAPI_API_CALL hllapi_init(LPSTR mode)
 {
 	trace("%s(%s)",__FUNCTION__,mode);

	try
	{
		if(hSession)
			delete hSession;
		hSession = session::create(mode);

		session::get_default()->set_display_charset();

		trace("hSession=%p",hSession);
	}
	catch(std::exception &e)
	{
		trace("Error \"%s\"",e.what());
		return HLLAPI_STATUS_SYSTEM_ERROR;
	}

 	return hSession ? HLLAPI_STATUS_SUCCESS : HLLAPI_STATUS_UNAVAILABLE;
 }

 HLLAPI_API_CALL hllapi_deinit(void)
 {
 	trace("%s()",__FUNCTION__);

 	try
 	{
		if(hSession)
		{
			delete hSession;
			hSession = NULL;
		}
	}
	catch(std::exception &e)
	{
		return HLLAPI_STATUS_SYSTEM_ERROR;
	}

 	return HLLAPI_STATUS_SUCCESS;
 }

 HLLAPI_API_CALL hllapi_get_revision(void)
 {
 	try
 	{
		return atoi(session::get_default()->get_revision().c_str());
	}
	catch(std::exception &e)
	{
		return -1;
	}
	return (DWORD) -1;
 }

 HLLAPI_API_CALL hllapi_connect(LPSTR uri, WORD wait)
 {
 	try
 	{
		session::get_default()->connect(uri,hllapi_timeout);
	}
	catch(std::exception &e)
	{
		return HLLAPI_STATUS_SYSTEM_ERROR;
	}

	return hllapi_get_state();
 }

 HLLAPI_API_CALL hllapi_is_connected(void)
 {
 	if(!session::has_default())
	{
		return 0;
	}

	return session::get_default()->is_connected();
 }

 HLLAPI_API_CALL hllapi_get_state(void)
 {
	switch(hllapi_get_message_id())
	{
	case LIB3270_MESSAGE_NONE:					// 0 - No message
		return HLLAPI_STATUS_SUCCESS;			// keyboard was unlocked and ready for input.

	case LIB3270_MESSAGE_DISCONNECTED:			// 4 - Disconnected from host
		return HLLAPI_STATUS_DISCONNECTED;		// Your application program was not connected to a valid session.

	case LIB3270_MESSAGE_MINUS:
	case LIB3270_MESSAGE_PROTECTED:
	case LIB3270_MESSAGE_NUMERIC:
	case LIB3270_MESSAGE_OVERFLOW:
	case LIB3270_MESSAGE_INHIBIT:
	case LIB3270_MESSAGE_KYBDLOCK:
		return HLLAPI_STATUS_KEYBOARD_LOCKED;	// keyboard is locked.

	case LIB3270_MESSAGE_SYSWAIT:
	case LIB3270_MESSAGE_TWAIT:
	case LIB3270_MESSAGE_AWAITING_FIRST:
	case LIB3270_MESSAGE_X:
	case LIB3270_MESSAGE_RESOLVING:
	case LIB3270_MESSAGE_CONNECTING:
		return HLLAPI_STATUS_WAITING;			// time-out while still busy (in XCLOCK or XSYSTEM in X) for the 3270 terminal emulation.
	}

	return HLLAPI_STATUS_SYSTEM_ERROR;
 }

 HLLAPI_API_CALL hllapi_disconnect(void)
 {
	session::get_default()->disconnect();
	return HLLAPI_STATUS_SUCCESS;
 }

 HLLAPI_API_CALL hllapi_wait_for_ready(WORD seconds)
 {
	if(!hllapi_is_connected())
		return HLLAPI_STATUS_DISCONNECTED;

	session::get_default()->wait_for_ready(seconds);

	return hllapi_get_state();
 }

 HLLAPI_API_CALL hllapi_wait(WORD seconds)
 {
	if(!hllapi_is_connected())
		return HLLAPI_STATUS_DISCONNECTED;

	session::get_default()->wait(seconds);

	return hllapi_get_state();
 }

 HLLAPI_API_CALL hllapi_get_message_id(void)
 {
	if(!hllapi_is_connected())
		return HLLAPI_STATUS_DISCONNECTED;

	return session::get_default()->get_program_message();
 }

 HLLAPI_API_CALL hllapi_get_screen_at(WORD row, WORD col, LPSTR buffer)
 {
	if(!hllapi_is_connected())
		return HLLAPI_STATUS_DISCONNECTED;

	if(!(buffer && *buffer))
		return HLLAPI_STATUS_SYSTEM_ERROR;

	try
	{
		size_t	  sz = strlen(buffer);
		string	  str = session::get_default()->get_string_at(row,col,sz);
		strncpy(buffer,str.c_str(),sz);
	}
	catch(std::exception &e)
	{
		return HLLAPI_STATUS_SYSTEM_ERROR;
	}

 	return HLLAPI_STATUS_SUCCESS;
 }

 HLLAPI_API_CALL hllapi_enter(void)
 {
	if(!hllapi_is_connected())
		return HLLAPI_STATUS_DISCONNECTED;

	return session::get_default()->enter();
 }

 HLLAPI_API_CALL hllapi_set_text_at(WORD row, WORD col, LPSTR text)
 {
	if(!hllapi_is_connected())
		return HLLAPI_STATUS_DISCONNECTED;

	try
	{
		session::get_default()->set_string_at(row,col,text);
	}
	catch(std::exception &e)
	{
		return HLLAPI_STATUS_SYSTEM_ERROR;
	}

 	return HLLAPI_STATUS_SUCCESS;
 }

 HLLAPI_API_CALL hllapi_cmp_text_at(WORD row, WORD col, LPSTR text)
 {

	if(!hllapi_is_connected())
		return HLLAPI_STATUS_DISCONNECTED;

 	int rc = HLLAPI_STATUS_SYSTEM_ERROR;

	try
	{
		rc = session::get_default()->cmp_string_at(row,col,text);
	}
	catch(std::exception &e)
	{
		return HLLAPI_STATUS_SYSTEM_ERROR;
	}

 	return rc;
 }

 HLLAPI_API_CALL hllapi_find_text(LPSTR text)
 {
	if(!hllapi_is_connected())
		return HLLAPI_STATUS_DISCONNECTED;

 	return (int) session::get_default()->find_string((const char *) text, false);
 }

 HLLAPI_API_CALL hllapi_set_unlock_delay(WORD ms)
 {
	session::get_default()->set_unlock_delay(ms);
	return 0;
 }

 HLLAPI_API_CALL hllapi_set_charset(LPSTR text)
 {
 	try
 	{

		session::get_default()->set_display_charset(NULL, (const char *) text);

 	}
	catch(std::exception &e)
	{
		return HLLAPI_STATUS_SYSTEM_ERROR;
	}

	return 0;
 }

 HLLAPI_API_CALL hllapi_pfkey(WORD key)
 {
	if(!hllapi_is_connected())
		return HLLAPI_STATUS_DISCONNECTED;

	return session::get_default()->pfkey(key);
 }

 HLLAPI_API_CALL hllapi_pakey(WORD key)
 {
	if(!hllapi_is_connected())
		return HLLAPI_STATUS_DISCONNECTED;

	return session::get_default()->pakey(key);
 }

 HLLAPI_API_CALL hllapi_get_datadir(LPSTR datadir)
 {
 #ifdef _WIN32
	HKEY 			hKey	= 0;
 	unsigned long	datalen = strlen(datadir);

	*datadir = 0;

	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,"Software\\pw3270",0,KEY_QUERY_VALUE,&hKey) == ERROR_SUCCESS)
	{
		unsigned long datatype;					// #defined in winnt.h (predefined types 0-11)
		if(RegQueryValueExA(hKey,"datadir",NULL,&datatype,(LPBYTE) datadir,&datalen) != ERROR_SUCCESS)
			*datadir = 0;
		RegCloseKey(hKey);
	}
#endif // _WIN32

	return *datadir;
 }

 HLLAPI_API_CALL hllapi_setcursor(WORD pos)
 {
	if(!hllapi_is_connected())
		return HLLAPI_STATUS_DISCONNECTED;

	session::get_default()->set_cursor_addr(pos-1);

	return HLLAPI_STATUS_SUCCESS;

 }

 HLLAPI_API_CALL hllapi_set_cursor_address(WORD pos)
 {
	if(!hllapi_is_connected())
		return HLLAPI_STATUS_DISCONNECTED;

	session::get_default()->set_cursor_addr(pos-1);

	return HLLAPI_STATUS_SUCCESS;

 }

 HLLAPI_API_CALL hllapi_get_cursor_address()
 {
	return session::get_default()->get_cursor_addr()+1;
 }

 HLLAPI_API_CALL hllapi_getcursor()
 {
	return session::get_default()->get_cursor_addr()+1;
 }

 HLLAPI_API_CALL hllapi_get_screen(WORD offset, LPSTR buffer, WORD len)
 {
	if(!hllapi_is_connected())
		return HLLAPI_STATUS_DISCONNECTED;

	int rc = HLLAPI_STATUS_SYSTEM_ERROR;

	if(offset < 1)
	{
		return HLLAPI_STATUS_BAD_PARAMETER;
	}

	offset--;

	if(!session::has_default())
	{
		return HLLAPI_STATUS_DISCONNECTED;
	}

	if(!(buffer && *buffer)) {
		return HLLAPI_STATUS_BAD_PARAMETER;
	}

	try
	{
		size_t szBuffer;

		if(len > 0)
		{
			szBuffer = (size_t) len;
		}
		else
		{
			return HLLAPI_STATUS_BAD_PARAMETER;
		}

		memset(buffer,' ',szBuffer);

		string str = session::get_default()->get_string(offset,szBuffer,false);
		strncpy(buffer,str.c_str(),szBuffer);
		rc = HLLAPI_STATUS_SUCCESS;
	}
	catch(std::exception &e)
	{
		rc = HLLAPI_STATUS_SYSTEM_ERROR;
	}

	return rc;
 }

 HLLAPI_API_CALL hllapi_emulate_input(const LPSTR buffer, WORD len, WORD pasting)
 {
	if(!hllapi_is_connected())
		return HLLAPI_STATUS_DISCONNECTED;

	try
	{
		session::get_default()->input_string(buffer);
	}
	catch(std::exception &e)
	{
		return HLLAPI_STATUS_SYSTEM_ERROR;
	}

	return HLLAPI_STATUS_SUCCESS;
 }

 HLLAPI_API_CALL hllapi_erase(void)
 {
 	try
 	{
		session::get_default()->erase();
 	}
	catch(std::exception &e)
	{
		return HLLAPI_STATUS_SYSTEM_ERROR;
	}
	return HLLAPI_STATUS_SUCCESS;
 }

 HLLAPI_API_CALL hllapi_erase_eof(void)
 {
	if(!hllapi_is_connected())
		return HLLAPI_STATUS_DISCONNECTED;

 	try
 	{
		session::get_default()->erase_eof();
 	}
	catch(std::exception &e)
	{
		return HLLAPI_STATUS_SYSTEM_ERROR;
	}
	return HLLAPI_STATUS_SUCCESS;
 }

 HLLAPI_API_CALL hllapi_erase_eol(void)
 {
	if(!hllapi_is_connected())
		return HLLAPI_STATUS_DISCONNECTED;

 	try
 	{
		session::get_default()->erase_eol();
 	}
	catch(std::exception &e)
	{
		return HLLAPI_STATUS_SYSTEM_ERROR;
	}
	return HLLAPI_STATUS_SUCCESS;
 }

 HLLAPI_API_CALL hllapi_erase_input(void)
 {
	if(!hllapi_is_connected())
		return HLLAPI_STATUS_DISCONNECTED;

 	try
 	{
		session::get_default()->erase_input();
 	}
	catch(std::exception &e)
	{
		return HLLAPI_STATUS_SYSTEM_ERROR;
	}
	return HLLAPI_STATUS_SUCCESS;
 }

 HLLAPI_API_CALL hllapi_action(LPSTR buffer) {
 	try
 	{
		session::get_default()->action((const char *) buffer);
 	}
	catch(std::exception &e)
	{
		return HLLAPI_STATUS_SYSTEM_ERROR;
	}
	return HLLAPI_STATUS_SUCCESS;
 }

 HLLAPI_API_CALL hllapi_print(void)
 {
	return session::get_default()->print();
 }

 char * hllapi_get_string(int offset, size_t len)
 {
	try
	{
		string str = session::get_default()->get_string(offset-1,len);
		char * ret = strdup(str.c_str());
		return ret;
	}
	catch(std::exception &e)
	{
	}

	return NULL;
 }

 void hllapi_free(void *p)
 {
 	free(p);
 }

 HLLAPI_API_CALL hllapi_reset(void)
 {
	return HLLAPI_STATUS_SUCCESS;
 }

 HLLAPI_API_CALL hllapi_input_string(LPSTR input, WORD length)
 {
	static const char control_char = '@';

	size_t	  szText;
	char 	* text;
	int		  rc	= 0;

	if(!hllapi_is_connected()) {
		return HLLAPI_STATUS_DISCONNECTED;
	}

	if(!input)
	{
		return HLLAPI_STATUS_BAD_PARAMETER;
	}

	if(length > 0 )
		szText = length;
	else
		szText = strlen(input);

	text = (char *) malloc(szText+2);
	memcpy(text,input,szText);
	text[szText] = 0;

	trace("input[%s]",text);

	if(strchr(text,control_char))
	{
		// Convert control char
		char	* buffer = text;
		char	* ptr;

		for(ptr = strchr(text,control_char);ptr;ptr = strchr(buffer,control_char))
		{
			*(ptr++) = 0;

			trace("input[%s]",buffer);
			hllapi_emulate_input(buffer,-1,0);

			switch(*(ptr++))
			{
			case 'P':	// Print
				rc = hllapi_print();
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

			case 'd':	// PF13
				hllapi_pfkey(13);
				break;

			case 'e':	// PF14
				hllapi_pfkey(14);
				break;

			case 'f':	// PF15
				hllapi_pfkey(15);
				break;

			case 'g':	// PF16
				hllapi_pfkey(16);
				break;

			case 'h':	// PF17
				hllapi_pfkey(17);
				break;

			case 'i':	// PF18
				hllapi_pfkey(18);
				break;

			case 'j':	// PF19
				hllapi_pfkey(19);
				break;

			case 'k':	// PF20
				hllapi_pfkey(20);
				break;

			case 'l':	// PF21
				hllapi_pfkey(21);
				break;

			case 'm':	// PF22
				hllapi_pfkey(22);
				break;

			case 'n':	// PF23
				hllapi_pfkey(23);
				break;

			case 'o':	// PF24
				hllapi_pfkey(24);
				break;

			case '@':	// Send '@' character
				hllapi_emulate_input((LPSTR) "@",-1,0);
				break;

			case 'x':	// PA1
				hllapi_pakey(1);
				break;

			case 'y':	// PA2
				hllapi_pakey(2);
				break;

			case 'z':	// PA3
				hllapi_pakey(3);
				break;

			case 'B':	// PC_LEFTTAB = "@B"
				break;

			case 'T':	// PC_RIGHTTAB = "@T"
				break;

			case 'N':	// PC_NEWLINE = "@N"
				break;

			case 'C':	// PC_CLEAR = "@C"
				hllapi_erase_input();
				break;

			case 'D':	// PC_DELETE = "@D"
				break;

			case 'H':	// PC_HELP = "@H"
				break;

			case 'I':	// PC_INSERT = "@I"
				break;

			case 'L':	// PC_CURSORLEFT = "@L"
				break;

			case 'R':	// PC_RESET = "@R"
				hllapi_reset();
				break;

			case 'U':	// PC_CURSORUP = "@U"
				break;

			case 'V':	// PC_CURSORDOWN = "@V"
				break;

			case 'Z':	// PC_CURSORRIGHT = "@Z"
				break;

			case '0':	// PC_HOME = "@0"
				break;

			case 'p':	// PC_PLUSKEY = "@p"
				break;

			case 'q':	// PC_END = "@q"
				break;

			case 's':	// PC_SCRLK = "@s"
				break;

			case 't':	// PC_NUMLOCK = "@t"
				break;

			case 'u':	// PC_PAGEUP = "@u"
				break;

			case 'v':	// PC_PAGEDOWN = "@v"
				break;

			case '/':	// PC_OVERRUNOFQUEUE = "@/"   ' Queue overflow, used in Get Key only
				break;

			case '$':	// PC_ALTCURSOR = "@$"        ' Presentation Manager only, unused in VB environment
				break;

			case '<':	// PC_BACKSPACE = "@<"
				break;


/*

Global Const PC_TEST = "@A@C"
Global Const PC_WORDDELETE = "@A@D"
Global Const PC_FIELDEXIT = "@A@E"
Global Const PC_ERASEINPUT = "@A@F"
Global Const PC_SYSTEMREQUEST = "@A@H"
Global Const PC_INSERTTOGGLE = "@A@I"
Global Const PC_CURSORSELECT = "@A@J"
Global Const PC_CURSLEFTFAST = "@A@L"
Global Const PC_GETCURSOR = "@A@N"
Global Const PC_LOCATECURSOR = "@A@O"
Global Const PC_ATTENTION = "@A@Q"
Global Const PC_DEVICECANCEL = "@A@R"
Global Const PC_PRINTPS = "@A@T"
Global Const PC_CURSUPFAST = "@A@U"
Global Const PC_CURSDOWNFAST = "@A@V"
Global Const PC_HEX = "@A@X"
Global Const PC_FUNCTIONKEY = "@A@Y"
Global Const PC_CURSRIGHTFAST = "@A@Z"

Global Const PC_REVERSEVIDEO = "@A@9"
Global Const PC_UNDERSCORE = "@A@b"
Global Const PC_BLINK = "@A@c"
Global Const PC_RED = "@A@d"
Global Const PC_PINK = "@A@e"
Global Const PC_GREEN = "@A@f"
Global Const PC_YELLOW = "@A@g"
Global Const PC_BLUE = "@A@h"
Global Const PC_TURQOISE = "@A@i"
Global Const PC_WHITE = "@A@j"
Global Const PC_RSTHOSTCOLORS = "@A@l"
Global Const PC_PRINTPC = "@A@t"

Global Const PC_FIELDMINUS = "@A@-"
Global Const PC_FIELDPLUS = "@A@+"

*/

			}

			buffer = ptr;

		}

		if(*buffer)
			hllapi_emulate_input(buffer,-1,0);

	}
	else
	{
		hllapi_emulate_input(text,szText,0);
	}

	free(text);

	return rc;
 }

