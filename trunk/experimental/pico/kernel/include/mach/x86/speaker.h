/*
 * mach/x86/speaker.h
 */
#ifndef __MACH_X86_SPEAKER_H__
#define __MACH_X86_SPEAKER_H__

#include <speaker.h>

#define X86_FREQUENCY_PORT 0x42
#define X86_SPEAKER_PORT   0x61

extern speaker_device_t x86_speaker_device;

#endif /* __MACH_X86_SPEAKER_H__ */
