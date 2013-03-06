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
 * Este programa está nomeado como calls.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <lib3270.h>
 #include <malloc.h>
 #include <string.h>
 #include <errno.h>
 #include <stdio.h>
 #include <time.h>
 #include <lib3270/log.h>

 #include "client.h"
 #include "packets.h"

/*--[ Globals ]--------------------------------------------------------------------------------------*/

/*--[ Implement ]------------------------------------------------------------------------------------*/

 void * hllapi_pipe_init(const char *id)
 {
	HANDLE			  hPipe  = INVALID_HANDLE_VALUE;
	static DWORD	  dwMode = PIPE_READMODE_MESSAGE;
 	char	 		  buffer[4096];
 	char			* name = strdup(id);
 	char			* ptr;

	trace("%s(%s)",__FUNCTION__,id);

 	for(ptr=name;*ptr;ptr++)
	{
		if(*ptr == ':')
			*ptr = '_';
	}

	snprintf(buffer,4095,"\\\\.\\pipe\\%s",name);

	free(name);

	trace("Opening \"%s\"",buffer);

	if(!WaitNamedPipe(buffer,NMPWAIT_USE_DEFAULT_WAIT))
	{
		trace("%s: Pipe not found",__FUNCTION__);
		errno = ENOENT;
		return NULL;
	}

	hPipe = CreateFile(buffer,GENERIC_WRITE|GENERIC_READ,0,NULL,OPEN_EXISTING,0,NULL);

	if(hPipe == INVALID_HANDLE_VALUE)
	{
		errno = GetLastError();
		return NULL;
	}

	if(!SetNamedPipeHandleState(hPipe,&dwMode,NULL,NULL))
	{
		errno = GetLastError();
		return NULL;
	}

	trace("hPipe=%p",(void *) hPipe);
 	return hPipe;
 }

 void hllapi_pipe_deinit(void *h)
 {
	trace("%s(%p)",__FUNCTION__,h);

	if(!h)
		return;

	CloseHandle((HANDLE) h);
 }

 const char * hllapi_pipe_get_revision(void)
 {
	return PACKAGE_REVISION;
 }

 int hllapi_pipe_connect(void *h, const char *n, int wait)
 {
	struct hllapi_packet_connect	* pkt;
	struct hllapi_packet_result		  response;
	DWORD							  cbSize;

	if(!n)
		n = "";

	cbSize	= sizeof(struct hllapi_packet_connect)+strlen(n);
	pkt 	= malloc(cbSize);

	pkt->packet_id	= HLLAPI_PACKET_CONNECT;
	pkt->wait		= (unsigned char) wait;
	strcpy(pkt->hostname,n);

	trace("Sending %s",pkt->hostname);

	if(!TransactNamedPipe((HANDLE) h,(LPVOID) pkt, cbSize, &response, sizeof(response), &cbSize,NULL))
	{
		errno 		= GetLastError();
		response.rc = -1;
	}

	free(pkt);

	return response.rc;
 }

 void hllapi_pipe_disconnect(void *h)
 {
	static const struct hllapi_packet_query query		= { HLLAPI_PACKET_DISCONNECT };
	struct hllapi_packet_result		  		response;
	DWORD							  		cbSize		= sizeof(query);
	TransactNamedPipe((HANDLE) h,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
 }

 LIB3270_MESSAGE hllapi_pipe_get_message(void *h)
 {
	static const struct hllapi_packet_query query		= { HLLAPI_PACKET_GET_PROGRAM_MESSAGE };
	struct hllapi_packet_result		  		response;
	DWORD							  		cbSize		= sizeof(query);
	TransactNamedPipe((HANDLE) h,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
	return (LIB3270_MESSAGE) response.rc;
 }

 char * hllapi_pipe_get_text_at(void *h, int row, int col, int len)
 {
	struct hllapi_packet_query_at	  query		= { HLLAPI_PACKET_GET_TEXT_AT, };
	struct hllapi_packet_text		* response;
	DWORD							  cbSize	= sizeof(struct hllapi_packet_text)+len;
	char 							* text		= NULL;

	response = malloc(cbSize+2);
	memset(response,0,cbSize+2);

	if(!TransactNamedPipe((HANDLE) h,(LPVOID) &query, sizeof(struct hllapi_packet_query_at), &response, cbSize, &cbSize,NULL))
		return NULL;

	if(response->packet_id)
		errno = response->packet_id;
	else
		text  = strdup(response->text);

	free(response);
	return text;
 }

 int hllapi_pipe_enter(void *h)
 {
	static const struct hllapi_packet_query query		= { HLLAPI_PACKET_ENTER };
	struct hllapi_packet_result		  		response;
	DWORD							  		cbSize		= sizeof(query);
	TransactNamedPipe((HANDLE) h,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
	return response.rc;
 }

 int hllapi_pipe_erase_eof(void *h)
 {
	static const struct hllapi_packet_query query		= { HLLAPI_PACKET_ERASE_EOF };
	struct hllapi_packet_result		  		response;
	DWORD							  		cbSize		= sizeof(query);
	TransactNamedPipe((HANDLE) h,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
	return response.rc;
 }


 int hllapi_pipe_set_text_at(void *h, int row, int col, const unsigned char *str)
 {
	struct hllapi_packet_text_at 	* query;
	struct hllapi_packet_result	  	  response;
	DWORD							  cbSize		= sizeof(struct hllapi_packet_text_at)+strlen((const char *) str);

	query = malloc(cbSize);
	query->packet_id 	= HLLAPI_PACKET_SET_TEXT_AT;
	query->row			= row;
	query->col			= col;
	strcpy(query->text,(const char *) str);

	TransactNamedPipe((HANDLE) h,(LPVOID) query, cbSize, &response, sizeof(response), &cbSize,NULL);

	free(query);

	return response.rc;
 }

 int hllapi_pipe_cmp_text_at(void *h, int row, int col, const char *text)
 {
	struct hllapi_packet_text_at 	* query;
	struct hllapi_packet_result	  	  response;
	DWORD							  cbSize		= sizeof(struct hllapi_packet_text_at)+strlen(text);

	query = malloc(cbSize);
	query->packet_id 	= HLLAPI_PACKET_CMP_TEXT_AT;
	query->row			= row;
	query->col			= col;
	strcpy(query->text,text);

	TransactNamedPipe((HANDLE) h,(LPVOID) query, cbSize, &response, sizeof(response), &cbSize,NULL);

	free(query);

	return response.rc;
 }

 int hllapi_pipe_pfkey(void *h, int key)
 {
	struct hllapi_packet_keycode	query		= { HLLAPI_PACKET_PFKEY, key };
	struct hllapi_packet_result		response;
	DWORD							cbSize		= sizeof(query);
	TransactNamedPipe((HANDLE) h,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
	return response.rc;
 }

 int hllapi_pipe_pakey(void *h, int key)
 {
	struct hllapi_packet_keycode	query		= { HLLAPI_PACKET_PAKEY, key };
	struct hllapi_packet_result		response;
	DWORD							cbSize		= sizeof(query);
	TransactNamedPipe((HANDLE) h,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
	return response.rc;
 }

 void hllapi_pipe_release_memory(void *p)
 {
 	free(p);
 }

 int hllapi_pipe_wait_for_ready(void *h, int seconds)
 {
 	time_t end = time(0)+seconds;

	while(time(0) < end)
	{
		if(!hllapi_pipe_is_connected(h))
			return ENOTCONN;

		if(hllapi_pipe_get_message(h) == 0)
			return 0;
		Sleep(250);
	}

	return ETIMEDOUT;
 }

 int hllapi_pipe_is_connected(void *h)
 {
	static const struct hllapi_packet_query query		= { HLLAPI_PACKET_IS_CONNECTED };
	struct hllapi_packet_result		  		response;
	DWORD							  		cbSize		= sizeof(query);
	TransactNamedPipe((HANDLE) h,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
	return (LIB3270_MESSAGE) response.rc;
 }

 int hllapi_pipe_sleep(void *h, int seconds)
 {
 	time_t end = time(0)+seconds;

	while(time(0) < end)
	{
		if(!hllapi_pipe_is_connected(h))
			return ENOTCONN;
		Sleep(500);
	}

	return 0;
 }

 int hllapi_pipe_getcursor(void *h)
 {
	static const struct hllapi_packet_query query		= { HLLAPI_PACKET_GET_CURSOR };
	struct hllapi_packet_result		  		response;
	DWORD							  		cbSize		= sizeof(query);

	trace("%s",__FUNCTION__);

	TransactNamedPipe((HANDLE) h,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
	return (LIB3270_MESSAGE) response.rc;

 }

 int hllapi_pipe_setcursor(void *h, int baddr)
 {
	struct hllapi_packet_addr		query		= { HLLAPI_PACKET_SET_CURSOR, baddr };
	struct hllapi_packet_result		response;
	DWORD							cbSize		= sizeof(query);

	trace("%s(%d)",__FUNCTION__,query.addr);

	TransactNamedPipe((HANDLE) h,(LPVOID) &query, cbSize, &response, sizeof(response), &cbSize,NULL);
	return response.rc;
 }

 char * hllapi_pipe_get_text(void *h, int offset, int len)
 {
	struct hllapi_packet_query_offset	  query		= { HLLAPI_PACKET_GET_TEXT_AT_OFFSET, offset, len };
	struct hllapi_packet_text			* response;
	DWORD								  cbSize	= sizeof(struct hllapi_packet_text)+len;
	char 								* text		= NULL;

	trace("cbSize=%d",(int) cbSize);

	response = malloc(cbSize+2);
	memset(response,0,cbSize+2);

	if(!TransactNamedPipe((HANDLE) h,(LPVOID) &query, sizeof(query), response, cbSize, &cbSize,NULL))
		return NULL;

	trace("rc=%d",response->packet_id);

	if(response->packet_id)
		errno = response->packet_id;
	else
		text  = strdup(response->text);

	free(response);
	return text;
 }

 int hllapi_pipe_emulate_input(void *h, const char *text, int len, int pasting)
 {
	struct hllapi_packet_emulate_input 	* query;
	struct hllapi_packet_result	  		  response;
	DWORD								  cbSize		= sizeof(struct hllapi_packet_emulate_input)+strlen(text);

	query = malloc(cbSize);
	query->packet_id 	= HLLAPI_PACKET_EMULATE_INPUT;
	query->len			= len;
	query->pasting		= pasting;
	strcpy(query->text,text);

	TransactNamedPipe((HANDLE) h,(LPVOID) query, cbSize, &response, sizeof(response), &cbSize,NULL);

	free(query);

	return response.rc;
 }
