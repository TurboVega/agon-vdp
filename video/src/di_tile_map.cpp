// di_tile_map.cpp - Function definitions for drawing tile maps
//
// A tile map is a set of rectangular tiles, where each tile is a bitmap of
// the same size (width and height). Tiles are arranged in a rectangular
// grid, where the entire portion of the grid that fits within the visible
// area of the screen may be displayed at any given moment. In other words
// multiple tiles show at the same time.
//
// The tile map may be scrolled in any of 8 directions, by setting m_x and m_y
// to the scroll distances on the 2 axes. Just be careful not to scroll too far,
// such that the visible screen would contain pixels not in the logical map.
//
// Copyright (c) 2023 Curtis Whitley
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 

#include "di_tile_map.h"
#include "esp_heap_caps.h"
#include <cstring>

extern "C" {
IRAM_ATTR void DiTileMap_paint(void* this_ptr, const DiPaintParams *params);
}

DiTileMap::DiTileMap(uint32_t screen_width, uint32_t screen_height,
                      uint32_t bitmaps, uint32_t columns, uint32_t rows,
                      uint32_t width, uint32_t height, bool hscroll):
  DiPrimitiveXYWH(0, 0, width * columns, height * rows),
  m_bitmaps(bitmaps),
  m_columns(columns),
  m_rows(rows) {
  m_draw_words_per_line = (width + sizeof(uint32_t) - 1) / sizeof(uint32_t);
  m_words_per_line = m_draw_words_per_line + 2;
  m_bytes_per_line = m_words_per_line * sizeof(uint32_t);
  m_words_per_position = m_words_per_line * height;
  m_bytes_per_position = m_words_per_position * sizeof(uint32_t);
  m_words_per_bitmap = m_words_per_position * (hscroll ? 4 : 1);
  m_bytes_per_bitmap = m_words_per_bitmap * sizeof(uint32_t);
  m_words_per_row = columns;
  m_bytes_per_row = m_words_per_row * sizeof(uint32_t);
  m_words_for_bitmaps = m_words_per_bitmap * bitmaps;
  m_bytes_for_bitmaps = m_words_for_bitmaps * sizeof(uint32_t);
  m_words_for_tiles = columns * rows;
  m_bytes_for_tiles = m_words_for_tiles * sizeof(uint32_t);
  m_words_for_offsets = rows * height * 2;
  m_bytes_for_offsets = m_words_for_offsets * sizeof(uint32_t);
  m_visible_columns = screen_width / width;
  m_visible_rows = screen_height / height;

  size_t new_size = (size_t)(m_bytes_for_tiles);
  void* p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_INTERNAL);
  m_tiles = (uint32_t**)p;

  new_size = (size_t)(m_bytes_for_bitmaps);
  p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_8BIT|MALLOC_CAP_INTERNAL);
  m_pixels = (uint32_t*)p;
  memset(m_pixels, 0x00, m_bytes_for_bitmaps);

  new_size = (size_t)(m_bytes_for_offsets);
  p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_INTERNAL);
  m_offsets = (uint32_t*)p;

  for (uint32_t row = 0; row < rows; row++) {
    for (uint32_t y = 0; y < height; y++) {
      m_offsets[(row * height + y) * 2] = (uint32_t)(m_tiles + row * m_words_per_row); // points to tile map row
      m_offsets[(row * height + y) * 2 + 1] = y * m_bytes_per_line; // offset to bitmap line
    }

    for (uint32_t col = 0; col < columns; col++) {
      set_tile(col, row, 0);
    }
  }
}

DiTileMap::~DiTileMap() {
  if (m_tiles) {
    delete [] m_tiles;
  }
  if (m_pixels) {
    delete [] m_pixels;
  }
  if (m_offsets) {
    delete [] m_offsets;
  }
}

void DiTileMap::set_position(int32_t x, int32_t y) {
  //int32_t dx = x - m_x;
  m_x = x;
  //m_x_extent += dx;
  //int32_t dy = y - m_y;
  m_y = y;
  //m_y_extent += dy;
}

void DiTileMap::set_pixel(int32_t bitmap, int32_t x, int32_t y, uint8_t color) { 
  pixels(m_pixels)[bitmap * m_bytes_per_bitmap + y * m_bytes_per_line + FIX_INDEX(x)] =
    (color & 0x3F) | SYNCS_OFF;
}

void DiTileMap::set_pixel_hscroll(int32_t bitmap, int32_t x, int32_t y, uint8_t color) { 
  for (uint32_t pos = 0; pos < 4; pos++) {
    pixels(m_pixels)[bitmap * m_bytes_per_bitmap + pos * m_bytes_per_position + y * m_bytes_per_line + FIX_INDEX(pos + x)] =
      (color & 0x3F) | SYNCS_OFF;
  }
}

void DiTileMap::set_tile(int32_t column, int32_t row, int32_t bitmap) {
  m_tiles[row * m_words_per_row + column] = m_pixels + bitmap * m_words_per_bitmap;
}

uint8_t DiTileMap::get_tile(int32_t column, int32_t row) {
  uint32_t offset = (uint32_t) m_tiles[row * m_words_per_row + column] - (uint32_t) m_pixels;
  return offset / m_words_per_bitmap;
}

void IRAM_ATTR DiTileMap::paint(const DiPaintParams *params) {
  DiTileMap_paint((void*)this, params);
}
