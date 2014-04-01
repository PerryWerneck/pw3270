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
 * Este programa está nomeado como testprogram.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

 #include <pw3270/class.h>
 #include <unistd.h>
 #include <iostream>

 using namespace std;
 using namespace PW3270_NAMESPACE;

/*--[ Implement ]------------------------------------------------------------------------------------*/

 int main(int numpar, char *param[])
 {

 	{
		string	*s;
		session	*session = session::start("pw3270:a");
	// 	session	*session = session::start("new");

		cout << "pw3270 version:  " << session->get_version() << endl;
		cout << "pw3270 revision: " << session->get_revision() << endl << endl;

		if(session->is_connected())
			cout << "\tConnected to host" << endl;
		else
			cout << "\tDisconnected" << endl;

		cout << "\tSession state:   " << session->get_cstate() << endl;

		s = session->get_display_charset();
		cout << "\tDisplay charset: " << s->c_str() << endl;
		delete s;

		s = session->get_host_charset();
		cout << "\tHost charset:    " << s->c_str() << endl;
		delete s;

		session->connect(false);
		delete session;
 	}

	// Waits
	sleep(2);

	// Create another session
	{
		session	*session = session::start("pw3270:a");

		session->disconnect();
		delete session;

	}


 	return 0;
 }


