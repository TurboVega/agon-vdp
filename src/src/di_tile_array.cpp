// di_tile_array.cpp - Function definitions for drawing tile arrays
//
// A tile array is a set of rectangular tiles, where each tile is a bitmap of
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

#include "di_tile_array.h"
#include <cstring>
//extern void debug_log(const char* fmt, ...);
//#include "freertos/FreeRTOS.h"

DiTileArray::DiTileArray(uint32_t screen_width, uint32_t screen_height,
                      uint32_t columns, uint32_t rows,
                      uint32_t tile_width, uint32_t tile_height, uint16_t flags) {
  //size_t s = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
  //debug_log(" @TM mem %u ", s);
  m_tile_width = tile_width;
  m_tile_height = tile_height;
  m_rows = rows;
  m_columns = columns;
  m_flags = flags;
  uint32_t draw_words_per_line = (tile_width + sizeof(uint32_t) - 1) / sizeof(uint32_t);
  uint32_t words_per_line = draw_words_per_line;

  if (flags & PRIM_FLAG_H_SCROLL) {
    draw_words_per_line += 2;
  }

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

  m_tiles = new DiTileBitmap*[rows * columns];
  if (m_tiles) {
    memset(m_tiles, 0, rows * columns * sizeof(DiTileBitmap*));
  }
}

DiTileArray::~DiTileArray() {
  for (auto map = m_id_to_bitmap_map.begin(); map != m_id_to_bitmap_map.end(); map++) {
    delete map->second;
  }

  if (m_tiles) {
    delete [] m_tiles;
  }
}

void IRAM_ATTR DiTileArray::delete_instructions() {
  for (auto bitmap = m_id_to_bitmap_map.begin(); bitmap != m_id_to_bitmap_map.end(); bitmap++) {
    bitmap->second->delete_instructions();
  }
}
//extern void debug_log(const char* fmt, ...);
void IRAM_ATTR DiTileArray::generate_instructions() {
  //debug_log(" tm @%i flags=%hX w=%u h=%u\n", __LINE__, m_flags, m_width, m_height);
  for (auto bitmap = m_id_to_bitmap_map.begin(); bitmap != m_id_to_bitmap_map.end(); bitmap++) {
    //debug_log("GEN %hu...", bitmap->second->get_id());
    bitmap->second->generate_instructions(m_draw_x, 0, m_tile_width);
    //debug_log("GEN!\n");
  }
}
extern void delay(uint32_t);
void DiTileArray::create_bitmap(DiTileBitmapID bm_id) {
  auto bitmap_item = m_id_to_bitmap_map.find(bm_id);
  if (bitmap_item == m_id_to_bitmap_map.end()) {
    auto bitmap = new DiTileBitmap(bm_id, m_tile_width, m_tile_height, m_flags);
    if (bitmap) {
      //debug_log(" @%i created tbm %08X, flags=%04hX\n", __LINE__, bm_id, m_flags);
    } else {
      //debug_log(" @%i NO MEM tbm %08X, flags=%04hX\n", __LINE__, bm_id, m_flags);
      while(true) delay(10);
    }
    m_id_to_bitmap_map[bm_id] = bitmap;
  }
}

void DiTileArray::set_pixel(DiTileBitmapID bm_id, int32_t x, int32_t y, uint8_t color) {
  m_id_to_bitmap_map[bm_id]->set_transparent_pixel(x, y, color);
}

void DiTileArray::set_tile(int16_t column, int16_t row, DiTileBitmapID bm_id) {
  auto bitmap_item = m_id_to_bitmap_map.find(bm_id);
  if (bitmap_item != m_id_to_bitmap_map.end()) {
    m_tiles[row * m_columns + column] = bitmap_item->second;
  }
}

void DiTileArray::unset_tile(int16_t column, int16_t row) {
    m_tiles[row * m_columns + column] = NULL;
}

DiTileBitmapID DiTileArray::get_tile(int16_t column, int16_t row) {
  auto tile_bitmap = m_tiles[row * m_columns + column];
  if (tile_bitmap) {
    return tile_bitmap->get_id();
  } else {
    return 0;
  }
}
//static bool done;
void IRAM_ATTR DiTileArray::paint(volatile uint32_t* p_scan_line, uint32_t line_index) {
  //debug_log("NO PAINT!"); return;
  auto y_offset_within_tile_array = (int32_t)line_index - m_abs_y;
  //if (!done) debug_log("paint li=%u yotm=%i mdx=%i max=%i ", line_index, y_offset_within_tile_array, m_draw_x, m_abs_x);
  if (y_offset_within_tile_array >= 0 && y_offset_within_tile_array < m_height) {
    auto row = y_offset_within_tile_array / (int32_t)m_tile_height;
    //if (!done) debug_log(" row=%i", row);
    auto start_x_offset_within_tile_array = m_draw_x - m_abs_x;
    auto start_column = (start_x_offset_within_tile_array + m_tile_width - 1) / m_tile_width;
    auto end_x_offset_within_tile_array = m_draw_x_extent - m_abs_x;
    auto end_column = end_x_offset_within_tile_array / m_tile_width;
    auto fcn_index = m_draw_x & 0x3;
    uint32_t draw_x = m_draw_x & 0xFFFFFFFC;
    auto y_offset_within_tile = y_offset_within_tile_array % (int32_t)m_tile_height;
    auto src_pixels_offset = fcn_index * m_bytes_per_position + y_offset_within_tile * m_bytes_per_line;
    //if (!done) debug_log(" sx=%i sc=%i ex=%i ec=%i drx=%i yot=%i spo=%u",
    //  start_x_offset_within_tile_array, start_column, end_x_offset_within_tile_array, end_column, draw_x, y_offset_within_tile, src_pixels_offset);
    auto p_tile_bitmaps = &m_tiles[row * m_columns];
    for (uint32_t col = 0; col < m_columns; col++) {
      auto p_tile_bitmap = *p_tile_bitmaps++;
      if (p_tile_bitmap) {
        //if (!done) debug_log(" paint col=%i drx=%u", col, draw_x);
        p_tile_bitmap->paint(this, fcn_index, p_scan_line, y_offset_within_tile, draw_x, src_pixels_offset);
      }
      draw_x += m_tile_width;
    }
  }
  //if (!done) debug_log("\n");
  //done=true;
}
