/*
 * mach/x86/rtc.h
 */
#ifndef __MACH_X86_RTC_H__
#define __MACH_X86_RTC_H__

#include <sys/types.h>

#define RTC_EQUIPMENT       0x14
#define RTCEQB_DISP_MASK    0x30
#define RTCEQB_DISP_CGA40   0x10
#define RTCEQB_DISP_CGA80   0x20
#define RTCEQB_DISP_MONO80  0x30
#define RTCEQB_DISP_EGAVGA  0x00

/* 
 * Common definitions for CMOS RTC (Real Time Clock and Memory)  
*/
#define RTC_ADDR_ADDR         0x70  /* port for CMOS ram address  */
#define RTC_ADDR_DATA         0x71  /* port for CMOS data */

extern uchar_t rtcin(uchar_t addr);
extern void rtcout(uchar_t addr, uchar_t val);

#endif /* __MACH_X86_RTC_H__ */
