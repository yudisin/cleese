/*
 *  Copyright (C) 2003 Gavin Thomas Nicol
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * hal.c
 */
#include <sys/types.h>
#include <hal.h>
#include <kstdlib.h>

#include <mach/x86/speaker.h>
#include <mach/x86/console.h>
#include <mach/x86/cpu.h>
#include <mach/x86/pic.h>
#include <mach/x86/memory.h>
#include <mach/x86/thread.h>
#include <mach/x86/clock.h>
#include <mach/x86/keyboard.h>

/*
 * Initialize all our associated devices
 */
static void x86_hal_initialize(void *bootinfo)
{
    HAL.m_console->initialize(&HAL, bootinfo);
    HAL.m_speaker->initialize(&HAL, bootinfo);
    HAL.m_cpu->initialize(&HAL, bootinfo);
    HAL.m_memory->initialize(&HAL, bootinfo);
    HAL.m_thread->initialize(&HAL, bootinfo);
    HAL.m_pic->initialize(&HAL, bootinfo);
    HAL.m_clock->initialize(&HAL, bootinfo);
    HAL.m_keyboard->initialize(&HAL, bootinfo);
}

/*
 * Shut down everything.
 * TODO:
 */
static void x86_hal_shutdown()
{
}

/*
 * Single global structure holding the abstractions of the underlying
 * hardware.  
 */
hardware_abstraction_layer_t HAL = {
    x86_hal_initialize,
    x86_hal_shutdown,
    &x86_speaker_device,	/* speaker */
    &x86_console_device,	/* console */
    &x86_cpu_device,		/* CPU */
    &x86_memory_device,		/* memory/GDT */
    &x86_thread_device,		/* thread/TSS */
    &x86_pic_device,		/* PIC/IDT */
    &x86_clock_device,		/* clock */
    &x86_keyboard_device,	/* keyboard */
};
