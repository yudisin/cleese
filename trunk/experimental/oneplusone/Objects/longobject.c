#include "Python.h"

PyObject *
PyLong_FromVoidPtr(void *p)
{
  return PyInt_FromLong((long)p);
}
