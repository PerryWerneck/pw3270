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
 * Este programa está nomeado como session.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

 #ifndef LIB3270_SESSION_H_INCLUDED

	#define LIB3270_SESSION_H_INCLUDED 1
	#include <sys/time.h>

	#define LIB3270_LUNAME_LENGTH			16
	#define LIB3270_FULL_MODEL_NAME_LENGTH	13

	#define LIB3270_TELNET_N_OPTS			256

	/**  extended attributes */
	struct ea
	{
		unsigned char cc;		/**< EBCDIC or ASCII character code */
		unsigned char fa;		/**< field attribute, it nonzero */
		unsigned char fg;		/**< foreground color (0x00 or 0xf<n>) */
		unsigned char bg;		/**< background color (0x00 or 0xf<n>) */
		unsigned char gr;		/**< ANSI graphics rendition bits */
		unsigned char cs;		/**< character set (GE flag, or 0..2) */
		unsigned char ic;		/**< input control (DBCS) */
		unsigned char db;		/**< DBCS state */
	};

	struct lib3270_text
	{
		unsigned char  chr;		/**< ASCII character code */
		unsigned short attr;	/**< Converted character attribute (color & etc) */
	};

	struct lib3270_toggle
	{
		char value;				/**< toggle value */
		void (*upcall)(H3270 *, struct lib3270_toggle *, LIB3270_TOGGLE_TYPE); /**< change value */
	};

#ifndef HEADER_SSL_H
	#define SSL void
#endif // !HEADER_SSL_H

	struct _h3270
	{
		unsigned short 		  	  sz;					/**< Struct size */

		// Connection info
		int						  sock;					/**< Network socket */
		#ifdef _WIN32
			HANDLE				  sockEvent;			/**< Event semaphore */
		#endif // WIN32
		LIB3270_CSTATE			  cstate;				/**< Connection state */

		// flags
		int						  bgthread					: 1;	/**< Running on a background thread ? */
		int					  	  selected					: 1;	/**< Has selected region? */
		int						  rectsel					: 1;	/**< Selected region is a rectangle ? */
		int						  vcontrol					: 1;	/**< Visible control ? */
		int						  modified_sel				: 1;
		int						  mono						: 1;	/**< Forces monochrome display */
		int						  m3279						: 1;
		int 					  extended					: 1;
		int						  typeahead					: 1;
		int						  numeric_lock				: 1;
		int						  oerr_lock					: 1;
		int						  unlock_delay				: 1;
		int			 			  auto_reconnect_inprogress	: 1;
		int						  color8					: 1;
		int						  apl_mode					: 1;
		int						  icrnl						: 1;
		int						  inlcr						: 1;
		int						  onlcr						: 1;
		int						  bsd_tm					: 1;
		int 					  syncing					: 1;
		int						  reverse 					: 1;	/**< reverse-input mode */
		int						  dbcs						: 1;

		char					* oversize;

		LIB3270_SSL_STATE		  secure;

		struct lib3270_toggle	  toggle[LIB3270_TOGGLE_COUNT];

		// Network & Termtype
		char   		 			* hostname;
		char					* connected_type;
		char					* connected_lu;
		char					  luname[LIB3270_LUNAME_LENGTH+1];

		char					  full_model_name[LIB3270_FULL_MODEL_NAME_LENGTH+1];
		char					* model_name;
		int						  model_num;
		char  	     	    	* termtype;

		char					* current_host;			/**< the hostname part, stripped of qualifiers, luname and port number */
		char 	          		* full_current_host;	/**< the entire string, for use in reconnecting */
		char		       		* qualified_host;
		char					* proxy;				/**< Proxy server (type:host[:port]) */
		char					* termname;
		char					* charset;

		LIB3270_MESSAGE			  oia_status;

		unsigned char	 		  oia_flag[LIB3270_FLAG_COUNT];

		unsigned short			  current_port;

		// Misc
		void					* ft;					/**< Active file transfer data */

		// screen info
		const char				* host_charset;
		int						  ov_rows;
		int						  ov_cols;
		int						  maxROWS;
		int						  maxCOLS;
		unsigned short			  rows;
		unsigned short			  cols;
		int						  cursor_addr;
		int						  buffer_addr;
		char					  flipped;
		int						  screen_alt;			/**< alternate screen? */
		int						  is_altbuffer;

		int						  formatted;			/**< set in screen_disp */

		// Screen contents
		void 					* buffer[2];			/**< Internal buffers */
		struct ea  	    		* ea_buf;				/**< 3270 device buffer. ea_buf[-1] is the dummy default field attribute */
		struct ea 				* aea_buf;				/** alternate 3270 extended attribute buffer */
		struct lib3270_text		* text;					/**< Converted 3270 chars */

		// host.c
		char	 				  std_ds_host;
		char 					  no_login_host;
		char 					  non_tn3270e_host;
		char 					  passthru_host;
		char 					  ssl_host;
		char 					  ever_3270;

		// ctlr.c
		int						  sscp_start;
		unsigned char			  default_fg;
		unsigned char			  default_bg;
		unsigned char			  default_gr;
		unsigned char			  default_cs;
		unsigned char			  default_ic;
		char					  reply_mode;
		int 					  trace_primed 		: 1;
		int						  crm_nattr;
		unsigned char			  crm_attr[16];
		unsigned char 			* zero_buf;				/**< empty buffer, for area clears */

		// Telnet.c
		unsigned char 			* ibuf;
		int      				  ibuf_size;			/**< size of ibuf */
		unsigned char 			  myopts[LIB3270_TELNET_N_OPTS];
		unsigned char			  hisopts[LIB3270_TELNET_N_OPTS];

		// kybd.c
		unsigned int			  kybdlock;
		unsigned char			  aid;					/**< current attention ID */
		void					* unlock_id;
		time_t					  unlock_delay_time;

		// Widget info
		void					* widget;

		// selection
		char					* paste_buffer;
		struct
		{
			int start;
			int end;
		} select;

		// xio
		void 					* ns_read_id;
		void 					* ns_exception_id;

		int 					  reading				: 1;
		int						  excepting				: 1;

		// SSL Data (Always defined to mantain the same structure size
		unsigned long 			  last_ssl_error;
		SSL 					* ssl_con;

		// State change callbacks.
		struct lib3270_state_callback *st_callbacks[LIB3270_STATE_USER];
		struct lib3270_state_callback *st_last[LIB3270_STATE_USER];

		// Session based callbacks
		void (*configure)(H3270 *session, unsigned short rows, unsigned short cols);
		void (*update)(H3270 *session, int baddr, unsigned char c, unsigned short attr, unsigned char cursor);
		void (*changed)(H3270 *session, int offset, int len);
		void (*display)(H3270 *session);

		void (*update_cursor)(H3270 *session, unsigned short row, unsigned short col, unsigned char c, unsigned short attr);
		void (*update_oia)(H3270 *session, LIB3270_FLAG id, unsigned char on);
		void (*update_toggle)(H3270 *session, LIB3270_TOGGLE ix, unsigned char value, LIB3270_TOGGLE_TYPE reason, const char *name);
		void (*update_luname)(H3270 *session, const char *name);
		void (*update_status)(H3270 *session, LIB3270_MESSAGE id);
		void (*update_connect)(H3270 *session, unsigned char connected);
		void (*update_model)(H3270 *session, const char *name, int model, int rows, int cols);
		void (*update_selection)(H3270 *session, int start, int end);
		void (*update_ssl)(H3270 *session, LIB3270_SSL_STATE state);

		void (*set_timer)(H3270 *session, unsigned char on);
		void (*erase)(H3270 *session);
		void (*suspend)(H3270 *session);
		void (*resume)(H3270 *session);
		void (*cursor)(H3270 *session, LIB3270_CURSOR id);
		void (*set_selection)(H3270 *session, unsigned char on);
		void (*ctlr_done)(H3270 *session);

		void (*message)(H3270 *session, LIB3270_NOTIFY id , const char *title, const char *message, const char *text);

	};


#endif // LIB3270_SESSION_H_INCLUDED


