// di_tile_bitmap.h - Function declarations for drawing tile bitmaps 
//
// A tile bitmap is essentially like a regular bitmap, except that it is not
// based on a primitive, thus reducing memory requirements. It still holds an
// array of pixels, and contains similar drawing code.
//
// An opaque bitmap is a rectangle of fully opaque pixels of various colors.
//
// A masked bitmap is a combination of fully opaque pixels of various colors,
// and fully transparent pixels.
//
// A transparent bitmap is a rectangle that is a combination of fully transparent pixels,
// partially transparent pixels, and fully opaque pixels, of various colors. 
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

typedef uint32_t DiTileBitmapID;

class DiTileBitmap {
  public:
  // Construct a bitmap.
  DiTileBitmap(DiTileBitmapID bm_id, uint32_t width, uint32_t height, uint16_t flags);

  // Destroy a bitmap.
  ~DiTileBitmap();

  // Set a single pixel within the allocated bitmap. The upper 2 bits of the color
  // are the transparency level (00BBGGRR is 25% opaque, 01BBGGRR is 50% opaque,
  // 10BBGGRR is 75% opaque, and 11BBGGRR is 100% opaque). If the given color value
  // equals the already-set transparent color, then the pixel will be fully transparent,
  // meaning 0% opaque.
  void set_transparent_pixel(int32_t x, int32_t y, uint8_t color);

  // Set the single 8-bit color value used to represent a transparent pixel. This should be
  // an unused color value in the visible image when designing the image. This does take out
  // 1 of the 256 possible color values. The upper 2 bits of the color are the transparency
  // level (00BBGGRR is 25% opaque, 01BBGGRR is 50% opaque, 10BBGGRR is 75% opaque, and
  // 11BBGGRR is 100% opaque).
  void set_transparent_color(uint8_t color);

  // Get the bitmap ID.
  inline DiTileBitmapID get_id() { return m_bm_id; } 

  // Get a pointer to the pixel data.
  inline uint32_t* get_pixels() { return m_pixels; }

  protected:
  // Set a single pixel with an adjusted color value.
  void set_pixel(int32_t x, int32_t y, uint8_t color);

  DiTileBitmapID m_bm_id;
  uint32_t    m_words_per_line;
  uint32_t    m_bytes_per_line;
  uint32_t    m_words_per_position;
  uint32_t    m_bytes_per_position;
  uint32_t*   m_visible_start;
  uint32_t*   m_pixels;
  uint32_t    m_save_height;
  uint32_t    m_built_width;
  uint16_t    m_flags;
  uint8_t     m_transparent_color;
};

class DiPaintableTileBitmap : public DiTileBitmap {
  public:
  // Construct a paintable tile bitmap.
  DiPaintableTileBitmap(DiTileBitmapID bm_id, uint32_t width, uint32_t height, uint16_t flags);

  // Destroy a paintable tile bitmap.
  ~DiPaintableTileBitmap();

  // Clear the custom instructions needed to draw the primitive.
  void IRAM_ATTR delete_instructions();
   
  // Reassemble the custom instructions needed to draw the primitive.
  void IRAM_ATTR generate_instructions(uint32_t draw_x, int32_t x, uint32_t draw_width);
   
  void IRAM_ATTR paint(DiPrimitive* tile_map, int32_t fcn_index, volatile uint32_t* p_scan_line,
                      uint32_t line_index, uint32_t draw_x, uint32_t src_pixels_offset);

  protected:
  EspFunction m_paint_fcn[4];
};
