// di_set_pixel.cpp - Function definitions for setting individual pixels
//
// A pixel is the smallest visible dot on the screen.
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

#include "di_set_pixel.h"

DiSetPixel::DiSetPixel(int32_t x, int32_t y, uint8_t color) {
  m_opaqueness = DiPrimitive::color_to_opaqueness(color);
  m_rel_x = x;
  m_rel_y = y;
  m_width = 1;
  m_height = 1;
  m_color = (color & 0x3F) | SYNCS_OFF;
}

void IRAM_ATTR DiSetPixel::delete_instructions() {
  m_paint_fcn.clear();
}
  
void IRAM_ATTR DiSetPixel::generate_instructions() {
  m_paint_fcn.clear();
  if (m_flags & PRIM_FLAGS_CAN_DRAW) {
    EspFixups fixups;
    m_paint_fcn.draw_line(fixups, m_draw_x, 1, true, m_opaqueness);
    m_paint_fcn.do_fixups(fixups);
  }
}

void IRAM_ATTR DiSetPixel::paint(volatile uint32_t* p_scan_line, uint32_t line_index) {
  m_paint_fcn.call(this, p_scan_line, line_index);
}
