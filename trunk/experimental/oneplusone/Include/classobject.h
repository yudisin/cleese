#ifndef Py_CLASSOBJECT_H
#define Py_CLASSOBJECT_H

PyAPI_DATA(PyTypeObject) PyInstance_Type;

#define PyInstance_Check(op) ((op)->ob_type == &PyInstance_Type)

#endif /* !Py_CLASSOBJECT_H */
