// di_masked_bitmap.cpp - Function definitions for drawing masked bitmaps 
//
// A masked bitmap is a combination of fully opaque of various colors,and fully
// transparent pixels.
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

#include "di_masked_bitmap.h"
#include "esp_heap_caps.h"
#include <cstring>

extern "C" {
IRAM_ATTR void DiMaskedBitmap_paint(void* this_ptr, const DiPaintParams *params);
}

DiMaskedBitmap::DiMaskedBitmap(uint32_t width, uint32_t height, ScrollMode scroll_mode) {
  m_width = width;
  m_height = height;
  m_visible_start = m_pixels;
  m_scroll_mode = (uint32_t)scroll_mode;

  switch (scroll_mode) {
    case NONE:
    case VERTICAL:
      m_words_per_line = ((width + sizeof(uint32_t) - 1) / sizeof(uint32_t)) * 2;
      m_bytes_per_line = m_words_per_line * sizeof(uint32_t);
      m_words_per_position = m_words_per_line * height;
      m_bytes_per_position = m_words_per_position * sizeof(uint32_t);
      {
        uint32_t* p = m_pixels;
        for (uint32_t i = 0; i < m_words_per_position; i+=2) {
          *p++ = 0xFFFFFFFF; // inverted mask
          *p++ = SYNCS_OFF_X4; // color
        }
      }
      break;

    case HORIZONTAL:
    case BOTH:
      m_words_per_line = ((width + sizeof(uint32_t) - 1) / sizeof(uint32_t) + 2) * 2;
      m_bytes_per_line = m_words_per_line * sizeof(uint32_t);
      m_words_per_position = m_words_per_line * height;
      m_bytes_per_position = m_words_per_position * sizeof(uint32_t);
      {
        uint32_t* p = m_pixels;
        uint32_t n = m_words_per_position * 4;
        for (uint32_t i = 0; i < n; i+=2) {
          *p++ = 0xFFFFFFFF; // inverted mask
          *p++ = SYNCS_OFF_X4; // color
        }
      }
      break;
  }
}

void* DiMaskedBitmap::operator new(size_t size, uint32_t width, uint32_t height, ScrollMode scroll_mode) {
  size_t new_size;
  uint32_t wpl;
  uint32_t wpp;
  uint32_t bpp;
  switch (scroll_mode) {
    case NONE:
    case VERTICAL:
      wpl = ((width + sizeof(uint32_t) - 1) / sizeof(uint32_t)) * 2;
      wpp = wpl * height;
      bpp = wpp * sizeof(uint32_t);
      new_size = (size_t)(sizeof(DiMaskedBitmap) - sizeof(uint32_t) + (bpp));
      break;

    case HORIZONTAL:
    case BOTH:
      wpl = ((width + sizeof(uint32_t) - 1) / sizeof(uint32_t) + 2) * 2;
      wpp = wpl * height;
      bpp = wpp * sizeof(uint32_t);
      new_size = (size_t)(sizeof(DiMaskedBitmap) - sizeof(uint32_t) + (bpp * 4));
      break;
  }
  void* p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_8BIT|MALLOC_CAP_SPIRAM);
  return p;
}

void DiMaskedBitmap::set_position(int32_t x, int32_t y) {
  set_relative_position(x, y);
  m_visible_start = m_pixels;
}

void DiMaskedBitmap::set_position(int32_t x, int32_t y, uint32_t start_line, uint32_t height) {
  set_relative_position(x, y);
  m_height = height;
  m_visible_start = m_pixels + start_line * m_words_per_line;
}

void DiMaskedBitmap::set_masked_pixel(int32_t x, int32_t y, uint8_t color) {
  if (color & 0xC0) {
    set_pixel(x, y, (color & 0x3F) | SYNCS_OFF);
  }
}

void DiMaskedBitmap::set_pixel(int32_t x, int32_t y, uint8_t color) {
  uint8_t* p;
  int32_t index;

  switch ((ScrollMode)m_scroll_mode) {
    case NONE:
    case VERTICAL:
      p = pixels(m_pixels + y * m_words_per_line + (x / 4) * 2);
      index = FIX_INDEX(x&3);
      p[index] = 0x00; // inverted mask
      p[index + 4] = color;
      break;

    case HORIZONTAL:
    case BOTH:
      for (uint32_t pos = 0; pos < 4; pos++) {
        p = pixels(m_pixels + pos * m_words_per_position + y * m_words_per_line + ((pos+x) / 4) * 2);
        index = FIX_INDEX((pos+x)&3);
        p[index] = 0x00; // inverted mask
        p[index + 4] = color;
      }
      break;
  }
}

void IRAM_ATTR DiMaskedBitmap::paint(const DiPaintParams *params) {
  DiMaskedBitmap_paint((void*)this, params);
}
