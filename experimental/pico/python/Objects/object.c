#include "Python.h"

PyObject *
PyObject_Init(PyObject *op, PyTypeObject *tp)
{
	LOG("> PyObject_Init\n");
	if (op == NULL)
	  Py_FatalError("out of memory");
	/* Any changes should be reflected in PyObject_INIT (objimpl.h) */
	op->ob_type = tp;
	_Py_NewReference(op);
	LOG("< PyObject_Init\n");
	return op;
}

#define RICHCOMPARE(t) (PyType_HasFeature((t), Py_TPFLAGS_HAVE_RICHCOMPARE) \
                         ? (t)->tp_richcompare : NULL)

#define NESTING_LIMIT 20

static int
internal_print(PyObject *op)
{
	return (*op->ob_type->tp_print)(op);
}

int
PyObject_Print(PyObject *op)
{
	return internal_print(op);
}

long
PyObject_Hash(PyObject *v)
{
	PyTypeObject *tp = v->ob_type;
	if (tp->tp_hash != NULL)
		return (*tp->tp_hash)(v);
	/* TO DO */
	return -1;
}

int
PyObject_IsTrue(PyObject *v)
{
	int res;
	if (v == Py_True)
		return 1;
	if (v == Py_False)
		return 0;
	if (v == Py_None)
		return 0;
	else if (v->ob_type->tp_as_number != NULL &&
		 v->ob_type->tp_as_number->nb_nonzero != NULL)
		res = (*v->ob_type->tp_as_number->nb_nonzero)(v);
	else if (v->ob_type->tp_as_mapping != NULL &&
		 v->ob_type->tp_as_mapping->mp_length != NULL)
		res = (*v->ob_type->tp_as_mapping->mp_length)(v);
	else if (v->ob_type->tp_as_sequence != NULL &&
		 v->ob_type->tp_as_sequence->sq_length != NULL)
		res = (*v->ob_type->tp_as_sequence->sq_length)(v);
	else
		return 1;
	return (res > 0) ? 1 : res;
}

PyObject *
PyObject_Str(PyObject *v)
{
	PyObject *res;

	if (v == NULL)
		return PyString_FromString("<NULL>");
	if (PyString_CheckExact(v)) {
		Py_INCREF(v);
		return v;
	}
	Py_FatalError("PyObject_Str doesn't support non-strings yet.");
	if (v->ob_type->tp_str == NULL)
		; //return PyObject_Repr(v);

	res = (*v->ob_type->tp_str)(v);
	if (res == NULL)
		return NULL;
	if (!PyString_Check(res)) {
		/* ERROR */
		Py_DECREF(res);
		return NULL;
	}
	return res;
}


/* Coerce two numeric types to the "larger" one.
   Increment the reference count on each argument.
   Return value:
   -1 if an error occurred;
   0 if the coercion succeeded (and then the reference counts are increased);
   1 if no coercion is possible (and no error is raised).
*/
int
PyNumber_CoerceEx(PyObject **pv, PyObject **pw)
{
	register PyObject *v = *pv;
	register PyObject *w = *pw;
	int res;

	/* Shortcut only for old-style types */
	if (v->ob_type == w->ob_type &&
	    !PyType_HasFeature(v->ob_type, Py_TPFLAGS_CHECKTYPES))
	{
		Py_INCREF(v);
		Py_INCREF(w);
		return 0;
	}
	if (v->ob_type->tp_as_number && v->ob_type->tp_as_number->nb_coerce) {
		res = (*v->ob_type->tp_as_number->nb_coerce)(pv, pw);
		if (res <= 0)
			return res;
	}
	if (w->ob_type->tp_as_number && w->ob_type->tp_as_number->nb_coerce) {
		res = (*w->ob_type->tp_as_number->nb_coerce)(pw, pv);
		if (res <= 0)
			return res;
	}
	return 1;
}


/* ... */

static PyTypeObject PyNone_Type = {
  PyObject_HEAD_INIT(&PyType_Type)
  0,
  "NoneType",
  0,
  0,
  0, //(destructor)none_dealloc,	     /*tp_dealloc*/ /*never called*/
  0,		/*tp_print*/
  0,		/*tp_getattr*/
  0,		/*tp_setattr*/
  0,		/*tp_compare*/
  0, //(reprfunc)none_repr, /*tp_repr*/
  0,		/*tp_as_number*/
  0,		/*tp_as_sequence*/
  0,		/*tp_as_mapping*/
  0,		/*tp_hash */
};

PyObject _Py_NoneStruct = {
  PyObject_HEAD_INIT(&PyNone_Type)
};

static PyTypeObject PyNotImplemented_Type = {
  PyObject_HEAD_INIT(&PyType_Type)
  0,
  "NotImplementedType",
  0,
  0,
  0, //(destructor)none_dealloc,	     /*tp_dealloc*/ /*never called*/
  0,		/*tp_print*/
  0,		/*tp_getattr*/
  0,		/*tp_setattr*/
  0,		/*tp_compare*/
  0, //(reprfunc)NotImplemented_repr, /*tp_repr*/
  0,		/*tp_as_number*/
  0,		/*tp_as_sequence*/
  0,		/*tp_as_mapping*/
  0,		/*tp_hash */
};

PyObject _Py_NotImplementedStruct = {
	PyObject_HEAD_INIT(&PyNotImplemented_Type)
};

void
_Py_ReadyTypes(void)
{
	if (PyType_Ready(&PyType_Type) < 0)
		Py_FatalError("Can't initialize 'type'");

	if (PyType_Ready(&PyString_Type) < 0)
		Py_FatalError("Can't initialize 'str'");

	if (PyType_Ready(&PyNone_Type) < 0)
		Py_FatalError("Can't initialize type(None)");

	if (PyType_Ready(&PyNotImplemented_Type) < 0)
		Py_FatalError("Can't initialize type(NotImplemented)");
}

