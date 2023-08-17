// di_opaque_bitmap.cpp - Function definitions for drawing opaque bitmaps 
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

#include "di_opaque_bitmap.h"
#include "esp_heap_caps.h"
#include <cstring>

extern "C" {
IRAM_ATTR void DiOpaqueBitmap_paint(void* this_ptr, const DiPaintParams *params);
}

DiBitmap::DiBitmap() {}

DiBitmap::~DiBitmap() {}

void IRAM_ATTR DiBitmap::set_position(int32_t x, int32_t y) {}

void IRAM_ATTR DiBitmap::set_position(int32_t x, int32_t y, uint32_t start_line, uint32_t height) {}

//-----------------------------------------------------

DiOpaqueBitmap::DiOpaqueBitmap(uint32_t width, uint32_t height, ScrollMode scroll_mode) {
  m_width = width;
  m_height = height;
  m_scroll_mode = (uint32_t)scroll_mode;

  switch (scroll_mode) {
    case NONE:
    case VERTICAL:
      m_words_per_line = (width + sizeof(uint32_t) - 1) / sizeof(uint32_t);
      m_bytes_per_line = m_words_per_line * sizeof(uint32_t);
      m_words_per_position = m_words_per_line * height;
      m_bytes_per_position = m_words_per_position * sizeof(uint32_t);
      m_pixels = new uint32_t[m_words_per_position];
      memset(m_pixels, SYNCS_OFF, m_bytes_per_position);
      break;

    case HORIZONTAL:
    case BOTH:
      m_words_per_line = (width + sizeof(uint32_t) - 1) / sizeof(uint32_t) + 2;
      m_bytes_per_line = m_words_per_line * sizeof(uint32_t);
      m_words_per_position = m_words_per_line * height;
      m_bytes_per_position = m_words_per_position * sizeof(uint32_t);
      m_pixels = new uint32_t[m_words_per_position * 4];
      memset(m_pixels, SYNCS_OFF, m_bytes_per_position * 4);
      break;
  }
  m_visible_start = m_pixels;
}

DiOpaqueBitmap::~DiOpaqueBitmap() {
  delete [] m_pixels;
}

void DiOpaqueBitmap::set_position(int32_t x, int32_t y) {
  set_relative_position(x, y);
  m_visible_start = m_pixels;
}

void DiOpaqueBitmap::set_position(int32_t x, int32_t y, uint32_t start_line, uint32_t height) {
  set_relative_position(x, y);
  m_height = height;
  m_visible_start = m_pixels + start_line * m_words_per_line;
}

void DiOpaqueBitmap::set_opaque_pixel(int32_t x, int32_t y, uint8_t color) {
  set_pixel(x, y, (color & 0x3F) | SYNCS_OFF);
}

void DiOpaqueBitmap::set_pixel(int32_t x, int32_t y, uint8_t color) {
  uint32_t* p;
  int32_t index;

  switch ((ScrollMode)m_scroll_mode) {
    case NONE:
    case VERTICAL:
      p = m_pixels + y * m_words_per_line + (x / 4);
      index = FIX_INDEX(x&3);
      pixels(p)[index] = color;
      break;

    case HORIZONTAL:
    case BOTH:
      for (uint32_t pos = 0; pos < 4; pos++) {
        p = m_pixels + pos * m_words_per_position + y * m_words_per_line + ((pos+x) / 4);
        index = FIX_INDEX((pos+x)&3);
        pixels(p)[index] = color;
      }
      break;
  }
}

void IRAM_ATTR DiOpaqueBitmap::paint(const DiPaintParams *params) {
  DiOpaqueBitmap_paint((void*)this, params);
}
