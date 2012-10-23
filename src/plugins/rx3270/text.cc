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
 * Este programa está nomeado como text.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "rx3270.h"
 #include <lib3270/actions.h>

 #include <string.h>


/*--[ Globals ]--------------------------------------------------------------------------------------*/

#ifdef HAVE_ICONV
	iconv_t outputConv = (iconv_t) (-1);
	iconv_t inputConv = (iconv_t) (-1);
#endif

/*--[ Implement ]------------------------------------------------------------------------------------*/

char * get_contents(H3270 *hSession, int start, int sz)
{
	unsigned char	  str[sz+1];
	unsigned short	  attr[sz+1];

	if(!lib3270_get_contents(hSession,start,start+sz,str,attr))
		return NULL;

#ifdef HAVE_ICONV
	// Convert received string to rexx encoding
	if(outputConv != (iconv_t)(-1))
	{
		size_t	in = strlen((char *) str);
		size_t out = (in << 1);
		char *ptr;
		char *buffer = (char *) malloc(out);
		char *ret;

		memset(ptr=buffer,0,out);

		iconv(outputConv,NULL,NULL,NULL,NULL);	// Reset state

		if(iconv(outputConv,(char **) &str,&in,&ptr,&out) == ((size_t) -1))
			ret = strdup((char *) str);
		else
			ret = strdup(buffer);

		free(buffer);

		return ret;
	}

#endif // HAVE_ICONV

	return strdup((char *) str);
}

