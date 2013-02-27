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
 * Este programa está nomeado como local.cxx e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

 #include "globals.hpp"
 #include <errno.h>

/*
 * NOTE:	Take a better look at osl_createEmptySocketAddr() & osl_connectSocketTo() to see if there's
 *			a way to use this calls to connect with the host for better performance.
 */

/*---[ Statics ]-------------------------------------------------------------------------------------------*/


/*---[ Implement ]-----------------------------------------------------------------------------------------*/

 pw3270::lib3270_session::lib3270_session()
 {
	struct _call
	{
		void 		**entry;
		const char 	* name;
	} call[] =
	{
		{ (void **) & _get_revision,		"lib3270_get_revision"			},
		{ (void **) & _get_text_at,			"lib3270_get_text_at"			},
		{ (void **) & _set_text_at,			"lib3270_set_string_at"			},
		{ (void **) & _cmp_text_at,			"lib3270_cmp_text_at"			},
		{ (void **) & _enter,				"lib3270_enter"					},
		{ (void **) & _pfkey,				"lib3270_pfkey"					},
		{ (void **) & _pakey,				"lib3270_pakey"					},
		{ (void **) & _get_program_message,	"lib3270_get_program_message"	},
		{ (void **) & _mem_free,			"lib3270_free"					}

	};

 	void * (*lib3270_new)(const char *);

	hThread  = NULL;
	hSession = NULL;

	trace("%s",__FUNCTION__);
	hModule = osl_loadModuleAscii("lib3270.so",SAL_LOADMODULE_NOW);
	if(!hModule)
		return;

	for(int f = 0; f < (sizeof (call) / sizeof ((call)[0]));f++)
		*call[f].entry = (void *) osl_getAsciiFunctionSymbol(hModule,call[f].name);

	/* Get lib3270 session handle */
	lib3270_new = (void * (*)(const char *)) osl_getAsciiFunctionSymbol(hModule,"lib3270_session_new");
	hSession = lib3270_new("");

 }

 pw3270::lib3270_session::~lib3270_session()
 {

	trace("%s hModule=%p hSession=%p",__FUNCTION__,hModule,hSession);

	disconnect();
	osl_yieldThread();

	if(hThread)
		osl_joinWithThread(hThread);

	if(hModule)
	{
		if(hSession)
		{
			void (*lib3270_free)(void *) = (void (*)(void *)) osl_getAsciiFunctionSymbol(hModule,"lib3270_session_free");
			lib3270_free(hSession);
			hSession = NULL;
		}
		osl_unloadModule(hModule);
		hModule = NULL;
	}

 }

 int pw3270::lib3270_session::get_revision(void)
 {
	if(!_get_revision)
		return -1;
	return atoi(_get_revision());
 }

 int pw3270::lib3270_session::connect(const char *uri)
 {
 	const char * (*set_host)(void *h, const char *n);

	if(!(hModule && hSession))
		return EINVAL;

	if(hThread)
		return EBUSY;

	set_host = (const char * (*)(void *,const char *)) osl_getAsciiFunctionSymbol(hModule,"lib3270_set_host");
	if(!set_host)
		return EINVAL;

	set_host(hSession,uri);

	enabled = true;
	hThread = osl_createThread((oslWorkerFunction) pw3270::lib3270_session::start_connect, this);

	osl_yieldThread();

	if(!hThread)
		return -1;

	osl_yieldThread();

	return 0;
 }

 int pw3270::lib3270_session::disconnect(void)
 {
 	enabled = false;
	return 0;
 }

 void pw3270::lib3270_session::start_connect(lib3270_session *session)
 {
	session->network_loop();
	session->hThread = NULL;
	session->enabled = false;
 }

 void pw3270::lib3270_session::network_loop(void)
 {
	/* Lib3270 entry points */
	void (* _disconnect)(void *h) =
			(void (*)(void *)) osl_getAsciiFunctionSymbol(hModule,"lib3270_disconnect");

	int  (* _connect)(void *h,const char *n, int wait) =
			(int  (*)(void *,const char *,int)) osl_getAsciiFunctionSymbol(hModule,"lib3270_connect");

	int  (* _status)(void *h) =
			(int  (*)(void *)) osl_getAsciiFunctionSymbol(hModule,"lib3270_disconnected");

	void (*_iterate)(void *h, int wait) =
			(void (*)(void *, int)) osl_getAsciiFunctionSymbol(hModule,"lib3270_main_iterate");

	_connect(hSession,NULL,1);

	while(enabled && !_status(hSession))
	{
		osl_yieldThread();
		_iterate(hSession,1);
	}

	osl_yieldThread();

	_disconnect(hSession);

 }

 bool pw3270::lib3270_session::connected(void)
 {
	return enabled;
 }

 int pw3270::lib3270_session::enter(void)
 {
	if(!hSession)
		return EINVAL;
	return _enter(hSession);
 }

 int pw3270::lib3270_session::pfkey(int key)
 {
	if(!hSession)
		return EINVAL;
	return _pfkey(hSession,key);
 }

 int pw3270::lib3270_session::pakey(int key)
 {
	if(!hSession)
		return EINVAL;
	return _pakey(hSession,key);
 }

 int pw3270::lib3270_session::get_state(void)
 {
	if(!hSession)
		return -1;
	return _get_program_message(hSession);
 }

 void pw3270::lib3270_session::mem_free(void *ptr)
 {
	_mem_free(ptr);
 }

 char * pw3270::lib3270_session::get_text_at(int row, int col, int len)
 {
	if(!hSession)
		return NULL;
	return _get_text_at(hSession,row,col,len);
 }
