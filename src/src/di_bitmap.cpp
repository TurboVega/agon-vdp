// di_bitmap.cpp - Function definitions for drawing bitmaps 
//
// An opaque bitmap is a rectangle of fully opaque pixels of various colors.
//
// A masked bitmap is a combination of fully opaque of various colors,and fully
// transparent pixels.
//
// An transparent bitmap is a rectangle that is a combination of fully transparent pixels,
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

DiBitmap::DiBitmap(uint32_t width, uint32_t height, ScrollMode scroll_mode) {
  m_width = width;
  m_height = height;
  m_scroll_mode = (uint32_t)scroll_mode;

  switch (scroll_mode) {
    case NONE:
    case VERTICAL:
      m_words_per_line = ((width + sizeof(uint32_t) - 1) / sizeof(uint32_t));
      m_bytes_per_line = m_words_per_line * sizeof(uint32_t);
      m_words_per_position = m_words_per_line * height;
      m_bytes_per_position = m_words_per_position * sizeof(uint32_t);
      m_pixels = new uint32_t[m_words_per_position];
      memset(m_pixels, 0x00, m_bytes_per_position);
      break;

    case HORIZONTAL:
    case BOTH:
      m_words_per_line = ((width + sizeof(uint32_t) - 1) / sizeof(uint32_t) + 2);
      m_bytes_per_line = m_words_per_line * sizeof(uint32_t);
      m_words_per_position = m_words_per_line * height;
      m_bytes_per_position = m_words_per_position * sizeof(uint32_t);
      m_pixels = new uint32_t[m_words_per_position * 4];
      memset(m_pixels, 0x00, m_bytes_per_position * 4);
      break;
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
  set_pixel(x, y, color);
}

void DiBitmap::set_transparent_color(uint8_t color) {
  m_transparent_color =
    (((uint32_t)color) << 24) |
    (((uint32_t)color) << 16) |
    (((uint32_t)color) << 8) |
    ((uint32_t)color);
}

void DiBitmap::set_pixel(int32_t x, int32_t y, uint8_t color) {
  uint32_t* p;
  int32_t index;

  switch ((ScrollMode)m_scroll_mode) {
    case NONE:
    case VERTICAL:
      p = m_pixels + y * m_words_per_line + (x / 4);
      index = x&3;
      pixels(p)[index] = color;
      break;

    case HORIZONTAL:
    case BOTH:
      for (uint32_t pos = 0; pos < 4; pos++) {
        p = m_pixels + pos * m_words_per_position + y * m_words_per_line + ((pos+x) / 4);
        index = (pos+x)&3;
        pixels(p)[index] = color;
      }
      break;
  }
}

void IRAM_ATTR DiBitmap::delete_instructions() {
  m_paint_fcn.clear();
}
  
void IRAM_ATTR DiBitmap::generate_instructions() {
  m_paint_fcn.clear();
  if (m_flags & PRIM_FLAGS_CAN_DRAW) {
    EspFixups fixups;
    uint32_t at_jump_table = m_paint_fcn.init_jump_table(m_line_pieces.m_num_pieces);
    for (uint32_t i = 0; i < m_line_pieces.m_num_pieces; i++) {
      m_paint_fcn.align32();
      m_paint_fcn.j_to_here(at_jump_table + i * sizeof(uint32_t));
      m_paint_fcn.draw_line(fixups, piece->m_x, piece->m_width, false, m_opaqueness);
    }
    m_paint_fcn.do_fixups(fixups);
  }
}

void IRAM_ATTR DiBitmap::paint(volatile uint32_t* p_scan_line, uint32_t line_index) {
  m_paint_fcn.call(this, p_scan_line, line_index);
}
