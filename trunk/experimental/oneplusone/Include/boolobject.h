#ifndef Py_BOOLOBJECT_H
#define Py_BOOLOBJECT_H

typedef PyIntObject PyBoolObject;

PyAPI_DATA(PyTypeObject) PyBool_Type;

#define PyBool_Check(x) ((x)->ob_type == &PyBool_Type)

PyAPI_DATA(PyIntObject) _Py_ZeroStruct, _Py_TrueStruct;

#define Py_False ((PyObject *) &_Py_ZeroStruct)
#define Py_True ((PyObject *) &_Py_TrueStruct)

#endif /* !Py_BOOLOBJECT_H */
