#ifndef Py_FRAMEOBJECT_H
#define Py_FRAMEOBJECT_H

typedef struct _frame {
  PyObject_VAR_HEAD
  struct _frame *f_back;
  PyCodeObject *f_code;
  PyObject *f_builtins;
  PyObject *f_globals;
  PyObject *f_locals;

  PyObject **f_valuestack;
  PyObject **f_stacktop;
  PyObject *f_trace;
  PyObject *f_exc_type, *f_exc_value, *f_exc_traceback;
  PyThreadState *f_tstate;

  int f_lasti;
  int f_lineno;
  int f_restricted;
  int f_iblock;

  int f_nlocals;
  int f_ncells;
  int f_nfreevars;
  int f_stacksize;
  PyObject *f_localsplus[1];
} PyFrameObject;

PyAPI_FUNC(PyFrameObject *) PyFrame_New(PyThreadState *, PyCodeObject *,
					PyObject *, PyObject *);

#endif /* !Py_FRAMEOBJECT_H */
