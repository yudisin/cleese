#include <kstdlib.h>
#include <hal.h>
#include <mach/x86/multiboot.h>
#include <mach/x86/x86.h>

extern void Py_Main();

int main(multiboot_information_t * mi)
{
    HAL.initialize(mi);

    kputs("Welcome to CLEESE!\n");

    enable_interrupts();

    Py_Main();

    HAL.shutdown();
    return 0;
}
