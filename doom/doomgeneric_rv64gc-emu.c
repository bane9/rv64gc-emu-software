#include "doomkeys.h"
#include "m_argv.h"
#include "doomgeneric.h"

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

#include <stdbool.h>
#include "rv64gc_config.h"

int DOOMGENERIC_RESX;
int DOOMGENERIC_RESY;
int DOOMGENERIC_CHANNELS;

void get_fb_info()
{
  uint32_t resolution = *(uint32_t *)fb_dimensions;

  DOOMGENERIC_RESX = resolution & 0xffff;
  DOOMGENERIC_RESY = (resolution >> 16) & 0xffff;
  DOOMGENERIC_CHANNELS = *fb_channels;
}

void set_terminal_resolution(uint64_t cols, uint64_t rows)
{
  *(uint32_t *)term_dimensions = (rows << 16) | cols;
}

void set_resolution(uint64_t width, uint64_t height)
{
  *(uint32_t *)fb_dimensions = (height << 16) | width;
}

void DG_Init()
{
  set_resolution(400, 300);
  set_terminal_resolution(40, 16);
  get_fb_info();
}

void DG_DrawFrame()
{
  *fb_render = 1;
}

void DG_SleepMs(uint32_t ms)
{
  uint32_t start = DG_GetTicksMs();

  while (DG_GetTicksMs() < start + ms)
  {
  }
}

uint32_t DG_GetTicksMs()
{
  uint64_t result;
  asm volatile("csrr %0, 0xc10;"
               : "=r"(result));
  return result;
}

typedef struct
{
  char host_key;
  int doom_key;
  int state;
} key;

key keys[] = {
    {.host_key = 'W',
     .doom_key = KEY_UPARROW},

    {.host_key = 'A',
     .doom_key = KEY_LEFTARROW},

    {.host_key = 'S',
     .doom_key = KEY_DOWNARROW},

    {.host_key = 'D',
     .doom_key = KEY_RIGHTARROW},

    {.host_key = '\n',
     .doom_key = KEY_ENTER},

    {.host_key = ' ',
     .doom_key = KEY_FIRE},

    {.host_key = 'E',
     .doom_key = KEY_USE},

    {.host_key = 'Q',
     .doom_key = KEY_ESCAPE},
};
const int keys_size = sizeof(keys) / sizeof(keys[0]);

int DG_GetKey(int *pressed, unsigned char *doomKey)
{
  char c = *uart_read;
  c = toupper(c);

  for (int i = 0; i < keys_size; i++)
  {
    if (keys[i].state == 1)
    {
      if (keys[i].host_key == c)
      {
        continue;
      }
      else
      {
        keys[i].state = 0;
        *doomKey = keys[i].doom_key;
        *pressed = 0;
        return 1;
      }
    }
    else if (keys[i].host_key == c)
    {
      keys[i].state = 1;
      *doomKey = keys[i].doom_key;
      *pressed = 1;
      return 1;
    }
  }

  return 0;
}

void DG_SetWindowTitle(const char *title)
{
}
