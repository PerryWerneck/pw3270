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
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como remotectl.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 * Agradecimento:
 *
 * Hélio Passos
 *
 */

 #define ENABLE_NLS
 #define GETTEXT_PACKAGE PACKAGE_NAME

 #include <libintl.h>
 #include <glib/gi18n.h>
 #include <gtk/gtk.h>

 #include <lib3270.h>
 #include <lib3270/log.h>
 #include <pw3270/hllapi.h>

 typedef struct _remotequery
 {
#ifdef _WIN32
	HANDLE			  hPipe;	/**< Pipe handle (for response) */
#endif // _WIN32

	H3270 			* hSession;	/**< 3270 Session */
 	int				  cmd;		/**< Command */
	int 			  rc;		/**< Response status */

	int 			  pos;
	unsigned short	  length;	/**< Query string length */
 	const gchar		* text;		/**< Query string */

 } QUERY;

 G_GNUC_INTERNAL void enqueue_request(QUERY *qry);
 G_GNUC_INTERNAL void request_complete(QUERY *qry, int rc, const gchar *text);

 G_GNUC_INTERNAL void request_status(QUERY *qry, int rc);
 G_GNUC_INTERNAL void request_value(QUERY *qry,  int rc, unsigned int value);
 G_GNUC_INTERNAL void request_buffer(QUERY *qry, int rc, size_t sz, const gpointer buffer);

// int run_hllapi(unsigned long function, char *string, unsigned short length, unsigned short rc);


#ifdef _WIN32

	#define PIPE_BUFFER_LENGTH 8192

	void init_source_pipe(HANDLE hPipe);
	void popup_lasterror(const gchar *fmt, ...);

#endif // _WIN32






