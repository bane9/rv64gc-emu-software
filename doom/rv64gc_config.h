#ifndef RVCONFIG_H_
#define RVCONFIG_H_
#include <stdint.h>

#define fb_render_addr ((volatile unsigned char *)0xf900000)
#define fb_dimensions_addr (fb_render + sizeof(uint32_t))
#define term_dimensions_addr (fb_dimensions + sizeof(uint32_t))
#define fb_channels_addr (term_dimensions + sizeof(uint32_t))
#define framebuffer_start_addr (fb_channels + sizeof(uint32_t))

#define uart_write_addr ((volatile unsigned char *)0x10000000)
#define uart_read_addr ((volatile unsigned char *)0x10000000)

#define syscon_addr ((volatile unsigned short *)0x11100000)
#define syscon_poweroff (0x5555)
#define syscon_reboot (0x7777)

#endif
