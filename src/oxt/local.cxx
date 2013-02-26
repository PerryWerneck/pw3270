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
 * Este programa está nomeado como local.cxx e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

 #include "globals.hpp"

// osl_createEmptySocketAddr
// osl_connectSocketTo

/*---[ Statics ]-------------------------------------------------------------------------------------------*/


/*---[ Implement ]-----------------------------------------------------------------------------------------*/

 pw3270::lib3270_session::lib3270_session()
 {
 	void * (*lib3270_new)(const char *);

	trace("%s",__FUNCTION__);
	hModule = osl_loadModuleAscii("lib3270.so",SAL_LOADMODULE_NOW);
	if(!hModule)
		return;

	_get_revision = (const char	* (*)(void)) osl_getAsciiFunctionSymbol(hModule,"lib3270_get_revision");


	/* Get lib3270 session handle */
	lib3270_new = (void * (*)(const char *)) osl_getAsciiFunctionSymbol(hModule,"lib3270_session_new");
	hSession = lib3270_new("");

 }

 pw3270::lib3270_session::~lib3270_session()
 {

	trace("%s hModule=%p hSession=%p",__FUNCTION__,hModule,hSession);

	if(hModule)
	{
		if(hSession)
		{
			void (*lib3270_free)(void *) = (void (*)(void *)) osl_getAsciiFunctionSymbol(hModule,"lib3270_session_free");
			lib3270_free(hSession);
		}
		osl_unloadModule(hModule);
	}

 }

 int pw3270::lib3270_session::get_revision(void)
 {
	if(!_get_revision)
		return -1;
	return atoi(_get_revision());
 }

