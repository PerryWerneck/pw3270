/*
 * Modifications Copyright 1996, 1999 2000 by Paul Mattes.
 * Copyright Octover 1995 by Dick Altenbern.
 * Based in part on code Copyright 1993, 1994, 1995 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 *
 * x3270, c3270, s3270 and tcl3270 are distributed in the hope that they will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the file LICENSE
 * for more details.
 */

/* DFT-style file transfer codes. */

/* Host requests. */
#define TR_OPEN_REQ		0x0012	/* open request */
#define TR_CLOSE_REQ		0x4112	/* close request */

#define TR_SET_CUR_REQ		0x4511	/* set cursor request */
#define TR_GET_REQ		0x4611	/* get request */
#define TR_INSERT_REQ		0x4711	/* insert request */

#define TR_DATA_INSERT		0x4704	/* data to insert */

/* PC replies. */
#define TR_GET_REPLY		0x4605	/* data for get */
#define TR_NORMAL_REPLY		0x4705	/* insert normal reply */
#define TR_ERROR_REPLY		0x08	/* error reply (low 8 bits) */
#define TR_CLOSE_REPLY		0x4109	/* close acknowledgement */

/* Other headers. */
#define TR_RECNUM_HDR		0x6306	/* record number header */
#define TR_ERROR_HDR		0x6904	/* error header */
#define TR_NOT_COMPRESSED	0xc080	/* data not compressed */
#define TR_BEGIN_DATA		0x61	/* beginning of data */

/* Error codes. */
#define TR_ERR_EOF		0x2200	/* get past end of file */
#define TR_ERR_CMDFAIL		0x0100	/* command failed */
