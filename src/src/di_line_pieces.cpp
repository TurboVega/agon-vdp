// di_line_points.cpp - Function definitions for generating line points
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

#include "di_line_pieces.h"
#include <cstddef>
#include <string.h>

typedef union {
  int64_t value64;
  struct {
    uint32_t low;
    int32_t high;
  } value32;
} Overlay;

DiLinePieces::DiLinePieces() {
  m_pieces = NULL;
}

DiLinePieces::~DiLinePieces() {
  if (m_pieces) {
    delete [] m_pieces;
  }
}

//void debug_log(const char *format, ...);
void DiLinePieces::generate_line_pieces(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
  m_min_x = MIN(x1, x2);
  m_max_x = MAX(x1, x2);
  m_min_y = MIN(y1, y2);
  m_max_y = MAX(y1, y2);

  int16_t dx = m_max_x - m_min_x;
  int16_t dy = m_max_y - m_min_y;
  int16_t delta = MAX(dx, dy);

  ?? delta == 0 ??
  
  //debug_log("glp x1 %i y1 %i, x2 %i y2 %i, mnx %i mny %i, mxx %i, mxy %i, dx %i, dy %i, delta %i\n",
  //x1, y1, x2, y2, m_min_x, m_min_y, m_max_x, m_max_y, dx, dy, delta);
  Overlay x;
  x.value32.low = 0;
  x.value32.high = m_min_x;
  int64_t delta_x = (((int64_t)dx) << 32) / delta + 1;

  Overlay y;
  y.value32.low = 0;
  y.value32.high = m_min_y;
  int64_t delta_y = (((int64_t)(dy)) << 32) / delta + 1;

//debug_log("delta_x %llX delta_y %llX\n", delta_x, delta_y);
  m_pieces = new DiLinePiece[delta+1];
  int32_t first_x = x.value32.high;
  int32_t first_y = y.value32.high;
  uint16_t i = 0;

  bool x_at_end = (x1 == x2);
  bool y_at_end = (y1 == y2);

  while (i < delta) {
    //debug_log("@77 i %i fx %i fy %i xe %i ye %i\n", i, first_x, first_y, x_at_end, y_at_end);
    Overlay nx;
    Overlay ny;
    if (!x_at_end) {
      nx.value64 = x.value64 + delta_x;
      if (nx.value32.high == m_max_x) {
        x_at_end = true;
      }
    }
    
    if (!y_at_end) {
      ny.value64 = y.value64 + delta_y;
      if (ny.value32.high == m_max_y) {
        y_at_end = true;
      }
    }

    if (ny.value32.high != first_y) {
      m_pieces[i].m_x = (int16_t)first_x;
      m_pieces[i].m_y = (int16_t)first_y;
      uint16_t width = (uint16_t)(ABS(nx.value32.high - first_x));
      if (width == 0) {
          width = 1;
      }
      m_pieces[i++].m_width = width;
      //debug_log("-- x %i y %i w %i\n", first_x, first_y, width);
      
      first_x = nx.value32.high;
      first_y = ny.value32.high;
    }

    if (x_at_end && y_at_end) {
      break;
    }

    x.value64 += delta_x;
    y.value64 += delta_y;
  }
  
  uint16_t width = (int16_t)(ABS(m_max_x - first_x));
  m_pieces[i].m_x = (int16_t)first_x;
  m_pieces[i].m_y = (int16_t)first_y;
  if (width == 0) {
      width = 1;
  }
  m_pieces[i++].m_width = width;
  m_num_pieces = i;
  
//  for (uint16_t j = 0; j < i; j++) {
//      debug_log("@127 %i %i %i\n", j, m_pieces[j].m_x, m_pieces[j].m_width);
//  }

  if (x1<x2 && y1>y2) {
    // Flip the line vertically
    for (uint16_t j = 0; j < i; j++) {
        DiLinePiece t = m_pieces[j];
        m_pieces[j] = m_pieces[m_num_pieces - 1 - j];
        m_pieces[m_num_pieces - 1 - j] = t;
    }
  } else if (x1>x2 && y1<y2) {
    // Flip the line horizontally
    for (uint16_t j = 0; j < i; j++) {
        m_pieces[j].m_x = m_max_x - m_pieces[j].m_x + m_min_x;
    }
  }

//  for (uint16_t j = 0; j < i; j++) {
//      debug_log("@138 %i %i %i\n", j, m_pieces[j].m_x, m_pieces[j].m_width);
//  }

}

void DiLinePieces::generate_line_pieces(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3) {
  DiLinePieces lp[3];
  lp[0].generate_line_pieces(x1, y1, x2, y2);
  lp[1].generate_line_pieces(x2, y2, x3, y3);
  lp[2].generate_line_pieces(x3, y3, x1, y1);

  int16_t min_x = MIN(x1, x2);
  int16_t max_x = MAX(x1, x2);
  int16_t min_y = MIN(y1, y2);
  int16_t max_y = MAX(y1, y2);
  m_min_x = MIN(min_x, x3);
  m_max_x = MAX(max_x, x3);
  m_min_y = MIN(min_y, y3);
  m_max_y = MAX(max_y, y3);

  //debug_log("tri: x1 %i y1 %i, x2 %i y2 %i, x3 %i y3 %i\n", x1, y1, x2, y2, x3, y3);
  //debug_log("tri: mnx %i mny %i, mxx %i, mxy %i", m_min_x, m_min_y, m_max_x, m_max_y);

  m_num_pieces = m_max_y - m_min_y + 1;
  m_pieces = new DiLinePiece[m_num_pieces];

  for (uint16_t i = 0; i < m_num_pieces; i++) {
    DiLinePiece* merge_piece = &m_pieces[i];
    merge_piece->m_y = m_min_y + i;
    merge_piece->m_flags = 0;
  }

  for (uint16_t line = 0; line < 3; line++) {
    DiLinePieces* lpn = &lp[line];
    for (uint16_t i = 0; i < lpn->m_num_pieces; i++) {
      DiLinePiece* line_piece = &lpn->m_pieces[i];
      uint16_t merge_index = line_piece->m_y - m_min_y;
      DiLinePiece* merge_piece = &m_pieces[merge_index];
      if (merge_piece->m_flags) {
        uint16_t left = MIN(line_piece->m_x, merge_piece->m_x);
        uint16_t right1 = line_piece->m_x + line_piece->m_width - 1;
        uint16_t right2 = merge_piece->m_x + merge_piece->m_width - 1;
        uint16_t right = MAX(right1, right2);
        merge_piece->m_x = left;
        merge_piece->m_width = right - left + 1;
        //if (merge_piece->m_x == 224) debug_log("1 line %i i %i w %i\n", line, i, merge_piece->m_width);
      } else {
        merge_piece->m_x = line_piece->m_x;
        merge_piece->m_width = line_piece->m_width;
        merge_piece->m_flags = 1;
        //if (merge_piece->m_x == 224) debug_log("2 line %i i %i w %i\n", line, i, merge_piece->m_width);
      }
    }
  }
}