#include "Python.h"
#include "longintrepr.h"

/* Get a C long int from a long int object.
   Returns -1 and sets an error condition if overflow occurs. */

long
PyLong_AsLong(PyObject *vv)
{
	/* This version by Tim Peters */
	register PyLongObject *v;
	unsigned long x, prev;
	int i, sign;

	if (vv == NULL || !PyLong_Check(vv)) {
		if (vv != NULL && PyInt_Check(vv))
			return PyInt_AsLong(vv);
		PyErr_BadInternalCall();
		return -1;
	}
	v = (PyLongObject *)vv;
	i = v->ob_size;
	sign = 1;
	x = 0;
	if (i < 0) {
		sign = -1;
		i = -(i);
	}
	while (--i >= 0) {
		prev = x;
		x = (x << SHIFT) + v->ob_digit[i];
		if ((x >> SHIFT) != prev)
			goto overflow;
	}
	/* Haven't lost any bits, but if the sign bit is set we're in
	 * trouble *unless* this is the min negative number.  So,
	 * trouble iff sign bit set && (positive || some bit set other
	 * than the sign bit).
	 */
	if ((long)x < 0 && (sign > 0 || (x << 1) != 0))
		goto overflow;
	return (long)x * sign;

 overflow:
	PyErr_SetString(PyExc_OverflowError,
			"long int too large to convert to int");
	return -1;
}

/* Create a new long (or int) object from a C pointer */

PyObject *
PyLong_FromVoidPtr(void *p)
{
#if SIZEOF_VOID_P <= SIZEOF_LONG
	return PyInt_FromLong((long)p);
#else

#ifndef HAVE_LONG_LONG
#   error "PyLong_FromVoidPtr: sizeof(void*) > sizeof(long), but no long long"
#endif
#if SIZEOF_LONG_LONG < SIZEOF_VOID_P
#   error "PyLong_FromVoidPtr: sizeof(PY_LONG_LONG) < sizeof(void*)"
#endif
	/* optimize null pointers */
	if (p == NULL)
		return PyInt_FromLong(0);
	return PyLong_FromLongLong((PY_LONG_LONG)p);

#endif /* SIZEOF_VOID_P <= SIZEOF_LONG */
}

/* Get a C unsigned long int from a long int object, ignoring the high bits.
   Returns -1 and sets an error condition if an error occurs. */

unsigned long
PyLong_AsUnsignedLongMask(PyObject *vv)
{
	register PyLongObject *v;
	unsigned long x;
	int i, sign;

	if (vv == NULL || !PyLong_Check(vv)) {
		PyErr_BadInternalCall();
		return (unsigned long) -1;
	}
	v = (PyLongObject *)vv;
	i = v->ob_size;
	sign = 1;
	x = 0;
	if (i < 0) {
		sign = -1;
		i = -i;
	}
	while (--i >= 0) {
		x = (x << SHIFT) + v->ob_digit[i];
	}
	return x * sign;
}

static PyNumberMethods long_as_number = {
  0, //(binaryfunc)	long_add,	/*nb_add*/
  0, //(binaryfunc)	long_sub,	/*nb_subtract*/
  0, //(binaryfunc)	long_mul,	/*nb_multiply*/
  0, //(binaryfunc)	long_classic_div, /*nb_divide*/
  0, //(binaryfunc)	long_mod,	/*nb_remainder*/
  0, //(binaryfunc)	long_divmod,	/*nb_divmod*/
  0, //(ternaryfunc)	long_pow,	/*nb_power*/
  0, //(unaryfunc) 	long_neg,	/*nb_negative*/
  0, //(unaryfunc) 	long_pos,	/*tp_positive*/
  0, //(unaryfunc) 	long_abs,	/*tp_absolute*/
  0, //(inquiry)	long_nonzero,	/*tp_nonzero*/
  0, //(unaryfunc)	long_invert,	/*nb_invert*/
  0, //(binaryfunc)	long_lshift,	/*nb_lshift*/
  0, //(binaryfunc)	long_rshift,	/*nb_rshift*/
  0, //(binaryfunc)	long_and,	/*nb_and*/
  0, //(binaryfunc)	long_xor,	/*nb_xor*/
  0, //(binaryfunc)	long_or,	/*nb_or*/
  0, //(coercion)	long_coerce,	/*nb_coerce*/
  0, //(unaryfunc)	long_int,	/*nb_int*/
  0, //(unaryfunc)	long_long,	/*nb_long*/
  0, //(unaryfunc)	long_float,	/*nb_float*/
  0, //(unaryfunc)	long_oct,	/*nb_oct*/
  0, //(unaryfunc)	long_hex,	/*nb_hex*/
	0,				/* nb_inplace_add */
	0,				/* nb_inplace_subtract */
	0,				/* nb_inplace_multiply */
	0,				/* nb_inplace_divide */
	0,				/* nb_inplace_remainder */
	0,				/* nb_inplace_power */
	0,				/* nb_inplace_lshift */
	0,				/* nb_inplace_rshift */
	0,				/* nb_inplace_and */
	0,				/* nb_inplace_xor */
	0,				/* nb_inplace_or */
  0, //(binaryfunc)long_div,		/* nb_floor_divide */
  0, //long_true_divide,		/* nb_true_divide */
	0,				/* nb_inplace_floor_divide */
	0,				/* nb_inplace_true_divide */
};

PyTypeObject PyLong_Type = {
	PyObject_HEAD_INIT(&PyType_Type)
	0,					/* ob_size */
	"long",					/* tp_name */
	sizeof(PyLongObject) - sizeof(digit),	/* tp_basicsize */
	sizeof(digit),				/* tp_itemsize */
	0, //(destructor)long_dealloc,		/* tp_dealloc */
	0,					/* tp_print */
	0,					/* tp_getattr */
	0,					/* tp_setattr */
	0, //(cmpfunc)long_compare,			/* tp_compare */
	0, //(reprfunc)long_repr,			/* tp_repr */
	&long_as_number,			/* tp_as_number */
	0,					/* tp_as_sequence */
	0,					/* tp_as_mapping */
	0, //(hashfunc)long_hash,			/* tp_hash */
        0,              			/* tp_call */
        0, //(reprfunc)long_str,			/* tp_str */
	0, //PyObject_GenericGetAttr,		/* tp_getattro */
	0,					/* tp_setattro */
	0,					/* tp_as_buffer */
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_CHECKTYPES |
		Py_TPFLAGS_BASETYPE,		/* tp_flags */
	0, //long_doc,				/* tp_doc */
	0,					/* tp_traverse */
	0,					/* tp_clear */
	0,					/* tp_richcompare */
	0,					/* tp_weaklistoffset */
	0,					/* tp_iter */
	0,					/* tp_iternext */
	0, //long_methods,				/* tp_methods */
	0,					/* tp_members */
	0,					/* tp_getset */
	0,					/* tp_base */
	0,					/* tp_dict */
	0,					/* tp_descr_get */
	0,					/* tp_descr_set */
	0,					/* tp_dictoffset */
	0,					/* tp_init */
	0,					/* tp_alloc */
	0, //long_new,				/* tp_new */
	0, //PyObject_Del,                           /* tp_free */
};
