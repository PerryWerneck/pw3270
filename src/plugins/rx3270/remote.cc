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
 * Este programa está nomeado como local.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "rx3270.h"

#if defined(HAVE_DBUS)
	#include <stdio.h>
	#include <dbus/dbus.h>
#endif // HAVE_DBUS

 #include <string.h>

/*--[ Class definition ]-----------------------------------------------------------------------------*/

 class remote : public rx3270
 {
 public:
	remote(const char *session);
	~remote();

	const char		* get_version(void);
	LIB3270_CSTATE	  get_cstate(void);
	int				  disconnect(void);
	int				  connect(const char *uri, bool wait = true);
	bool			  is_connected(void);
	bool			  is_ready(void);

	int				  iterate(bool wait);
	int				  wait(int seconds);
	int				  wait_for_ready(int seconds);

	char 			* get_text_at(int row, int col, size_t sz);
	int				  cmp_text_at(int row, int col, const char *text);
	int 			  set_text_at(int row, int col, const char *str);

	int				  set_cursor_position(int row, int col);

	void 			  set_toggle(LIB3270_TOGGLE ix, bool value);

	int				  enter(void);
	int				  pfkey(int key);
	int				  pakey(int key);

 private:
#if defined(WIN32)

#elif defined(HAVE_DBUS)
	DBusConnection	* conn;
	char			* service_name;
	char			* interface_name;
	DBusMessage		* create_message(const char *method);

#endif


 };

/*--[ Globals ]--------------------------------------------------------------------------------------*/

#if defined(HAVE_DBUS)
 static const char * prefix	= "br.com.bb.";
 static const char * object	= "br/com/bb/" PACKAGE_NAME;
#else
 #error AQUI
#endif // HAVE_DBUS

/*--[ Implement ]------------------------------------------------------------------------------------*/

#if defined(HAVE_DBUS)
DBusMessage * remote::create_message(const char *method)
{
	DBusMessage * msg = dbus_message_new_method_call(	service_name,
														object,
														interface_name,
														method);

	if (!msg)
		fprintf(stderr, "Error creating message for method %s\n",method);

	return msg;
}
#endif // HAVE_DBUS


rx3270 * rx3270::create_remote(const char *name)
{
	return new remote(name);
}


remote::remote(const char *name)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)
	DBusError	  err;
	int			  rc;
	char		* str = strdup(name);
	char		* ptr = strchr(str,':');

	if(ptr)
	{
		size_t 		  sz;

		*(ptr++) = 0;

		sz = strlen(ptr)+strlen(name)+strlen(prefix)+2;

		service_name = (char *) malloc(sz+1);
		strncpy(service_name,prefix,sz);
		strncat(service_name,".",sz);
		strncat(service_name,name,sz);
		strncat(service_name,".",sz);
		strncat(service_name,ptr,sz);

		sz = strlen(prefix)+strlen(name)+1;
		interface_name = (char *) malloc(sz+1);
		strncpy(interface_name,prefix,sz);
		strncat(interface_name,".",sz);
		strncat(interface_name,name,sz);
	}
	else
	{
		size_t sz = strlen(name)+strlen(prefix)+1;

		service_name = (char *) malloc(sz+1);
		strncpy(service_name,prefix,sz);
		strncat(service_name,".",sz);
		strncat(service_name,name,sz);

		sz = strlen(prefix)+strlen(name)+1;
		interface_name = (char *) malloc(sz+1);
		strncpy(interface_name,prefix,sz);
		strncat(interface_name,".",sz);
		strncat(interface_name,name,sz);

	}

	trace("service_name:   [%s]", service_name);
	trace("interface_name: [%s]", interface_name);

	free(str);

	dbus_error_init(&err);

	conn = dbus_bus_get(DBUS_BUS_SESSION, &err);

	if (dbus_error_is_set(&err))
	{
		fprintf(stderr, "DBUS Connection Error (%s)\n", err.message);
		dbus_error_free(&err);
	}

	if(!conn)
	{
		fprintf(stderr, "%s\n", "DBUS Connection failed");
		dbus_connection_close(conn);
		conn = NULL;
		return;
	}

	rc = dbus_bus_request_name(conn, "br.com.bb." PACKAGE_NAME ".rexx", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);

	if (dbus_error_is_set(&err))
	{
		fprintf(stderr, "Name Error (%s)\n", err.message);
		dbus_error_free(&err);
	}

	if(rc != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
	{
		fprintf(stderr, "%s\n", "DBUS request name failed");
		dbus_connection_close(conn);
		conn = NULL;
		return;
	}


#else

#endif
}

remote::~remote()
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

	if(conn)
		dbus_connection_close(conn);

	free(service_name);
	free(interface_name);

#else

#endif
}

const char * remote::get_version(void)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return NULL;
}

LIB3270_CSTATE remote::get_cstate(void)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return (LIB3270_CSTATE) -1;

}

int remote::disconnect(void)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return -1;
}

int remote::connect(const char *uri, bool wait)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return -1;
}

bool remote::is_connected(void)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return false;
}

bool remote::is_ready(void)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return false;
}

int remote::iterate(bool wait)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return -1;
}

int remote::wait(int seconds)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return -1;
}

int remote::wait_for_ready(int seconds)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return -1;
}

char * remote::get_text_at(int row, int col, size_t sz)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return NULL;
}

int remote::cmp_text_at(int row, int col, const char *text)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return 0;
}

int remote::set_text_at(int row, int col, const char *str)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return -1;
}

int remote::set_cursor_position(int row, int col)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return -1;
}

int remote::enter(void)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return -1;
}

int remote::pfkey(int key)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return -1;
}

int remote::pakey(int key)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif

	return -1;
}

void remote::set_toggle(LIB3270_TOGGLE ix, bool value)
{
#if defined(WIN32)

#elif defined(HAVE_DBUS)

#endif
}
