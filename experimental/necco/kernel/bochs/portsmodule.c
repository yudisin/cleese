#include "Python.h"

static PyObject *
ports_inb(PyObject *self, PyObject *args)
{
	PyObject *v = NULL;

	if (!PyArg_UnpackTuple(args, "inb", 1, 1, &v))
		return NULL;
	
	if (!PyInt_CheckExact(v)) {
		/* ERROR */
		return NULL;
	} else {
		unsigned short port = PyInt_AS_LONG(v);
		unsigned char data = in(port);
		PyObject *result = PyInt_FromLong(data);

		return result;
	}
}

static PyObject *
ports_outb(PyObject *self, PyObject *args)
{
	PyObject *v, *a;

	if(!PyArg_UnpackTuple(args, "outb", 2, 2, &v, &a))
		return NULL;
	if(!PyInt_CheckExact(v) || !PyInt_CheckExact(a))
		return NULL;

	out(PyInt_AS_LONG(a),PyInt_AS_LONG(v));
	
	Py_INCREF(Py_True);
	return Py_True;
}

static PyObject *
ports_querypair(PyObject *self, PyObject *args)
{
	PyObject *v, *a;

	if(!PyArg_UnpackTuple(args, "querypair", 2, 2, &a, &v))
		return NULL;
	if(!PyInt_CheckExact(v) || !PyInt_CheckExact(a))
		return NULL;

	out(PyInt_AS_LONG(a),PyInt_AS_LONG(v));
	return PyInt_FromLong(in(PyInt_AS_LONG(a)+1));
}

static PyObject *
ports_writevec(PyObject *self, PyObject *args)
{
	PyObject *ad, *wr, *v;
	int aport, wport;

        int i, l = PyTuple_GET_SIZE(args);
        if (l < 2)
                return NULL;

	ad = PyTuple_GET_ITEM(args, 0);
	wr = PyTuple_GET_ITEM(args, 1);

	if(!PyInt_CheckExact(ad) || !PyInt_CheckExact(wr))
		return NULL;

	aport = PyInt_AS_LONG(ad);
	wport = PyInt_AS_LONG(wr);

	for(i = 2; i < l; ++i)	{
		v = PyTuple_GET_ITEM(args, i);
		out(aport, i-2);
		out(wport, PyInt_AS_LONG(v));
	}

	Py_INCREF(Py_True);
	return Py_True;
}

static PyMethodDef ports_methods[] = {
 	{"inb",	        ports_inb,		METH_VARARGS, NULL/*doc*/},
	{"outb",	ports_outb,		METH_VARARGS, NULL/*doc*/},
	{"querypair",	ports_querypair,	METH_VARARGS, NULL/*doc*/},
	{"writevec",	ports_writevec,		METH_VARARGS, NULL/*doc*/},
	{NULL,		NULL},
};

PyObject *
_Ports_Init(void)
{
	return Py_InitModule4("ports", ports_methods,
			     NULL/*doc*/, (PyObject *)NULL,
			     PYTHON_API_VERSION);
}
