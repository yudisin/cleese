
/* Function object implementation */

#include "Python.h"

PyTypeObject PyFunction_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,
	"function",
	sizeof(PyFunctionObject),
	0,
	0, //(destructor)func_dealloc,		/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0,					/* tp_compare */
	0, //(reprfunc)func_repr,			/* tp_repr */
	0,					/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0,					/* tp_hash */
	0, //function_call,				/* tp_call */
	0,					/* tp_str */
	0, //PyObject_GenericGetAttr,		/* tp_getattro */
	0, //PyObject_GenericSetAttr,		/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,/* tp_flags */
	0, //func_doc,				/* tp_doc */
	0, //(traverseproc)func_traverse,		/* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0, //offsetof(PyFunctionObject, func_weakreflist), /* tp_weaklistoffset */
	0,					/* tp_iter */
	0,					/* tp_iternext */
	0,					/* tp_methods */
	0, //func_memberlist,			/* tp_members */
	0, //func_getsetlist,			/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0, //func_descr_get,				/* tp_descr_get */
	0,					/* tp_descr_set */
	0, //offsetof(PyFunctionObject, func_dict),	/* tp_dictoffset */
	0,					/* tp_init */
	0,					/* tp_alloc */
	0, //func_new,				/* tp_new */
};
