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
 * Este programa está nomeado como pluginmain.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#ifndef RX3270_H_INCLUDED

 #define RX3270_H_INCLUDED 1

 #include <errno.h>
 #include <stdio.h>
 #include <lib3270/config.h>
 #include <lib3270.h>
 #include <lib3270/log.h>
 #include <oorexxapi.h>

/*---[ Exports ]---------------------------------------------------------------------------------------------*/

 LIB3270_EXPORT RexxRoutineEntry rx3270_functions[];
 LIB3270_EXPORT RexxPackageEntry rx3270_package_entry;

/*---[ Globals ]---------------------------------------------------------------------------------------------*/


/*--[ Prototipes ]-------------------------------------------------------------------------------------------*/

 LIB3270_EXPORT REXX_TYPED_ROUTINE_PROTOTYPE(rx3270version);



#endif // RX3270_H_INCLUDED
