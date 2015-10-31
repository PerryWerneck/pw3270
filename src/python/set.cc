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
 * Este programa está nomeado como set.cc e possui - linhas de código.
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

 PyObject * terminal_set_string_at(PyObject *self, PyObject *args) {

	int row, col, rc;
	const char *text;

	if (!PyArg_ParseTuple(args, "iis", &row, &col, &text)) {
		PyErr_SetString(terminalError, strerror(EINVAL));
		return NULL;
	}

	try {

		rc = ((pw3270_TerminalObject *) self)->session->set_string_at(row,col,text);

	} catch(std::exception &e) {

		PyErr_SetString(terminalError, e.what());
		return NULL;
	}

	return PyLong_FromLong(rc);

 }

 PyObject * terminal_set_cursor_at(PyObject *self, PyObject *args) {

 	int row, col, rc;

	if (!PyArg_ParseTuple(args, "ii", &row, &col)) {
		PyErr_SetString(terminalError, strerror(EINVAL));
		return NULL;
	}

	try {

		rc = ((pw3270_TerminalObject *) self)->session->set_cursor_position(row,col);

	} catch(std::exception &e) {

		PyErr_SetString(terminalError, e.what());
		return NULL;
	}

	return PyLong_FromLong(rc);

 }

