#ifndef __CPU_H__
#define __CPU_H__

#include <sys/types.h>

/*
 * Description of a CPU
 */
typedef struct {
  char    m_vendor[16];
  char    m_family[16];
  int     m_model;
} cpu_descriptor_t;

/*
 * CPU device interface allowing us to query for the # of CPU's and
 * also information about each one.
 */
typedef struct {
  void (*initialize)(void* hal, void* data);
  void (*shutdown)();
  int  (*count)();
  cpu_descriptor_t* (*get_description)(int);
} cpu_device_t;

#endif /* __CPU_H__ */
