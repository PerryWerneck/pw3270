/*
 * "Software PW3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
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
 * Este programa está nomeado como filetransfer.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas de Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */

#ifndef LIB3270_FILETRANSFER_INCLUDED

	#define LIB3270_FILETRANSFER_INCLUDED 1
	#include <stdio.h>
	#include <sys/time.h>
	#include <lib3270/charset.h>

	#define LIB3270_FT_OPTION_SEND 0x0000

	#define LIB3270_FT_RECORD_FORMAT_DEFAULT		0x0000
	#define LIB3270_FT_RECORD_FORMAT_FIXED			0x0100
	#define LIB3270_FT_RECORD_FORMAT_VARIABLE		0x0200
	#define LIB3270_FT_RECORD_FORMAT_UNDEFINED		0x0300
	#define LIB3270_FT_RECORD_FORMAT_MASK 			LIB3270_FT_RECORD_FORMAT_UNDEFINED

	#define LIB3270_FT_ALLOCATION_UNITS_DEFAULT		0x0000
	#define LIB3270_FT_ALLOCATION_UNITS_TRACKS		0x1000
	#define LIB3270_FT_ALLOCATION_UNITS_CYLINDERS	0x2000
	#define LIB3270_FT_ALLOCATION_UNITS_AVBLOCK		0x3000
	#define LIB3270_FT_ALLOCATION_UNITS_MASK		LIB3270_FT_ALLOCATION_UNITS_AVBLOCK

	typedef enum _lib3270_FT_FLAG
	{
		LIB3270_FT_OPTION_RECEIVE		= 0x0001,
		LIB3270_FT_OPTION_ASCII			= 0x0002,		/**< Convert to ascii */
		LIB3270_FT_OPTION_CRLF			= 0x0004,		/**< Add crlf to each line */
		LIB3270_FT_OPTION_APPEND		= 0x0008,
		LIB3270_FT_OPTION_REMAP			= 0x0010		/**< Remap ASCII<->EBCDIC */

	} LIB3270_FT_OPTION;

	typedef enum _lib3270_ft_state
	{
		LIB3270_FT_STATE_NONE,			/**< No transfer in progress */
		LIB3270_FT_STATE_AWAIT_ACK,		/**< IND$FILE sent, awaiting acknowledgement message */
		LIB3270_FT_STATE_RUNNING,		/**< Ack received, data flowing */
		LIB3270_FT_STATE_ABORT_WAIT,	/**< Awaiting chance to send an abort */
		LIB3270_FT_STATE_ABORT_SENT		/**< Abort sent; awaiting response */
	} LIB3270_FT_STATE;

	#define LIB3270_XLATE_NBUF	4

	typedef struct _h3270ft
	{
		unsigned short			  sz;					/**< Size of FT data structure */

		int						  ft_last_cr	: 1;	/**< CR was last char in local file */
		int 					  remap_flag	: 1;	/**< Remap ASCII<->EBCDIC */
		int						  cr_flag		: 1;
		int						  message_flag	: 1;	/**< Open Request for msg received */
		int						  ascii_flag	: 1;	/**< Convert to ascii */
		int						  ft_is_cut		: 1;	/**< File transfer is CUT-style */
		int						  dft_eof		: 1;


		H3270					* host;
		void					* widget;				/**< File transfer dialog handle */
		FILE 					* local_file;			/**< File descriptor for local file */
		unsigned long			  length;				/**< File length */

		LIB3270_FT_STATE		  state;
		LIB3270_FT_OPTION		  flags;

		int 					  lrecl;
		int 					  blksize;
		int						  primspace;
		int						  secspace;
		int						  dft;

		unsigned long			  ft_length;			/**< Length of transfer */

		struct timeval			  starting_time;		/**< Starting time */

		const char 				* local;				/**< Local filename */
		const char				* remote;				/**< Remote filename */

		// ft_dft.c
		char 					* abort_string;
		unsigned long			  recnum;
		unsigned char			* dft_savebuf;
		int						  dft_savebuf_len;
		int						  dft_savebuf_max;

		// ft_cut.c
		int						  quadrant;
		unsigned long			  expanded_length;
		char					* saved_errmsg;
		int						  xlate_buffered;					/**< buffer count */
		int						  xlate_buf_ix;						/**< buffer index */
		unsigned char			  xlate_buf[LIB3270_XLATE_NBUF];	/**< buffer */

		// Charset
		struct lib3270_charset	  charset;

		// Callbacks
		void (*complete)(struct _h3270ft *ft,unsigned long length,double kbytes_sec,const char *mode);
		void (*message)(struct _h3270ft *ft, const char *msg);
		void (*update)(struct _h3270ft *ft, unsigned long current, unsigned long length, double kbytes_sec);
		void (*running)(struct _h3270ft *ft, int is_cut);
		void (*aborting)(struct _h3270ft *ft);
		void (*state_changed)(struct _h3270ft *ft, LIB3270_FT_STATE state);

	} H3270FT;

	/**
	 * Create a new file transfer session.
	 *
	 * @param session
	 * @param flags
	 * @param local
	 * @param remote
	 * @param lrecl
	 * @param blksize
	 * @param primspace
	 * @param secspace
	 * @param dft
	 * @param msg			Pointer to error message.
	 *
	 * @return Filetransfer handle if ok, NULL if failed
	 *
	 */
	LIB3270_EXPORT H3270FT	* lib3270_ft_new(H3270 *hSession, LIB3270_FT_OPTION flags, const char *local, const char *remote, int lrecl, int blksize, int primspace, int secspace, int dft, const char **msg);

	LIB3270_EXPORT int		  lib3270_ft_start(H3270 *hSession);
	LIB3270_EXPORT int	 	  lib3270_ft_destroy(H3270 *hSession);

	LIB3270_EXPORT int		  lib3270_ft_cancel(H3270 *hSession, int force);


	LIB3270_EXPORT LIB3270_FT_STATE lib3270_get_ft_state(H3270 *session);


#endif // LIB3270_FILETRANSFER_INCLUDED
