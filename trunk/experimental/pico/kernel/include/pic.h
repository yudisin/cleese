#ifndef __PIC_H__
#define __PIC_H__

#include <sys/types.h>

typedef void (*interrupt_handler_t)(void*);

/*
 * A PIC device. Something that manages interrupts
 * and interrupt handlers.
 */
typedef struct {
  void (*initialize)(void* hal, void* data);
  void (*shutdown)();
  
  void (*enable_interrupts)();
  void (*disable_interrupts)();

  void (*enable)(uchar_t);
  void (*disable)(uchar_t);

  void (*register_handler)(uchar_t, interrupt_handler_t);
  void (*unregister_handler)(uchar_t);
} pic_device_t;

#endif /* __PIC_H__ */
