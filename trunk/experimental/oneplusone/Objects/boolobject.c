#include "Python.h"

PyTypeObject PyBool_Type = {
  PyObject_HEAD_INIT(&PyType_Type)
  /* TO DO */
};

PyIntObject _Py_ZeroStruct = {
  PyObject_HEAD_INIT(&PyBool_Type)
  0
};

PyIntObject _Py_TrueStruct = {
  PyObject_HEAD_INIT(&PyBool_Type)
  1
};

