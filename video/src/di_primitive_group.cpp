// di_primitive_group.cpp - Function definitions for grouping primitives
//
// A primitive group is a group of display primtives that can be shown,
// hidden, or moved as a unit (as a collection).
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

#include "di_primitive_group.h"

DiPrimitiveGroup::DiPrimitiveGroup() {
  m_delta_x = 0;
  m_delta_y = 0;
}

DiPrimitiveGroup::~DiPrimitiveGroup() {
  for (auto prim = m_children.begin(); prim != m_children.end(); ++prim) {
    delete (*prim);
  }
}

void DiPrimitiveGroup::get_vertical_line_range(int32_t* min_y, int32_t* max_y) {
  int32_t tmin_y = 0;
  int32_t tmax_y = 0;
  if (!m_children.empty()) {
    auto prim = m_children.begin();
    (*prim)->get_vertical_line_range(&tmin_y, &tmax_y);
    for (++prim; prim != m_children.end(); ++prim) {
      int32_t min_y2 = 0;
      int32_t max_y2 = 0;
      (*prim)->get_vertical_line_range(&min_y2, &max_y2);
      if (min_y2 < tmin_y) {
        tmin_y = min_y2;
      }
      if (max_y2 > tmax_y) {
        tmax_y = max_y2;
      }
    }
  }
  *min_y = tmin_y - m_delta_y;
  *max_y = tmax_y - m_delta_y;
}

void IRAM_ATTR DiPrimitiveGroup::paint(const DiPaintParams *params) {
  if (m_delta_x || m_delta_y) {
    DiPaintParams adjusted_params;
    adjusted_params.m_line32 = params->m_line32;
    adjusted_params.m_line8 = params->m_line8;
    adjusted_params.m_line_index = params->m_line_index;
    adjusted_params.m_screen_width = params->m_screen_width;
    adjusted_params.m_screen_height = params->m_screen_height;
    adjusted_params.m_horiz_scroll = params->m_horiz_scroll + m_delta_x;
    adjusted_params.m_vert_scroll = params->m_vert_scroll + m_delta_y;
    adjusted_params.m_scrolled_y = params->m_scrolled_y + m_delta_y;
    for (auto prim = m_children.begin(); prim != m_children.end(); ++prim) {
      (*prim)->paint(&adjusted_params);
    }
  } else {
    for (auto prim = m_children.begin(); prim != m_children.end(); ++prim) {
      (*prim)->paint(params);
    }
  }
}

void DiPrimitiveGroup::add_primitive(DiPrimitive* prim) {
  m_children.push_back(prim);
}

void DiPrimitiveGroup::remove_primitive(DiPrimitive* prim) {
  for (auto p = m_children.begin(); p != m_children.end(); ++p) {
    if (*p == prim) {
      delete prim;
      m_children.erase(p);
      break;
    }
  }
}

void DiPrimitiveGroup::set_offsets(int32_t delta_x, int32_t delta_y) {
  m_delta_x = delta_x; // the assembler code assumes positive X means across-from-left-to-right
  m_delta_y = -delta_y; // the assembler code assumes positive Y means down-from-top-to-bottom
}
