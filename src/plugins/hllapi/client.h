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
 * Este programa está nomeado como private.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <windows.h>
 #include <pw3270/hllapi.h>

 #ifndef ETIMEDOUT
	#define ETIMEDOUT 1238
 #endif // ETIMEDOUT

 #define PIPE_BUFFER_LENGTH 8192

 #define set_active(x) /* x */

 #if defined(DEBUG) && defined(_WIN32)
	#undef trace
	#define trace( fmt, ... )	{ FILE *out = fopen("c:\\Users\\Perry\\hllapi.log","a"); if(out) { fprintf(out, "%s(%d) " fmt "\n", __FILE__, __LINE__, __VA_ARGS__ ); fclose(out); } }
 #endif // DEBUG

 void				* hllapi_pipe_init(const char *id);
 void				  hllapi_pipe_deinit(void *h);
 const char			* hllapi_pipe_get_revision(void);
 void  				  hllapi_pipe_release_memory(void *p);
 int				  hllapi_pipe_connect(void *h, const char *n, int wait);
 void 				  hllapi_pipe_disconnect(void *h);
 LIB3270_MESSAGE	  hllapi_pipe_get_message(void *h);
 char 				* hllapi_pipe_get_text_at(void *h, int row, int col, int len);
 char 				* hllapi_pipe_get_text(void *h, int offset, int len);
 int  			  	  hllapi_pipe_enter(void *h);
 int  			  	  hllapi_pipe_erase_eof(void *h);
 int 			  	  hllapi_pipe_set_text_at(void *h, int row, int col, const unsigned char *str);
 int 			      hllapi_pipe_cmp_text_at(void *h, int row, int col, const char *text);
 int				  hllapi_pipe_pfkey(void *h, int key);
 int				  hllapi_pipe_pakey(void *h, int key);
 int 				  hllapi_pipe_wait_for_ready(void *h, int seconds);
 int 				  hllapi_pipe_sleep(void *h, int seconds);
 int 				  hllapi_pipe_is_connected(void *h);
 int				  hllapi_pipe_getcursor(void *h);
 int				  hllapi_pipe_setcursor(void *h, int baddr);
 int				  hllapi_pipe_emulate_input(void *hSession, const char *s, int len, int pasting);
 int				  hllapi_pipe_print(void *h);

 char				* hllapi_get_string(int offset, size_t len);
 void				  hllapi_free(void *p);



