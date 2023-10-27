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
  memset(this, 0, sizeof(DiPrimitive));
  m_flags = PRIM_FLAGS_DEFAULT;
}

DiPrimitive::~DiPrimitive() {
  deallocate_functions();
}

void DiPrimitive::allocate_functions(uint32_t width) {
  if (m_flags & PRIM_FLAG_H_SCROLL_1) {
    // scroll on 1-pixel boundary
    // width-1 functions for left side
    // 1 function for middle
    // width-1 functions for right side
    // 4 copies for 1-pixel boundaries
    m_num_fcns = ((width - 1) * 2 + 1) * 4;
  } else if (m_flags & PRIM_FLAG_H_SCROLL_4) {
    // scroll on 4-pixel boundary
    // width-1 functions for left side
    // 1 function for middle
    // width-1 functions for right side
    m_num_fcns = ((width + 3) / 4) * 2 + 1;    
  } else {
    // stationary primitive
    m_num_fcns = 1;
  }

  m_functions = new EspFunction[m_num_fcns];
  if (!m_functions) {
    m_num_fcns = 0;
  }
}

void DiPrimitive::deallocate_functions() {
  if (m_num_fcns) {
    delete [] m_functions;
    m_num_fcns = 0;
    m_functions = 0;
    m_cur_fcn = 0;
  }
}

/*
    +==========================================+
    [                                          ]
    [                                          ]
  +-----+             +-----+               +-----+
  |     |             |     |               |     |
  |  L  |             |  M  |               |  R  |
  +-----+             +-----+               +-----+
    [                                          ]
    [                                          ]
    +==========================================+
*/
int32_t DiPrimitive::get_function_index(int32_t width, int32_t x, int32_t view_x_extent) {
  if (m_num_fcns) {
    if (m_flags & PRIM_FLAG_H_SCROLL_1) {
      // scroll on 1-pixel boundary
      // width-1 functions for left side
      // 1 function for middle
      // width-1 functions for right side
      // 4 copies for 1-pixel boundaries
      // m_num_fcns = ((width - 1) * 2 + 1) * 4;
      if (x < 0) {
        // Case L
      } else if (x + width > view_x_extent) {
        // Case R
      } else {
        // Case M
        return 0;
      }
    } else if (m_flags & PRIM_FLAG_H_SCROLL_4) {
      // scroll on 4-pixel boundary
      // width-1 functions for left side
      // 1 function for middle
      // width-1 functions for right side
      // m_num_fcns = ((width + 3) / 4) * 2 + 1;    
      if (x < 0) {
        // Case L
        return -x;
      } else if (x + width > view_x_extent) {
        // Case R
        return 
      } else {
        // Case M
        return 0;
      }
    } else {
      // stationary primitive
      return 0;
    }
  }
}

void DiPrimitive::set_current_function(int32_t width, int32_t x, int32_t view_x_extent) {
  auto index = get_function_index(width, x, view_x_extent);
  m_cur_fcn = &m_functions[index];
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
