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
 * Referencias:
 *
 * * http://www.oorexx.org/docs/rexxpg/x14097.htm
 * * http://www.oorexx.org/docs/rexxpg/c2539.htm
 *
 */

 #define ENABLE_NLS
 #define GETTEXT_PACKAGE PACKAGE_NAME

 #include "rx3270.h"

 #include <libintl.h>
 #include <glib/gi18n.h>
 #include <gtk/gtk.h>

 #include <string.h>
 #include <pw3270.h>
 #include <pw3270/plugin.h>
 #include <pw3270/v3270.h>
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

/*--[ Globals ]--------------------------------------------------------------------------------------*/

 static plugin			* session	= NULL;
 static GMutex			  mutex;

/*--[ Implement ]------------------------------------------------------------------------------------*/

 LIB3270_EXPORT int pw3270_plugin_init(GtkWidget *window)
 {
	g_mutex_init(&mutex);
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
	g_mutex_clear(&mutex);
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

 static void call_rexx_script(GtkAction *action, GtkWidget *widget, const gchar *filename)
 {
	const gchar			* args = (const gchar *) g_object_get_data(G_OBJECT(action),"args");

	RexxInstance 		* instance;
	RexxThreadContext	* threadContext;
	RexxOption			  options[25];

	memset(options,0,sizeof(options));

	if(!RexxCreateInterpreter(&instance, &threadContext, options))
	{
		GtkWidget *dialog = gtk_message_dialog_new(	GTK_WINDOW(gtk_widget_get_toplevel(widget)),
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_ERROR,
													GTK_BUTTONS_CANCEL,
													"%s", _(  "Can't start script" ));

		gtk_window_set_title(GTK_WINDOW(dialog),_( "Rexx error" ));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s",_( "Can't create rexx interpreter instance" ));

        gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);
	}
	else
	{
		RexxArrayObject rxArgs;

		if(args)
		{
			gchar   **arg	= g_strsplit(args,",",-1);
			size_t	  sz	= g_strv_length(arg);

			rxArgs = threadContext->NewArray(sz);
			for(unsigned int i = 0; i<sz; i++)
				threadContext->ArrayPut(rxArgs, threadContext->String(arg[i]), i + 1);

			g_strfreev(arg);
		}
		else
		{
			rxArgs = threadContext->NewArray(1);
			threadContext->ArrayPut(rxArgs, threadContext->String(""),1);
		}

		RexxObjectPtr result = threadContext->CallProgram(filename, rxArgs);

		if (threadContext->CheckCondition())
		{
			RexxCondition condition;

			// retrieve the error information and get it into a decoded form
			RexxDirectoryObject cond = threadContext->GetConditionInfo();
			threadContext->DecodeConditionInfo(cond, &condition);
			// display the errors
			GtkWidget *dialog = gtk_message_dialog_new(	GTK_WINDOW(gtk_widget_get_toplevel(widget)),
														GTK_DIALOG_DESTROY_WITH_PARENT,
														GTK_MESSAGE_ERROR,
														GTK_BUTTONS_CANCEL,
														"%s", _(  "Script error" ));

			gtk_window_set_title(GTK_WINDOW(dialog),_( "System busy" ));

			gtk_message_dialog_format_secondary_text(
					GTK_MESSAGE_DIALOG(dialog),
							_( "Rexx error %d: %s\n%s" ),
									(int) condition.code,
									threadContext->CString(condition.errortext),
									threadContext->CString(condition.message)

			);

			gtk_dialog_run(GTK_DIALOG (dialog));
			gtk_widget_destroy(dialog);

		}
		else if (result != NULLOBJECT)
        {
            CSTRING resultString = threadContext->CString(result);
			lib3270_write_log(NULL,"REXX","%s exits with rc=%s",filename,resultString);
        }

		instance->Terminate();
	}

 }


extern "C"
{
 LIB3270_EXPORT void pw3270_action_rexx_activated(GtkAction *action, GtkWidget *widget)
 {
	gchar *filename = (gchar *) g_object_get_data(G_OBJECT(action),"src");

	lib3270_trace_event(v3270_get_session(widget),"Action %s activated on widget %p",gtk_action_get_name(action),widget);

	if(!g_mutex_trylock(&mutex))
	{
		GtkWidget *dialog = gtk_message_dialog_new(	GTK_WINDOW(gtk_widget_get_toplevel(widget)),
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_ERROR,
													GTK_BUTTONS_CANCEL,
													"%s", _(  "Can't start script" ));

		gtk_window_set_title(GTK_WINDOW(dialog),_( "System busy" ));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s",_( "Please, try again in a few moments" ));

        gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);
		return;
	}

	gtk_action_set_sensitive(action,FALSE);

	if(filename)
	{
		// Has filename, call it directly
		call_rexx_script(action,widget,filename);
	}
	else
	{
		// No filename, ask user
		static const struct _list
		{
			const gchar *name;
			const gchar *pattern;
		} list[] =
		{
			{ N_( "Rexx script file" ),	"*.rex" },
			{ N_( "Rexx class file" ),	"*.cls" }
		};

		GtkFileFilter * filter[G_N_ELEMENTS(list)+1];
		unsigned int f;

		memset(filter,0,sizeof(filter));

		for(f=0;f<G_N_ELEMENTS(list);f++)
		{
			filter[f] = gtk_file_filter_new();
			gtk_file_filter_set_name(filter[f],gettext(list[f].name));
			gtk_file_filter_add_pattern(filter[f],list[f].pattern);
		}

		filename = pw3270_get_filename(widget,"rexx","script",filter,_( "Select Rexx script to run" ));

		if(filename)
		{
			call_rexx_script(action,widget,filename);
			g_free(filename);
		}

	}

	gtk_action_set_sensitive(action,TRUE);
	g_mutex_unlock(&mutex);

 }

}

