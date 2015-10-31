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

static PyMethodDef terminal_methods[] = {

    { "Version",  				terminal_get_version,			METH_NOARGS,	"Get the lib3270 version string."						},
    { "Revision",  				terminal_get_revision,			METH_NOARGS,	"Get the lib3270 revision number."						},

    { "IsConnected",			terminal_is_connected,			METH_NOARGS,	"True if the terminal is connected to the host."		},
    { "IsReady",  				terminal_is_ready,				METH_NOARGS,	"True if the terminal has finished network activity."	},
    { "IsProtected",  			terminal_is_protected_at,		METH_VARARGS,	"True if the position is read-only."					},

    { "SetCursorPosition",		terminal_set_cursor_at,			METH_VARARGS,	"Set cursor position."									},

    { "WaitForStringAt",		terminal_wait_for_string_at,	METH_VARARGS,	"Wait for string at position"							},
    { "WaitForReady",			terminal_wait_for_ready,		METH_VARARGS,	"Wait for network communication to finish"				},

    { "Connect",				terminal_connect,				METH_VARARGS,	"Connect to the host."									},
    { "Disconnect",  			terminal_disconnect,			METH_NOARGS,	"Disconnect from host."									},

    { "CmpStringAt",			terminal_cmp_string_at,			METH_VARARGS,	"Compare string with terminal buffer at the position."	},
    { "GetStringAt",  			terminal_get_string_at,			METH_VARARGS,	"Get string from terminal buffer."						},
    { "SetStringAt",  			terminal_set_string_at,			METH_VARARGS,	"Set string in terminal buffer."						},

    { "PFKey",  				terminal_pfkey,					METH_VARARGS,	"Send PF key."											},
    { "PAKey",  				terminal_pakey,					METH_VARARGS,	"Send PA key."											},
    { "Enter",  				terminal_enter,					METH_NOARGS,	"Send Enter Key."										},
    { "Action",  				terminal_action,				METH_VARARGS,	"Send Action by name."									},

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

    { "Revision",  get_revision, METH_VARARGS,	"Get module revision."	},

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
    PyModule_AddObject(m, "Terminal", (PyObject *)&pw3270_TerminalType);

}
