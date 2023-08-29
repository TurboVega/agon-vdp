// di_diag_left_line.cpp - Function definitions for drawing diagonal lines to the left
//
// A diagonal line is 1 pixel thick, and connects the diagonal points of a square.
// A 'left' line decreases in X as it increases in Y (going down).
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

#include "di_diag_left_line.h"

DiDiagonalLeftLine::DiDiagonalLeftLine() {
}

void DiDiagonalLeftLine::init_params(int32_t x, int32_t y, int32_t length, uint8_t color) {
  m_rel_x = x - length + 1;
  m_rel_y = y;
  m_width = length;
  m_height = length;
  m_color = (color & 0x3F) | SYNCS_OFF;
}

void IRAM_ATTR DiDiagonalLeftLine::paint(const DiPaintParams *params) {
}
