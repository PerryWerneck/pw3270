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
 * Este programa está nomeado como testprogram.cxx e possui - linhas de código.
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
#ifdef WIN32
	#include <windows.h>
	#define sleep(x) Sleep(x)
#endif

#define trace( fmt, ... ) fprintf(stderr, "%s(%d) " fmt "\n", __FILE__, __LINE__, __VA_ARGS__ ); fflush(stderr);

#include "globals.hpp"
#include <cppuhelper/bootstrap.hxx>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/XImplementationRegistration.hpp>
#include <com/sun/star/lang/XComponent.hpp>

using namespace com::sun::star::registry;
using namespace com::sun::star::lang;
using namespace cppu;

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

int SAL_CALL main(int argc, char **argv)
{
	Reference< XSimpleRegistry > xReg = createSimpleRegistry();

	OSL_ENSURE( xReg.is(), "### cannot get service instance of \"com.sun.star.regiystry.SimpleRegistry\"!" );

	xReg->open(OUString::createFromAscii("pw3270.uno.rdb"), sal_False, sal_False);

	OSL_ENSURE( xReg->isValid(), "### cannot open test registry \"pw3270.uno.rdb\"!" );

	trace("%s","Calling bootstrap_InitialComponentContext");
	Reference< XComponentContext > xContext = bootstrap_InitialComponentContext(xReg);
	OSL_ENSURE( xContext.is(), "### cannot creage intial component context!" );

	trace("%s","Calling getServiceManager\n");
	Reference< XMultiComponentFactory > xMgr = xContext->getServiceManager();
	OSL_ENSURE( xMgr.is(), "### cannot get initial service manager!" );

	// register my component
	trace("%s","Calling createInstanceWithContext");

	Reference< XImplementationRegistration > xImplReg(
	xMgr->createInstanceWithContext(OUString::createFromAscii("com.sun.star.registry.ImplementationRegistration"), xContext), UNO_QUERY);
	OSL_ENSURE( xImplReg.is(), "### cannot get service instance of \"com.sun.star.registry.ImplementationRegistration\"!" );

	if (xImplReg.is())
	{
        const char *libname = ".bin/Debug/pw3270.uno.so";

        trace("Loading %s",libname);

		xImplReg->registerImplementation(
                OUString::createFromAscii("com.sun.star.loader.SharedLibrary"), // loader for component
                OUString::createFromAscii(libname),		// component location
                Reference< XSimpleRegistry >()	        // registry omitted,
                                                        // defaulting to service manager registry used
			);

		// get an object instance
		printf("Calling createInstanceWithContext(%s)\n",IMPLNAME);

		Reference< XInterface > xx ;
		xx = xMgr->createInstanceWithContext(OUString::createFromAscii(IMPLNAME), xContext);

        printf("Instance: %p\n",&xx);

		Reference< pw3270intf > srv( xx, UNO_QUERY );

		OSL_ENSURE( srv.is(), "### cannot get service instance!");

		printf("object.is(): %d\n",srv.is());

		if(srv.is())
		{
			// Wait for commands
			char buffer[4096];
			OString	str;

			try
			{

			srv->setSession(OUString::createFromAscii("pw3270:a"));

			}
			catch( RuntimeException & e )
			{
				OString o = OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US );
				fprintf( stderr, "%s\n", o.pData->buffer );
				exit(-1);
			}

			printf("Revision:\t%d\n",srv->getRevision());

			srv->dsTrace(true);
			srv->screenTrace(true);

			printf("getConnectionState: %d\n", srv->getConnectionState());
			printf("Connect(): %d\n" , srv->Connect(OUString::createFromAscii("L:3270.df.bb:9023")));

			srv->sleep(2);
			printf("getConnectionState: %d\n", srv->getConnectionState());
			srv->sleep(2);

			str	= OUStringToOString( srv->getTextAt(1,1,2000),RTL_TEXTENCODING_UTF8);
			printf("ContentsAt(1,1):\n%s\n",str.pData->buffer);

			printf("getConnectionState: %d\n", srv->getConnectionState());

			srv->sleep(1);
			srv->enter();
			srv->sleep(1);

			str	= OUStringToOString( srv->getTextAt(1,1,2000),RTL_TEXTENCODING_UTF8);
			printf("ContentsAt(1,1):\n%s\n",str.pData->buffer);

			/*
			printf("waitForStringAt(SISBB) returned %d\n",srv->waitForStringAt(20,39,OUString::createFromAscii("SISBB"),20));
			printf("sendEnterKey() returned %d\n",srv->sendEnterKey());
			printf("waitForStringAt(Senha) returned %d\n",srv->waitForStringAt(14,2,OUString::createFromAscii("Senha"),20));
			printf("setStringAt returned %d\n",srv->setStringAt(13,21,OUString::createFromAscii("c1103788")));

			str	=  OUStringToOString( srv->getScreenContent(),RTL_TEXTENCODING_UTF8);
			printf("Entire screen:\n%s\n",str.pData->buffer);


			*/

			printf("Enter to exit...\n");
			fgets(buffer,80,stdin);

			printf("Disconnect(): %d\n" , srv->Disconnect());

			srv->sleep(5);

		}
	}


	Reference< XComponent >::query( xContext )->dispose();

	return 0;
}
