/*
 * mach/x86/keyboard.h
 */
#ifndef __MACH_X86_KEYBOARD_H__
#define __MACH_X86_KEYBOARD_H__

#include <keyboard.h>

/*
 * Ports for keyboard management.
 */
#define KBD_DATA_PORT     0x60
#define KBD_CONTROL_PORT  0x61
#define KBD_STATUS_PORT   0x64

/*
 * Bit in KBD_CTL for strobing enable
 */
#define KBD_ENABLE 0x80

/*
 * Bits in KBD_STATUS
 */
#define KBD_DATA   0x1
#define KBD_BUSY   0x2
#define KBD_WRITE  0xD1

/*
 * Interrupt vector 
 */
#define KBD_IRQ 1

extern keyboard_device_t x86_keyboard_device;

#endif /* __MACH_X86_KEYBOARD_H__ */
