#ifndef Py_PYTHON_H
#define Py_PYTHON_H

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>


#define EOF (-1)

/* from stdlib.h */
void *malloc(size_t __size);


#ifdef __KERNEL__
# include <kstdlib.h>
# define PRINT kputs
# define PRINTF kprintf
# define LOG(msg) kputs(msg)
# define LOGF(format, arg1) kprintf(format, arg1)
#else
# define PRINT puts
# define PRINTF printf
# define LOG(msg) puts(msg)
# define LOGF(format, arg1) printf(format, arg1)
#endif


/* from pyport.h */
#define PyAPI_FUNC(RTYPE) RTYPE
#define PyAPI_DATA(RTYPE) extern RTYPE
typedef unsigned int Py_uintptr_t;

/* from pydebug.h */
PyAPI_FUNC(void) Py_FatalError(const char *message);

/* from ? */
#define offsetof(type, member) ( (int) & ((type*)0) -> member )

#include "pymem.h"

#include "object.h"
#include "objimpl.h"

#include "intobject.h"
#include "boolobject.h"
#include "stringobject.h"
#include "tupleobject.h"
#include "dictobject.h"
#include "methodobject.h"
#include "moduleobject.h"
#include "funcobject.h"
#include "classobject.h"

#include "pystate.h"

#include "modsupport.h"
#include "pythonrun.h"
#include "ceval.h"

#include "import.h"

#include "abstract.h"
#include "compile.h"
#include "frameobject.h"

#endif /* !Py_PYTHON_H */
