#include "Python.h"

static unsigned char saved[4000];

static PyObject *
vga_savetext(PyObject *self, PyObject *args)
{
	memcpy(saved, (void *)0xb8000, 80*25*2);
	Py_INCREF(Py_True);
	return Py_True;
}

static PyObject *
vga_restoretext(PyObject *self, PyObject *args)
{
	memcpy((void *)0xb8000, saved, 80*25*2);
	Py_INCREF(Py_True);
	return Py_True;
}

static PyMethodDef vga_methods[] = {
	{"savetext",	vga_savetext,	METH_VARARGS, NULL/*doc*/},
	{"restoretext",	vga_restoretext,METH_VARARGS, NULL/*doc*/},
	{NULL,		NULL},
};

extern unsigned char bootscreen[];

PyObject *
_VGA_Init(void)
{
	PyObject *mod, *dict;
	mod = Py_InitModule4("vga", vga_methods,
			     NULL/*doc*/, (PyObject *)NULL,
			     PYTHON_API_VERSION);

	dict = PyModule_GetDict(mod);

	#define SETBUILTIN(NAME, OBJECT) \
	if (PyDict_SetItemString(dict, NAME, (PyObject *)OBJECT) < 0)	\
		return NULL;

	SETBUILTIN("framebuffer",
		PyBuffer_FromReadWriteMemory((void *)0xa0000, 0x10000));
	SETBUILTIN("textbuffer",
		PyBuffer_FromReadWriteMemory((void *)0xb8000, 80*25*2));
	SETBUILTIN("splashscreen",
		PyBuffer_FromMemory(bootscreen, 0x10000));

	init_screen();

	return mod;
}
