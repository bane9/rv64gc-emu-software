#ifndef RVCONFIG_H_
#define RVCONFIG_H_
#include <stdint.h>

#define fb_render ((volatile unsigned char *)0xf900000)
#define fb_dimensions (fb_render + sizeof(uint32_t))
#define term_dimensions (fb_dimensions + sizeof(uint32_t))
#define fb_channels (term_dimensions + sizeof(uint32_t))
#define framebuffer_start (fb_channels + sizeof(uint32_t))

#define uart_write ((volatile unsigned char *)0x10000000)
#define uart_read ((volatile unsigned char *)0x10000000)

#endif