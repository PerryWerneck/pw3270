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
 * Este programa está nomeado como pluginmain.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "rx3270.h"
 #include <string.h>
 #include <pw3270/plugin.h>
 #include <lib3270/actions.h>
 #include <lib3270/log.h>

/*--[ Plugin session object ]--------------------------------------------------------------------------------*/

 class plugin : public rx3270
 {
 public:
	plugin(H3270 *hSession);

	char			* get_version(void);
	LIB3270_CSTATE	  get_cstate(void);
	int				  disconnect(void);
	int				  connect(const char *uri, bool wait = true);
	bool			  is_connected(void);
	bool			  is_ready(void);

	void 			  logva(const char *fmt, va_list args);

	int				  iterate(bool wait);
	int				  wait(int seconds);
	int				  wait_for_ready(int seconds);

	char			* get_text(int baddr, size_t len);
	char 			* get_text_at(int row, int col, size_t sz);
	int				  cmp_text_at(int row, int col, const char *text);
	int 			  set_text_at(int row, int col, const char *str);

	int				  set_cursor_position(int row, int col);

	int 			  set_toggle(LIB3270_TOGGLE ix, bool value);

	int				  enter(void);
	int				  pfkey(int key);
	int				  pakey(int key);

 private:
	H3270 *hSession;

 };

 static plugin * session = NULL;

/*--[ Implement ]------------------------------------------------------------------------------------*/

 LIB3270_EXPORT int pw3270_plugin_init(GtkWidget *window)
 {
	session = new plugin(lib3270_get_default_session_handle());
	session->set_plugin();
	trace("%s: Rexx object is %p",__FUNCTION__,session);
	return 0;
 }

 LIB3270_EXPORT int pw3270_plugin_deinit(GtkWidget *window)
 {
	if(session)
	{
		delete session;
		session = NULL;
	}
	return 0;
 }

 plugin::plugin(H3270 *hSession) : rx3270()
 {
	this->hSession = hSession;
 }

 char * plugin::get_version(void)
 {
	return strdup(lib3270_get_version());
 }

 LIB3270_CSTATE plugin::get_cstate(void)
 {
 	return lib3270_get_connection_state(hSession);
 }

 int plugin::disconnect(void)
 {
	lib3270_disconnect(hSession);
	return 0;
 }

 int plugin::connect(const char *uri, bool wait)
 {
 	return lib3270_connect(hSession,uri,wait);
 }

 bool plugin::is_connected(void)
 {
 	return lib3270_is_connected(hSession) != 0;
 }

 int plugin::iterate(bool wait)
 {
	if(!lib3270_is_connected(hSession))
		return ENOTCONN;

	lib3270_main_iterate(hSession,wait);

	return 0;
 }

 int plugin::wait(int seconds)
 {
	return lib3270_wait(hSession,seconds);
 }

 int plugin::enter(void)
 {
	return lib3270_enter(hSession);
 }

 int plugin::pfkey(int key)
 {
	return lib3270_pfkey(hSession,key);
 }

 int plugin::pakey(int key)
 {
	return lib3270_pakey(hSession,key);
 }

 int plugin::wait_for_ready(int seconds)
 {
	return lib3270_wait_for_ready(hSession,seconds);
 }

 char * plugin::get_text_at(int row, int col, size_t sz)
 {
	return lib3270_get_text_at(hSession,row,col,(int) sz);
 }

 int plugin::cmp_text_at(int row, int col, const char *text)
 {
	return lib3270_cmp_text_at(hSession,row,col,text);
 }

 int plugin::set_text_at(int row, int col, const char *str)
 {
	return lib3270_set_text_at(hSession,row,col,(const unsigned char *) str);
 }

 bool plugin::is_ready(void)
 {
	return lib3270_is_ready(hSession) != 0;
 }

 int plugin::set_cursor_position(int row, int col)
 {
	return lib3270_set_cursor_position(hSession,row,col);
 }

 int plugin::set_toggle(LIB3270_TOGGLE ix, bool value)
 {
	return lib3270_set_toggle(hSession,ix,(int) value);
 }

 void plugin::logva(const char *fmt, va_list args)
 {
	lib3270_write_va_log(hSession,"REXX",fmt,args);
 }

 char * plugin::get_text(int baddr, size_t len)
 {
	return lib3270_get_text(hSession,baddr,len);
 }
