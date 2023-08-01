// di_diag_left_line_asm.S - Low-level assembler function for drawing diagonal lines to the left
//
// A diagonal line is 1 pixel thick, and connects the diagonal points of a square.
// A 'left' line decreases in X as it increases in Y (going down).
//
// Copyright (c) 2023 Curtis Whitley
// 
// The function defined in this source may be specified
// in a C source that calls it, using this declaration:
//
// extern "C" {
// IRAM_ATTR void DiDiagonalLeftLine_paint(void* this_ptr, const DiPaintParams *params);
// }
//
// Upon entry to the function, registers are set as follows:
//
// a0: return address
// a1: stack pointer
// a2: DiDiagonalLeftLine* pointer (i.e., 'this')
//     [a2] vtable pointer
//     [a2+4] m_x
//     [a2+8] m_y
//     [a2+12] m_width
//     [a2+16] m_x_extent
//     [a2+20] m_height
//     [a2+24] m_y_extent
//     [a2+28] m_color
//
// a3: const DiPaintParams *params
//     [a3] m_line32
//     [a3+4] m_line8
//     [a3+8] m_line_index
//     [a3+12] m_scrolled_y
//     [a3+16] m_horiz_scroll
//     [a3+20] m_vert_scroll
//     [a3+24] m_screen_width
//     [a3+28] m_screen_height
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

  .section .iram1.text
  .align
  .global   DiDiagonalLeftLine_paint
  .type     DiDiagonalLeftLine_paint,@function

DiDiagonalLeftLine_paint:
  entry     sp,16
  l32i      a4,a3,12      // a4 <-- m_scrolled_y
  l32i      a5,a2,8       // a5 <-- m_y
  blt       a4,a5,skip    // skip if scan line is above diagonal line
  l32i      a6,a2,24      // a6 <-- m_y_extent
  bge       a4,a6,skip    // skip if scan line is below diagonal line
  sub       a7,a4,a5      // a7 <-- m_scrolled_y - m_y
  l32i      a4,a3,4       // a4 <-- m_line8
  l32i      a5,a2,4       // a5 <-- m_x
  l32i      a6,a3,16      // a6 <-- m_horiz_scroll
  add       a5,a5,a6      // a5 <-- m_x + m_horiz_scroll
  sub       a5,a5,a7      // a5 <-- (m_x + m_horiz_scroll) - (m_scrolled_y - m_y)
  bltz      a5,skip       // skip if final x < 0
  l32i      a6,a3,24      // a6 <-- m_screen_width
  bge       a5,a6,skip    // skip if final x >= m_screen_width
  movi      a6,2          // value for adjusting index
  xor       a5,a5,a6      // a5 <-- final x ^ 2
  add       a4,a4,a5      // a4 <-- m_line8 + m_x
  l8ui      a5,a2,28      // a5 <-- m_color
  s8i       a5,a4,0       // m_line8[index] <-- m_color
skip:
  retw.n
