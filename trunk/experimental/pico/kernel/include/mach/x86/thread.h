/*
 * mach/x86/thread.h
 *
 */
#ifndef __MACH_X86_THREAD_H__
#define __MACH_X86_THREAD_H__

#include <thread.h>
#include <mach/x86/tss.h>

/*
 * One of these per kernel
 */
extern thread_device_t x86_thread_device;

/*
 * We only use a single TSS as well
 */
extern x86_tss_t x86_global_tss;

#endif  /* __MACH_X86_THREAD_H__ */
