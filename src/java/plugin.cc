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
 * Este programa está nomeado como plugin.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "private.h"

 #include <libintl.h>
 #include <glib/gi18n.h>
 #include <gtk/gtk.h>

 #include <pw3270.h>
 #include <pw3270/plugin.h>
 #include <pw3270/v3270.h>
 #include <lib3270/actions.h>
 #include <lib3270/log.h>
 #include <lib3270/trace.h>
 #include <lib3270/charset.h>
 #include <pw3270/class.h>


/*--[ Globals ]--------------------------------------------------------------------------------------*/

#if GTK_CHECK_VERSION(2,32,0)
 static GMutex            mutex;
#else
 static GStaticMutex	  mutex = G_STATIC_MUTEX_INIT;
#endif // GTK_CHECK_VERSION

/*--[ Plugin session object ]--------------------------------------------------------------------------------*/

 class plugin : public PW3270_NAMESPACE::session
 {
 private:
	H3270           * hSession;

 public:
	plugin(H3270 *hSession) : PW3270_NAMESPACE::session() {
		this->hSession = hSession;
	}

	virtual ~plugin() {
		trace("%s",__FUNCTION__);
	}

	const string get_version(void) {
		return string(lib3270_get_version());
	}

	LIB3270_CSTATE get_cstate(void) {
		return lib3270_get_connection_state(hSession);
	}

	int disconnect(void) {
		lib3270_disconnect(hSession);
		return 0;
	}

	int connect(void) {
		return lib3270_connect(hSession,0);
	}

	bool is_connected(void) {
		return lib3270_is_connected(hSession);
	}

	int	iterate(bool wait) {
		if(!lib3270_is_connected(hSession))
			return ENOTCONN;

		lib3270_main_iterate(hSession,wait);

		return 0;

	}

	int	wait(int seconds) {
		return lib3270_wait(hSession,seconds);
	}

	int wait_for_ready(int seconds) {
		return lib3270_wait_for_ready(hSession,seconds);
	}

	bool is_ready(void) {
		return lib3270_is_ready(hSession) != 0;
	}

	void logva(const char *fmt, va_list args) {
		lib3270_write_va_log(hSession,"JAVA",fmt,args);
	}

	string get_text(int baddr, size_t len) {

		string	  rc;
		char	* ptr = lib3270_get_text(hSession,baddr,len);

		if(ptr)
		{
			rc.assign(ptr);
			lib3270_free(ptr);
		}

		return rc;
	}

	string get_text_at(int row, int col, size_t sz) {

		string 	  rc;
		char	* ptr = lib3270_get_text_at(hSession,row,col,(int) sz);

		if(ptr)
		{
			rc.assign(ptr);
			lib3270_free(ptr);
		}

		return rc;

	}

	int cmp_text_at(int row, int col, const char *text) {
		return lib3270_cmp_text_at(hSession,row,col,text);
	}

	int set_text_at(int row, int col, const char *str) {
		return lib3270_set_text_at(hSession,row,col,(const unsigned char *) str);
	}

	int	set_cursor_position(int row, int col) {
		return lib3270_set_cursor_position(hSession,row,col);
	}

	int	set_cursor_addr(int addr) {
		return lib3270_set_cursor_address(hSession,addr);
	}

	int	get_cursor_addr(void) {
		return lib3270_get_cursor_address(hSession);
	}

    int emulate_input(const char *str) {
		return lib3270_emulate_input(hSession, str, -1, 1);
    }

	int set_toggle(LIB3270_TOGGLE ix, bool value) {
		return lib3270_set_toggle(hSession,ix,(int) value);
	}

	int	enter(void) {
		return lib3270_enter(hSession);
	}

	int pfkey(int key) {
		return lib3270_pfkey(hSession,key);
	}

	int pakey(int key) {
		return lib3270_pakey(hSession,key);
	}

	int erase_eof(void) {
		return lib3270_eraseeof(hSession);
	}

	int print(void) {
		return lib3270_print(hSession);
	}

	int get_field_start(int baddr = -1) {
		return lib3270_get_field_start(hSession,baddr);
	}

	int get_field_len(int baddr = -1) {
		return lib3270_get_field_len(hSession,baddr);
	}

	int get_next_unprotected(int baddr = -1) {
		return lib3270_get_next_unprotected(hSession,baddr);
	}

	int set_copy(const char *text) {
		v3270_set_copy(GTK_WIDGET(lib3270_get_user_data(hSession)),text);
		return 0;
	}

	string get_copy(void) {

		string	  rc;
		gchar	* ptr = v3270_get_copy(GTK_WIDGET(lib3270_get_user_data(hSession)));

		if(ptr)
		{
			rc.assign(ptr);
			g_free(ptr);
		}

		return rc;

	}

    string get_clipboard(void) {

		string	  rc;
		gchar	* ptr = gtk_clipboard_wait_for_text(gtk_widget_get_clipboard(pw3270_get_toplevel(),GDK_SELECTION_CLIPBOARD));

		if(ptr)
		{
			rc.assign(ptr);
			g_free(ptr);
		}

		return rc;

    }

    int set_clipboard(const char *text) {
		gtk_clipboard_set_text(gtk_widget_get_clipboard(pw3270_get_toplevel(),GDK_SELECTION_CLIPBOARD),(gchar *) text, -1);
		return 0;
    }

	void free(void *ptr) {
		g_free(ptr);
	}

	int popup_dialog(LIB3270_NOTIFY id , const char *title, const char *message, const char *fmt, ...) {
		va_list	args;
		va_start(args, fmt);
		lib3270_popup_va(hSession, id, title, message, fmt, args);
		va_end(args);
		return 0;
	}

	string file_chooser_dialog(GtkFileChooserAction action, const char *title, const char *extension, const char *filename) {
		string	  rc;
		gchar	* ptr = pw3270_file_chooser(action, "java", title, filename, extension);

		if(ptr)
		{
			rc.assign((char *) ptr);
			g_free(ptr);
		}

		return rc;
	}

	int set_host_charset(const char *charset) {
		return lib3270_set_host_charset(hSession,charset);
	}

	string get_host_charset(void) {
		return string(lib3270_get_host_charset(hSession));
	}

	string get_display_charset(void) {
		return string(lib3270_get_display_charset(hSession));
	}

	const char * asc2ebc(unsigned char *str, int sz = -1) {
		return lib3270_asc2ebc(hSession,str,sz);
	}

	const char * ebc2asc(unsigned char *str, int sz = -1) {
		return lib3270_ebc2asc(hSession,str,sz);
	}

	int set_url(const char *uri) {
		return lib3270_set_url(hSession,uri) != NULL ? 1 : 0;
	}

	int file_transfer(LIB3270_FT_OPTION options, const gchar *local, const gchar *remote, int lrecl = 0, int blksize = 0, int primspace = 0, int secspace = 0, int dft = 4096) {
		return v3270_transfer_file(v3270_get_default_widget(),options,local,remote,lrecl,blksize,primspace,secspace,dft);
	}

    int quit(void) {
		gtk_main_quit();
		return 0;
    }

 };


/*---[ Implement ]----------------------------------------------------------------------------------*/

 static session * factory(const char *name)
 {
    return new plugin(lib3270_get_default_session_handle());
 }

 LIB3270_EXPORT int pw3270_plugin_start(GtkWidget *window)
 {
	trace("%s",__FUNCTION__);
#if GTK_CHECK_VERSION(2,32,0)
	g_mutex_init(&mutex);
#endif // GTK_CHECK_VERSION
	set_java_session_factory(factory);
	return 0;
 }

 LIB3270_EXPORT int pw3270_plugin_stop(GtkWidget *window)
 {
#if GTK_CHECK_VERSION(2,32,0)
	g_mutex_clear(&mutex);
#endif // GTK_CHECK_VERSION
    trace("%s",__FUNCTION__);
	return 0;
 }

