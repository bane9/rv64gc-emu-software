#include "doomkeys.h"
#include "m_argv.h"
#include "doomgeneric.h"

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

#include <stdbool.h>
#include "rv64gc_config.h"

#if LINUX_TARGET
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <termios.h>

static struct termios old_term;
#endif

#define TARGET_RESX 400
#define TARGET_RESY 300
#define TARGET_CHANNELS 3

int DOOMGENERIC_RESX;
int DOOMGENERIC_RESY;
int DOOMGENERIC_CHANNELS;
volatile unsigned char *DOOMGENERIC_FB;

static int old_resx;
static int old_resy;

static volatile unsigned char *fb_dimensions;
static volatile unsigned char *fb_channels;
static volatile unsigned char *term_dimensions;
static volatile unsigned char *fb_render;
static volatile unsigned char *uart_read;

static int term_cols;
static int term_rows;

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

void get_terminal_resolution(void)
{
  uint32_t dimensions = *(uint32_t *)term_dimensions;
  term_cols = dimensions & 0xffff;
  term_rows = (dimensions >> 16) & 0xffff;
}

void set_resolution(uint64_t width, uint64_t height)
{
  *(uint32_t *)fb_dimensions = (height << 16) | width;
}

#if LINUX_TARGET
void cleanup(void)
{
  tcsetattr(0, TCSAFLUSH, &old_term);
  set_resolution(old_resx, old_resy);
  set_terminal_resolution(term_cols, term_rows);
}
#endif

void DG_Init()
{
#if LINUX_TARGET
  int fd = open("/dev/mem", O_RDWR | O_SYNC);
  if (fd < 0)
  {
    perror("Failed to open /dev/mem");
    exit(1);
  }

  fb_render = mmap(NULL, 5 * sizeof(uint32_t) + TARGET_RESX * TARGET_RESY * TARGET_CHANNELS, PROT_READ | PROT_WRITE, MAP_SHARED, fd, fb_render_addr);
  if (fb_dimensions == MAP_FAILED)
  {
    perror("Failed to mmap fb_dimensions");
    exit(1);
  }

  fb_dimensions = fb_render + 1 * sizeof(uint32_t);
  term_dimensions = fb_render + 2 * sizeof(uint32_t);
  fb_channels = fb_render + 3 * sizeof(uint32_t);
  DOOMGENERIC_FB = fb_render + 4 * sizeof(uint32_t);

  close(fd);

  struct termios term;
  tcgetattr(0, &term);
  tcgetattr(0, &old_term);
  term.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(0, TCSAFLUSH, &term);

  get_fb_info();

  old_resx = DOOMGENERIC_RESX;
  old_resy = DOOMGENERIC_RESY;

  get_terminal_resolution();

  atexit(cleanup);
  at_quick_exit(cleanup);
#else
  fb_render = fb_render_addr;
  fb_channels = fb_channels_addr;
  term_dimensions = term_dimensions_addr;
  fb_dimensions = fb_dimensions_addr;
  uart_read = uart_read_addr;
  DOOMGENERIC_FB = framebuffer_start_addr;
#endif

  set_resolution(TARGET_RESX, TARGET_RESY);
  get_fb_info();
  set_terminal_resolution(40, 16);
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
  asm volatile("rdtime %0;"
               : "=r"(result));
  return result / 1000;
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

    {.host_key = 'Y',
     .doom_key = 'y'},

    {.host_key = 'N',
     .doom_key = 'n'},

    {.host_key = 'P'},
};
const int keys_size = sizeof(keys) / sizeof(keys[0]);

int speed_toggle = 0;

#if LINUX_TARGET
static char getCharFromStdin(void)
{
  fd_set fds;
  struct timeval timeout;
  char ch = '\0';

  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds);

  timeout.tv_sec = 0; // Set timeout to zero for non-blocking check
  timeout.tv_usec = 0;

  int ready = select(STDIN_FILENO + 1, &fds, NULL, NULL, &timeout);
  if (ready == -1)
  {
    perror("select");
    return '\0';
  }

  if (FD_ISSET(STDIN_FILENO, &fds))
  {
    ch = getchar();
    return ch;
  }

  return '\0';
}
#endif

int DG_GetKey(int *pressed, unsigned char *doomKey)
{
  char c;

#if LINUX_TARGET
  c = getCharFromStdin();
#else
  c = *uart_read;
#endif

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
        if (c == 'P')
        {
          return 0;
        }
        else
        {
          *doomKey = keys[i].doom_key;
          *pressed = 0;
          return 1;
        }
      }
    }
    else if (keys[i].host_key == c)
    {
      keys[i].state = 1;
      if (c == 'P')
      {
        speed_toggle = !speed_toggle;
        return 0;
      }
      else
      {
        *doomKey = keys[i].doom_key;
        *pressed = 1;
        return 1;
      }
    }
  }

  return 0;
}

void DG_SetWindowTitle(const char *title)
{
}
