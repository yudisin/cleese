/*
 * stdio.h
 *   Just enough to hack kernels...
 */
#ifndef __STDIO_H__
#define __STDIO_H__

#include <sys/types.h>
#include <stdarg.h>

#if !defined(TRUE) && !defined(FALSE)
#  define TRUE (1)
#  define FALSE (0)
#endif

#define EOF (-1)

#ifndef NULL
#  define NULL (0)
#endif

#ifndef MAX
#  define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#  define MIN(a, b) (((a) <= (b)) ? (a) : (b))
#endif

/*
 * Basic sprintf functionality
 */
extern int snprintf(char* dst, int n, const char* fmt, ...);
extern int va_snprintf(char* dst, int n, const char* fmt, va_list);

#endif /* __STDIO_H__ */
