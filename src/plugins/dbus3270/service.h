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
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como service.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

#ifndef _PW3270_DBUS_SERVICE_H

	#define _PW3270_DBUS_SERVICE_H 1

	#define ENABLE_NLS
	#define GETTEXT_PACKAGE PACKAGE_NAME

	#include "globals.h"

	#define PW3270_TYPE_DBUS			(pw3270_dbus_get_type ())
	#define PW3270_DBUS(object)			(G_TYPE_CHECK_INSTANCE_CAST ((object), PW3270_TYPE_DBUS, PW3270Dbus))
	#define PW3270_DBUS_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST ((klass), PW3270_TYPE_DBUS, PW3270DbusClass))
	#define IS_PW3270_DBUS(object)		(G_TYPE_CHECK_INSTANCE_TYPE ((object), PW3270_TYPE_DBUS))
	#define IS_PW3270_DBUS_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), PW3270_TYPE_DBUS))
	#define PW3270_DBUS_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), PW3270_TYPE_DBUS, PW3270DbusClass))

	G_BEGIN_DECLS

	typedef struct _PW3270Dbus		PW3270Dbus;
	typedef struct _PW3270DbusClass	PW3270DbusClass;

	struct _PW3270Dbus
	{
			GObject parent;
	};

	struct _PW3270DbusClass
	{
			GObjectClass parent;
	};

	PW3270Dbus	* pw3270_dbus_new (void);
	GType 		  pw3270_dbus_get_type (void);

	void		  pw3270_dbus_get_revision(PW3270Dbus *object, DBusGMethodInvocation *context);
	void		  pw3270_dbus_quit(PW3270Dbus *object, DBusGMethodInvocation *context);
	void		  pw3270_dbus_connect(PW3270Dbus *object, const gchar *uri, DBusGMethodInvocation *context);
	void		  pw3270_dbus_get_ur_l(PW3270Dbus *object, DBusGMethodInvocation *context);
	void		  pw3270_dbus_set_ur_l(PW3270Dbus *object, const gchar *uri, DBusGMethodInvocation *context);
	void		  pw3270_dbus_disconnect(PW3270Dbus *object, DBusGMethodInvocation *context);

	void		  pw3270_dbus_get_message_id(PW3270Dbus *object, DBusGMethodInvocation *context);
	void		  pw3270_dbus_get_connection_state(PW3270Dbus *object, DBusGMethodInvocation *context);
	void		  pw3270_dbus_get_secure_state(PW3270Dbus *object, DBusGMethodInvocation *context);

	void		  pw3270_dbus_get_screen_contents(PW3270Dbus *object, DBusGMethodInvocation *context);
	H3270		* pw3270_dbus_get_session_handle(PW3270Dbus *object);
	GError		* pw3270_dbus_get_error_from_errno(int code);

	void		  pw3270_dbus_is_connected(PW3270Dbus *object, DBusGMethodInvocation *context);
	void		  pw3270_dbus_is_ready(PW3270Dbus *object, DBusGMethodInvocation *context);
	void		  pw3270_dbus_in_tn3270_e(PW3270Dbus *object, DBusGMethodInvocation *context);

	void		  pw3270_dbus_set_cursor_at(PW3270Dbus *object, int row, int col, DBusGMethodInvocation *context);
	void		  pw3270_dbus_set_cursor_address(PW3270Dbus *object, int addr, DBusGMethodInvocation *context);
	void		  pw3270_dbus_get_cursor_address(PW3270Dbus *object, DBusGMethodInvocation *context);

	void		  pw3270_dbus_get_screen_width(PW3270Dbus *object, DBusGMethodInvocation *context);
	void		  pw3270_dbus_get_screen_height(PW3270Dbus *object, DBusGMethodInvocation *context);
	void		  pw3270_dbus_get_screen_length(PW3270Dbus *object, DBusGMethodInvocation *context);

	void		  pw3270_dbus_set_toggle(PW3270Dbus *object, int id, int value, DBusGMethodInvocation *context);

	void		  pw3270_dbus_wait_for_ready(PW3270Dbus *object, int timeout, DBusGMethodInvocation *context);

	void		  pw3270_dbus_get_field_start(PW3270Dbus *object, int baddr, DBusGMethodInvocation *context);
	void		  pw3270_dbus_get_field_length(PW3270Dbus *object, int baddr, DBusGMethodInvocation *context);
	void			pw3270_dbus_get_next_unprotected(PW3270Dbus *object, int baddr, DBusGMethodInvocation *context);

	void		  pw3270_dbus_get_is_protected(PW3270Dbus *object, int baddr, DBusGMethodInvocation *context);
	void		  pw3270_dbus_get_is_protected_at(PW3270Dbus *object, int row, int col, DBusGMethodInvocation *context);

	void 		  pw3270_dbus_set_script(PW3270Dbus *object, const gchar *text, int mode, DBusGMethodInvocation *context);

	void		  pw3270_dbus_show_popup(PW3270Dbus *object, int id, const gchar *title, const gchar *msg, const gchar *text, DBusGMethodInvocation *context);

	void		  pw3270_dbus_action(PW3270Dbus *object, const gchar *text, DBusGMethodInvocation *context);

	// Actions
	void		  pw3270_dbus_enter(PW3270Dbus *object, DBusGMethodInvocation *context);
	void		  pw3270_dbus_pf_key(PW3270Dbus *object, int key, DBusGMethodInvocation *context);
	void		  pw3270_dbus_pa_key(PW3270Dbus *object, int key, DBusGMethodInvocation *context);
	void		  pw3270_dbus_set_text_at(PW3270Dbus *object, int row, int col, const gchar *text, DBusGMethodInvocation *context);
	void		  pw3270_dbus_get_text_at(PW3270Dbus *object, int row, int col, int len, char lf, DBusGMethodInvocation *context);
	void		  pw3270_dbus_get_text(PW3270Dbus *object, int offset, int len, char lf, DBusGMethodInvocation *context);
	void		  pw3270_dbus_cmp_text_at(PW3270Dbus *object, int row, int col, const gchar *text, char lf, DBusGMethodInvocation *context);
    void          pw3270_dbus_input(PW3270Dbus *object, const gchar *utftext, DBusGMethodInvocation *context);

	void		  pw3270_dbus_set_clipboard(PW3270Dbus *object, const gchar *text, DBusGMethodInvocation *context);
	void		  pw3270_dbus_get_clipboard(PW3270Dbus *object, DBusGMethodInvocation *context);

	void		  pw3270_dbus_get_display_charset(PW3270Dbus *object, DBusGMethodInvocation *context);
	void		  pw3270_dbus_get_host_charset(PW3270Dbus *object, DBusGMethodInvocation *context);
	void		  pw3270_dbus_set_host_charset(PW3270Dbus *object, const gchar *charset, DBusGMethodInvocation *context);
	void		  pw3270_dbus_erase_eof(PW3270Dbus *object, DBusGMethodInvocation *context);
	void		  pw3270_dbus_print(PW3270Dbus *object, DBusGMethodInvocation *context);

	void		  pw3270_dbus_asc2ebc(PW3270Dbus *object, const gchar *from, DBusGMethodInvocation *context);
	void		  pw3270_dbus_ebc2asc(PW3270Dbus *object, const gchar *from, DBusGMethodInvocation *context);

	void		  pw3270_dbus_filetransfer(PW3270Dbus *object, const gchar *local, const gchar *remote, int flags, int lrecl, int blksize, int primspace, int secspace, int dft, DBusGMethodInvocation *context);

	void 		  pw3270_dbus_set_unlock_delay(PW3270Dbus *object, int value, DBusGMethodInvocation *context);

	G_END_DECLS

#endif // _PW3270_DBUS_SERVICE_H
