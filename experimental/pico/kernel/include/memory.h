#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <sys/types.h>

/*
 * The basic physical memory. We hae this here so we can call
 * initialization if necessary.
 */
typedef struct {
  void (*initialize)(void* hal, void* data);
  void (*shutdown)();
} memory_device_t;

#endif /* __MEMORY_H__ */
