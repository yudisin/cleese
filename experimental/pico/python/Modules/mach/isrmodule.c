#include "Python.h"

#include <hal.h>
#include <mach/x86/x86.h>
#include <mach/x86/io.h>
#include <mach/x86/pic.h>
#include <sys/types.h>

#define MAX_ISR 255

extern PyFrameObject *PyEval_GetFrame(void);

/*
 * Table of interrupt handlers
 */
static struct {
  int       tripped;
  PyObject *func;
} PyISR_InterruptHandlers[MAX_ISR];

volatile int PyISR_interrupt_pending = 0;

/*
 * This is called occasionally to check for pending interrupts.
 */
int
PyISR_CheckInterrupts(void)
{
  int i;
  PyObject *f;

  /*
   * Quick check to see if there's anything to do.
   */
  if (PyISR_interrupt_pending == 0)
    return(0);
  
  
  if (!(f = (PyObject *)PyEval_GetFrame()))
    f = Py_None;
	
  for (i = 0; i < MAX_ISR; i++) {
    if (PyISR_InterruptHandlers[i].tripped == 1) {
      PyObject *result = NULL;
      PyObject *arglist = NULL;
      PyISR_InterruptHandlers[i].tripped = 0;

      if (arglist) {
	result = PyObject_Call(PyISR_InterruptHandlers[i].func,
			       arglist, NULL);
	Py_DECREF(arglist);
      }
      if (result == NULL)
	return(-1);

      Py_DECREF(result);
    }
  }
  PyISR_interrupt_pending = 0;
  return(0);
}

/*
 * The global interrupt handler
 */
static void isr_global_isr(void *p)
{
  x86_trap_frame_t *t = (x86_trap_frame_t *) p;

  int x = t->traptype;

  PRINTF("python interrupt: %d\n",x);
  if(x >= 0 && x <= MAX_ISR) {
    if(PyISR_InterruptHandlers[x].func != NULL) {
      PyISR_InterruptHandlers[x].tripped = 1;
      PyISR_interrupt_pending = 1;
      Py_AddPendingCall(PyISR_CheckInterrupts, NULL);
    }
  }
}

/*
 * Enable the specified interrupt
 */
static PyObject *
isr_enable(PyObject *self, PyObject *args)
{
  int       num;
  PyObject* v = NULL;

  /*
   * Try to get the handler for a given ISR
   */
  if (!PyArg_UnpackTuple(args, "get_handler", 1, 1, &v))
    return(NULL);

  if (!PyInt_CheckExact(v)) {
    /* TODO: Error handling */
    return(NULL);
  } else {
    num = PyInt_AS_LONG(v);
  }

  /*
   * Make sure the number passed is within range
   */
  if (num < 0 || num >= MAX_ISR) {
    /* TODO: Error handling */
    return(NULL);
  }

  HAL.m_pic->enable(num);

  return(NULL);
}

/*
 * Disable the specified interrupt
 */
static PyObject *
isr_disable(PyObject *self, PyObject *args)
{
  int       num;
  PyObject* v = NULL;

  /*
   * Try to get the handler for a given ISR
   */
  if (!PyArg_UnpackTuple(args, "get_handler", 1, 1, &v))
    return(NULL);

  if (!PyInt_CheckExact(v)) {
    /* TODO: Error handling */
    return(NULL);
  } else {
    num = PyInt_AS_LONG(v);
  }

  /*
   * Make sure the number passed is within range
   */
  if (num < 0 || num >= MAX_ISR) {
    /* TODO: Error handling */
    return(NULL);
  }

  HAL.m_pic->disable(num);

  return(NULL);
}

/*
 * Get the handler associated with a given interrupt vector
 */
static PyObject *
isr_get_handler(PyObject *self, PyObject *args)
{
  int       num;
  PyObject* handler;
  PyObject* v = NULL;

  /*
   * Try to get the handler for a given ISR
   */
  if (!PyArg_UnpackTuple(args, "get_handler", 1, 1, &v))
    return(NULL);

  if (!PyInt_CheckExact(v)) {
    /* TODO: Error handling */
    return(NULL);
  } else {
    num = PyInt_AS_LONG(v);
  }

  /*
   * Make sure the number passed is within range
   */
  if (num < 0 || num >= MAX_ISR) {
    /* TODO: Error handling */
    return(NULL);
  }

  /*
   * Get the handler and return it.
   */
  handler = PyISR_InterruptHandlers[num].func;
  Py_INCREF(handler);
  return(handler);
}

/*
 * Set the handler for a given ISR. Note that interrupts will be
 * disabled while this function is called.
 */
static PyObject *
isr_set_handler(PyObject *self, PyObject *args)
{
  PyObject *obj;
  PyObject *handler;
  PyObject* v = NULL;

  int num;

  /*
   * We don't want to be interrupted
   */
  HAL.m_pic->disable_interrupts();

  /*
   * Get our arguments
   */
  if (!PyArg_UnpackTuple(args, "set_handler", 2, 2, &v, &obj))
    return(NULL);

  if (!PyInt_CheckExact(v)) {
    /* TODO: Error handling */
    return(NULL);
  } else {
    num = PyInt_AS_LONG(v);
  }

  /*
   * Make sure the ISR number is within range.
   */
  if (num < 0 || num >= MAX_ISR) {
    /* TODO: Error handling */
    return(NULL);
  }

  /*
   * Make sure the handler is a function or somesuch.
   */
#ifdef NNNN
  if (!PyCallable_Check(obj)) {
    /* TODO: Error handling */
    return(NULL);
  }
#endif 

  handler = PyISR_InterruptHandlers[num].func;

  /*
   * Set the handler for the given interrupt
   */
  Py_INCREF(obj);
  PyISR_InterruptHandlers[num].func = obj;

  /*
   * Interrupts are now OK.
   */
  HAL.m_pic->enable_interrupts();

  return(handler);
}

/* 
 * List of functions defined in the module 
 */
static PyMethodDef isr_methods[] = {
  {"enable",      isr_enable,      METH_VARARGS, NULL},
  {"disable",     isr_disable,     METH_VARARGS, NULL},
  {"set_handler", isr_set_handler, METH_VARARGS, NULL},
  {"get_handler", isr_get_handler, METH_VARARGS, NULL},
  {NULL,			NULL}		/* sentinel */
};

/*
 * Initialize the module
 */
PyObject *
init_isr(void)
{
  PyObject *m;
  int i;

  /*
   * Set all the interrupt handlers to NULL initially
   */
  for (i = 0; i < MAX_ISR; i++) {
    PyISR_InterruptHandlers[i].func = NULL;
    PyISR_InterruptHandlers[i].tripped = 1;
    HAL.m_pic->disable(i);
    HAL.m_pic->register_handler(i, isr_global_isr);
  }

  /*
   * Create the module and add the functions 
   */
  m = Py_InitModule4("ISR", isr_methods,
		     NULL, (PyObject *)NULL,
		     PYTHON_API_VERSION);
  return(m);
}
