/*
 * keyboard.h
 */
#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <sys/types.h>

/*
 * A keyboard device.
 */
typedef struct {
  void (*initialize)(void* hal, void* data);
  void (*shutdown)();
} keyboard_device_t;

#endif /* __KEYBOARD_H__ */
