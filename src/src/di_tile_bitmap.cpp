// di_tile_bitmap.cpp - Function definitions for drawing tile bitmaps 
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

#include "di_tile_bitmap.h"
#include <cstring>
//extern void debug_log(const char* fmt, ...);

DiTileBitmap::DiTileBitmap(DiTileBitmapID bm_id, uint32_t width, uint32_t height, uint8_t flags) {
  m_bm_id = bm_id;
  m_save_height = height;
  m_flags = flags;
  m_is_transparent = false;
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

DiTileBitmap::~DiTileBitmap() {
  delete [] m_pixels;
}

void DiTileBitmap::set_transparent_pixel(int32_t x, int32_t y, uint8_t color) {
  // Invert the meaning of the alpha bits.
  //debug_log("x=%u y=%u c=%02hX i=%02hX, ", x, y, color, PIXEL_ALPHA_INV_MASK(color));
  set_pixel(x, y, PIXEL_ALPHA_INV_MASK(color));
}

void DiTileBitmap::set_transparent_color(uint8_t color) {
  m_is_transparent = true;
  m_transparent_color = color;
}

void DiTileBitmap::set_pixel(int32_t x, int32_t y, uint8_t color) {
  uint32_t* p;
  int32_t index;

  if (m_flags & PRIM_FLAG_H_SCROLL) {
    for (uint32_t pos = 0; pos < 4; pos++) {
      p = m_pixels + pos * m_words_per_position + y * m_words_per_line + (FIX_INDEX(pos+x) / 4);
      index = FIX_INDEX((pos+x)&3);
      ((uint8_t*)(p))[index] = color;
    }
  } else {
    p = m_pixels + y * m_words_per_line + (FIX_INDEX(x) / 4);
    //debug_log("  p=%08X\n", p);
    index = FIX_INDEX(x&3);
    ((uint8_t*)(p))[index] = color;
  }
}

void IRAM_ATTR DiTileBitmap::delete_instructions() {
  for (uint32_t pos = 0; pos < 4; pos++) {
    m_paint_fcn[pos].clear();
  }
}

void IRAM_ATTR DiTileBitmap::generate_instructions(int32_t draw_x, uint32_t draw_width) {
  //debug_log(" @%i flags=%hX", __LINE__, m_flags);
  delete_instructions();
  if (m_flags & PRIM_FLAG_H_SCROLL) {
//debug_log(" @%i ", __LINE__);
    // Bitmap can be positioned on any horizontal byte boundary (pixel offsets 0..3).
    for (uint32_t pos = 0; pos < 4; pos++) {
//debug_log(" @%i ", __LINE__);
      EspFixups fixups;
      EspFunction* paint_fcn = &m_paint_fcn[pos];
      uint32_t at_jump_table = paint_fcn->init_jump_table(m_save_height);
      uint32_t* src_pixels = m_pixels + pos * m_words_per_position;
      for (uint32_t line = 0; line < m_save_height; line++) {
//debug_log(" @%i ", __LINE__);
        paint_fcn->align32();
        paint_fcn->j_to_here(at_jump_table + line * sizeof(uint32_t));
        //debug_log("gen line=%u, ", line);
        paint_fcn->copy_line(fixups, draw_x, draw_width, false, m_is_transparent, m_transparent_color, src_pixels);
        src_pixels += m_words_per_line;
      }
      paint_fcn->do_fixups(fixups);
    }
  } else {
//debug_log(" @%i ", __LINE__);
    // Bitmap must be positioned on a 4-byte boundary (pixel offset 0)!
    EspFixups fixups;
    EspFunction* paint_fcn = &m_paint_fcn[0];
    uint32_t at_jump_table = paint_fcn->init_jump_table(m_save_height);
    uint32_t* src_pixels = m_pixels;
    for (uint32_t line = 0; line < m_save_height; line++) {
//debug_log(" @%i ", __LINE__);
      paint_fcn->align32();
      paint_fcn->j_to_here(at_jump_table + line * sizeof(uint32_t));
      //debug_log("gen line=%u, ", line);
      paint_fcn->copy_line(fixups, draw_x, draw_width, false, m_is_transparent, m_transparent_color, src_pixels);
      src_pixels += m_words_per_line;
    }
    paint_fcn->do_fixups(fixups);
//debug_log(" @%i ", __LINE__);
  }
}
static bool done;
void IRAM_ATTR DiTileBitmap::paint(DiPrimitive* tile_map, int32_t draw_x, volatile uint32_t* p_scan_line, uint32_t line_index) {
  if (!done) {
    //debug_log(" pf %i size %u line %u from %08X to %08X\n", draw_x & 3, m_paint_fcn[draw_x & 3].get_code_size(), line_index,
    //  m_paint_fcn[draw_x & 3].get_real_address(0), m_paint_fcn[draw_x & 3].get_real_address(m_paint_fcn[draw_x & 3].get_code_size()));
    done=true;
  }
  m_paint_fcn[draw_x & 3].call(tile_map, p_scan_line, line_index);
}
