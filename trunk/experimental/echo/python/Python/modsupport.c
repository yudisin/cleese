
/* Module support implementation */

#include "Python.h"

PyObject *
Py_InitModule4(char *name, PyMethodDef *methods, char *doc,
	       PyObject *passthrough, int module_api_version)
{
	PyObject *m, *d, *v, *n;
	PyMethodDef *ml;
	if ((m = PyImport_AddModule(name)) == NULL)
		return NULL;
	d = PyModule_GetDict(m);
	if (methods != NULL) {
		n = PyString_FromString(name);
		if (n == NULL)
			return NULL;
		for (ml = methods; ml->ml_name != NULL; ml++) {
			v = PyCFunction_NewEx(ml, passthrough, n);
			if (v == NULL)
				return NULL;
			if (PyDict_SetItemString(d, ml->ml_name, v) != 0) {
				Py_DECREF(v);
				return NULL;
			}
			Py_DECREF(v);
		}
	}
	return m;
}
