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
 * Este programa está nomeado como charset.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

/*
 *	charset.c
 *		This module handles character sets.
 */

#include "globals.h"

/*
 * EBCDIC-to-Unicode translation tables.
 * Each table maps EBCDIC codes X'41' through X'FE' to UCS-2.
 * Other codes are mapped programmatically.
 */
#define UT_SIZE		190
#define UT_OFFSET	0x41

typedef struct
{
	const char 		* name;
	const char 		* host_codepage;
	const char		* cgcsgid;
	const char		* display_charset;
	unsigned short	  code[UT_SIZE];
} charset_table;


/*---[ Statics ]--------------------------------------------------------------------------------------------------------------*/

const unsigned short ebc2asc0[256] =
{
	/*00*/	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	/*08*/	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	/*10*/	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	/*18*/	0x20, 0x20, 0x20, 0x20, 0x2a, 0x20, 0x3b, 0x20,
	/*20*/	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	/*28*/	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	/*30*/	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	/*38*/	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
	/*40*/	0x20, 0x20, 0xe2, 0xe4, 0xe0, 0xe1, 0xe3, 0xe5,
	/*48*/	0xe7, 0xf1, 0xa2, 0x2e, 0x3c, 0x28, 0x2b, 0x7c,
	/*50*/	0x26, 0xe9, 0xea, 0xeb, 0xe8, 0xed, 0xee, 0xef,
	/*58*/	0xec, 0xdf, 0x21, 0x24, 0x2a, 0x29, 0x3b, 0xac,
	/*60*/	0x2d, 0x2f, 0xc2, 0xc4, 0xc0, 0xc1, 0xc3, 0xc5,
	/*68*/	0xc7, 0xd1, 0xa6, 0x2c, 0x25, 0x5f, 0x3e, 0x3f,
	/*70*/	0xf8, 0xc9, 0xca, 0xcb, 0xc8, 0xcd, 0xce, 0xcf,
	/*78*/	0xcc, 0x60, 0x3a, 0x23, 0x40, 0x27, 0x3d, 0x22,
	/*80*/	0xd8, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	/*88*/	0x68, 0x69, 0xab, 0xbb, 0xf0, 0xfd, 0xfe, 0xb1,
	/*90*/	0xb0, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
	/*98*/	0x71, 0x72, 0xaa, 0xba, 0xe6, 0xb8, 0xc6, 0xa4,
	/*a0*/	0xb5, 0x7e, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
	/*a8*/	0x79, 0x7a, 0xa1, 0xbf, 0xd0, 0xdd, 0xde, 0xae,
	/*b0*/	0x5e, 0xa3, 0xa5, 0xb7, 0xa9, 0xa7, 0xb6, 0xbc,
	/*b8*/	0xbd, 0xbe, 0x5b, 0x5d, 0xaf, 0xa8, 0xb4, 0xd7,
	/*c0*/	0x7b, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	/*c8*/	0x48, 0x49, 0xad, 0xf4, 0xf6, 0xf2, 0xf3, 0xf5,
	/*d0*/	0x7d, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,
	/*d8*/	0x51, 0x52, 0xb9, 0xfb, 0xfc, 0xf9, 0xfa, 0xff,
	/*e0*/	0x5c, 0xf7, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
	/*e8*/	0x59, 0x5a, 0xb2, 0xd4, 0xd6, 0xd2, 0xd3, 0xd5,
	/*f0*/	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	/*f8*/	0x38, 0x39, 0xb3, 0xdb, 0xdc, 0xd9, 0xda, 0x20
};

static const unsigned short asc2ebc0[256] =
{
	/*00*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/*08*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/*10*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/*18*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/*20*/  0x40, 0x5a, 0x7f, 0x7b, 0x5b, 0x6c, 0x50, 0x7d,
	/*28*/  0x4d, 0x5d, 0x5c, 0x4e, 0x6b, 0x60, 0x4b, 0x61,
	/*30*/  0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
	/*38*/  0xf8, 0xf9, 0x7a, 0x5e, 0x4c, 0x7e, 0x6e, 0x6f,
	/*40*/  0x7c, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
	/*48*/  0xc8, 0xc9, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6,
	/*50*/  0xd7, 0xd8, 0xd9, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6,
	/*58*/  0xe7, 0xe8, 0xe9, 0xba, 0xe0, 0xbb, 0xb0, 0x6d,
	/*60*/  0x79, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
	/*68*/  0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96,
	/*70*/  0x97, 0x98, 0x99, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6,
	/*78*/  0xa7, 0xa8, 0xa9, 0xc0, 0x4f, 0xd0, 0xa1, 0x00,
	/*80*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/*88*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/*90*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/*98*/  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/*a0*/  0x41, 0xaa, 0x4a, 0xb1, 0x9f, 0xb2, 0x6a, 0xb5,
	/*a8*/  0xbd, 0xb4, 0x9a, 0x8a, 0x5f, 0xca, 0xaf, 0xbc,
	/*b0*/  0x90, 0x8f, 0xea, 0xfa, 0xbe, 0xa0, 0xb6, 0xb3,
	/*b8*/  0x9d, 0xda, 0x9b, 0x8b, 0xb7, 0xb8, 0xb9, 0xab,
	/*c0*/  0x64, 0x65, 0x62, 0x66, 0x63, 0x67, 0x9e, 0x68,
	/*c8*/  0x74, 0x71, 0x72, 0x73, 0x78, 0x75, 0x76, 0x77,
	/*d0*/  0xac, 0x69, 0xed, 0xee, 0xeb, 0xef, 0xec, 0xbf,
	/*d8*/  0x80, 0xfd, 0xfe, 0xfb, 0xfc, 0xad, 0xae, 0x59,
	/*e0*/  0x44, 0x45, 0x42, 0x46, 0x43, 0x47, 0x9c, 0x48,
	/*e8*/  0x54, 0x51, 0x52, 0x53, 0x58, 0x55, 0x56, 0x57,
	/*f0*/  0x8c, 0x49, 0xcd, 0xce, 0xcb, 0xcf, 0xcc, 0xe1,
	/*f8*/  0x70, 0xdd, 0xde, 0xdb, 0xdc, 0x8d, 0x8e, 0xdf
};

static const unsigned short ft2asc[256] =
{
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0xa2,0x5c,0x7c,0xac,0x5f,
	0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0xa6,0x7d,0x7e,0x7f,
	0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
	0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0xf7,
	0xa0,0xe2,0xe4,0xe0,0xe1,0xe3,0xe5,0xe7,0xf1,0xe9,0xea,0xeb,0xe8,0xed,0xee,0xef,
	0xec,0xdf,0xc2,0xc4,0xc0,0xc1,0xc3,0xc5,0xc7,0xd1,0xf8,0xc9,0xca,0xcb,0xc8,0xcd,
	0xce,0xcf,0xcc,0xd8,0xab,0xbb,0xf0,0xfd,0xfe,0xb1,0xb0,0xaa,0xba,0xe6,0xb8,0xc6,
	0xa4,0xb5,0xa1,0xbf,0xd0,0xdd,0xde,0xae,0x5e,0xa3,0xa5,0xb7,0xa9,0xa7,0xb6,0xbc,
	0xbd,0xbe,0x5b,0x5d,0xaf,0xa8,0xb4,0xd7,0xad,0xf4,0xf6,0xf2,0xf3,0xf5,0xb9,0xfb,
	0xfc,0xf9,0xfa,0xff,0xb2,0xd4,0xd6,0xd2,0xd3,0xd5,0xb3,0xdb,0xdc,0xd9,0xda,0xff
};

const unsigned short asc2ft[256] =
{
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
	0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,
	0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0xe2,0x5c,0xe3,0xd8,0x5f,
	0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b,0x5d,0x7d,0x7e,0x7f,
	0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,
	0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x00,
	0xa0,0xd2,0x5b,0xd9,0xd0,0xda,0x7c,0xdd,0xe5,0xdc,0xcb,0xc4,0x5e,0xe8,0xd7,0xe4,
	0xca,0xc9,0xf4,0xfa,0xe6,0xd1,0xde,0xdb,0xce,0xee,0xcc,0xc5,0xdf,0xe0,0xe1,0xd3,
	0xb4,0xb5,0xb2,0xb6,0xb3,0xb7,0xcf,0xb8,0xbe,0xbb,0xbc,0xbd,0xc2,0xbf,0xc0,0xc1,
	0xd4,0xb9,0xf7,0xf8,0xf5,0xf9,0xf6,0xe7,0xc3,0xfd,0xfe,0xfb,0xfc,0xd5,0xd6,0xb1,
	0xa3,0xa4,0xa1,0xa5,0xa2,0xa6,0xcd,0xa7,0xac,0xa9,0xaa,0xab,0xb0,0xad,0xae,0xaf,
	0xc6,0xa8,0xeb,0xec,0xe9,0xed,0xea,0x9f,0xba,0xf1,0xf2,0xef,0xf0,0xc7,0xc8,0xf3
};

static const unsigned short asc2uc[UT_SIZE] =
{
	/*40*/	      0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	/*48*/	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	/*50*/	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	/*58*/	0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	/*60*/	0x60, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
	/*68*/	0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	/*70*/	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
	/*78*/	0x58, 0x59, 0x5a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
	/*80*/	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
	/*88*/	0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	/*90*/	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
	/*98*/	0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
	/*a0*/	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
	/*a8*/	0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
	/*b0*/	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
	/*b8*/	0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	/*c0*/	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
	/*c8*/	0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
	/*d0*/	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
	/*d8*/	0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	/*e0*/	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
	/*e8*/	0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
	/*f0*/	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xf7,
	/*f8*/	0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde
};


/*---[ Implement ]------------------------------------------------------------------------------------------------------------*/

static void copy_charset(const unsigned short *from, unsigned short *to)
{
	int f;
	for(f=0;f < UT_SIZE;f++)
		to[f+UT_OFFSET] = from[f];
}

LIB3270_EXPORT struct lib3270_charset * lib3270_load_charset(H3270 *hSession, const char *name)
{
	int f;

	hSession->charset.host = "bracket";
	hSession->charset.display = "ISO-8859-1";

	lib3270_write_log(hSession,"charset","host.charset=%s display.charset=%s",
								hSession->charset.host,hSession->charset.display);

	memcpy(hSession->charset.ebc2asc,	ebc2asc0,	sizeof(hSession->charset.ebc2asc));
	memcpy(hSession->charset.asc2ebc,	asc2ebc0,	sizeof(hSession->charset.asc2ebc));

	for(f=0;f<UT_OFFSET;f++)
		hSession->charset.asc2uc[f] = f;
	copy_charset(asc2uc,hSession->charset.asc2uc);

#if defined(X3270_FT)
	memcpy(hSession->charset.ft2asc,	ft2asc,		sizeof(hSession->charset.ft2asc));
	memcpy(hSession->charset.asc2ft,	asc2ft,		sizeof(hSession->charset.asc2ft));
#endif

	return &hSession->charset;
}


LIB3270_EXPORT const char * lib3270_get_default_charset(void)
{
	return "ISO-8859-1";
}

LIB3270_EXPORT const char * lib3270_get_charset(H3270 *hSession)
{
	CHECK_SESSION_HANDLE(hSession);
	return hSession->charset.display ? hSession->charset.display : "ISO-8859-1";
}

/*ISO-8859-1

#include "resources.h"
// #include "appres.h"
#include "cg.h"

#include "charsetc.h"
#include "kybdc.h"
#include "popupsc.h"

#ifndef ANDROID
	#include <stdlib.h>
#endif // !ANDROID

#include "tablesc.h"
#include "utf8c.h"
#include "utilc.h"
#include "widec.h"
#include "X11keysym.h"

#include <errno.h>

#define EURO_SUFFIX	"-euro"
#define ES_SIZE		(sizeof(EURO_SUFFIX) - 1)

// Globals.
const char *default_display_charset = "3270cg-1a,3270cg-1,iso8859-1";

// Statics.
static enum cs_result resource_charset(H3270 *hSession, const char *csname, char *cs, char *ftcs);

typedef enum { CS_ONLY, FT_ONLY, BOTH } remap_scope;

static enum cs_result remap_chars(H3270 *hSession, const char *csname, char *spec, remap_scope scope, int *ne);
static void remap_one(H3270 *hSession, unsigned char ebc, KeySym iso, remap_scope scope,Boolean one_way);

#if defined(DEBUG_CHARSET)
static enum cs_result check_charset(void);
static char *char_if_ascii7(unsigned long l);
#endif

static void set_cgcsgids(H3270 *hSession, const char *spec);
static int set_cgcsgid(char *spec, unsigned long *idp);

static KeySym StringToKeysym(char *s);

struct charset_buffer
{
	unsigned char ebc2cg[256];
	unsigned char cg2ebc[256];
	unsigned char ebc2asc[256];
	unsigned char asc2ebc[256];

	#if defined(X3270_FT)
	unsigned char ft2asc[256];
	unsigned char asc2ft[256];
	#endif
};


static void save_charset(H3270 *hSession, struct charset_buffer *save)
{
	(void) memcpy((char *)save->ebc2cg, (char *) hSession->charset.ebc2cg, 256);
	(void) memcpy((char *)save->cg2ebc, (char *) hSession->charset.cg2ebc, 256);
	(void) memcpy((char *)save->ebc2asc, (char *) hSession->charset.ebc2asc, 256);
	(void) memcpy((char *)save->asc2ebc, (char *) hSession->charset.asc2ebc, 256);
#if defined(X3270_FT)
	(void) memcpy((char *)save->ft2asc, (char *) hSession->charset.ft2asc, 256);
	(void) memcpy((char *)save->asc2ft, (char *) hSession->charset.asc2ft, 256);
#endif
}

static void restore_charset(H3270 *hSession, struct charset_buffer *save)
{
	(void) memcpy((char *)hSession->charset.ebc2cg, (char *)save->ebc2cg, 256);
	(void) memcpy((char *)hSession->charset.cg2ebc, (char *)save->cg2ebc, 256);
	(void) memcpy((char *)hSession->charset.ebc2asc, (char *)save->ebc2asc, 256);
	(void) memcpy((char *)hSession->charset.asc2ebc, (char *)save->asc2ebc, 256);
#if defined(X3270_FT)
	(void) memcpy((char *)hSession->charset.ft2asc, (char *)save->ft2asc, 256);
	(void) memcpy((char *)hSession->charset.asc2ft, (char *)save->asc2ft, 256);
#endif
}

//
// Change character sets.
//
enum cs_result charset_init(H3270 *hSession, const char *csname)
{
	enum cs_result rc;
	char *ccs, *cftcs;
	const char	*ak;
	struct charset_buffer save;

	// Do nothing, successfully.
	if (csname == CN || !strcasecmp(csname, "us"))
	{
		charset_defaults(hSession);
		set_cgcsgids(hSession,CN);
		set_display_charset(hSession, "ISO-8859-1");
		return CS_OKAY;
	}

	// Figure out if it's already in a resource or in a file.
#ifdef ANDROID
	ccs = strdup("0xad: [ \n 0xba: Yacute \n0xbd: ] \n 0xbb: diaeresis \n");
#else
	ccs = lib3270_get_resource_string(hSession,"charset", csname, NULL);
#endif
	if (!ccs)
		return CS_NOTFOUND;

	// Grab the File Transfer character set.
	cftcs = lib3270_get_resource_string(hSession,"ftCharset",csname,NULL);

	// Save the current definitions, and start over with the defaults.
	save_charset(hSession,&save);
	charset_defaults(hSession);

	// Check for auto-keymap.
	ak = lib3270_get_resource_string(hSession,"autoKeymap", csname, NULL);
	if (ak != NULL)
		hSession->auto_keymap = strcasecmp(ak, "true") ? 0 : 1;
	else
		hSession->auto_keymap = 0;

	// Interpret them.
	rc = resource_charset(hSession,csname, ccs, cftcs);

	// Free them.
	lib3270_free(ccs);
	lib3270_free(cftcs);

#if defined(DEBUG_CHARSET)
	if (rc == CS_OKAY)
		rc = check_charset();
#endif

	if (rc != CS_OKAY)
		restore_charset(hSession,&save);

	return rc;
}

//
// Set a CGCSGID.  Return 0 for success, -1 for failure.
//
static int set_cgcsgid(char *spec, unsigned long *r)
{
	unsigned long cp;
	char *ptr;

	if (spec != CN &&
	    (cp = strtoul(spec, &ptr, 0)) &&
	    ptr != spec &&
	    *ptr == '\0') {
		if (!(cp & ~0xffffL))
			*r = LIB3270_DEFAULT_CGEN | cp;
		else
			*r = cp;
		return 0;
	} else
		return -1;
}

// Set the CGCSGIDs.
static void set_cgcsgids(H3270 *hSession, const char *spec)
{
	int n_ids = 0;
	char *spec_copy;
	char *buf;
	char *token;

	if (spec != CN) {
		buf = spec_copy = NewString(spec);
		while (n_ids >= 0 && (token = strtok(buf, "+")) != CN) {
			unsigned long *idp = NULL;

			buf = CN;
			switch (n_ids) {
			case 0:
			    idp = &hSession->cgcsgid;
			    break;
#if defined(X3270_DBCS)
			case 1:
			    idp = &hSession->cgcsgid_dbcs;
			    break;
#endif
			default:
			    popup_an_error(hSession,_( "Extra CGCSGID(s), ignoring" ));
			    break;
			}
			if (idp == NULL)
				break;
			if (set_cgcsgid(token, idp) < 0) {
				popup_an_error(hSession,_( "Invalid CGCSGID '%s', ignoring" ),token);
				n_ids = -1;
				break;
			}
			n_ids++;
		}
		lib3270_free(spec_copy);
		if (n_ids > 0)
			return;
	}

	hSession->cgcsgid = LIB3270_DEFAULT_CGEN | LIB3270_DEFAULT_CSET;
#if defined(X3270_DBCS)
	hSession->cgcsgid_dbcs = 0L;
#endif
}

// Define a charset from resources.
static enum cs_result resource_charset(H3270 *hSession, const char *csname, char *cs, char *ftcs)
{
	enum cs_result	  rc;
	int				  ne	= 0;
	char			* rcs	= CN;
	int				  n_rcs	= 0;
	char			* dcs;

	// Interpret the spec.
	rc = remap_chars(hSession, csname, cs, (ftcs == NULL)? BOTH: CS_ONLY, &ne);
	if (rc != CS_OKAY)
		return rc;
	if (ftcs != NULL) {
		rc = remap_chars(hSession, csname, ftcs, FT_ONLY, &ne);
		if (rc != CS_OKAY)
			return rc;
	}

	rcs = lib3270_get_resource_string(hSession,"displayCharset", csname, NULL);

	// Isolate the pieces.
	if (rcs != CN)
	{
		char *buf, *token;

		buf = rcs;
		while ((token = strtok(buf, "+")) != CN)
		{
			buf = CN;
			switch (n_rcs)
			{
			case 0:
#if defined(X3270_DBCS)
			case 1:
#endif
			    break;
			default:
			    popup_an_error(NULL,"Extra value(s) in displayCharset.%s, ignoring", csname);
			    break;
			}
			n_rcs++;
		}
	}

	lib3270_free(rcs);

	// Set up the cgcsgid.
//	set_cgcsgids(get_fresource("%s.%s", "codepage", csname));
	{
		char *ptr = lib3270_get_resource_string(hSession,"codepage", csname, NULL);
		set_cgcsgids(hSession,ptr);
		lib3270_free(ptr);
	}

//	dcs = get_fresource("%s.%s", "displayCharset", csname);
	dcs = lib3270_get_resource_string(hSession,"displayCharset", csname, NULL);

	if (dcs != NULL)
		set_display_charset(hSession,dcs);
	else
		set_display_charset(hSession,"ISO-8859-1");

	lib3270_free(dcs);

	// Set up the character set name.
//	set_charset_name(csname);

	return CS_OKAY;
}

//
// Map a keysym name or literal string into a character.
//Returns NoSymbol if there is a problem.
//
static KeySym
parse_keysym(char *s, Boolean extended)
{
	KeySym	k;

	k = StringToKeysym(s);
	if (k == NoSymbol) {
		if (strlen(s) == 1)
			k = *s & 0xff;
		else if (s[0] == '0' && s[1] == 'x') {
			unsigned long l;
			char *ptr;

			l = strtoul(s, &ptr, 16);
			if (*ptr != '\0' || (l & ~0xffff))
				return NoSymbol;
			return (KeySym)l;
		} else
			return NoSymbol;
	}
	if (k < ' ' || (!extended && k > 0xff))
		return NoSymbol;
	else
		return k;
}

// Process a single character definition.
static void remap_one(H3270 *hSession, unsigned char ebc, KeySym iso, remap_scope scope, Boolean one_way)
{
	unsigned char cg;

	// Ignore mappings of EBCDIC control codes and the space character.
	if (ebc <= 0x40)
		return;

	// If they want to map to a NULL or a blank, make it a one-way blank.
	if (iso == 0x0)
		iso = 0x20;
	if (iso == 0x20)
		one_way = True;

	if (!hSession->auto_keymap || iso <= 0xff) {
#if defined(X3270_FT)
		unsigned char aa;
#endif

		if (scope == BOTH || scope == CS_ONLY) {
			if (iso <= 0xff) {
				cg = hSession->charset.asc2cg[iso];

				if (hSession->charset.cg2asc[cg] == iso || iso == 0)
				{
					// well-defined
					hSession->charset.ebc2cg[ebc] = cg;
					if (!one_way)
						hSession->charset.cg2ebc[cg] = ebc;
				}
				else
				{
					// into a hole
					hSession->charset.ebc2cg[ebc] = CG_boxsolid;
				}
			}
			if (ebc > 0x40)
			{
				hSession->charset.ebc2asc[ebc] = iso;
				if (!one_way)
					hSession->charset.asc2ebc[iso] = ebc;
			}
		}
#if defined(X3270_FT)
		if (iso <= 0xff && ebc > 0x40) {
			// Change the file transfer translation table.
			if (scope == BOTH) {
				//
				// We have an alternate mapping of an EBCDIC
				// code to an ASCII code.  Modify the existing
				// ASCII(ft)-to-ASCII(desired) maps.
				//
				// This is done by figuring out which ASCII
				// code the host usually translates the given
				// EBCDIC code to (asc2ft0[ebc2asc0[ebc]]).
				// Now we want to translate that code to the
				// given ISO code, and vice-versa.
				//
				aa = asc2ft0[ebc2asc0[ebc]];
				if (aa != ' ') {
					hSession->charset.ft2asc[aa] = iso;
					hSession->charset.asc2ft[iso] = aa;
				}
			} else if (scope == FT_ONLY) {
				//
				// We have a map of how the host translates
				// the given EBCDIC code to an ASCII code.
				// Generate the translation between that code
				// and the ISO code that we would normally
				// use to display that EBCDIC code.
				//
				hSession->charset.ft2asc[iso] = hSession->charset.ebc2asc[ebc];
				hSession->charset.asc2ft[hSession->charset.ebc2asc[ebc]] = iso;
			}
		}
#endif
	} else {
		// Auto-keymap.
		add_xk(iso, (KeySym)hSession->charset.ebc2asc[ebc]);
	}
}

//
// Parse an EBCDIC character set map, a series of pairs of numeric EBCDIC codes and keysyms.
//
// If the keysym is in the range 1..255, it is a remapping of the EBCDIC code
// for a standard Latin-1 graphic, and the CG-to-EBCDIC map will be modified
// to match.
//
// Otherwise (keysym > 255), it is a definition for the EBCDIC code to use for
// a multibyte keysym.  This is intended for 8-bit fonts that with special
// characters that replace certain standard Latin-1 graphics.  The keysym
// will be entered into the extended keysym translation table.
//
static enum cs_result remap_chars(H3270 *hSession, const char *csname, char *spec, remap_scope scope, int *ne)
{
	char *s;
	char *ebcs, *isos;
	unsigned char ebc;
	KeySym iso;
	int ns;
	enum cs_result rc = CS_OKAY;
	Boolean is_table = False;
	Boolean one_way = False;

	// Pick apart a copy of the spec.
	s = spec = NewString(spec);
	while (isspace(*s)) {
		s++;
	}
	if (!strncmp(s, "#table", 6)) {
		is_table = True;
		s += 6;
	}

	if (is_table) {
		int ebc = 0;
		char *tok;
		char *ptr;

		while ((tok = strtok(s, " \t\n")) != CN) {
			if (ebc >= 256) {
				popup_an_error(hSession,_( "Charset has more than 256 entries" ));
				rc = CS_BAD;
				break;
			}
			if (tok[0] == '*') {
				one_way = True;
				tok++;
			} else
				one_way = False;
			iso = strtoul(tok, &ptr, 0);
			if (ptr == tok || *ptr != '\0' || iso > 256L) {
				if (strlen(tok) == 1)
					iso = tok[0] & 0xff;
				else {
					popup_an_error(hSession,_( "Invalid charset entry '%s' (#%d)" ),tok, ebc);
					rc = CS_BAD;
					break;
				}
			}
			remap_one(hSession, ebc, iso, scope, one_way);

			ebc++;
			s = CN;
		}
		if (ebc != 256) {
			popup_an_error(NULL,_( "Charset has %d entries, need 256" ), ebc);
			rc = CS_BAD;
		} else {
			//
			// The entire EBCDIC-to-ASCII mapping has been defined.
			// Make sure that any printable ASCII character that
			// doesn't now map back onto itself is mapped onto an
			// EBCDIC NUL.
			//
			int i;

			for (i = 0; i < 256; i++) {
				if ((i & 0x7f) > 0x20 && i != 0x7f &&
						hSession->charset.asc2ebc[i] != 0 &&
						hSession->charset.ebc2asc[hSession->charset.asc2ebc[i]] != i) {
					hSession->charset.asc2ebc[i] = 0;
				}
			}
		}
	} else {
		while ((ns = split_dresource(&s, &ebcs, &isos))) {
			char *ptr;

			(*ne)++;
			if (ebcs[0] == '*') {
				one_way = True;
				ebcs++;
			} else
				one_way = False;
			if (ns < 0 ||
			    ((ebc = strtoul(ebcs, &ptr, 0)),
			     ptr == ebcs || *ptr != '\0') ||
			    (iso = parse_keysym(isos, True)) == NoSymbol) {
				popup_an_error(hSession,_( "Cannot parse %s \"%s\", entry %d" ), "charset", csname, *ne);
				rc = CS_BAD;
				break;
			}
			remap_one(hSession, ebc, iso, scope, one_way);
		}
	}
	lib3270_free(spec);
	return rc;
}

#if defined(DEBUG_CHARSET)
static char *
char_if_ascii7(unsigned long l)
{
	static char buf[6];

	if (((l & 0x7f) > ' ' && (l & 0x7f) < 0x7f) || l == 0xff) {
		(void) sprintf(buf, " ('%c')", (char)l);
		return buf;
	} else
		return "";
}
#endif


#if defined(DEBUG_CHARSET)
//
// Verify that a character set is not ambiguous.
// (All this checks is that multiple EBCDIC codes map onto the same ISO code.
//  Hmm.  God, I find the CG stuff confusing.)
//
static enum cs_result
check_charset(void)
{
	unsigned long iso;
	unsigned char ebc;
	enum cs_result rc = CS_OKAY;

	for (iso = 1; iso <= 255; iso++) {
		unsigned char multi[256];
		int n_multi = 0;

		if (iso == ' ')
			continue;

		for (ebc = 0x41; ebc < 0xff; ebc++) {
			if (cg2asc[ebc2cg[ebc]] == iso) {
				multi[n_multi] = ebc;
				n_multi++;
			}
		}
		if (n_multi > 1) {
			xs_warning("Display character 0x%02x%s has multiple "
			    "EBCDIC definitions: X'%02X', X'%02X'%s",
			    iso, char_if_ascii7(iso),
			    multi[0], multi[1], (n_multi > 2)? ", ...": "");
			rc = CS_BAD;
		}
	}
	return rc;
}
#endif

void set_display_charset(H3270 *session, const char *dcs)
{
	session->charset.display = strdup(dcs);
}

static KeySym StringToKeysym(char *s)
{
	static struct
	{
		const char *name;
		KeySym keysym;
	} latin1[] =
	{
		{ "space", XK_space },
		{ "exclam", XK_exclam },
		{ "quotedbl", XK_quotedbl },
		{ "numbersign", XK_numbersign },
		{ "dollar", XK_dollar },
		{ "percent", XK_percent },
		{ "ampersand", XK_ampersand },
		{ "apostrophe", XK_apostrophe },
		{ "quoteright", XK_quoteright },
		{ "parenleft", XK_parenleft },
		{ "parenright", XK_parenright },
		{ "asterisk", XK_asterisk },
		{ "plus", XK_plus },
		{ "comma", XK_comma },
		{ "minus", XK_minus },
		{ "period", XK_period },
		{ "slash", XK_slash },
		{ "0", XK_0 },
		{ "1", XK_1 },
		{ "2", XK_2 },
		{ "3", XK_3 },
		{ "4", XK_4 },
		{ "5", XK_5 },
		{ "6", XK_6 },
		{ "7", XK_7 },
		{ "8", XK_8 },
		{ "9", XK_9 },
		{ "colon", XK_colon },
		{ "semicolon", XK_semicolon },
		{ "less", XK_less },
		{ "equal", XK_equal },
		{ "greater", XK_greater },
		{ "question", XK_question },
		{ "at", XK_at },
		{ "A", XK_A },
		{ "B", XK_B },
		{ "C", XK_C },
		{ "D", XK_D },
		{ "E", XK_E },
		{ "F", XK_F },
		{ "G", XK_G },
		{ "H", XK_H },
		{ "I", XK_I },
		{ "J", XK_J },
		{ "K", XK_K },
		{ "L", XK_L },
		{ "M", XK_M },
		{ "N", XK_N },
		{ "O", XK_O },
		{ "P", XK_P },
		{ "Q", XK_Q },
		{ "R", XK_R },
		{ "S", XK_S },
		{ "T", XK_T },
		{ "U", XK_U },
		{ "V", XK_V },
		{ "W", XK_W },
		{ "X", XK_X },
		{ "Y", XK_Y },
		{ "Z", XK_Z },
		{ "bracketleft", XK_bracketleft },
		{ "backslash", XK_backslash },
		{ "bracketright", XK_bracketright },
		{ "asciicircum", XK_asciicircum },
		{ "underscore", XK_underscore },
		{ "grave", XK_grave },
		{ "quoteleft", XK_quoteleft },
		{ "a", XK_a },
		{ "b", XK_b },
		{ "c", XK_c },
		{ "d", XK_d },
		{ "e", XK_e },
		{ "f", XK_f },
		{ "g", XK_g },
		{ "h", XK_h },
		{ "i", XK_i },
		{ "j", XK_j },
		{ "k", XK_k },
		{ "l", XK_l },
		{ "m", XK_m },
		{ "n", XK_n },
		{ "o", XK_o },
		{ "p", XK_p },
		{ "q", XK_q },
		{ "r", XK_r },
		{ "s", XK_s },
		{ "t", XK_t },
		{ "u", XK_u },
		{ "v", XK_v },
		{ "w", XK_w },
		{ "x", XK_x },
		{ "y", XK_y },
		{ "z", XK_z },
		{ "braceleft", XK_braceleft },
		{ "bar", XK_bar },
		{ "braceright", XK_braceright },
		{ "asciitilde", XK_asciitilde },
		{ "nobreakspace", XK_nobreakspace },
		{ "exclamdown", XK_exclamdown },
		{ "cent", XK_cent },
		{ "sterling", XK_sterling },
		{ "currency", XK_currency },
		{ "yen", XK_yen },
		{ "brokenbar", XK_brokenbar },
		{ "section", XK_section },
		{ "diaeresis", XK_diaeresis },
		{ "copyright", XK_copyright },
		{ "ordfeminine", XK_ordfeminine },
		{ "guillemotleft", XK_guillemotleft },
		{ "notsign", XK_notsign },
		{ "hyphen", XK_hyphen },
		{ "registered", XK_registered },
		{ "macron", XK_macron },
		{ "degree", XK_degree },
		{ "plusminus", XK_plusminus },
		{ "twosuperior", XK_twosuperior },
		{ "threesuperior", XK_threesuperior },
		{ "acute", XK_acute },
		{ "mu", XK_mu },
		{ "paragraph", XK_paragraph },
		{ "periodcentered", XK_periodcentered },
		{ "cedilla", XK_cedilla },
		{ "onesuperior", XK_onesuperior },
		{ "masculine", XK_masculine },
		{ "guillemotright", XK_guillemotright },
		{ "onequarter", XK_onequarter },
		{ "onehalf", XK_onehalf },
		{ "threequarters", XK_threequarters },
		{ "questiondown", XK_questiondown },
		{ "Agrave", XK_Agrave },
		{ "Aacute", XK_Aacute },
		{ "Acircumflex", XK_Acircumflex },
		{ "Atilde", XK_Atilde },
		{ "Adiaeresis", XK_Adiaeresis },
		{ "Aring", XK_Aring },
		{ "AE", XK_AE },
		{ "Ccedilla", XK_Ccedilla },
		{ "Egrave", XK_Egrave },
		{ "Eacute", XK_Eacute },
		{ "Ecircumflex", XK_Ecircumflex },
		{ "Ediaeresis", XK_Ediaeresis },
		{ "Igrave", XK_Igrave },
		{ "Iacute", XK_Iacute },
		{ "Icircumflex", XK_Icircumflex },
		{ "Idiaeresis", XK_Idiaeresis },
		{ "ETH", XK_ETH },
		{ "Eth", XK_Eth },
		{ "Ntilde", XK_Ntilde },
		{ "Ograve", XK_Ograve },
		{ "Oacute", XK_Oacute },
		{ "Ocircumflex", XK_Ocircumflex },
		{ "Otilde", XK_Otilde },
		{ "Odiaeresis", XK_Odiaeresis },
		{ "multiply", XK_multiply },
		{ "Ooblique", XK_Ooblique },
		{ "Ugrave", XK_Ugrave },
		{ "Uacute", XK_Uacute },
		{ "Ucircumflex", XK_Ucircumflex },
		{ "Udiaeresis", XK_Udiaeresis },
		{ "Yacute", XK_Yacute },
		{ "THORN", XK_THORN },
		{ "Thorn", XK_Thorn },
		{ "ssharp", XK_ssharp },
		{ "agrave", XK_agrave },
		{ "aacute", XK_aacute },
		{ "acircumflex", XK_acircumflex },
		{ "atilde", XK_atilde },
		{ "adiaeresis", XK_adiaeresis },
		{ "aring", XK_aring },
		{ "ae", XK_ae },
		{ "ccedilla", XK_ccedilla },
		{ "egrave", XK_egrave },
		{ "eacute", XK_eacute },
		{ "ecircumflex", XK_ecircumflex },
		{ "ediaeresis", XK_ediaeresis },
		{ "igrave", XK_igrave },
		{ "iacute", XK_iacute },
		{ "icircumflex", XK_icircumflex },
		{ "idiaeresis", XK_idiaeresis },
		{ "eth", XK_eth },
		{ "ntilde", XK_ntilde },
		{ "ograve", XK_ograve },
		{ "oacute", XK_oacute },
		{ "ocircumflex", XK_ocircumflex },
		{ "otilde", XK_otilde },
		{ "odiaeresis", XK_odiaeresis },
		{ "division", XK_division },
		{ "oslash", XK_oslash },
		{ "ugrave", XK_ugrave },
		{ "uacute", XK_uacute },
		{ "ucircumflex", XK_ucircumflex },
		{ "udiaeresis", XK_udiaeresis },
		{ "yacute", XK_yacute },
		{ "thorn", XK_thorn },
		{ "ydiaeresis", XK_ydiaeresis },

		// The following are, umm, hacks to allow symbolic names for
		// control codes.
	#if !defined(_WIN32)
		{ "BackSpace", 0x08 },
		{ "Tab", 0x09 },
		{ "Linefeed", 0x0a },
		{ "Return", 0x0d },
		{ "Escape", 0x1b },
		{ "Delete", 0x7f },
	#endif

		{ (char *)NULL, NoSymbol }
	};

	int i;

	if (strlen(s) == 1 && (*(unsigned char *)s & 0x7f) > ' ')
		return (KeySym)*(unsigned char *)s;
	for (i = 0; latin1[i].name != (char *)NULL; i++) {
		if (!strcmp(s, latin1[i].name))
			return latin1[i].keysym;
	}
	return NoSymbol;
}

*/


