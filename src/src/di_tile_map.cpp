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
#include <cstring>
extern void debug_log(const char* fmt, ...);

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
  for (auto map = m_row_to_col_map.begin(); map != m_row_to_col_map.end(); map++) {
    delete map->second;
  }
}

void IRAM_ATTR DiTileMap::delete_instructions() {
  for (auto bitmap = m_id_to_type_map.begin(); bitmap != m_id_to_type_map.end(); bitmap++) {
    bitmap->second->delete_instructions();
  }
}

void IRAM_ATTR DiTileMap::generate_instructions() {
  debug_log(" tm @%i flags=%hX\n", __LINE__, m_flags);
  for (auto bitmap = m_id_to_type_map.begin(); bitmap != m_id_to_type_map.end(); bitmap++) {
    bitmap->second->generate_instructions(0, m_tile_width);
  }
}

void DiTileMap::create_bitmap(DiTileBitmapID bm_id) {
  debug_log(" @%i flags=%02X\n", m_flags);
  auto bitmap_item = m_id_to_type_map.find(bm_id);
  if (bitmap_item == m_id_to_type_map.end()) {
    auto bitmap = new DiTileBitmap(bm_id, m_tile_width, m_tile_height, m_flags);
    m_id_to_type_map[bm_id] = bitmap;
  }
}

void DiTileMap::set_pixel(DiTileBitmapID bm_id, int32_t x, int32_t y, uint8_t color) {
  m_id_to_type_map[bm_id]->set_transparent_pixel(x, y, color);
}

void DiTileMap::set_tile(int16_t column, int16_t row, DiTileBitmapID bm_id) {
  debug_log("set_tile %hi %hi %hi\n", column, row, bm_id);
  auto bitmap_item = m_id_to_type_map.find(bm_id);
  if (bitmap_item != m_id_to_type_map.end()) {
    auto row_item = m_row_to_col_map.find(row);
    if (row_item != m_row_to_col_map.end()) {
      auto cb_map = row_item->second;
      (*cb_map)[column] = bitmap_item->second;
    } else {
      auto cb_map = new DiTileColumnToBitmapMap();
      m_row_to_col_map[row] = cb_map;
      (*cb_map)[column] = bitmap_item->second;
    }
  }
}

void DiTileMap::unset_tile(int16_t column, int16_t row) {
  auto row_item = m_row_to_col_map.find(row);
  if (row_item != m_row_to_col_map.end()) {
    auto cb_map = row_item->second;
    auto bitmap_item = cb_map->find(column);
    if (bitmap_item != cb_map->end()) {
      cb_map->erase(bitmap_item);
      if (!cb_map->size()) {
        delete cb_map;
        m_row_to_col_map.erase(row_item);
      }
    }
  }
}

DiTileBitmapID DiTileMap::get_tile(int16_t column, int16_t row) {
  auto row_item = m_row_to_col_map.find(row);
  if (row_item != m_row_to_col_map.end()) {
    auto cb_map = row_item->second;
    auto bitmap_item = cb_map->find(column);
    if (bitmap_item != cb_map->end()) {
      cb_map->erase(bitmap_item);
      if (!cb_map->size()) {
        delete cb_map;
        m_row_to_col_map.erase(row_item);
      }
    }
  }
  return 0;
}
static bool done;
void IRAM_ATTR DiTileMap::paint(volatile uint32_t* p_scan_line, uint32_t line_index) {
  auto y_offset_within_tile_map = (int32_t)line_index - m_abs_y;
  if (!done) debug_log("paint li=%u yotm=%i", line_index, y_offset_within_tile_map);
  if (y_offset_within_tile_map >= 0 && y_offset_within_tile_map < m_height) {
    auto row = y_offset_within_tile_map / (int32_t)m_tile_height;
    if (!done) debug_log(" row=%i", row);
    auto row_item = m_row_to_col_map.find((uint16_t)row);
    if (row_item != m_row_to_col_map.end()) {
      auto cb_map = row_item->second;
      auto start_x_offset_within_tile_map = m_draw_x - m_abs_x;
      auto start_column = (start_x_offset_within_tile_map + m_tile_width - 1) / m_tile_width;
      auto end_x_offset_within_tile_map = m_draw_x_extent - m_abs_x;
      auto end_column = end_x_offset_within_tile_map / m_tile_width;
      auto x = m_draw_x & 0xFFFFFFFC; // TBD
      auto y_offset_within_tile = y_offset_within_tile_map % (int32_t)m_tile_height;
      if (!done) debug_log(" sx=%i sc=%i ex=%i ec=%i x=%i yot=%i",
        start_x_offset_within_tile_map, start_column, end_x_offset_within_tile_map, end_column, x, y_offset_within_tile);
      for (auto column = start_column; column < end_column; column++) {
        auto bitmap_item = cb_map->find(column);
        if (bitmap_item != cb_map->end()) {
          auto bitmap = bitmap_item->second;
          if (!done) debug_log(" paint col=%i x=%i", column, x);
          bitmap->paint(this, x, p_scan_line, y_offset_within_tile);
        }
        x += m_tile_width;
      }
    }
  }
  if (!done) debug_log("\n");
  done=true;
}
