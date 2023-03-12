#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rv64gc_config.h"

void *_sbrk(int incr)
{
    extern uint8_t _estack;
    extern uint8_t _end;

    static uint8_t *heap_ptr = &_end;

    if (heap_ptr + incr > &_estack)
    {
        errno = ENOMEM;
        return NULL;
    }

    uint8_t *old_ptr = heap_ptr;
    heap_ptr += incr;

    return old_ptr;
}

int _write(int handle, char *data, int size)
{
    for (int i = 0; i < size; i++)
    {
        *uart_write = data[i];
    }

    return size;
}

int _putc_r(struct _reent *u1, int ch, FILE *u2)
{
    *uart_write = ch;
    return 0;
}

int main()
{
    char *argv[] = {"doom", "-iwad", "doom.wad", NULL};
    int argc = sizeof(argv) / sizeof(argv[0]) - 1;

    extern void doom_main(int argc, char **argv);
    doom_main(argc, argv);
}

__attribute__((naked)) void trap_vector(void)
{
    __asm__ volatile(".option norvc;"
                     "mret;"
                     "mret;"
                     "mret;"
                     "mret;"
                     "mret;"
                     "mret;"
                     "mret;"
                     "mret;"
                     "mret;"
                     "mret;"
                     "mret;"
                     "mret;"
                     ".option rvc;");
}

__attribute__((section(".text.init"), naked, noreturn)) void _start()
{
    __asm__ volatile(".option push;"
                     ".option norelax;"
                     "la    gp, __global_pointer$;"
                     ".option pop;"
                     "csrw mtvec, %0;"
                     :
                     : "r"((uintptr_t)trap_vector | 1));

    extern uint8_t _sidata;

    extern uint8_t _sdata;
    extern uint8_t _edata;

    extern uint8_t _sbss;
    extern uint8_t _ebss;

    int bss_size = &_ebss - &_sbss;
    uint8_t *bss_dst = &_sbss;

    for (int i = 0; i < bss_size; i++)
    {
        bss_dst[i] = 0;
    }

    int data_size = &_edata - &_sdata;
    uint8_t *data_src = &_sidata;
    uint8_t *data_dst = &_sdata;

    for (int i = 0; i < data_size; i++)
    {
        data_dst[i] = data_src[i];
    }

    typedef void (*function_t)(void);

    extern function_t __preinit_array_start;
    extern function_t __preinit_array_end;

    for (const function_t *entry = &__preinit_array_start; entry < &__preinit_array_end; ++entry)
    {
        (*entry)();
    }

    extern function_t __init_array_start;
    extern function_t __init_array_end;

    for (const function_t *entry = &__init_array_start; entry < &__init_array_end; ++entry)
    {
        (*entry)();
    }

    main();

    extern function_t __fini_array_start;
    extern function_t __fini_array_end;

    for (const function_t *entry = &__fini_array_start; entry < &__fini_array_end; ++entry)
    {
        (*entry)();
    }

    asm volatile("csrw mtvec, 0;"
                 "ecall;");

    while (1)
        ;
}
