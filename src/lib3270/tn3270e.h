/*
 * Copyright 1996, 1999, 2001 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 *
 * x3270, c3270, s3270, tcl3270 and pr3287 are distributed in the hope that
 * they will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * file LICENSE for more details.
 */

/*
 *	tn3270e.h
 *
 *		Header file for the TN3270E Protocol, RFC 2355.
 */

/* Negotiation operations. */
#define TN3270E_OP_ASSOCIATE		0
#define TN3270E_OP_CONNECT		1
#define TN3270E_OP_DEVICE_TYPE		2
#define TN3270E_OP_FUNCTIONS		3
#define TN3270E_OP_IS			4
#define TN3270E_OP_REASON		5
#define TN3270E_OP_REJECT		6
#define TN3270E_OP_REQUEST		7
#define TN3270E_OP_SEND			8

/* Negotiation reason-codes. */
#define TN3270E_REASON_CONN_PARTNER	0
#define TN3270E_REASON_DEVICE_IN_USE	1
#define TN3270E_REASON_INV_ASSOCIATE	2
#define TN3270E_REASON_INV_DEVICE_NAME	3
#define TN3270E_REASON_INV_DEVICE_TYPE	4
#define TN3270E_REASON_TYPE_NAME_ERROR	5
#define TN3270E_REASON_UNKNOWN_ERROR	6
#define TN3270E_REASON_UNSUPPORTED_REQ	7

/* Negotiation function Names. */
#define TN3270E_FUNC_BIND_IMAGE		0
#define TN3270E_FUNC_DATA_STREAM_CTL	1
#define TN3270E_FUNC_RESPONSES		2
#define TN3270E_FUNC_SCS_CTL_CODES	3
#define TN3270E_FUNC_SYSREQ		4

/* Header data type names. */
#define TN3270E_DT_3270_DATA		0x00
#define TN3270E_DT_SCS_DATA		0x01
#define TN3270E_DT_RESPONSE		0x02
#define TN3270E_DT_BIND_IMAGE		0x03
#define TN3270E_DT_UNBIND		0x04
#define TN3270E_DT_NVT_DATA		0x05
#define TN3270E_DT_REQUEST		0x06
#define TN3270E_DT_SSCP_LU_DATA		0x07
#define TN3270E_DT_PRINT_EOJ		0x08

/* Header request flags. */
#define TN3270E_RQF_ERR_COND_CLEARED	0x00

/* Header response flags. */
#define TN3270E_RSF_NO_RESPONSE		0x00
#define TN3270E_RSF_ERROR_RESPONSE	0x01
#define TN3270E_RSF_ALWAYS_RESPONSE	0x02
#define TN3270E_RSF_POSITIVE_RESPONSE	0x00
#define TN3270E_RSF_NEGATIVE_RESPONSE	0x01

/* Header response data. */
#define TN3270E_POS_DEVICE_END		0x00
#define TN3270E_NEG_COMMAND_REJECT	0x00
#define TN3270E_NEG_INTERVENTION_REQUIRED 0x01
#define TN3270E_NEG_OPERATION_CHECK	0x02
#define TN3270E_NEG_COMPONENT_DISCONNECTED 0x03

/* TN3270E data header. */
typedef struct {
    unsigned char data_type;
    unsigned char request_flag;
    unsigned char response_flag;
    unsigned char seq_number[2]; /* actually, 16 bits, unaligned (!) */
} tn3270e_header;

#define EH_SIZE 5
