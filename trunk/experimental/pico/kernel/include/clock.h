/*
 * clock.h
 */
#ifndef __CLOCK_H__
#define __CLOCK_H__

#include <sys/types.h>
#include <hal.h>

/*
 * A clock device.
 */
typedef struct {
  void (*initialize)(void* hal, void* data);
  void (*shutdown)();

  ulong_t (*ticks)();
  
} clock_device_t;

#endif /* __CLOCK_H__ */
