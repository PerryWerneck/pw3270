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
 * Este programa está nomeado como py3270.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 * Implementa métodos básicos inicio/final do objeto python
 *
 * Referências:
 *
 * <https://docs.python.org/2/extending/newtypes.html>
 * <https://docs.python.org/2.7/extending/extending.html#a-simple-example>
 *
 */

 #include "private.h"


/*---[ Implement ]----------------------------------------------------------------------------------*/

PyObject * terminal_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {

    pw3270_TerminalObject *self = (pw3270_TerminalObject *) type->tp_alloc(type, 0);

	trace("%s: self=%p",__FUNCTION__,self);

	self->session = NULL;

    return (PyObject *)self;
}


int terminal_init(pw3270_TerminalObject *self, PyObject *args, PyObject *kwds) {

	const char *id = "";

	if (!PyArg_ParseTuple(args, "s", &id)) {
		id = "";
	}

	trace("%s(%s)",__FUNCTION__,id);

	try {

		self->session = PW3270_NAMESPACE::session::create(id);

	} catch(std::exception &e) {

		trace("%s failed: %s",__FUNCTION__,e.what());
		PyErr_SetString(terminalError, e.what());

	}


	return 0;

}

void terminal_dealloc(pw3270_TerminalObject * self) {

	trace("%s",__FUNCTION__);

	delete self->session;

    self->ob_type->tp_free((PyObject*)self);

}
