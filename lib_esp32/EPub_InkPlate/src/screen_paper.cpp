#define __SCREEN__ 1
#include "screen.hpp"

#if defined(BOARD_TYPE_PAPER)

extern "C" {
  #include <epdiy.h>
  #include <epd_highlevel.h>
  #include <epd_display.h>
}

// Board definition implemented in EpdiyPaperBoard.c
extern "C" {
  extern const EpdBoardDefinition paper_board;
}

#ifndef EPD_WIDTH
#define EPD_WIDTH 960
#endif

#ifndef EPD_HEIGHT
#define EPD_HEIGHT 540
#endif

static EpdiyHighlevelState s_hl;
static bool s_epd_initialized = false;
static uint8_t *s_framebuffer = nullptr;
static bool s_force_full = true;
static int16_t s_partial_count = 0;
static const int16_t PARTIAL_COUNT_ALLOWED = 10;
static int s_temperature = 20; // TODO: hook real temperature sensor

Screen Screen::singleton;

uint16_t Screen::width  = EPD_WIDTH;
uint16_t Screen::height = EPD_HEIGHT;

void Screen::clear()
{
  if (!s_epd_initialized) return;
  epd_hl_set_all_white(&s_hl);
}

void Screen::update(bool no_full)
{
  if (!s_epd_initialized) return;

  if (s_force_full) {
    epd_hl_update_screen(&s_hl, MODE_GC16, s_temperature);
    s_force_full = false;
    s_partial_count = PARTIAL_COUNT_ALLOWED;
    return;
  }

  if (no_full) {
    epd_hl_update_screen(&s_hl, MODE_GL16, s_temperature);
    s_partial_count = 0;
    return;
  }

  if (s_partial_count <= 0) {
    epd_hl_update_screen(&s_hl, MODE_GC16, s_temperature);
    s_partial_count = PARTIAL_COUNT_ALLOWED;
  } else {
    epd_hl_update_screen(&s_hl, MODE_GL16, s_temperature);
    s_partial_count--;
  }
}

void Screen::force_full_update()
{
  s_force_full = true;
  s_partial_count = 0;
}

void Screen::setup(PixelResolution resolution, Orientation orientation)
{
  if (!s_epd_initialized) {
    epd_set_board(&paper_board);
    epd_init(epd_current_board(), &ED047TC2, EPD_OPTIONS_DEFAULT);
    // Rotate the epdiy drawing coordinates so that the logical page is
    // portrait when the device is held with USB-C at the bottom and the
    // power button on the right.
    epd_set_rotation(EPD_ROT_INVERTED_PORTRAIT);
    // The C fallback for the ESP32 LUT path is slower than the original
    // vector assembly, so we run the LCD at 5 MHz for stability.
    epd_set_lcd_pixel_clock_MHz(5);

    s_hl = epd_hl_init(EPD_BUILTIN_WAVEFORM);
    epd_hl_set_all_white(&s_hl);
    s_framebuffer = epd_hl_get_framebuffer(&s_hl);

    epd_poweron();
    // Ensure any previous image on the panel is fully cleared on first
    // boot so we start from a clean white screen.
    epd_fullclear(&s_hl, s_temperature);
    s_epd_initialized = true;
    s_force_full = false;
    s_partial_count = PARTIAL_COUNT_ALLOWED;
  }

  pixel_resolution = resolution;
  set_orientation(orientation);
}

void Screen::set_orientation(Orientation orient)
{
  orientation = orient;
  // With EPD_ROT_INVERTED_PORTRAIT set at init time, epdiy exposes a
  // logical portrait space of 540x960 (EPD_HEIGHT x EPD_WIDTH). Keep the
  // logical Screen dimensions fixed to that space regardless of the
  // orientation enum so the layout engine can use the full page.
  width  = EPD_HEIGHT;  // 540
  height = EPD_WIDTH;   // 960
}

static inline uint8_t map_gray(uint8_t v)
{
  // Convert an 8-bit grayscale value (0=black..255=white) into an epdiy
  // 4-bit grayscale nibble (0=white..15=black).
  return (uint8_t)(15 - (v >> 4));
}

static inline void set_pixel_nibble_physical(uint16_t x, uint16_t y, uint8_t nibble)
{
  // Write a 4-bpp pixel directly into the epdiy framebuffer.
  // x: 0..EPD_WIDTH-1 (960), y: 0..EPD_HEIGHT-1 (540)
  uint8_t * buf_ptr = &s_framebuffer[y * (EPD_WIDTH / 2) + (x >> 1)];
  if (x & 1) {
    *buf_ptr = (uint8_t)((*buf_ptr & 0x0F) | ((nibble & 0x0F) << 4));
  } else {
    *buf_ptr = (uint8_t)((*buf_ptr & 0xF0) | (nibble & 0x0F));
  }
}

static inline void set_pixel_gray(uint16_t x, uint16_t y, uint8_t gray)
{
  // Screen coordinates for Paper are logical portrait (width=540, height=960)
  // with epdiy set to EPD_ROT_INVERTED_PORTRAIT.
  // The equivalent physical coordinates in the 960x540 framebuffer are:
  //   x_phys = y
  //   y_phys = (EPD_HEIGHT - 1) - x
  set_pixel_nibble_physical(y, (uint16_t)((EPD_HEIGHT - 1) - x), map_gray(gray));
}

void Screen::draw_bitmap(const unsigned char * bitmap_data, Dim dim, Pos pos)
{
  if (!s_epd_initialized || (bitmap_data == nullptr)) return;

  if ((pos.x >= width) || (pos.y >= height)) return;

  uint32_t x_max = pos.x + dim.width;
  uint32_t y_max = pos.y + dim.height;

  if (y_max > height) y_max = height;
  if (x_max > width)  x_max = width;

  // Simple grayscale blit. Each pixel in bitmap_data is 0..255.
  for (uint32_t y = pos.y, row_idx = 0; y < y_max; y++, row_idx++) {
    for (uint32_t x = pos.x, col_idx = 0; x < x_max; x++, col_idx++) {
      uint32_t idx = row_idx * dim.width + col_idx;
      uint8_t gray = bitmap_data[idx];
      set_pixel_gray((uint16_t)x, (uint16_t)y, gray);
    }
  }
}

void Screen::put_highlight(Dim dim, Pos pos)
{
  if (!s_epd_initialized) return;

  if ((pos.x >= width) || (pos.y >= height)) return;

  uint32_t x_max = pos.x + dim.width;
  uint32_t y_max = pos.y + dim.height;

  if (y_max > height) y_max = height;
  if (x_max > width)  x_max = width;

  // Invert pixels to create highlight effect
  for (uint32_t y = pos.y; y < y_max; y++) {
    for (uint32_t x = pos.x; x < x_max; x++) {
      // Read current pixel from physical framebuffer
      uint16_t x_phys = (uint16_t)y;
      uint16_t y_phys = (uint16_t)((EPD_HEIGHT - 1) - x);
      uint8_t * buf_ptr = &s_framebuffer[y_phys * (EPD_WIDTH / 2) + (x_phys >> 1)];
      uint8_t val = *buf_ptr;
      uint8_t nibble;

      if (x_phys & 1) {
        nibble = (val >> 4) & 0x0F;
        nibble = 15 - nibble;  // invert
        *buf_ptr = (uint8_t)((val & 0x0F) | (nibble << 4));
      } else {
        nibble = val & 0x0F;
        nibble = 15 - nibble;  // invert
        *buf_ptr = (uint8_t)((val & 0xF0) | nibble);
      }
    }
  }
}

#if EPUB_INKPLATE_BUILD
  #include "esp.hpp"
  #include "alloc.hpp"
  #include "logging.hpp"

  Alloc         alloc;
  TTF           ttf;
  Fonts         fonts;
  Books         books;
  EPub          epub;
  #if !(DEBUGGING)
    Unzipper      unzipper;
  #endif
  PageLocs      page_locs;
  Screen      & screen         = Screen::singleton;
  InkPlate      inkplate;
  Controllers   controllers;
  ESP           esp;
#endif

void 
Screen::get_orientation(Orientation  & orient, 
                        int16_t      & width_val, 
                        int16_t      & height_val)
{
  orient     = orientation;
  width_val  = width;
  height_val = height;
}

void Screen::clear(const Region & region)
{
  if (!s_epd_initialized) return;
  
  // Fill region with white (gray=255)
  uint16_t x_max = region.pos.x + region.dim.width;
  uint16_t y_max = region.pos.y + region.dim.height;

  if (y_max > height) y_max = height;
  if (x_max > width)  x_max = width;

  for (uint16_t y = region.pos.y; y < y_max; y++) {
    for (uint16_t x = region.pos.x; x < x_max; x++) {
      set_pixel_gray(x, y, 255);
    }
  }
}

Screen::PixelResolution Screen::get_pixel_resolution() 
{
  return pixel_resolution;
}

#endif // BOARD_TYPE_PAPER
