// di_solid_rectangle.cpp - Function definitions for drawing solid rectangles
//
// A solid rectangle is filled with a single color.
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

#include "di_solid_rectangle.h"

DiSolidRectangle::DiSolidRectangle() {
}

void DiSolidRectangle::make_rectangle(uint16_t flags, int32_t x, int32_t y, uint32_t width, uint32_t height, uint8_t color) {
  m_flags = flags;
  m_opaqueness = DiPrimitive::normal_alpha_to_opaqueness(color);
  m_rel_x = x;
  m_rel_y = y;
  m_width = width;
  m_height = height;
  m_color = PIXEL_COLOR_X4(color);
  m_paint_fcn.enter_and_leave_outer_function();
}

void IRAM_ATTR DiSolidRectangle::delete_instructions() {
  m_paint_fcn.clear();
}

void IRAM_ATTR DiSolidRectangle::generate_instructions() {
  m_paint_fcn.clear();
  if (m_flags & PRIM_FLAGS_CAN_DRAW) {
    EspFixups fixups;
    auto width = (uint16_t)m_width;
    DiLineSections sections;
    sections.add_piece(1, 0, width, false);
    m_paint_fcn.draw_line_as_outer_fcn(fixups, m_draw_x, m_draw_x, &sections, m_flags, m_opaqueness);
    m_paint_fcn.do_fixups(fixups);
  }
}

void IRAM_ATTR DiSolidRectangle::paint(volatile uint32_t* p_scan_line, uint32_t line_index) {
  m_paint_fcn.call(this, p_scan_line, line_index);
}