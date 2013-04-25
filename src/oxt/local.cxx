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
 #include <string.h>

 #ifdef HAVE_SYSLOG
	#include <syslog.h>
	 #include <stdarg.h>
 #endif // HAVE_SYSLOG

/*
 * NOTE:	Take a better look at osl_createEmptySocketAddr() & osl_connectSocketTo() to see if there's
 *			a way to use this calls to connect with the host for better performance.
 */

/*---[ Statics ]-------------------------------------------------------------------------------------------*/


/*---[ Implement ]-----------------------------------------------------------------------------------------*/

 static void loghandler(H3270 *session, const char *module, int rc, const char *fmt, va_list args)
 {
#ifdef HAVE_SYSLOG
	openlog(PACKAGE_NAME, LOG_NDELAY, LOG_USER);
	vsyslog(LOG_INFO,fmt,args);
	closelog();
#endif // HAVE_SYSLOG
 }

 static void tracehandler(H3270 *session, const char *fmt, va_list args)
 {
#ifdef HAVE_SYSLOG

	#define MAX_LOG_LENGTH 200

	static char	  line[MAX_LOG_LENGTH+1];
	char 		  temp[MAX_LOG_LENGTH];
	char		* ptr;
	size_t		  len = strlen(line);

	vsnprintf(temp,MAX_LOG_LENGTH-len,fmt,args);

	ptr = strchr(temp,'\n');
	if(!ptr)
	{
		strncat(line,temp,MAX_LOG_LENGTH);
		if(strlen(line) >= MAX_LOG_LENGTH)
		{
			openlog(PACKAGE_NAME, LOG_NDELAY, LOG_USER);
			syslog(LOG_INFO,line);
			closelog();
			*line = 0;
		}
		return;
	}

	*ptr = 0;
	strncat(line,temp,MAX_LOG_LENGTH);

	openlog(PACKAGE_NAME, LOG_NDELAY, LOG_USER);
	syslog(LOG_DEBUG,line);
	closelog();

	strncpy(line,ptr+1,MAX_LOG_LENGTH);

#endif // HAVE_SYSLOG
 }

 pw3270::lib3270_session::lib3270_session(uno_impl *obj) throw( RuntimeException )
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
		{ (void **) & _in_tn3270e,			"lib3270_in_tn3270e"			},
		{ (void **) & _get_program_message,	"lib3270_get_program_message"	},
		{ (void **) & _mem_free,			"lib3270_free"					},
		{ (void **) & _set_toggle,			"lib3270_set_toggle"			}

	};

 	H3270 * (*lib3270_new)(const char *);
 	void	(*set_log_handler)(void (*loghandler)(H3270 *, const char *, int, const char *, va_list));
	void 	(*set_trace_handler)( void (*handler)(H3270 *session, const char *fmt, va_list args) );

	hThread  = NULL;
	hSession = NULL;

	hModule = osl_loadModuleAscii("lib3270.so." PACKAGE_VERSION,SAL_LOADMODULE_NOW);
	trace("%s: hModule(lib3270.so." PACKAGE_VERSION ")=%p",__FUNCTION__,hModule);

	if(!hModule)
	{
		hModule = osl_loadModuleAscii("lib3270.so." PACKAGE_VERSION,SAL_LOADMODULE_NOW);
		trace("%s: hModule(lib3270.so)=%p",__FUNCTION__,hModule);
	}

	if(!hModule)
	{
		obj->failed("%s","Can't load lib3270");
		return;
	}

	for(int f = 0; f < (sizeof (call) / sizeof ((call)[0]));f++)
	{
		*call[f].entry = (void *) osl_getAsciiFunctionSymbol(hModule,call[f].name);
		if(!*call[f].entry)
			obj->failed("Error loading lib3270::%s",call[f].name);
	}

	/* Get lib3270 session handle */
	set_log_handler   = (void (*)(void (*loghandler)(H3270 *, const char *, int, const char *, va_list))) osl_getAsciiFunctionSymbol(hModule,"lib3270_set_log_handler");
	set_trace_handler = (void (*)(void (*handler)(H3270 *session, const char *fmt, va_list args) )) osl_getAsciiFunctionSymbol(hModule,"lib3270_set_trace_handler");

	if(set_log_handler)
		set_log_handler(loghandler);

	if(set_trace_handler)
		set_trace_handler(tracehandler);

	lib3270_new = (H3270 * (*)(const char *)) osl_getAsciiFunctionSymbol(hModule,"lib3270_session_new");
	hSession = lib3270_new("");

	log("%s UNO extension loaded",PACKAGE_NAME);
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

	log("%s UNO extension unloaded",PACKAGE_NAME);
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

	trace("%s starts",__FUNCTION__);
	_connect(hSession,NULL,1);

	trace("%s network loop begin",__FUNCTION__);
	while(enabled && !_status(hSession))
	{
		osl_yieldThread();
		_iterate(hSession,1);
	}
	trace("%s network loop ends",__FUNCTION__);

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

 LIB3270_MESSAGE pw3270::lib3270_session::get_state(void)
 {
	if(!hSession)
		return LIB3270_MESSAGE_DISCONNECTED;
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

 int pw3270::lib3270_session::set_text_at(int row, int col, const char *text)
 {
	if(!hSession)
		return EINVAL;
	return _set_text_at(hSession,row,col,(const unsigned char *) text);
 }

 int pw3270::lib3270_session::cmp_text_at(int row, int col,const char *text)
 {
	if(!hSession)
		return EINVAL;
	return _cmp_text_at(hSession,row,col,text);
 }

 bool pw3270::lib3270_session::in_tn3270e(void)
 {
	if(!hSession)
		return false;
	return _in_tn3270e(hSession) != 0;
 }

 void pw3270::lib3270_session::set_toggle(LIB3270_TOGGLE toggle, bool state)
 {
	if(hSession)
		_set_toggle(hSession,toggle,(int) state);
 }
