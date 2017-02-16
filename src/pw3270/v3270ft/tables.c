/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob
 * o nome G3270.
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
 * Este programa está nomeado como v3270ft.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <limits.h>
 #include "private.h"


/*--[ Globals ]--------------------------------------------------------------------------------------*/

const struct v3270ft_option ft_option[NUM_OPTIONS_WIDGETS] = {

	// Transfer options
	{
		LIB3270_FT_OPTION_ASCII,
		"ascii",
		N_("_Text file"),
		N_( "Check this if the file consists of character data only.")
	},
	{
		LIB3270_FT_OPTION_CRLF,
		"crlf",
		N_("Follow the convention for _ASCII text files."),
#ifdef _WIN32
		N_( "Following the convention for ASCII text files, CR/LF pairs are used to terminate records in the PC file, and a CTRL-Z (x'1A') marks the end of file.")
#else
		N_( "Following the convention for ASCII text files, LF is used to terminate records in the PC file.")
#endif // _WIN32

	},
	{
		LIB3270_FT_OPTION_APPEND,
		"append",
		N_("A_ppend to file"),
		N_( "Appends the source file to the destination file.")
	},
	{
		LIB3270_FT_OPTION_REMAP,
		"remap",
		N_("Re_map ASCII Characters."),
		N_("Remap the text to ensure maximum compatibility between the workstation's character set and encoding and the host's EBCDIC code page.")
	},

	// Record format
	{
		LIB3270_FT_RECORD_FORMAT_DEFAULT,
		"recfm.default",
		N_("Default"),
		N_("Use host default record format.")
	},
	{
		LIB3270_FT_RECORD_FORMAT_FIXED,
		"recfm.fixed",
		N_("Fixed"),
		N_("Creates a file with fixed-length records.")
	},
	{
		LIB3270_FT_RECORD_FORMAT_VARIABLE,
		"recfm.variable",
		N_("Variable"),
		N_("Creates a file with variable-length records.")
	},
	{
		LIB3270_FT_RECORD_FORMAT_UNDEFINED,
		"recfm.undefined",
		N_("Undefined"),
		N_("Creates a file with undefined-length records (TSO hosts only).")
	},

	// Space allocation units
	{
		LIB3270_FT_ALLOCATION_UNITS_DEFAULT,
		"units.default",
		N_("Default"),
		NULL
	},
	{
		LIB3270_FT_ALLOCATION_UNITS_TRACKS,
		"units.tracks",
		N_("Tracks"),
		NULL
	},
	{
		LIB3270_FT_ALLOCATION_UNITS_CYLINDERS,
		"units.cylinders",
		N_("Cylinders"),
		NULL
	},
	{
		LIB3270_FT_ALLOCATION_UNITS_AVBLOCK,
		"units.avblock",
		N_("Avblock"),
		NULL
	},

};

const struct v3270ft_type ft_type[] = {

	{
		LIB3270_FT_OPTION_SEND,
		"send",
		"binary",
		N_("Send file")
	},
	{
		LIB3270_FT_OPTION_RECEIVE,
		"receive",
		"binary",
		N_("Receive file")
	},
	{
		LIB3270_FT_OPTION_SEND|LIB3270_FT_OPTION_ASCII|LIB3270_FT_OPTION_CRLF|LIB3270_FT_OPTION_REMAP,
		"send",
		"text",
		N_("Send text file")
	},
	{
		LIB3270_FT_OPTION_RECEIVE|LIB3270_FT_OPTION_ASCII|LIB3270_FT_OPTION_CRLF|LIB3270_FT_OPTION_REMAP,
		"receive",
		"text",
		N_("Receive text file")
	}
};

const struct v3270ft_value ft_value[] = {
	{
		"lrecl",
		0, 32760,
		N_( "Record Length:" ),
		N_( "Specifies the logical record length (n) for a data set consisting of fixed length records or the maximum logical record length for a data set consisting of variable length records." )
	},


	{
		"primary",
		0,99999,
		N_( "Primary space:" ),
		N_( "Primary allocation for a file created on a TSO host.\nThe units are given by the space allocation units option." )
	},

	{
		"blksize",
		0,32760,
		N_( "Block size:" ),
		N_( "Specifies the block size (n) for a new data set. For data sets containing fixed " \
			"length records, the block size must be a multiple of the record length. " \
			"For data sets containing variable length records, the block size must be " \
			"greater than or equal to the record length plus four bytes. The block size " \
			"must not exceed the track length of the device on which the data set resides." )
	},

	{
		"secondary",
		0,99999,
		N_( "Secondary space:" ),
		N_( "Secondary allocation for a file created on a TSO host.\nThe units are given by the space allocation units option." )
	},

	{
		"dft",
		0,99999,
		N_( "DFT B_uffer size:" ),
		N_("Specifies the default buffer size for DFT IND$FILE file transfers.")
	},

};

