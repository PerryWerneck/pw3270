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
 * Este programa está nomeado como testprogram.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <windows.h>
 #include <stdio.h>
 #include <pw3270/hllapi.h>

/*---[ Implement ]--------------------------------------------------------------------------------*/

 int main(int numpar, char *param[])
 {
	char buffer[1024];
	unsigned short rc;

	static const struct _cmd
	{
		const char 		* name;
		unsigned long	  fn;
		const char		* arg;
	} cmd[] =
	{
		{ "ConnectPS", 		HLLAPI_CMD_CONNECTPS, 	"pw3270A"	},
		{ "GetRevision",	HLLAPI_CMD_GETREVISION,	""			},
		{ "InputString",	HLLAPI_CMD_INPUTSTRING,	"test"		},

		{ "DisconnectPS", 	HLLAPI_CMD_DISCONNECTPS, ""	},
	};

	int f;


	for(f=0;f< (sizeof(cmd)/sizeof(struct _cmd)); f++)
	{
		unsigned short len = 1024;
		int result;

		strcpy(buffer,cmd[f].arg);
		result = hllapi(&cmd[f].fn,buffer,&len,&rc);
		printf("%s exits with %d\n[%s]\n",cmd[f].name,result,buffer);

	}

 	return 0;
 }
