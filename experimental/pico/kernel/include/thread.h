#ifndef __THREAD_H__
#define __THREAD_H__

#include <sys/types.h>

/*
 * The basic physical thread initialization stuff.
 */
typedef struct {
  void (*initialize)(void* hal, void* data);
  void (*shutdown)();
} thread_device_t;

#endif /* __THREAD_H__ */
