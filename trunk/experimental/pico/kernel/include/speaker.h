/*
 * speaker.h
 */
#ifndef __SPEAKER_H__
#define __SPEAKER_H__

#include <sys/types.h>

/*
 * A speaker device.
 */
typedef struct {
  void (*initialize)(void* hal, void* data);
  void (*shutdown)();
  
  void (*beep)(uint_t hz);
  void (*stop)();
} speaker_device_t;

#endif /* __SPEAKER_H__ */
