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

#pragma pack(1)

struct hllapi_packet_result
{
	int 			rc;
};

struct hllapi_packet_query
{
	int				packet_id;
};

struct hllapi_packet_connect
{
	int				packet_id;
	unsigned char	wait;
	char			hostname[1];
};

struct hllapi_packet_keycode
{
	int				packet_id;
	unsigned short	keycode;
};

struct hllapi_packet_cursor
{
	int				packet_id;
	unsigned short	row;
	unsigned short	col;
};

struct hllapi_packet_text
{
	int				packet_id;
	char 			text[1];
};

struct hllapi_packet_at
{
	int				packet_id;
	unsigned short	row;
	unsigned short	col;
	unsigned short	len;
};

struct hllapi_packet_text_at
{
	int				packet_id;
	unsigned short	row;
	unsigned short	col;
	char 			text[1];
};

struct hllapi_packet_query_at
{
	int				packet_id;
	unsigned short	row;
	unsigned short	col;
	unsigned short	len;
};

struct hllapi_packet_wait
{
	int				packet_id;
	int				timeout;
};


#pragma pack()

