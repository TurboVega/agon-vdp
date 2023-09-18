// di_general_line.cpp - Function definitions for drawing general lines
//
// A general line is 1 pixel thick, and connects any 2 points, except that
// it should not be used for vertical, horizontal, or precisely diagonal
// lines, because there are other optimized classes for those cases.
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

#include "di_general_line.h"
//void debug_log(const char *format, ...);

static int32_t min3(int32_t a, int32_t b, int32_t c) {
  int32_t m = MIN(a, b);
  return MIN(m, c);
}

static int32_t max3(int32_t a, int32_t b, int32_t c) {
  int32_t m = MAX(a, b);
  return MAX(m, c);
}

DiGeneralLine::DiGeneralLine() {}

void DiGeneralLine::init_params(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t color, uint8_t opaqueness) {
  m_opaqueness = opaqueness;
  m_rel_x = MIN(x1,x2);
  m_rel_y = MIN(y1,y2);
  m_width = MAX(x1,x2) - m_rel_x + 1;
  m_height = MAX(y1,y2) - m_rel_y + 1;
  color &= 0x3F; // remove any alpha bits
  m_color =
    (((uint32_t)color) << 24) |
    (((uint32_t)color) << 16) |
    (((uint32_t)color) << 8) |
    ((uint32_t)color) | SYNCS_OFF_X4;
  
  m_line_pieces.generate_line_pieces(x1, y1, x2, y2);
}

void DiGeneralLine::init_params(int32_t x1, int32_t y1,
  int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint8_t color, uint8_t opaqueness) {
  m_opaqueness = opaqueness;
  m_rel_x = min3(x1,x2,x3);
  m_rel_y = min3(y1,y2,y3);
  m_width = max3(x1,x2,x3) - m_rel_x + 1;
  m_height = max3(y1,y2,y3) - m_rel_y + 1;
  color &= 0x3F; // remove any alpha bits
  m_color =
    (((uint32_t)color) << 24) |
    (((uint32_t)color) << 16) |
    (((uint32_t)color) << 8) |
    ((uint32_t)color) | SYNCS_OFF_X4;
  
  m_line_pieces.generate_line_pieces(x1, y1, x2, y2, x3, y3);
}

void IRAM_ATTR DiGeneralLine::delete_instructions() {
  m_paint_fcn.clear();
}
  
void IRAM_ATTR DiGeneralLine::generate_instructions() {
  //debug_log("start DiGeneralLine::generate_instructions\r\n");
  m_paint_fcn.clear();
  if (m_flags & PRIM_FLAGS_CAN_DRAW) {
    EspFixups fixups;
    uint32_t at_jump_table = m_paint_fcn.init_jump_table(m_line_pieces.m_num_pieces);
    //debug_log("m_line_pieces.m_num_pieces %u\r\n", m_line_pieces.m_num_pieces);
    for (uint32_t i = 0; i < m_line_pieces.m_num_pieces; i++) {
      DiLinePiece* piece = &m_line_pieces.m_pieces[i];
      //debug_log("\ni=%u x%i y%i w%i\n", i, piece->m_x, piece->m_y, piece->m_width);
      m_paint_fcn.align32();
      m_paint_fcn.j_to_here(at_jump_table + i * sizeof(uint32_t));
      m_paint_fcn.draw_line(fixups, piece->m_x, piece->m_width, false, m_opaqueness);
    }
    m_paint_fcn.do_fixups(fixups);
  }
  //debug_log("end DiGeneralLine::generate_instructions\r\n");

  //debug_log("\r\nleave DiGeneralLine::generate_instructions\r\n");
}

void IRAM_ATTR DiGeneralLine::paint(volatile uint32_t* p_scan_line, uint32_t line_index) {
  m_paint_fcn.call(this, p_scan_line, line_index);
  //debug_log("this=0x%X, code=0x%X, calc=0x%X\r\n", this, m_paint_fcn.get_code_start(), m_future32);
}