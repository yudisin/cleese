#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include <sys/types.h>

/*
 * A console device. The device is expected to understand VT52 escape
 * sequences as a minimum.
 */
typedef struct {
  void (*initialize)(void* hal, void* data);
  void (*shutdown)();
  
  void (*putc)(char ch);
  void (*puts)(const char* ch);
} console_device_t;

#endif /* __CONSOLE_H__ */
