// di_tile_map.h - Function declarations for drawing tile maps
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

#pragma once
#include "di_primitive.h"

class DiTileMap: public DiPrimitiveXYWH {
  public:
  uint32_t m_bitmaps;
  uint32_t m_columns;
  uint32_t m_rows;
  uint32_t m_words_per_line;
  uint32_t m_bytes_per_line;
  uint32_t m_words_per_bitmap;
  uint32_t m_bytes_per_bitmap;
  uint32_t m_words_for_bitmaps;
  uint32_t m_bytes_for_bitmaps;
  uint32_t m_words_per_row;
  uint32_t m_bytes_per_row;
  uint32_t m_words_for_tiles;
  uint32_t m_bytes_for_tiles;
  uint32_t m_words_for_offsets;
  uint32_t m_bytes_for_offsets;
  uint32_t m_visible_columns;
  uint32_t m_visible_rows;
  uint32_t m_words_per_position;
  uint32_t m_bytes_per_position;
  uint32_t m_draw_words_per_line;
  uint32_t** m_tiles;
  uint32_t* m_pixels;
  uint32_t* m_offsets;

  // Construct a tile map. This will allocate RAM to contain the bitmap data for each tile
  // (e.g., font glyph pixels), plus the tile indexes themselves (e.g., font character codes).
  //
  // Example:
  // DiManager manager;
  // DiTileMap* tile_map = manager.create_tile_map(800, 600, 256, 100, 77, 8, 8, false);
  //
  DiTileMap(uint32_t screen_width, uint32_t screen_height,
            uint32_t bitmaps, uint32_t columns, uint32_t rows,
            uint32_t width, uint32_t height, bool hscroll);

  // Destroy a tile map, including its allocated data.
  virtual ~DiTileMap();

  // Set the screen position of the tile map (i.e., scroll it).
  void set_position(int32_t x, int32_t y);

  // Save the pixel value of a particular pixel in a specific tile bitmap. A tile bitmap
  // may appear many times on the screen, based on the use of the bitmap index.
  // Use this function when the tile map should not support horizontal smooth scrolling.
  void set_pixel(int32_t bitmap, int32_t x, int32_t y, uint8_t color);

  // Save the pixel value of a particular pixel in a specific tile bitmap. A tile bitmap
  // may appear many times on the screen, based on the use of the bitmap index.
  // Use this function when the tile map should support horizontal smooth scrolling.
  void set_pixel_hscroll(int32_t bitmap, int32_t x, int32_t y, uint8_t color);

  // Set the bitmap index to use to draw a tile at a specific row and column.
  void set_tile(int32_t column, int32_t row, int32_t bitmap);

  // Get the bitmap index presently at the given row and column.
  uint8_t get_tile(int32_t column, int32_t row);

  virtual void IRAM_ATTR paint(const DiPaintParams *params);
};

