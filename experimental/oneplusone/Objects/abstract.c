#include "Python.h"

#define NEW_STYLE_NUMBER(o) PyType_HasFeature((o)->ob_type, \
				Py_TPFLAGS_CHECKTYPES)
int
PyNumber_Check(PyObject *o)
{
	return o && o->ob_type->tp_as_number &&
	       (o->ob_type->tp_as_number->nb_int ||
		o->ob_type->tp_as_number->nb_float);
}

#define NB_SLOT(x) offsetof(PyNumberMethods, x)
#define NB_BINOP(nb_methods, slot) \
		(*(binaryfunc*)(& ((char*)nb_methods)[slot]))

static PyObject *
binary_op1(PyObject *v, PyObject *w, const int op_slot)
{
	PyObject *x;
	binaryfunc slotv = NULL;
	binaryfunc slotw = NULL;

	if (v->ob_type->tp_as_number != NULL && NEW_STYLE_NUMBER(v))
		slotv = NB_BINOP(v->ob_type->tp_as_number, op_slot);
	if (w->ob_type != v->ob_type &&
	    w->ob_type->tp_as_number != NULL && NEW_STYLE_NUMBER(w)) {
		slotw = NB_BINOP(w->ob_type->tp_as_number, op_slot);
		if (slotw == slotv)
			slotw = NULL;
	}
	if (slotv) {
		if (slotw && PyType_IsSubtype(w->ob_type, v->ob_type)) {
			x = slotw(v, w);
			if (x != Py_NotImplemented)
				return x;
			Py_DECREF(x); /* can't do it */
			slotw = NULL;
		}
		x = slotv(v, w);
		if (x != Py_NotImplemented)
			return x;
		Py_DECREF(x); /* can't do it */
	}
	if (slotw) {
		x = slotw(v, w);
		if (x != Py_NotImplemented)
			return x;
		Py_DECREF(x); /* can't do it */
	}
	if (!NEW_STYLE_NUMBER(v) || !NEW_STYLE_NUMBER(w)) {
		int err = PyNumber_CoerceEx(&v, &w);
		if (err < 0) {
			return NULL;
		}
		if (err == 0) {
			PyNumberMethods *mv = v->ob_type->tp_as_number;
			if (mv) {
				binaryfunc slot;
				slot = NB_BINOP(mv, op_slot);
				if (slot) {
					PyObject *x = slot(v, w);
					Py_DECREF(v);
					Py_DECREF(w);
					return x;
				}
			}
			/* CoerceEx incremented the reference counts */
			Py_DECREF(v);
			Py_DECREF(w);
		}
	}
	Py_INCREF(Py_NotImplemented);
	return Py_NotImplemented;
}

static PyObject *
binop_type_error(PyObject *v, PyObject *w, const char *op_name)
{
  /* ERROR */
  return NULL;
}

PyObject *
PyNumber_Add(PyObject *v, PyObject *w)
{
	PyObject *result = binary_op1(v, w, NB_SLOT(nb_add));
	if (result == Py_NotImplemented) {
		PySequenceMethods *m = v->ob_type->tp_as_sequence;
		if (m && m->sq_concat) {
			Py_DECREF(result);
			result = (*m->sq_concat)(v, w);
		}
		if (result == Py_NotImplemented) {
			Py_DECREF(result);
			return binop_type_error(v, w, "+");
                }
	}
	return result;
}
