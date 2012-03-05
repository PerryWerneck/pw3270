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
 * Este programa está nomeado como macros.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */

 #include <string.h>
 #include <stdio.h>
 #include <lib3270.h>
 #include <lib3270/macros.h>

 #define LIB3270_MACRO_ENTRY( name )  { #name, lib3270_macro_ ## name }

 static const LIB3270_MACRO_LIST macro_list[] =
 {
 	LIB3270_MACRO_ENTRY( connect	),
 	LIB3270_MACRO_ENTRY( cstate		),
 	LIB3270_MACRO_ENTRY( disconnect	),
 	LIB3270_MACRO_ENTRY( encoding	),
 	LIB3270_MACRO_ENTRY( enter		),
 	LIB3270_MACRO_ENTRY( get		),
 	LIB3270_MACRO_ENTRY( luname		),
 	LIB3270_MACRO_ENTRY( pa			),
 	LIB3270_MACRO_ENTRY( pf			),
 	LIB3270_MACRO_ENTRY( set		),
 	LIB3270_MACRO_ENTRY( status		),

 	{NULL, NULL}
 };

 LIB3270_EXPORT const LIB3270_MACRO_LIST * get_3270_calls(void)
 {
 	return macro_list;
 }

/*
 static char * value_as_string(int val)
 {
 	char buffer[10];
 	snprintf(buffer,9,"%d",val);
 	return strdup(buffer);
 }
*/

 static const char * get_state(H3270 *h)
 {
 	#define DECLARE_XLAT_STATE(x) { x, #x }
	static const struct _xlat_state
	{
		LIB3270_CSTATE	  state;
		const char		* ret;
	} xlat_state[] =
	{
		DECLARE_XLAT_STATE( NOT_CONNECTED 		),
		DECLARE_XLAT_STATE( RESOLVING			),
		DECLARE_XLAT_STATE( PENDING				),
		DECLARE_XLAT_STATE( CONNECTED_INITIAL	),
		DECLARE_XLAT_STATE( CONNECTED_ANSI		),
		DECLARE_XLAT_STATE( CONNECTED_3270		),
		DECLARE_XLAT_STATE( CONNECTED_INITIAL_E	),
		DECLARE_XLAT_STATE( CONNECTED_NVT		),
		DECLARE_XLAT_STATE( CONNECTED_SSCP		),
		DECLARE_XLAT_STATE( CONNECTED_TN3270E	)
	};

	int f;

 	LIB3270_CSTATE state = lib3270_get_connection_state(h);

	for(f=0;f < (sizeof(xlat_state)/sizeof(struct _xlat_state)); f++)
	{
		if(state == xlat_state[f].state)
			return xlat_state[f].ret;
	}

	return "Unexpected";
 }

 LIB3270_MACRO( encoding )
 {
 	return strdup("ISO-8859-1");
 }

 LIB3270_MACRO( get )
 {
	int start, qtd, rows, cols, row, col;
	char *buffer = NULL;

	switch(argc)
	{
	case 1:	// Get entire screen
		get_3270_terminal_size(hSession,&rows,&cols);
		qtd = (rows*(cols+1)+1);
		buffer = malloc(qtd+2);

		Trace("Screen buffer size: %d (%dx%d)",qtd,rows,cols);

		memset(buffer,0,qtd+1);
		start = qtd = 0;
		for(row = 0; row < rows;row++)
		{
			screen_read(buffer+qtd,start,cols);
			qtd += cols;
			start += cols;
			buffer[qtd++] = '\n';
		}
		buffer[qtd] = 0;

		Trace("Bytes read: %d",qtd);
		return buffer;

	case 2:	// Just size, get current cursor position
		start	= 0;
		qtd 	= atoi(argv[1]);
		break;

	case 3:	// Use start position
		start	= atoi(argv[1]);
		qtd 	= atoi(argv[2]);
		break;

	case 4:	// Get start position from row/col
		get_3270_terminal_size(hSession,&rows,&cols);

		row = atoi(argv[1])-1;
		col = atoi(argv[2])-1;

		if(row < 0 || row > rows || col < 0 || col > cols)
		{
			errno = EINVAL;
			return NULL;
		}

		start 	= (row * cols) + col;
		qtd 	= atoi(argv[3]);
		break;

	default:
		errno = EINVAL;
		return NULL;
	}

	if(qtd < 1)
	{
		errno = EINVAL;
		return NULL;
	}

	buffer = malloc(qtd+1);
	screen_read(buffer, start, qtd);

	return buffer;
 }

 LIB3270_MACRO( set )
 {
 	const char *str = NULL;

	if(query_3270_terminal_status() != LIB3270_STATUS_BLANK)
	{
		errno = EBUSY;
		return NULL;
	}

	switch(argc)
	{
	case 1:
		lib3270_enter(hSession);
		break;

    case 2:
		str = argv[1];
		break;

	case 3:
        cursor_set_addr(atoi(argv[1]));
		str = argv[2];
        break;

    case 4:
        cursor_set_addr((atoi(argv[1])-1) * ctlr_get_cols() + (atoi(argv[2])-1));
		str = argv[3];
        break;

	default:
		errno = EINVAL;
		return NULL;
	}

	if(str)
		lib3270_set_string(NULL, (const unsigned char *) str);

	return strdup(get_state(hSession));
 }

 LIB3270_MACRO( status )
 {
	const char	* luname	= (const char *) get_connected_lu(hSession);
 	const char	* cstate	= get_state(hSession);
 	const char	* host		= (const char *) get_current_host(hSession);
 	char		* rsp;
 	size_t		  sz;

	if(!luname)
		luname = "none";

	if(!host)
		host = "-";

	sz = strlen(luname)+strlen(cstate)+strlen(host)+4;
	rsp = malloc(sz+1);
 	snprintf(rsp,sz,"%s %s %s",cstate,luname,host);
 	return rsp;
 }

 LIB3270_MACRO( cstate )
 {
	return strdup(get_state(hSession));
 }

 LIB3270_MACRO( luname )
 {
	const char	* luname = (const char *) get_connected_lu(hSession);
	return strdup(luname ? luname : "none" );
 }

 LIB3270_MACRO( pf )
 {
 	char ret[10];
 	if(argc != 2)
	{
		errno = EINVAL;
		return NULL;
	}
	snprintf(ret,9,"%d",lib3270_pfkey(hSession,atoi(argv[1])));
	return strdup(ret);
 }

 LIB3270_MACRO( pa )
 {
 	char ret[10];
 	if(argc != 2)
	{
		errno = EINVAL;
		return NULL;
	}
	snprintf(ret,9,"%d",lib3270_pakey(hSession,atoi(argv[1])));
	return strdup(ret);
 }

 LIB3270_MACRO( enter )
 {
 	char ret[10];
 	if(argc != 1)
	{
		errno = EINVAL;
		return NULL;
	}
	snprintf(ret,9,"%d",lib3270_enter(hSession));
	return strdup(ret);
 }

 LIB3270_MACRO( connect )
 {
 	int rc = EBUSY;
 	char ret[10];

	switch(argc)
	{
	case 1:
		rc = host_reconnect(0);
		break;

	case 2:
		rc = host_connect(argv[1],0);
		break;

	case 3:
		rc = host_connect(argv[1],atoi(argv[2]));
		break;

	default:
		return NULL;
	}

	snprintf(ret,9,"%d",rc);
	return strdup(ret);
 }

 LIB3270_MACRO( disconnect )
 {
	lib3270_disconnect(hSession);
	return strdup("0");
 }
