// di_bitmap.cpp - Function definitions for drawing bitmaps 
//
// An opaque bitmap is a rectangle of fully opaque pixels of various colors.
//
// A masked bitmap is a combination of fully opaque of various colors,
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

#include "di_bitmap.h"
#include "esp_heap_caps.h"
#include <cstring>
//extern void debug_log(const char* fmt, ...);

DiBitmap::DiBitmap(uint32_t width, uint32_t height, uint16_t flags) {
  m_width = width;
  m_height = height;
  m_save_height = height;
  m_flags = flags;
  m_transparent_color = 0;

  if (flags & PRIM_FLAG_H_SCROLL) {
      m_words_per_line = ((width + sizeof(uint32_t) - 1) / sizeof(uint32_t) + 2);
      m_bytes_per_line = m_words_per_line * sizeof(uint32_t);
      m_words_per_position = m_words_per_line * height;
      m_bytes_per_position = m_words_per_position * sizeof(uint32_t);
      m_pixels = new uint32_t[m_words_per_position * 4];
      memset(m_pixels, 0x00, m_bytes_per_position * 4);
  } else {
      m_words_per_line = ((width + sizeof(uint32_t) - 1) / sizeof(uint32_t));
      m_bytes_per_line = m_words_per_line * sizeof(uint32_t);
      m_words_per_position = m_words_per_line * height;
      m_bytes_per_position = m_words_per_position * sizeof(uint32_t);
      m_pixels = new uint32_t[m_words_per_position];
      memset(m_pixels, 0x00, m_bytes_per_position);
  }
  m_visible_start = m_pixels;
}

DiBitmap::~DiBitmap() {
  delete [] m_pixels;
}

void DiBitmap::set_relative_position(int32_t x, int32_t y) {
  DiPrimitive::set_relative_position(x, y);
  m_visible_start = m_pixels;
}

void DiBitmap::set_slice_position(int32_t x, int32_t y, uint32_t start_line, uint32_t height) {
  DiPrimitive::set_relative_position(x, y);
  m_height = height;
  m_visible_start = m_pixels + start_line * m_words_per_line;
}

void DiBitmap::set_transparent_pixel(int32_t x, int32_t y, uint8_t color) {
  // Invert the meaning of the alpha bits.
  //debug_log("x=%u y=%u c=%02hX i=%02hX, ", x, y, color, PIXEL_ALPHA_INV_MASK(color));
  set_pixel(x, y, PIXEL_ALPHA_INV_MASK(color));
}

void DiBitmap::set_transparent_color(uint8_t color) {
  m_transparent_color = color;
}

void DiBitmap::set_pixel(int32_t x, int32_t y, uint8_t color) {
  uint32_t* p;
  int32_t index;

  if (m_flags & PRIM_FLAG_H_SCROLL) {
    for (uint32_t pos = 0; pos < 4; pos++) {
      p = m_pixels + pos * m_words_per_position + y * m_words_per_line + (FIX_INDEX(pos+x) / 4);
      index = FIX_INDEX((pos+x)&3);
      pixels(p)[index] = color;
    }
  } else {
    p = m_pixels + y * m_words_per_line + (FIX_INDEX(x) / 4);
    //debug_log("  p=%08X\n", p);
    index = FIX_INDEX(x&3);
    pixels(p)[index] = color;
  }
}

void IRAM_ATTR DiBitmap::delete_instructions() {
  for (uint32_t pos = 0; pos < 4; pos++) {
    m_paint_fcn[pos].clear();
  }
}

void IRAM_ATTR DiBitmap::generate_instructions() {
  delete_instructions();
  if (m_flags & PRIM_FLAGS_CAN_DRAW) {
    if (m_flags & PRIM_FLAG_H_SCROLL) {
      // Bitmap can be positioned on any horizontal byte boundary (pixel offsets 0..3).
      for (uint32_t pos = 0; pos < 4; pos++) {
        EspFixups fixups;
        EspFunction* paint_fcn = &m_paint_fcn[pos];
        uint32_t* src_pixels = m_pixels + pos * m_words_per_position;
        uint32_t draw_width = m_draw_x_extent - m_draw_x;

        if (m_flags & PRIM_FLAGS_ALL_SAME) {
          paint_fcn->copy_line_as_outer_fcn(fixups, m_draw_x, draw_width, m_flags, m_transparent_color, src_pixels);          
        } else {
          uint32_t at_jump_table = paint_fcn->init_jump_table(m_save_height);
          for (uint32_t line = 0; line < m_save_height; line++) {
            paint_fcn->align32();
            paint_fcn->j_to_here(at_jump_table + line * sizeof(uint32_t));
            //debug_log("line=%u, ", line);
            paint_fcn->copy_line_as_inner_fcn(fixups, m_draw_x, draw_width, m_flags, m_transparent_color, src_pixels);
            src_pixels += m_words_per_line;
          }
        }
        paint_fcn->do_fixups(fixups);
      }
    } else {
      // Bitmap must be positioned on a 4-byte boundary (pixel offset 0)!
      EspFixups fixups;
      EspFunction* paint_fcn = &m_paint_fcn[0];
      uint32_t draw_width = m_draw_x_extent - m_draw_x;
      uint32_t* src_pixels = m_pixels;

      if (m_flags & PRIM_FLAGS_ALL_SAME) {
        paint_fcn->copy_line_as_outer_fcn(fixups, m_draw_x, draw_width, m_flags, m_transparent_color, src_pixels);          
      } else {
        uint32_t at_jump_table = paint_fcn->init_jump_table(m_save_height);
        for (uint32_t line = 0; line < m_save_height; line++) {
          paint_fcn->align32();
          paint_fcn->j_to_here(at_jump_table + line * sizeof(uint32_t));
          //debug_log("line=%u, ", line);
          paint_fcn->copy_line_as_inner_fcn(fixups, m_draw_x, draw_width, m_flags, m_transparent_color, src_pixels);
          src_pixels += m_words_per_line;
        }
      }
      paint_fcn->do_fixups(fixups);
    }
  }
}

void IRAM_ATTR DiBitmap::paint(volatile uint32_t* p_scan_line, uint32_t line_index) {
  m_paint_fcn[m_draw_x & 3].call(this, p_scan_line, line_index);
}
