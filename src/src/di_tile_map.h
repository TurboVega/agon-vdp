// di_tile_map.h - Function declarations for drawing tile maps
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

#pragma once
#include "di_primitive.h"
#include "di_code.h"
#include <map>

typedef uint16_t DiTileImageID;
typedef uint32_t DiRowColumn;

class DiTileImage {
  public:
  DiTileImage(uint32_t tile_width, uint32_t tile_height, uint8_t flags);
  ~DiTileImage();

  // Set the single 8-bit color value used to represent a transparent pixel. This should be
  // an unused color value in the visible image when designing the tile image. This does take
  // out (prevent from using) 1 of the 256 possible color values, when painting a pixel.
  // This function should be called prior to calling set_pixel().
  void set_transparent_color(uint8_t color);

  // Set a single pixel within the allocated bitmap. The upper 2 bits of the color
  // are the transparency level (00BBGGRR is 25% opaque, 01BBGGRR is 50% opaque,
  // 10BBGGRR is 75% opaque, and 11BBGGRR is 100% opaque). If the given color value
  // equals the already-set transparent color, then the pixel will be 100% transparent,
  // meaning 0% opaque. This function should be called after calling set_transparent_color(),
  // if the latter is called. This function should be used if the tile image does NOT
  // support horizontal scrolling at a 1 pixel boundary.
  void set_pixel(int32_t x, int32_t y, uint8_t color, uint32_t bytes_per_line);

  // Set a single pixel within the allocated bitmap. The upper 2 bits of the color
  // are the transparency level (00BBGGRR is 25% opaque, 01BBGGRR is 50% opaque,
  // 10BBGGRR is 75% opaque, and 11BBGGRR is 100% opaque). If the given color value
  // equals the already-set transparent color, then the pixel will be 100% transparent,
  // meaning 0% opaque. This function should be called after calling set_transparent_color(),
  // if the latter is called. This function should be used if the tile image DOES
  // support horizontal scrolling at a 1 pixel boundary.
  void set_pixel_hscroll(int32_t x, int32_t y, uint8_t color,
                uint32_t bytes_per_line, uint32_t bytes_per_position);

  // Clear the custom instructions needed to draw the image.
  void IRAM_ATTR delete_instructions();
   
  // Reassemble the custom instructions needed to draw the image.
  void IRAM_ATTR generate_instructions();

  void IRAM_ATTR paint(volatile uint32_t* p_scan_line, uint32_t line_index);

  protected:
  uint32_t      m_tile_width;         // width in pixels
  uint32_t      m_tile_height;        // height in pixels
  uint32_t*     m_pixels;             // array(s) of pixels
  EspFunction   m_paint_fcn;          // function(s) to paint the pixels
  bool          m_is_transparent;     // whether to use the transparent color
  uint8_t       m_transparent_color;  // value indicating not to draw the pixel
};

typedef std::map<DiTileImageID, DiTileImage*> DiTileIdToImageMap;
typedef std::map<uint16_t, DiTileImage*> DiTileColumnToImageMap;
typedef std::map<uint16_t, DiTileColumnToImageMap*> DiTileRowToColumnMap;

class DiTileMap: public DiPrimitive {
  public:
  // Construct a tile map.
  DiTileMap(uint32_t screen_width, uint32_t screen_height,
            uint32_t columns, uint32_t rows,
            uint32_t tile_width, uint32_t tile_height, uint8_t flags);

  // Destroy a tile map.
  virtual ~DiTileMap();

  // Set the X, Y position relative to the parent (which may be the screen).
  virtual void IRAM_ATTR set_relative_position(int32_t rel_x, int32_t rel_y);

  // Create the array of pixels for the tile image.
  void create_image(DiTileImageID img_id);

  // Save the pixel value of a particular pixel in a specific tile bitmap. A tile bitmap
  // may appear many times on the screen, based on the use of the image ID.
  // Use this function when the tile map should not support horizontal smooth scrolling.
  void set_pixel(DiTileImageID img_id, int32_t x, int32_t y, uint8_t color);

  // Save the pixel value of a particular pixel in a specific tile bitmap. A tile bitmap
  // may appear many times on the screen, based on the use of the image ID.
  // Use this function when the tile map should support horizontal smooth scrolling.
  void set_pixel_hscroll(DiTileImageID img_id, int32_t x, int32_t y, uint8_t color);

  // Set the image ID to use to draw a tile at a specific row and column.
  void set_tile(int32_t column, int32_t row, DiTileImageID img_id);

  // Unset the image ID to use to draw a tile at a specific row and column.
  void unset_tile(int32_t column, int32_t row);

  // Get the image ID presently at the given row and column.
  DiTileImageID get_tile(int16_t column, int16_t row);

  virtual void IRAM_ATTR paint(volatile uint32_t* p_scan_line, uint32_t line_index);

  protected:
  uint32_t  m_columns;              // number of columns (cells in each row)
  uint32_t  m_rows;                 // number of rows (cells in each column)
  uint32_t  m_bytes_per_line;       // number of 1-pixel bytes in each image line
  uint32_t  m_bytes_per_position;   // number of 1-pixel bytes in each image position
  uint32_t  m_visible_columns;      // number of columns that fit on the screen
  uint32_t  m_visible_rows;         // number of rows that fit on the screen
  uint32_t  m_tile_width;           // width of 1 tile in pixels
  uint32_t  m_tile_height;          // height of 1 tile in pixels
  bool      m_is_transparent;       // whether to use the transparent color
  uint8_t   m_transparent_color;    // value indicating not to draw the pixel
  DiTileIdToImageMap m_id_to_type_map;     // caches images based on image ID
  DiTileRowToColumnMap m_pos_to_type_map;  // refers to images based on row & column
};
