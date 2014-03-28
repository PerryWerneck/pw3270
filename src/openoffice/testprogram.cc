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
 * Este programa está nomeado como testprogram.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include "globals.hpp"
 #include <com/sun/star/registry/DefaultRegistry.hpp>
 #include "pw3270/lib3270.hpp"

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

using namespace ::com::sun::star::registry;

int SAL_CALL main(int argc, char **argv)
{
	Reference< XSimpleRegistry > xReg = DefaultRegistry();
/*

	Reference< XSimpleRegistry > xReg = createSimpleRegistry();

	OSL_ENSURE( xReg.is(), "### cannot get service instance of \"SimpleRegistry\"!" );

	xReg->open(OUString::createFromAscii("pw3270.rdb"), sal_False, sal_False);

	OSL_ENSURE( xReg->isValid(), "### cannot open test registry \"pw3270.rdb\"!" );


	TRACE("%s","Calling bootstrap_InitialComponentContext");
	Reference< XComponentContext > xContext = bootstrap_InitialComponentContext(xReg);
	OSL_ENSURE( xContext.is(), "### cannot creage intial component context!" );

	TRACE("%s","Calling getServiceManager\n");
	Reference< XMultiComponentFactory > xMgr = xContext->getServiceManager();
	OSL_ENSURE( xMgr.is(), "### cannot get initial service manager!" );

	// register my component
	TRACE("%s","Calling createInstanceWithContext");

	Reference< XImplementationRegistration > xImplReg(
	xMgr->createInstanceWithContext(OUString::createFromAscii("com.sun.star.registry.ImplementationRegistration"), xContext), UNO_QUERY);
	OSL_ENSURE( xImplReg.is(), "### cannot get service instance of \"com.sun.star.registry.ImplementationRegistration\"!" );

	if (xImplReg.is())
	{
        const char *libname = LIBNAME;

        TRACE("Loading %s",libname);

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
			OString	str;
			char	buffer[80];
			printf("getConnectionState: %d\n", srv->getConnectionState());

			str = OUStringToOString( srv->getVersion(),RTL_TEXTENCODING_UTF8);
			printf("Version:\t%s\n",str.pData->buffer);

			str = OUStringToOString( srv->getRevision(),RTL_TEXTENCODING_UTF8);
			printf("Revision:\t%s\n",str.pData->buffer);

			printf("Connect(): %d\n" , srv->Connect(OUString::createFromAscii("L:3270.df.bb:9023"),10));

			sleep(5);

			//str	=  OUStringToOString( srv->getScreenContentAt(20,39,5),RTL_TEXTENCODING_UTF8);
			//Trace("ContentsAt(20,39): \"%s\"",str.pData->buffer);
			printf("waitForStringAt(SISBB) returned %d\n",srv->waitForStringAt(20,39,OUString::createFromAscii("SISBB"),20));
			printf("sendEnterKey() returned %d\n",srv->sendEnterKey());
			printf("waitForStringAt(Senha) returned %d\n",srv->waitForStringAt(14,2,OUString::createFromAscii("Senha"),20));
			printf("setStringAt returned %d\n",srv->setStringAt(13,21,OUString::createFromAscii("c1103788")));

			str	=  OUStringToOString( srv->getScreenContent(),RTL_TEXTENCODING_UTF8);
			printf("Entire screen:\n%s\n",str.pData->buffer);

			printf("Enter to exit...\n");
			fgets(buffer,80,stdin);

			printf("Disconnect(): %d\n" , srv->Disconnect());

			sleep(5);

		}
	}


	Reference< XComponent >::query( xContext )->dispose();

*/

	return 0;
}
