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
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como log.c e possui 151 linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */


#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <lib3270/config.h>
#include <lib3270.h>

/*---[ Constants ]------------------------------------------------------------------------------------------*/

 static char logfile[FILENAME_MAX] = PACKAGE_NAME ".log";

/*---[ Implementacao ]--------------------------------------------------------------------------------------*/

 int Set3270Log(const char *filename)
 {
 	FILE *out;

 	if(strlen(filename) >= FILENAME_MAX)
		return EINVAL;

	out = fopen(filename,"a");

	if(out)
	{
		strcpy(logfile,filename);
		fclose(out);
#if defined(linux)
		printf("Logfile set to %s\n",logfile);
#endif
		return 0;
	}

	return errno;
 }

 static void writetime(FILE *out, const char *module)
 {
    time_t		ltime;
    char		wrk[40];

    time(&ltime);
    strftime(wrk, 39, "%d/%m/%Y %H:%M:%S", localtime(&ltime));
    fprintf(out,"%s %-8s\t",wrk,module);
 }

 static FILE *prefix(const char *module)
 {
    FILE *out = fopen(logfile,"a");
    if(out)
		writetime(out,module);
	return out;
 }

 /**
  * Grava uma entrada no arquivo de log.
  *
  * @param	module	Identificador do modulo para gravacao no arquivo
  * @param	fmt		String de formatacao para a mensagem no mesmo formato da funcao printf()
  * @param	...		Argumentos de acordo com a string de formatacao
  */
 int WriteLog(const char *module, const char *fmt, ...)
 {
    va_list arg_ptr;
    FILE    *out;

    out = prefix(module);
    if(!out)
       return -1;

    va_start(arg_ptr, fmt);
    vfprintf(out, fmt, arg_ptr);
    va_end(arg_ptr);
    fprintf(out,"\n");

    fclose(out);

    return 0;
 }

 /**
  * Grava mensagem de erro.
  *
  * Grava uma mensagem de erro no arquivo de log.
  *
  * @param	module	Identificador do modulo para gravacao no arquivo
  * @param	rc		Codigo de erro ou -1 para usar o valor de errno
  * @param	fmt		String de formatacao para a mensagem no mesmo formato da funcao printf()
  * @param	...		Argumentos de acordo com a string de formatacao
  */
 int WriteRCLog(const char *module, int rc, const char *fmt, ...)
 {
    FILE    *out;
    va_list arg_ptr;

	if(rc == -1)
	   rc = errno;

    if(!rc)
       return 0;

    out = prefix(module);
    if(!out)
		return -1;

    va_start(arg_ptr, fmt);
    vfprintf(out, fmt, arg_ptr);
    va_end(arg_ptr);

	fprintf(out,": %s (rc=%d)\n",strerror(rc),rc);

    fclose(out);

    return rc;
 }


