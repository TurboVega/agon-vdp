// di_primitive.cpp - Function definitions for base drawing primitives
//
// A drawing primitive tells how to draw a particular type of simple graphic object.
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

#include "di_primitive.h"
#include <cstring>

DiPrimitive::DiPrimitive() {
  // Zero out everything but the vtable pointer.
  memset(((uint8_t*)this)+sizeof(DiPrimitive*), 0, sizeof(DiPrimitive)-sizeof(DiPrimitive*));
  m_flags = PRIM_FLAGS_DEFAULT;
}

DiPrimitive::~DiPrimitive() {
}

void DiPrimitive::init_root() {
  // The root primitive covers the entire screen, and is not drawn.
  // The application should define what the base layer of the screen
  // is (e.g., solid rectangle, terminal, tile map, etc.).

  m_flags = PRIM_FLAG_PAINT_KIDS|PRIM_FLAG_CLIP_KIDS;
  m_width = ACT_PIXELS;
  m_height = ACT_LINES;
  m_x_extent = ACT_PIXELS;
  m_y_extent = ACT_LINES;
  m_view_x_extent = ACT_PIXELS;
  m_view_y_extent = ACT_LINES;
}

void DiPrimitive::set_id(uint16_t id) {
  m_id = id;
}

bool IRAM_ATTR DiPrimitive::get_vertical_group_range(int32_t& min_group, int32_t& max_group) {
  if (m_draw_x_extent <= m_draw_x || m_draw_y_extent <= m_draw_y) {
    // The primitive should not be drawn
    return false;
  } else {
    // The primitive should be drawn
    min_group = m_draw_y;
    max_group = m_draw_y_extent - 1;
    return true;
  }
}

void IRAM_ATTR DiPrimitive::paint(volatile uint32_t* p_scan_line, uint32_t line_index) {}

void IRAM_ATTR DiPrimitive::attach_child(DiPrimitive* child) {
  if (m_last_child) {
    m_last_child->m_next_sibling = child;
    child->m_prev_sibling = m_last_child;
  } else {
    m_first_child = child;
  }
  child->m_parent = this;
  m_last_child = child;
}

void IRAM_ATTR DiPrimitive::detach_child(DiPrimitive* child) {
  if (child->m_next_sibling) {
    child->m_next_sibling->m_prev_sibling = child->m_prev_sibling;
  }
  if (child->m_prev_sibling) {
    child->m_prev_sibling->m_next_sibling = child->m_next_sibling;
  }
  if (m_first_child == child) {
    m_first_child = child->m_next_sibling;
  }
  if (m_last_child == child) {
    m_last_child = child->m_prev_sibling;
  }
}

void IRAM_ATTR DiPrimitive::set_relative_position(int32_t rel_x, int32_t rel_y) {
  m_rel_x = rel_x;
  m_rel_y = rel_y;
}

void IRAM_ATTR DiPrimitive::set_relative_deltas(int32_t rel_dx, int32_t rel_dy, uint32_t auto_moves) {
  m_rel_dx = rel_dx;
  m_rel_dy = rel_dy;
  m_auto_moves = auto_moves;
}

void IRAM_ATTR DiPrimitive::set_size(uint32_t width, uint32_t height) {
  m_width = width;
  m_height = height;
}

extern void debug_log(const char* fmt, ...);
void IRAM_ATTR DiPrimitive::compute_absolute_geometry(
  int32_t view_x, int32_t view_y, int32_t view_x_extent, int32_t view_y_extent) {
  
  if (m_flags & PRIM_FLAG_ABSOLUTE) {
    m_abs_x = m_rel_x;
    m_abs_y = m_rel_y;
  } else {
    m_abs_x = m_parent->m_abs_x + m_rel_x;
    m_abs_y = m_parent->m_abs_y + m_rel_y;
  }
  
  m_x_extent = m_abs_x + m_width;
  m_y_extent = m_abs_y + m_height;

  if (m_flags & PRIM_FLAG_CLIP_THIS) {
    m_view_x = view_x;
    m_view_y = view_y;
    m_view_x_extent = view_x_extent;
    m_view_y_extent = view_y_extent;
  } else {
    m_view_x = 0;
    m_view_y = 0;
    m_view_x_extent = ACT_PIXELS;
    m_view_y_extent = ACT_LINES;
  }

  m_draw_x = MAX(m_abs_x, m_view_x);
  m_draw_y = MAX(m_abs_y, m_view_y);
  m_draw_x_extent = MIN(m_x_extent, m_view_x_extent);
  m_draw_y_extent = MIN(m_y_extent, m_view_y_extent);

  m_abs_x_word = m_abs_x & 0xFFFFFFFC;
  m_draw_x_offset = m_draw_x - m_abs_x;
  m_draw_y_offset = m_draw_y - m_abs_y;
  m_draw_x_word = m_draw_x & 0xFFFFFFFC;
  m_draw_x_word_offset = m_draw_x_word - m_abs_x_word;

  //if (m_id>2) debug_log(" GEO id %hu rel(%i,%i) abs(%i,%i), w=%hu, h=%hu, d(%i,%i), de(%i,%i), aw=%i, dxo=%i, dyo=%i, dxw=%i, dxwo=%o\n",
  //  m_id, m_rel_x, m_rel_y, m_abs_x, m_abs_y, m_width, m_height,
  //  m_draw_x, m_draw_y, m_draw_x_extent, m_draw_y_extent,
  //  m_abs_x_word, m_draw_x_offset, m_draw_y_offset, m_draw_x_word, m_draw_x_word_offset);

  DiPrimitive* child = m_first_child;
  while (child) {
    if (m_flags & PRIM_FLAG_CLIP_KIDS) {
      child->compute_absolute_geometry(m_view_x, m_view_y, m_view_x_extent, m_view_y_extent);
    } else {
      child->compute_absolute_geometry(view_x, view_y, view_x_extent, view_y_extent);
    }
    child = child->m_next_sibling;
  }
}

void DiPrimitive::clear_child_ptrs() {
  m_first_child = NULL;
  m_last_child = NULL;
}

void IRAM_ATTR DiPrimitive::delete_instructions() {
}

void IRAM_ATTR DiPrimitive::generate_instructions() {
}

// Convert normal alpha bits of color to opaqueness percentage.
// This will also remove the alpha bits from the color.
uint8_t DiPrimitive::normal_alpha_to_opaqueness(uint8_t &color) {
  uint8_t alpha = color >> 6;
  color &= 0x3F; // remove alpha bits
  switch (alpha) {
    case 0: return 25;
    case 1: return 50;
    case 2: return 75;
    default: return 100;
  }
}

// Convert inverted alpha bits of color to opaqueness percentage.  
// This will also remove the alpha bits from the color.
uint8_t DiPrimitive::inverted_alpha_to_opaqueness(uint8_t &color) {
  uint8_t alpha = color >> 6;
  color &= 0x3F; // remove alpha bits
  switch (alpha) {
    case 1: return 75;
    case 2: return 50;
    case 3: return 25;
    default: return 100;
  }
}
