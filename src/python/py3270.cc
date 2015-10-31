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
 * Este programa está nomeado como main.cc e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 * Implementa métodos básicos para a extensão python.
 *
 * Referências:
 *
 * <https://docs.python.org/2/extending/newtypes.html>
 * <https://docs.python.org/2.7/extending/extending.html#a-simple-example>
 *
 */

 #include "private.h"

/*---[ Globals ]------------------------------------------------------------------------------------*/

 PyObject * terminalError = NULL;

/*---[ Implement ]----------------------------------------------------------------------------------*/

static PyObject * get_revision(PyObject *self, PyObject *args) {

    return PyLong_FromLong(atoi(PACKAGE_REVISION));

}

static PyObject * terminal_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {

    pw3270_TerminalObject *self = (pw3270_TerminalObject *) type->tp_alloc(type, 0);

	trace("%s: self=%p",__FUNCTION__,self);

	self->session = NULL;

    return (PyObject *)self;
}


static int terminal_init(pw3270_TerminalObject *self, PyObject *args, PyObject *kwds) {

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

static void terminal_dealloc(pw3270_TerminalObject * self) {

	trace("%s",__FUNCTION__);

	delete self->session;

    self->ob_type->tp_free((PyObject*)self);

}

static PyMethodDef terminal_methods[] = {

    {NULL}	// Sentinel

};

/*
static PyMemberDef terminal_members[] = {

    { NULL }	// Sentinel

};
*/

static PyTypeObject pw3270_TerminalType = {
    PyObject_HEAD_INIT(NULL)
    0,								/*ob_size*/
    "py3270.terminal",				/*tp_name*/
    sizeof(pw3270_TerminalObject),	/*tp_basicsize*/
    0,								/*tp_itemsize*/
    (destructor) terminal_dealloc,	/*tp_dealloc*/
    0,								/*tp_print*/
    0,								/*tp_getattr*/
    0,								/*tp_setattr*/
    0,								/*tp_compare*/
    0,								/*tp_repr*/
    0,								/*tp_as_number*/
    0,								/*tp_as_sequence*/
    0,								/*tp_as_mapping*/
    0,								/*tp_hash */
    0,								/*tp_call*/
    0,								/*tp_str*/
    0,								/*tp_getattro*/
    0,								/*tp_setattro*/
    0,								/*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,	/*tp_flags*/
    "3270 terminal object",			/* tp_doc */
    0,								/* tp_traverse */
    0,								/* tp_clear */
    0,								/* tp_richcompare */
    0,								/* tp_weaklistoffset */
    0,								/* tp_iter */
    0,								/* tp_iternext */
    terminal_methods,				/* tp_methods */
    0, // terminal_members,				/* tp_members */
    0,								/* tp_getset */
    0,								/* tp_base */
    0,								/* tp_dict */
    0,								/* tp_descr_get */
    0,								/* tp_descr_set */
    0,								/* tp_dictoffset */
    (initproc) terminal_init,		/* tp_init */
    0,								/* tp_alloc */
    terminal_new,					/* tp_new */

};

static PyMethodDef MyMethods[] = {

    { "revision",  get_revision, METH_VARARGS,	"Get module revision."	},

    {NULL, NULL, 0, NULL}        /* Sentinel */

};

PyMODINIT_FUNC initpy3270(void) {

	// Cria o módulo

    PyObject *m = Py_InitModule("py3270", MyMethods);

    if (m == NULL)
        return;

	// Adiciona objeto para tratamento de erros.
	terminalError = PyErr_NewException((char *) "py3270.error", NULL, NULL);

	(void) Py_INCREF(terminalError);
	PyModule_AddObject(m, "error", terminalError);

	// Adiciona terminal
    if(PyType_Ready(&pw3270_TerminalType) < 0)
        return

	(void) Py_INCREF(&pw3270_TerminalType);
    PyModule_AddObject(m, "terminal", (PyObject *)&pw3270_TerminalType);

}
