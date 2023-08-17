// di_opaque_bitmap.h - Function declarations for drawing opaque bitmaps 
//
// An opaque bitmap is a rectangle of fully opaque pixels of various colors.
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

class DiBitmap: public DiPrimitive {
  public:
  // Construct a bitmap with common virtual functions.
  DiBitmap();

  // Destroy a bitmap.
  virtual ~DiBitmap();

  // Set the position of the bitmap, and assume using pixels starting at the given line in the bitmap.
  // This makes it possible to use a single (tall) bitmap to support animated sprites.
  virtual void IRAM_ATTR set_slice_position(int32_t x, int32_t y, uint32_t start_line, uint32_t height);
};

class DiOpaqueBitmap: public DiBitmap {
  public:
  // Construct an opaque bitmap.
  DiOpaqueBitmap(uint32_t width, uint32_t height, ScrollMode scroll_mode);

  // Destroy an opaque bitmap.
  virtual ~DiOpaqueBitmap();

  // Set the X, Y position relative to the parent (which may be the screen).
  virtual void IRAM_ATTR set_relative_position(int32_t rel_x, int32_t rel_y);

  // Set the position of the bitmap, and assume using pixels starting at the given line in the bitmap.
  // This makes it possible to use a single (tall) bitmap to support animated sprites.
  virtual void IRAM_ATTR set_slice_position(int32_t x, int32_t y, uint32_t start_line, uint32_t height);

  // Set a single pixel within the allocated bitmap. The upper 2 bits of the color
  // are ignored. The lower 6 bits are the raw color.
  void set_opaque_pixel(int32_t x, int32_t y, uint8_t color);

  virtual void IRAM_ATTR paint(const DiPaintParams *params);

  protected:
  // Set a single pixel with an adjusted color value.
  void set_pixel(int32_t x, int32_t y, uint8_t color);

  uint32_t m_words_per_line;
  uint32_t m_bytes_per_line;
  uint32_t m_words_per_position;
  uint32_t m_bytes_per_position;
  uint32_t* m_visible_start;
  uint32_t m_scroll_mode;
  uint32_t* m_pixels;
};
