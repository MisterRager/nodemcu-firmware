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

#define APA102_BUFFER_READY 1
#define APA102_BUFFER_CLEAR 0

typedef struct {
  uint8_t brightness;
  uint8_t b;
  uint8_t g;
  uint8_t r;
} apa102_pixel;

typedef struct {
  uint16_t size;
  uint8_t ready;
  apa102_pixel pixel_data[];
} apa102 ;

uint8_t apa102_initialized = 0;
apa102 *apa102_led_buffer;

apa102 * apa102_get_buffer(int size, uint8_t brightness) {
  int i;
  apa102 *buffer;
  buffer  = (apa102 *) os_malloc(sizeof(apa102) + sizeof(apa102_pixel) * size);
  buffer->size = size;

  for(i = 0; i < size; i++) {
    buffer->pixel_data[i].brightness = brightness;
    buffer->pixel_data[i].b = 0x00;
    buffer->pixel_data[i].r = 0x00;
    buffer->pixel_data[i].g = 0x00;
  }

  return buffer;
}

//Lua: (bool) apa102.init(int size, int brightness)
int apa102_init(lua_State* L) {
  int len = luaL_checkinteger(L, 1);
  int brightness = luaL_checkinteger(L, 2);

  apa102_initialized = 0;
  apa102_led_buffer = apa102_get_buffer(len, brightness);

  //Return TRUE
  lua_pushboolean(L, 1);
  return 1;
}

//Lua: (int) apa102.update(string bits)
static int ICACHE_FLASH_ATTR apa102_update(lua_State* L) {
  const uint8_t pin = luaL_checkinteger(L, 1);
  size_t length;
  const char *buffer = luaL_checklstring(L, 2, &length);
  int i, j, pixel_num;

  if(!apa102_initialized) {
    lua_pushint(-1);
    return 0;
  }


  for(i=0; i<length; i+=3) {
    pixel_num = i / 3;
    apa102_led_buffer->pixel_data[pixel_num].b = buffer[i + 0];
    apa102_led_buffer->pixel_data[pixel_num].g = buffer[i + 1];
    apa102_led_buffer->pixel_data[pixel_num].r = buffer[i + 2];
  }

  for(j=i/3; j<apa102_led_buffer->size; j++) {
    apa102_led_buffer->pixel_data[pixel_num].b = 0;
    apa102_led_buffer->pixel_data[pixel_num].g = 0;
    apa102_led_buffer->pixel_data[pixel_num].r = 0;
  }

  //Return number of bytes written from input
  lua_pushinteger(L, i);
  return 1;
}

//Lua: (int) apa102.flush()
static int ICACHE_FLASH_ATTR apa102_flush(lua_State* L) {
  int k;
  int id = 1;
  int end_bytes = apa102_led_buffer->size / 2;

  if(end_bytes < 4) end_bytes = 4;

  os_intr_lock();
  for(k=0; k<4; k++) {
    platform_spi_send_recv( id, 0);
  }
  for(k=0; k<apa102_led_buffer->size; k++) {
    platform_spi_send_recv( id, apa102_led_buffer->pixel_data[k].brightness );
    platform_spi_send_recv( id, apa102_led_buffer->pixel_data[k].b );
    platform_spi_send_recv( id, apa102_led_buffer->pixel_data[k].g );
    platform_spi_send_recv( id, apa102_led_buffer->pixel_data[k].r );
  }
  for(k=0; k<end_bytes; k++) {
    platform_spi_send_recv( id, 1 );
  }
  os_intr_lock();
}

 
#define MIN_OPT_LEVEL 2
#include "lrodefs.h"
const LUA_REG_TYPE apa102_map[] =
{
  { LSTRKEY( "init" ), LFUNCVAL( apa102_init )},
  { LSTRKEY( "update" ), LFUNCVAL( apa102_update )},
  { LNILKEY, LNILVAL}
};

LUALIB_API int luaopen_apa102(lua_State *L) {
  // TODO: Make sure that the GPIO system is initialized
  LREGISTER(L, AUXLIB_APA102, apa102_map);
  return 1;
}

// ----------------------------------------------------------------------------

