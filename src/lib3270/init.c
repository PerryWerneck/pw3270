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
 * Este programa está nomeado como init.c e possui - linhas de código.
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


#include "globals.h"
#include "appres.h"
#include "charsetc.h"

#include <malloc.h>

/*---[ Statics ]--------------------------------------------------------------------------------------------------------------*/

 static int parse_model_number(H3270 *session, const char *m);

/*---[ Implement ]------------------------------------------------------------------------------------------------------------*/

void lib3270_session_free(H3270 *h)
{
	int f;

	// Terminate session
	if(lib3270_connected(h))
		lib3270_disconnect(h);

	shutdown_toggles(h);

	// Release state change callbacks
	for(f=0;f<N_ST;f++)
	{
		while(h->st_callbacks[f])
		{
			struct lib3270_state_callback *next = h->st_callbacks[f]->next;
			Free(h->st_callbacks[f]);
			h->st_callbacks[f] = next;
		}
	}

	// Release memory
	#define RELEASE_BUFFER(x) if(x) { free(x); x = NULL; }

	RELEASE_BUFFER(h->charset);
	RELEASE_BUFFER(h->paste_buffer);

	for(f=0;f<(sizeof(h->buffer)/sizeof(h->buffer[0]));f++)
	{
		RELEASE_BUFFER(h->buffer[f]);
	}

}

static void update_char(H3270 *session, int addr, unsigned char chr, unsigned short attr, unsigned char cursor)
{
}

static void nop_char(H3270 *session, unsigned char chr)
{
}

static void nop(H3270 *session)
{
}

static void update_model(H3270 *session, const char *name, int model, int rows, int cols)
{
}

static void changed(H3270 *session, int bstart, int bend)
{
}

static void update_cursor(H3270 *session, unsigned short row, unsigned short col, unsigned char c, unsigned short attr)
{
}

static void update_oia(H3270 *session, LIB3270_FLAG id, unsigned char on)
{
}

static void update_selection(H3270 *session, int start, int end)
{
}

static void set_cursor(H3270 *session, LIB3270_CURSOR id)
{
}

static void message(H3270 *session, LIB3270_NOTIFY id , const char *title, const char *message, const char *text)
{
	lib3270_write_log(session,"%s",title);
	lib3270_write_log(session,"%s",message);
	lib3270_write_log(session,"%s",text);
}

static void update_ssl(H3270 *session, LIB3270_SSL_STATE state)
{
}

static void lib3270_session_init(H3270 *hSession, const char *model)
{
	int 	ovc, ovr;
	char	junk;
	int		model_number;

	memset(hSession,0,sizeof(H3270));
	hSession->sz = sizeof(H3270);

	// Set the defaults.
	hSession->extended  		= 1;
	hSession->typeahead			= 1;
	hSession->oerr_lock 		= 1;
	hSession->unlock_delay		= 1;
	hSession->icrnl 			= 1;
	hSession->onlcr				= 1;

	// Initialize toggles
	initialize_toggles(hSession);

	// Dummy calls to avoid "ifs"
	hSession->update 			= update_char;
	hSession->update_model		= update_model;
	hSession->update_cursor		= update_cursor;
	hSession->set_selection 	= nop_char;
	hSession->ctlr_done			= nop;
	hSession->changed			= changed;
	hSession->erase				= screen_disp;
	hSession->suspend			= nop;
	hSession->resume			= screen_disp;
	hSession->update_oia		= update_oia;
	hSession->update_selection	= update_selection;
	hSession->cursor 			= set_cursor;
	hSession->message			= message;
	hSession->update_ssl		= update_ssl;
	hSession->sock				= -1;

#ifdef _WIN32
	hSession->sockEvent	= NULL;
#endif // _WIN32

	hSession->model_num = -1;
	hSession->cstate = NOT_CONNECTED;
	hSession->oia_status = -1;

	strncpy(hSession->full_model_name,"IBM-",LIB3270_FULL_MODEL_NAME_LENGTH);
	hSession->model_name = &hSession->full_model_name[4];

	if(!*model)
		model = "2";	// No model, use the default one

	model_number = parse_model_number(hSession,model);
	if (model_number < 0)
	{
		popup_an_error(NULL,"Invalid model number: %s", model);
		model_number = 0;
	}

	if (!model_number)
	{
#if defined(RESTRICT_3279)
		model_number = 3;
#else
		model_number = 4;
#endif
	}

	if(hSession->mono)
		hSession->m3279 = 0;
	else
		hSession->m3279 = 1;

	if(!hSession->extended)
		hSession->oversize = CN;

#if defined(RESTRICT_3279)
	if (hSession->m3279 && model_number == 4)
		model_number = 3;
#endif

	Trace("Model_number: %d",model_number);

	if (!hSession->extended || hSession->oversize == CN || sscanf(hSession->oversize, "%dx%d%c", &ovc, &ovr, &junk) != 2)
	{
		ovc = 0;
		ovr = 0;
	}
	ctlr_set_rows_cols(hSession, model_number, ovc, ovr);

	if (hSession->termname != CN)
		hSession->termtype = hSession->termname;
	else
		hSession->termtype = hSession->full_model_name;

	Trace("Termtype: %s",hSession->termtype);

	if (hSession->apl_mode)
		appres.charset = "apl";

}

H3270 * lib3270_session_new(const char *model)
{
	static int configured = 0;

	H3270		*hSession = &h3270;

	Trace("%s - configured=%d",__FUNCTION__,configured);

	if(configured)
	{
		// TODO (perry#5#): Allocate a new structure.
		errno = EBUSY;
		return hSession;
	}

	configured = 1;

	lib3270_session_init(hSession, model);

	if(screen_init(hSession))
		return NULL;

	Trace("Charset: %s",appres.charset);
	if (charset_init(hSession,appres.charset) != CS_OKAY)
	{
		Warning(hSession, _( "Cannot find charset \"%s\", using defaults" ), appres.charset);
		(void) charset_init(hSession,CN);
	}

	kybd_init();
	ansi_init();

#if defined(X3270_FT)
	ft_init(hSession);
#endif

/*
#if defined(X3270_PRINTER)
	printer_init();
#endif
*/
	Trace("%s finished",__FUNCTION__);

	errno = 0;
	return hSession;
}

 /*
- * Parse the model number.
- * Returns -1 (error), 0 (default), or the specified number.
- */
static int parse_model_number(H3270 *session, const char *m)
{
	int sl;
	int n;

	if(!m)
		return 0;

	sl = strlen(m);

	/* An empty model number is no good. */
	if (!sl)
		return 0;

	if (sl > 1) {
		/*
		 * If it's longer than one character, it needs to start with
		 * '327[89]', and it sets the m3279 resource.
		 */
		if (!strncmp(m, "3278", 4))
		{
			session->m3279 = 0;
		}
		else if (!strncmp(m, "3279", 4))
		{
			session->m3279 = 1;
		}
		else
		{
			return -1;
		}
		m += 4;
		sl -= 4;

		/* Check more syntax.  -E is allowed, but ignored. */
		switch (m[0]) {
		case '\0':
			/* Use default model number. */
			return 0;
		case '-':
			/* Model number specified. */
			m++;
			sl--;
			break;
		default:
			return -1;
		}
		switch (sl) {
		case 1: /* n */
			break;
		case 3:	/* n-E */
			if (strcasecmp(m + 1, "-E")) {
				return -1;
			}
			break;
		default:
			return -1;
		}
	}

	/* Check the numeric model number. */
	n = atoi(m);
	if (n >= 2 && n <= 5) {
		return n;
	} else {
		return -1;
	}

}
