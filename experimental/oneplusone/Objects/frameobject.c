#include "Python.h"

#include "compile.h"
#include "frameobject.h"
#include "opcode.h"

static PyObject *builtin_object;

static PyFrameObject *free_list = NULL;
static int numfree = 0;

PyTypeObject PyFrame_Type = {
  PyObject_HEAD_INIT(&PyType_Type)
  /* TO DO */
};

PyFrameObject *
PyFrame_New(PyThreadState *tstate, PyCodeObject *code, PyObject *globals, 
	    PyObject *locals)
{
  printf("> PyFrame_New\n");
	PyFrameObject *back = tstate->frame;
	PyFrameObject *f;
	PyObject *builtins;
	int extras, ncells, nfrees;

	ncells = PyTuple_GET_SIZE(code->co_cellvars);
	nfrees = PyTuple_GET_SIZE(code->co_freevars);
	extras = code->co_stacksize + code->co_nlocals + ncells + nfrees;

	if (back == NULL || back->f_globals != globals) {
		builtins = PyDict_GetItem(globals, builtin_object);
		if (builtins) {
			if (PyModule_Check(builtins)) {
				builtins = PyModule_GetDict(builtins);
			}
			else if (!PyDict_Check(builtins))
				builtins = NULL;
		}
		if (builtins == NULL) {
			/* No builtins!  Make up a minimal one 
			   Give them 'None', at least. */
			builtins = PyDict_New();
			if (builtins == NULL || 
			    PyDict_SetItemString(
				    builtins, "None", Py_None) < 0)
				return NULL;
		}
		else
			Py_INCREF(builtins);

	}
	else {
		/* If we share the globals, we share the builtins.
		   Save a lookup and a call. */
		builtins = back->f_builtins;
		Py_INCREF(builtins);
	}

	if (free_list == NULL) {
		f = PyObject_GC_NewVar(PyFrameObject, &PyFrame_Type, extras);
		if (f == NULL)
			return NULL;
	}
	else {
		--numfree;
		f = free_list;
		free_list = free_list->f_back;
		if (f->ob_size < extras) {
			f = PyObject_GC_Resize(PyFrameObject, f, extras);
			if (f == NULL)
				return NULL;
		}
		_Py_NewReference((PyObject *)f);
	}

	f->f_builtins = builtins;
	Py_XINCREF(back);
	f->f_back = back;
	Py_INCREF(code);
	f->f_code = code;
	Py_INCREF(globals);
	f->f_globals = globals;

	/* Most functions have CO_NEWLOCALS and CO_OPTIMIZED set. */
	if ((code->co_flags & (CO_NEWLOCALS | CO_OPTIMIZED)) == 
		(CO_NEWLOCALS | CO_OPTIMIZED))
		locals = NULL; /* PyFrame_Fast2Locals() will set. */
	else if (code->co_flags & CO_NEWLOCALS) {
		locals = PyDict_New();
		if (locals == NULL) {
			Py_DECREF(f);
			return NULL;
		}
	}
	else {
		if (locals == NULL)
			locals = globals;
		Py_INCREF(locals);
	}

	f->f_locals = locals;
	f->f_trace = NULL;
	f->f_exc_type = f->f_exc_value = f->f_exc_traceback = NULL;
	f->f_tstate = tstate;

	f->f_lasti = -1;
	f->f_lineno = code->co_firstlineno;
	f->f_restricted = (builtins != tstate->interp->builtins);
	f->f_iblock = 0;
	f->f_nlocals = code->co_nlocals;
	f->f_stacksize = code->co_stacksize;
	f->f_ncells = ncells;
	f->f_nfreevars = nfrees;

	extras = f->f_nlocals + ncells + nfrees;
	memset(f->f_localsplus, 0, extras * sizeof(f->f_localsplus[0]));

	f->f_valuestack = f->f_localsplus + extras;
	f->f_stacktop = f->f_valuestack;

	_PyObject_GC_TRACK(f);
	return f;
}

int _PyFrame_Init()
{
	/* TO DO */
	return 1;
}

void
PyFrame_Fini(void)
{
	/* TO DO */
}
