#ifndef __HAL_H__
#define __HAL_H__

#include "console.h"
#include "speaker.h"
#include "cpu.h"
#include "pic.h"
#include "memory.h"
#include "thread.h"
#include "clock.h"
#include "keyboard.h"

/*
 * A structure holding an abstraction of the basic physical devices. 
 */
typedef struct {
  void (*initialize)(void*);
  void (*shutdown)();
  speaker_device_t*   m_speaker;
  console_device_t*   m_console;
  cpu_device_t*       m_cpu;
  memory_device_t*    m_memory;
  thread_device_t*    m_thread;
  pic_device_t*       m_pic;
  clock_device_t*     m_clock;
  keyboard_device_t*  m_keyboard;
} hardware_abstraction_layer_t;

/*
 * We have one of these for the kernel to use.
 */
extern hardware_abstraction_layer_t HAL;

#endif /* __HAL_H__ */
