/*
 * multiboot.h
 */
#ifndef __MULTIBOOT_H__
#define __MULTIBOOT_H__

#include <sys/types.h>

/* 
 * flags.  
 */
#define MULTIBOOT_INFO_MEMORY   0x01
#define MULTIBOOT_INFO_BOOTDEV  0x02
#define MULTIBOOT_INFO_CMDLINE  0x04
#define MULTIBOOT_INFO_MODULE   0x08
#define MULTIBOOT_INFO_SYMAOUT	0x10
#define MULTIBOOT_INFO_SYMELF	0x20

typedef struct {
  /* 
   * What information is available
   */
  uint_t m_flags;
  
  /* 
   * Memory ranges for our machine. Check the flags to see if they are
   * valid.
   */
  uint_t m_lower_memory;
  uint_t m_upper_memory;

  /* 
   * What was the boot device? Check flags to see if this is valid. 
   */
  uchar_t m_boot_device[4];
  
  /*
   * Commandline passed to the kernel. Check the flags to make sure
   * it's valid.
   */
  char* m_kernel_commandline;
  
  /*
   * How many modules tdo we have?
   */
  uint_t m_number_of_modules;

  /*
   * Base address for module information
   */
  void *m_module_information_base;
  
  union {
    struct {
      uint_t tabsize;
      uint_t strsize;
      uint_t addr;
    } aout;

    struct {
      uint_t num;
      uint_t size;
      uint_t addr;
      uint_t shndx;
    } elf;
  } symtab;

} multiboot_information_t; 

typedef struct {
  uint_t m_start;
  uint_t m_end;
  char*  m_string;
  uint_t m_reserved;
} multiboot_module_t;

#endif /* __MULTIBOOT.H__ */
