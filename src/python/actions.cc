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
 * Este programa está nomeado como actions.cc e possui - linhas de código.
 *
 * Contatos
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 * Referências:
 *
 * <https://docs.python.org/2/extending/newtypes.html>
 * <https://docs.python.org/2.7/extending/extending.html#a-simple-example>
 *
 */

 #include "private.h"


/*---[ Implement ]----------------------------------------------------------------------------------*/

 PyObject * terminal_pfkey(PyObject *self, PyObject *args) {

	int rc, key;

	if (!PyArg_ParseTuple(args, "i", &key)) {
		PyErr_SetString(terminalError, strerror(EINVAL));
		return NULL;
	}

	try {

		rc = ((pw3270_TerminalObject *) self)->session->pfkey(key);

	} catch(std::exception &e) {

		PyErr_SetString(terminalError, e.what());
		return NULL;
	}

	return PyLong_FromLong(rc);

 }

 PyObject * terminal_pakey(PyObject *self, PyObject *args) {

	int rc, key;

	if (!PyArg_ParseTuple(args, "i", &key)) {
		PyErr_SetString(terminalError, strerror(EINVAL));
		return NULL;
	}

	try {

		rc = ((pw3270_TerminalObject *) self)->session->pakey(key);

	} catch(std::exception &e) {

		PyErr_SetString(terminalError, e.what());
		return NULL;
	}

	return PyLong_FromLong(rc);

 }

 PyObject * terminal_enter(PyObject *self, PyObject *args) {

	int rc;

	try {

		rc = ((pw3270_TerminalObject *) self)->session->enter();

	} catch(std::exception &e) {

		PyErr_SetString(terminalError, e.what());
		return NULL;
	}

	return PyLong_FromLong(rc);


 }

 PyObject * terminal_action(PyObject *self, PyObject *args) {

	int rc;
	const char *name;

	if (!PyArg_ParseTuple(args, "s", &name)) {
		PyErr_SetString(terminalError, strerror(EINVAL));
		return NULL;
	}

	try {

		rc = ((pw3270_TerminalObject *) self)->session->action(name);

	} catch(std::exception &e) {

		PyErr_SetString(terminalError, e.what());
		return NULL;
	}

	return PyLong_FromLong(rc);


 }

