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

 #include <stdio.h>
 #include <string.h>
 #include <time.h>
 #include <pw3270/hllapi.h>

 #if defined(DEBUG) && defined(_WIN32)
	#undef trace
	#define trace( fmt, ... )	{ FILE *out = fopen("c:\\Users\\Perry\\hllapi.log","a"); if(out) { fprintf(out, "%s(%d) " fmt "\n", __FILE__, __LINE__, __VA_ARGS__ ); fclose(out); } }
 #endif // DEBUG

 #define MAX_DATA_SIZE 8000	//< Maximum data size for this application.

 static CHAR hllapi_data[MAX_DATA_SIZE];

/*---[ Implement ]--------------------------------------------------------------------------------*/

 static void connect_ps(const char *session)
 {
 	WORD len;
 	WORD fn		= HLLAPI_CMD_CONNECTPS;
 	WORD rc 	= 0;

	strncpy(hllapi_data,session,MAX_DATA_SIZE);

	len = strlen(session);
	hllapi(&fn,hllapi_data,&len,&rc);

	/*
	if(rc)
	{
		trace("HLLAPI_CMD_CONNECTPS(%s) exits with rc=%d", session, (int) rc);
	}
	*/

 }

 int main(int numpar, char *param[])
 {
 	const char *session = "pw3270:A";

 	connect_ps(session);

 	/*

 	printf("init(%s)=%d\n",session,(int) hllapi_init((LPSTR) session));
	printf("revision=%d\n",(int) hllapi_get_revision());
	printf("connect=%d\n",(int) hllapi_connect((char *) "fandezhi.efglobe.com:23",1));
	printf("wait_for_ready=%d\n",(int) hllapi_wait_for_ready(10));
	printf("connected=%s\n",(int) hllapi_is_connected() ? "Yes" : "No");

//	printf("disconnect=%d\n",(int) hllapi_disconnect());


	printf("deinit=%d\n",(int) hllapi_deinit());

 	DWORD	revision;
 	int 	rc;

 	rc = hllapi_init("");
 	if(rc)
	{
		printf("Init exits with rc=%d\n",rc);
		return rc;
	}

	if(!hllapi_get_revision(&revision))
		printf("Library revision is %d\n",(int) revision);


	printf("Deinit exits with rc=%d\n",rc);
	rc = hllapi_deinit();
*/
	return 0;
 }

/*
 int main(int numpar, char *param[])
 {
	char buffer[BUFFER_LENGTH];
	unsigned short rc;
	unsigned short len;
	unsigned long fn;

	static const struct _cmd
	{
		const char 		* name;
		unsigned short	  fn;
		const char		* arg;
	} cmd[] =
	{
		{ "GetRevision",	HLLAPI_CMD_GETREVISION,	"       "	},
		{ "ConnectPS", 		HLLAPI_CMD_CONNECTPS, 	"pw3270A"	},
		{ "InputString",	HLLAPI_CMD_INPUTSTRING,	"test"		},

	};

	int f;


	for(f=0;f< (sizeof(cmd)/sizeof(struct _cmd)); f++)
	{
		len = strlen(cmd[f].arg);
		memcpy(buffer,cmd[f].arg,len);
		hllapi((LPWORD) &cmd[f].fn,buffer,&len,&rc);
		printf("%s exits with rc=%d\n[%s]\n",cmd[f].name,rc,buffer);
	}

	len = 80;
	rc	= 1040;
	fn = HLLAPI_CMD_COPYPSTOSTR;
	hllapi((LPWORD) &fn,buffer,&len,&rc);
	printf("%s exits with rc=%d\n%s\n","HLLAPI_CMD_COPYPSTOSTR",rc,buffer);

	// Performance
	len = strlen(cmd[0].arg);
	memcpy(buffer,cmd[0].arg,len);
	if(hllapi((LPWORD) &cmd[0].fn,buffer,&len,&rc) == 0 && rc == 0)
	{
		time_t	end = time(0) + 5;
		int 	qtd = 0;
		do
		{
			qtd++;
		} while(hllapi((LPWORD) &cmd[0].fn,buffer,&len,&rc) == 0 && rc == 0 && time(0)<end);
		printf("%d interacoes em 5 segundos (rc=%d)\n",qtd,rc);

	}

	// Disconnect
	len = 10;
	rc	= 1;
	fn = HLLAPI_CMD_DISCONNECTPS;
	*buffer = 0;
	hllapi((LPWORD) &fn,buffer,&len,&rc);
	printf("%s exits with rc=%d\n[%s]\n","HLLAPI_CMD_DISCONNECTPS",rc,buffer);


 	return 0;
 }
*/
