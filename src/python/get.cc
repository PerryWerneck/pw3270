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
 * Este programa está nomeado como get.cc e possui - linhas de código.
 *
 * Contatos:
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

PyObject * terminal_get_version(PyObject *self, PyObject *args) {

    return PyString_FromString( ((pw3270_TerminalObject *) self)->session->get_version().c_str() );

}

PyObject * terminal_get_revision(PyObject *self, PyObject *args) {

    return PyString_FromString( ((pw3270_TerminalObject *) self)->session->get_revision().c_str() );

}

PyObject * terminal_is_connected(PyObject *self, PyObject *args) {

    return PyBool_FromLong( ((pw3270_TerminalObject *) self)->session->is_connected() );

}

PyObject * terminal_is_ready(PyObject *self, PyObject *args) {

    return PyBool_FromLong( ((pw3270_TerminalObject *) self)->session->is_ready() );

}

PyObject * terminal_is_protected_at(PyObject *self, PyObject *args) {

	int rc, row, col;

	if (!PyArg_ParseTuple(args, "ii", &row, &col)) {
		PyErr_SetString(terminalError, strerror(EINVAL));
		return NULL;
	}

	try {

		rc = ((pw3270_TerminalObject *) self)->session->get_is_protected_at(row,col);

	} catch(std::exception &e) {

		PyErr_SetString(terminalError, e.what());
		return NULL;
	}

    return PyBool_FromLong( rc );

}


PyObject * terminal_cmp_string_at(PyObject *self, PyObject *args) {

	int row, col, rc;
	const char *text;

	if (!PyArg_ParseTuple(args, "iis", &row, &col, &text)) {
		PyErr_SetString(terminalError, strerror(EINVAL));
		return NULL;
	}

	try {

		rc = ((pw3270_TerminalObject *) self)->session->cmp_string_at(row,col,text);

	} catch(std::exception &e) {

		PyErr_SetString(terminalError, e.what());
		return NULL;
	}

	return PyLong_FromLong(rc);

}

PyObject * terminal_get_string_at(PyObject *self, PyObject *args) {

	int row, col, sz;
	string rc;

	if (!PyArg_ParseTuple(args, "iii", &row, &col, &sz)) {
		PyErr_SetString(terminalError, strerror(EINVAL));
		return NULL;
	}

	try {

		rc = ((pw3270_TerminalObject *) self)->session->get_string_at(row,col,sz);

	} catch(std::exception &e) {

		PyErr_SetString(terminalError, e.what());
		return NULL;
	}

	return PyString_FromString(rc.c_str());

}

PyObject * terminal_get_contents(PyObject *self) {

	string rc;

	try {

		rc = ((pw3270_TerminalObject *) self)->session->get_string();

	} catch(std::exception &e) {

		PyErr_SetString(terminalError, e.what());
		return NULL;
	}

	return PyString_FromString(rc.c_str());



}
