// Copyright (c) 2020 Guy Turcotte
// Modified for M5Paper support
//
// MIT License. Look at file licenses.txt for details.

#define __SCREEN__ 1
#include "screen.hpp"

#if defined(BOARD_TYPE_M5PAPER)

#include "esp.hpp"
#include <M5EPD.h>

Screen Screen::singleton;

uint16_t Screen::width  = 960;
uint16_t Screen::height = 540;

static M5EPD_Canvas* canvas = nullptr;
static bool initialized = false;

void Screen::clear()
{
  if (!initialized) return;
  M5.EPD.Clear(true);
}

void Screen::update(bool no_full)
{
  if (!initialized || !canvas) return;
  
  if (no_full) {
    // Fast update (partial)
    canvas->pushCanvas(0, 0, UPDATE_MODE_DU4);
  } else {
    // Full update with ghosting removal
    canvas->pushCanvas(0, 0, UPDATE_MODE_GC16);
  }
}

void Screen::force_full_update()
{
  // Force next update to be full
  if (initialized && canvas) {
    canvas->pushCanvas(0, 0, UPDATE_MODE_INIT);
  }
}

void Screen::setup(PixelResolution resolution, Orientation orientation)
{
  if (!initialized) {
    M5.begin();
    M5.EPD.SetRotation(90); // Portrait mode
    M5.EPD.Clear(true);
    M5.RTC.begin();
    
    // Create canvas matching screen dimensions
    canvas = new M5EPD_Canvas(&M5.EPD);
    canvas->createCanvas(width, height);
    canvas->fillCanvas(0); // White background
    
    initialized = true;
  }
  
  pixel_resolution = resolution;
  set_orientation(orientation);
}

void Screen::set_orientation(Orientation orient)
{
  orientation = orient;
  // M5Paper resolution is fixed at 960x540 in landscape, 540x960 in portrait
  // We'll use portrait mode (90 degree rotation)
  width  = 540;
  height = 960;
}

void Screen::draw_bitmap(const unsigned char * bitmap_data, Dim dim, Pos pos)
{
  if (!initialized || !canvas || (bitmap_data == nullptr)) return;

  if ((pos.x >= width) || (pos.y >= height)) return;

  uint32_t x_max = pos.x + dim.width;
  uint32_t y_max = pos.y + dim.height;

  if (y_max > height) y_max = height;
  if (x_max > width)  x_max = width;

  // Draw grayscale bitmap pixel by pixel
  // M5EPD uses 0=black, 255=white (same as our bitmap_data)
  for (uint32_t y = pos.y, row_idx = 0; y < y_max; y++, row_idx++) {
    for (uint32_t x = pos.x, col_idx = 0; x < x_max; x++, col_idx++) {
      uint32_t idx = row_idx * dim.width + col_idx;
      uint8_t gray = bitmap_data[idx];
      // Convert 8-bit grayscale to 4-bit for M5EPD
      uint8_t gray4 = gray >> 4;  // 0-15 range
      canvas->drawPixel(x, y, gray4);
    }
  }
}

void Screen::put_highlight(Dim dim, Pos pos)
{
  if (!initialized || !canvas) return;

  if ((pos.x >= width) || (pos.y >= height)) return;

  uint32_t x_max = pos.x + dim.width;
  uint32_t y_max = pos.y + dim.height;

  if (y_max > height) y_max = height;
  if (x_max > width)  x_max = width;

  // Invert pixels to create highlight effect
  canvas->fillRect(pos.x, pos.y, dim.width, dim.height, 8); // Mid-gray for highlight
}

#if EPUB_INKPLATE_BUILD
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
  if (!initialized || !canvas) return;
  
  // Fill region with white
  uint16_t x_max = region.pos.x + region.dim.width;
  uint16_t y_max = region.pos.y + region.dim.height;

  if (y_max > height) y_max = height;
  if (x_max > width)  x_max = width;

  canvas->fillRect(region.pos.x, region.pos.y, 
                   x_max - region.pos.x, y_max - region.pos.y, 
                   0); // White
}

Screen::PixelResolution Screen::get_pixel_resolution() 
{
  return pixel_resolution;
}

#endif // BOARD_TYPE_M5PAPER
