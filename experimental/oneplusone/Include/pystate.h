#ifndef Py_PYSTATE_H
#define Py_PYSTATE_H

struct _ts; /* Forward */
struct _is; /* Forward */

typedef struct _is {

  struct _is *next;
  struct _ts *tstate_head;

  PyObject *modules;
  PyObject *builtins;

} PyInterpreterState;

typedef struct _ts {
	
  struct _ts *next;
  PyInterpreterState *interp;

  struct _frame *frame;
  int recursion_depth;

  PyObject *dict;

} PyThreadState;

PyAPI_FUNC(PyInterpreterState *) PyInterpreterState_New(void);
PyAPI_FUNC(void) PyInterpreterState_Clear(PyInterpreterState *);
PyAPI_FUNC(void) PyInterpreterState_Delete(PyInterpreterState *);

PyAPI_FUNC(PyThreadState *) PyThreadState_New(PyInterpreterState *);
PyAPI_FUNC(void) PyThreadState_Clear(PyThreadState *);
PyAPI_FUNC(void) PyThreadState_Delete(PyThreadState *);

PyAPI_FUNC(PyThreadState *) PyThreadState_Get(void);
PyAPI_FUNC(PyThreadState *) PyThreadState_Swap(PyThreadState *);
PyAPI_FUNC(PyObject *) PyThreadState_GetDict(void);

PyAPI_DATA(PyThreadState *) _PyThreadState_Current;

#define PyThreadState_GET() (_PyThreadState_Current)

#endif /* !Py_PYSTATE_H */
