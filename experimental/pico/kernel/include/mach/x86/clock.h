/*
 * mach/x86/clock.h
 */
#ifndef __MACH_X86_CLOCK_H__
#define __MACH_X86_CLOCK_H__

#include <clock.h>

#define HZ (20)

/*
 * Port address of the control port and timer channels
 */
#define PIT_CTRL 0x43
#define PIT_CH0 0x40
#define PIT_CH1 0x41
#define PIT_CH2 0x42

/*
 * Command to set rate generator mode
 */
#define CMD_SQR_WAVE 0x34

/*
 * Command to latch the timer registers
 */
#define CMD_LATCH 0x00

/*
 * The internal tick rate in ticks per second
 */
#define PIT_TICK 1193180

/*
 * The latch count value for the current HZ setting
 */
#define PIT_LATCH ((PIT_TICK + (HZ / 2)) / HZ)

extern clock_device_t x86_clock_device;

#endif /* __MACH_X86_CLOCK_H__ */
