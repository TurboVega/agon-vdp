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

void DiTileMap::set_pixel(DiTileBitmapID bm_id, int32_t x, int32_t y, uint8_t color) {
  m_id_to_type_map[bm_id]->set_transparent_pixel(x, y, color);
}

void DiTileMap::set_tile(int32_t column, int32_t row, DiTileBitmapID bm_id) {
  uint32_t pos = (row << 16) | column;
  auto rc_map = m_pos_to_type_map.find(pos);
  if (rc_map != m_pos_to_type_map.end()) {

  } else {
    auto cb_map = new DiTileColumnToBitmapMap();
    auto bitmap = new DiTileBitmap();
    cb_map[column] = bitmap;
  }
}

void DiTileMap::unset_tile(int32_t column, int32_t row) {
}

DiTileBitmapID DiTileMap::get_tile(int32_t column, int32_t row) {
}

void IRAM_ATTR DiTileMap::paint(volatile uint32_t* p_scan_line, uint32_t line_index) {
}
