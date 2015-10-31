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
 * Este programa está nomeado como private.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#ifndef PRIVATE_H_INCLUDED

	#define PRIVATE_H_INCLUDED

	#include <Python.h>

	#include <lib3270/config.h>
	#include <pw3270/class.h>

	using namespace std;

	typedef struct {

		PyObject_HEAD

		PW3270_NAMESPACE::session * session;

	} pw3270_TerminalObject;

	extern PyObject * terminalError;

	extern "C" {

		PyObject	* terminal_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
		int			  terminal_init(pw3270_TerminalObject *self, PyObject *args, PyObject *kwds);
		void		  terminal_dealloc(pw3270_TerminalObject * self);

		PyObject	* terminal_get_version(PyObject *self, PyObject *args);
		PyObject	* terminal_get_revision(PyObject *self, PyObject *args);

		PyObject	* terminal_is_connected(PyObject *self, PyObject *args);
		PyObject	* terminal_is_ready(PyObject *self, PyObject *args);

		PyObject	* terminal_connect(PyObject *self, PyObject *args);
		PyObject	* terminal_disconnect(PyObject *self, PyObject *args);

		PyObject 	* terminal_get_string_at(PyObject *self, PyObject *args);
		PyObject 	* terminal_set_string_at(PyObject *self, PyObject *args);
		PyObject	* terminal_cmp_string_at(PyObject *self, PyObject *args);

		PyObject 	* terminal_pfkey(PyObject *self, PyObject *args);
		PyObject 	* terminal_pakey(PyObject *self, PyObject *args);
		PyObject 	* terminal_enter(PyObject *self, PyObject *args);
		PyObject 	* terminal_action(PyObject *self, PyObject *args);

		PyObject 	* terminal_is_protected_at(PyObject *self, PyObject *args);
		PyObject 	* terminal_set_cursor_at(PyObject *self, PyObject *args);

		PyObject	* terminal_wait_for_ready(PyObject *self, PyObject *args);
		PyObject	* terminal_wait_for_string_at(PyObject *self, PyObject *args);

	}

#endif // PRIVATE_H_INCLUDED
