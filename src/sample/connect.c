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
 * Este programa está nomeado como main.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <lib3270.h>

/*--[ Implement ]------------------------------------------------------------------------------------*/

 int main(int numpar, char *param[])
 {
	H3270 *hSession;
	int rc;

	if(numpar != 2)
	{
		fprintf(stderr,"Inform host URI as argument\n");
		exit(-1);
	}

	/* Get session handle */
	hSession = lib3270_session_new("");

	/* Connect to the requested URI, wait for 3270 negotiation */
	rc = lib3270_connect(hSession,param[1],1);
	if(rc)
	{
		fprintf(stderr,"Can't connect to %s: %s\n",param[1],strerror(rc));
		return rc;
	}

	printf("Connected to LU %s\n",lib3270_get_luname(hSession));

	/* Wait until the host is ready for commands */
	rc = lib3270_wait_for_ready(hSession,60);
	if(rc)
	{
		fprintf(stderr,"Error waiting for session negotiation: %s\n",strerror(rc));
		return rc;
	}
	else
	{
		/* Host is ready, get screen contents */
		char *text = lib3270_get_text(hSession,0,-1);

		printf("\nScreen contents:\n%s\n",text);

		lib3270_free(text);
	}




	/* Release session handle */
	lib3270_session_free(hSession);
	return 0;
 }

