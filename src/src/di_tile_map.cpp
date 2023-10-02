// di_tile_map.cpp - Function definitions for drawing tile maps
//
// A tile map is a set of rectangular tiles, where each tile is a bitmap of
// the same size (width and height). Tiles are arranged in a rectangular
// grid, where the entire portion of the grid that fits within the visible
// area of the screen may be displayed at any given moment. In other words
// multiple tiles show at the same time.
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

DiTileImage::DiTileImage(uint32_t tile_width, uint32_t tile_height, uint8_t flags) {
  m_tile_width = tile_width;
  m_tile_height = tile_height;
  uint32_t draw_words_per_line = (tile_width + sizeof(uint32_t) - 1) / sizeof(uint32_t);
  uint32_t words_per_line = draw_words_per_line + 2;
  uint32_t words_per_position = words_per_line * tile_height;
  uint32_t words_per_bitmap = words_per_position * ((flags & PRIM_FLAG_H_SCROLL) ? 4 : 1);
  uint32_t bytes_per_bitmap = words_per_bitmap * sizeof(uint32_t);
  m_is_transparent = false;
  m_transparent_color = 0;

  size_t new_size = (size_t)(bytes_per_bitmap);
  void* p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_8BIT|MALLOC_CAP_INTERNAL);
  m_pixels = (uint32_t*)p;
  memset(m_pixels, 0x00, bytes_per_bitmap);
}

DiTileImage::~DiTileImage() {
  delete [] m_pixels;
}

void DiTileImage::set_pixel(int32_t x, int32_t y, uint8_t color, uint32_t bytes_per_line) {
  color = PIXEL_ALPHA_INV_MASK(color);
  ((uint8_t*)m_pixels)[y * bytes_per_line + FIX_INDEX(x)] = color;
}

void DiTileImage::set_pixel_hscroll(int32_t x, int32_t y, uint8_t color,
                uint32_t bytes_per_line, uint32_t bytes_per_position) {
  color = PIXEL_ALPHA_INV_MASK(color);
  for (uint32_t pos = 0; pos < 4; pos++) {
    ((uint8_t*)m_pixels)[pos * bytes_per_position + y * bytes_per_line + FIX_INDEX(x)] = color;
  }
}

void DiTileImage::set_transparent_color(uint8_t color) {
  m_is_transparent = true;
  m_transparent_color = PIXEL_ALPHA_INV_MASK(color);
}

void IRAM_ATTR DiTileImage::delete_instructions() {

}
  
void IRAM_ATTR DiTileImage::generate_instructions() {

}

inline void IRAM_ATTR DiTileImage::paint(volatile uint32_t* p_scan_line, uint32_t line_index) {

}

//------------------------------------------------------------------------------

DiTileMap::DiTileMap(uint32_t screen_width, uint32_t screen_height,
                      uint32_t columns, uint32_t rows,
                      uint32_t tile_width, uint32_t tile_height, uint8_t flags) {
  m_tile_width = tile_width;
  m_tile_height = tile_height;
  m_rows = rows;
  m_columns = columns;
  uint32_t draw_words_per_line = (tile_width + sizeof(uint32_t) - 1) / sizeof(uint32_t);
  uint32_t words_per_line = draw_words_per_line + 2;
  m_bytes_per_line = words_per_line * sizeof(uint32_t);
  uint32_t words_per_position = words_per_line * tile_height;
  m_bytes_per_position = words_per_position * sizeof(uint32_t);

  m_visible_columns = (screen_width + tile_width - 1) / tile_width;
  if (m_visible_columns > columns) {
    m_visible_columns = columns;
  }

  m_visible_rows = (screen_height + tile_height - 1) / tile_height;
  if (m_visible_rows > rows) {
    m_visible_rows = rows;
  }

  m_width = tile_width * columns;
  m_height = tile_height * rows;
}

DiTileMap::~DiTileMap() {
  for (auto map = m_id_to_type_map.begin(); map != m_id_to_type_map.end(); map++) {
    delete map->second;
  }
  for (auto map = m_pos_to_type_map.begin(); map != m_pos_to_type_map.end(); map++) {
    delete map->second;
  }
}

void IRAM_ATTR DiTileMap::set_relative_position(int32_t rel_x, int32_t rel_y) {
  // Tile maps are offset in the reverse direction. Setting a position more to
  // the right (in X) causes the tile map to scroll left, not right.
  DiPrimitive::set_relative_position(-rel_x, rel_y);
}

void DiTileMap::set_pixel(DiTileImageID img_id, int32_t x, int32_t y, uint8_t color) { 
}

void DiTileMap::set_pixel_hscroll(DiTileImageID img_id, int32_t x, int32_t y, uint8_t color) { 
  for (uint32_t pos = 0; pos < 4; pos++) {
    pixels(m_pixels)[bitmap * m_bytes_per_bitmap + pos * m_bytes_per_position + y * m_bytes_per_line + FIX_INDEX(pos + x)] =
      (color & 0x3F) | SYNCS_OFF;
  }
}

void DiTileMap::set_tile(int32_t column, int32_t row, DiTileImageID img_id) {
  m_tiles[row * m_words_per_row + column] = m_pixels + bitmap * m_words_per_bitmap;
}

void DiTileMap::unset_tile(int32_t column, int32_t row) {
  m_tiles[row * m_words_per_row + column] = m_pixels + bitmap * m_words_per_bitmap;
}

DiTileImageID DiTileMap::get_tile(int32_t column, int32_t row) {
  uint32_t offset = (uint32_t) m_tiles[row * m_words_per_row + column] - (uint32_t) m_pixels;
  return offset / m_words_per_bitmap;
}

void IRAM_ATTR DiTileMap::paint(volatile uint32_t* p_scan_line, uint32_t line_index) {
}
