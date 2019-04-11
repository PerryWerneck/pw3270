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
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como private.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#ifndef PRIVATE_H_INCLUDED

	#define PRIVATE_H_INCLUDED

	#include "v3270ft.h"

/*--[ Widget definition ]----------------------------------------------------------------------------*/

	enum _filename
	{
		FILENAME_LOCAL,
		FILENAME_HOST,

		FILENAME_COUNT
	};

	enum _value
	{
		VALUE_LRECL,
		VALUE_BLKSIZE,
		VALUE_PRIMSPACE,
		VALUE_SECSPACE,
		VALUE_DFT,

		VALUE_COUNT
	};

	enum _button
	{
		BUTTON_ASCII,
		BUTTON_CRLF,
		BUTTON_APPEND,
		BUTTON_REMAP,

		BUTTON_COUNT
	};

	struct _v3270FTD
	{
		GtkDialog			  parent;
		GtkWidget			* filename[FILENAME_COUNT];	/**< Filenames for the transfer */
		GtkWidget			* units;					/**< Units frame box */
		GtkWidget			* ready;					/**< Send/Save button */
		GtkToggleButton		* button[BUTTON_COUNT];		/**< Buttons */
		GtkToggleButton 	* recfm[4];					/**< Record format buttons */
		GtkToggleButton 	* btnUnits[4];				/**< Unit buttons */
		GtkSpinButton		* value[VALUE_COUNT];
		gboolean 			  local;					/**< TRUE if local filename is ok */
		gboolean			  remote;					/**< TRUE if remote filename is ok */
		LIB3270_FT_OPTION	  options;
	};

	struct _v3270FTDClass
	{
		GtkDialogClass parent_class;

		int dummy;
	};

//	G_GNUC_INTERNAL void browse_file(GtkButton *button,v3270FTD *parent);


#endif // PRIVATE_H_INCLUDED
