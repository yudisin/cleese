
/* Built-in functions */

#include "Python.h"

static PyObject *
builtin___import__(PyObject *self, PyObject *args)
{
	char *name;
	/* @@@ hack until I get ParseTuple working */
	name = ((PyStringObject *)(((PyTupleObject *)args)->ob_item[0]))->ob_sval;
	
	return PyImport_ImportModuleEx(name);
}

static PyObject *
builtin_ord(PyObject *self, PyObject *args)
{
        PyObject *obj;
        long ord;
        int size;

//        if (!PyArg_ParseTuple(args, "O:ord", &obj))
/**/	if(!PyArg_UnpackTuple(args, "ord", 1,1, &obj))
                return NULL;

        if (PyString_Check(obj)) {
                size = PyString_GET_SIZE(obj);
                if (size == 1) {
                        ord = (long)((unsigned char)*PyString_AS_STRING(obj));
                        return PyInt_FromLong(ord);
                }
//        } else if (PyUnicode_Check(obj)) {
//                size = PyUnicode_GET_SIZE(obj);
//                if (size == 1) {
//                        ord = (long)*PyUnicode_AS_UNICODE(obj);
//                        return PyInt_FromLong(ord);
//                }
//        } else {
//                PyErr_Format(PyExc_TypeError,
//                             "ord() expected string of length 1, but " \
//                             "%.200s found", obj->ob_type->tp_name);
//                return NULL;
        }
//
//        PyErr_Format(PyExc_TypeError,
//                     "ord() expected a character, "
//                     "but string of length %d found",
//                     size);
        return NULL;
}

static char ord_doc[] =
"ord(c) -> integer\n\
\n\
Return the integer ordinal of a one-character string.";


static PyMethodDef builtin_methods[] = {
 	{"__import__",	builtin___import__, METH_VARARGS, NULL/*doc*/},
        {"ord",         builtin_ord, 1, ord_doc},
	{NULL,		NULL},
};

PyObject *
_PyBuiltin_Init(void)
{
	PyObject *mod, *dict;
	mod = Py_InitModule4("__builtin__", builtin_methods,
			     NULL/*doc*/, (PyObject *)NULL,
			     PYTHON_API_VERSION);

	dict = PyModule_GetDict(mod);

	#define SETBUILTIN(NAME, OBJECT) \
	if (PyDict_SetItemString(dict, NAME, (PyObject *)OBJECT) < 0)	\
		return NULL;

	SETBUILTIN("None",		Py_None);

	return mod;
}
