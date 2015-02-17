#include "lualib.h"
#include "lauxlib.h"
#include "platform.h"
#include "auxmods.h"
#include "lrotable.h"
/**
 * All this code is mostly from http://www.esp8266.com/viewtopic.php?f=21&t=1143&sid=a620a377672cfe9f666d672398415fcb
 * from user Markus Gritsch.
 * I just put this code into its own module and pushed into a forked repo,
 * to easily create a pull request. Thanks to Markus Gritsch for the code.
 *
 */

typedef struct {
  uint8_t brightness;
  uint8_t b;
  uint8_t g;
  uint8_t r;
} apa102_pixel;

typedef struct {
  uint16_t size;
  apa102_pixel pixel_data[];
} apa102 ;

apa102 * get_apa102_buffer(int size) {
  int i;
  apa102 *buffer;
  buffer  = (apa102 *) os_malloc(sizeof(apa102) + sizeof(apa102_pixel) * size);
  buffer->size = size;

  for(i = 0; i < size; i++) {
    buffer->pixel_data[i].brightness = 0xff;
    buffer->pixel_data[i].b = 0x00;
    buffer->pixel_data[i].r = 0x00;
    buffer->pixel_data[i].g = 0x00;
  }

  return buffer;
}

/*
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE ws2812_map[] =
{
  { LSTRKEY( "write" ), LFUNCVAL( ws2812_write )},
  { LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_ws2812(lua_State *L) {
  // TODO: Make sure that the GPIO system is initialized
  LREGISTER(L, "ws2812", ws2812_map);
  return 1;
}
*/

// ----------------------------------------------------------------------------

