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
 * Este programa está nomeado como trace_dsc.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

/*
 *	trace_dsc.h
 *		Global declarations for trace_ds.c.
 */

#if defined(X3270_TRACE)

	LIB3270_INTERNAL Boolean trace_skipping;

	const char *rcba(H3270 *session, int baddr);

//	void toggle_dsTrace(H3270 *h, struct toggle *t, LIB3270_TOGGLE_TYPE tt);
//	void toggle_eventTrace(H3270 *h, struct toggle *t, LIB3270_TOGGLE_TYPE tt);
//	void toggle_screenTrace(H3270 *h, struct toggle *t, LIB3270_TOGGLE_TYPE tt);

	void trace_ansi_disc(void);
	void trace_char(char c);
	void trace_ds(const char *fmt, ...) printflike(1, 2);
	void trace_ds_nb(const char *fmt, ...) printflike(1, 2);
	void trace_dsn(const char *fmt, ...) printflike(1, 2);
	void trace_event(const char *fmt, ...) printflike(1, 2);
	void trace_screen(H3270 *session);
//	void trace_rollover_check(void);

#elif defined(__GNUC__)

	#define trace_ds(format, args...)
	#define trace_dsn(format, args...)
	#define trace_ds_nb(format, args...)
	#define trace_event(format, args...)

#else

	#define trace_ds 0 &&
	#define trace_ds_nb 0 &&
	#define trace_dsn 0 &&
	#define trace_event 0 &&
	#define rcba 0 &&

#endif
