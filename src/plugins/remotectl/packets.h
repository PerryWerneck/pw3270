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
 * Este programa está nomeado como packets.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 typedef enum _hllapi_packet
 {
		HLLAPI_PACKET_CONNECT,
		HLLAPI_PACKET_DISCONNECT,
		HLLAPI_PACKET_GET_PROGRAM_MESSAGE,
		HLLAPI_PACKET_GET_TEXT_AT,
		HLLAPI_PACKET_SET_TEXT_AT,
		HLLAPI_PACKET_CMP_TEXT_AT,
		HLLAPI_PACKET_ENTER,
		HLLAPI_PACKET_PFKEY,
		HLLAPI_PACKET_PAKEY,
		HLLAPI_PACKET_SET_CURSOR_POSITION,
		HLLAPI_PACKET_GET_CURSOR_POSITION,
		HLLAPI_PACKET_INPUT_STRING,
		HLLAPI_PACKET_IS_CONNECTED,
		HLLAPI_PACKET_SET_CURSOR,
		HLLAPI_PACKET_GET_CURSOR,

		HLLAPI_PACKET_INVALID

 } HLLAPI_PACKET;

#pragma pack(1)

struct hllapi_packet_result
{
	int 			rc;
};

struct hllapi_packet_query
{
	unsigned char	packet_id;
};

struct hllapi_packet_connect
{
	unsigned char	packet_id;
	unsigned char	wait;
	char			hostname[1];
};

struct hllapi_packet_keycode
{
	unsigned char	packet_id;
	unsigned short	keycode;
};

struct hllapi_packet_cursor
{
	unsigned char	packet_id;
	unsigned short	row;
	unsigned short	col;
};

struct hllapi_packet_text
{
	unsigned char	packet_id;
	char 			text[1];
};

struct hllapi_packet_at
{
	unsigned char	packet_id;
	unsigned short	row;
	unsigned short	col;
	unsigned short	len;
};

struct hllapi_packet_text_at
{
	unsigned char	packet_id;
	unsigned short	row;
	unsigned short	col;
	char 			text[1];
};

struct hllapi_packet_query_at
{
	unsigned char	packet_id;
	unsigned short	row;
	unsigned short	col;
	unsigned short	len;
};

struct hllapi_packet_wait
{
	unsigned char	packet_id;
	int				timeout;
};

struct hllapi_packet_addr
{
	unsigned char	packet_id;
	unsigned short	addr;
};


#pragma pack()

