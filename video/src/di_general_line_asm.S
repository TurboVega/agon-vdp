// di_general_line_asm.S - Low-level assembler function for drawing general lines
//
// A general line is 1 pixel thick, and connects any 2 points, except that
// it should not be used for vertical, horizontal, or precisely diagonal
// lines, because there are other optimized classes for those cases.
//
// Copyright (c) 2023 Curtis Whitley
// 
// The function defined in this source may be specified
// in a C source that calls it, using this declaration:
//
// extern "C" {
// IRAM_ATTR void DiGeneralLine_paint(void* this_ptr, const DiPaintParams *params);
// }
//
// Upon entry to the function, registers are set as follows:
//
// a0: return address
// a1: stack pointer
// a2: DiGeneralLine* pointer (i.e., 'this')
//     [a2] vtable pointer
//     [a2 + 4] DiPrimitive base class data members
//     [a2 + sizeof_DiPrimitive + 0] m_line_pieces.m_pieces
//               [0] m_abs_x
//               [2] m_abs_y
//               [4] m_width
//               [6] m_flags
//     [a2 + sizeof_DiPrimitive + 4] m_line_pieces.m_min_x
//     [a2 + sizeof_DiPrimitive + 6] m_line_pieces.m_min_y
//     [a2 + sizeof_DiPrimitive + 8] m_line_pieces.m_max_x
//     [a2 + sizeof_DiPrimitive + 10] m_line_pieces.m_max_y
//     [a2 + sizeof_DiPrimitive + 12] m_line_pieces.m_num_pieces
//
// a3: const DiPaintParams *params
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

  .include "src/di_primitive_const.INC"
  .section .iram1.text
  .align
  .global   DiGeneralLine_paint
  .type     DiGeneralLine_paint,@function

DiGeneralLine_paint:
  entry     sp,16
  l32i      a4,a3,m_line_index // a4 <-- m_line_index
  l32i      a5,a2,m_draw_y // a5 <-- m_draw_y
  l32i      a6,a2,m_draw_y_extent // a6 <-- m_draw_y_extent
  l32i      a12,a2,(sizeof_DiPrimitive+0) // a12 <-- m_line_pieces.m_pieces

  blt       a4,a5,skip    // skip if scan line is above this line
  bge       a4,a6,skip    // skip if scan line is below this line

  l32i      a5,a2,m_abs_y // a5 <-- m_abs_y 
  sub       a4,a4,a5      // a4 <-- y offset from top of this line
  slli      a4,a4,3       // a4 <-- array offset to proper DiLinePiece
  add       a12,a12,a4    // a12 <-- pointer to proper DiLinePiece

  l32i      a4,a3,m_line8 // a4 <-- m_line8
  l16ui     a5,a12,0      // a5 <-- x
  l16ui     a8,a12,4      // a8 <-- width
  add       a8,a8,a5      // a8 <-- x + width (current x extent)
  l32i      a9,a2,m_draw_x // a9 <-- m_draw_x
  l32i      a7,a2,m_draw_x_extent // a7 <-- m_draw_x_extent
  bge       a5,a7,skip    // skip if left_hand_x >= m_draw_x_extent
  bge       a5,a9,left_ok // don't shorten line if left_hand_x >= m_draw_x
  mov       a5,a9         // a5 (left_hand_x) <-- m_draw_x
left_ok:
  beq       a8,a9,skip    // skip if right_hand_x == m_draw_x
  blt       a8,a9,skip    // skip if right_hand_x < m_draw_x
  blt       a8,a7,right_ok  // don't shorten line if right_hand_x < m_draw_x_extent
  mov       a8,a7         // a8 (right_hand_x) <-- m_draw_x_extent
right_ok:
  sub       a3,a8,a5      // a3 <-- final line width
  beqz      a3,skip       // skip if nothing to draw
  bltz      a3,skip       // skip if nothing to draw
  l32i      a2,a2,m_color // a2 <-- m_color (4 bytes of same color)

  movi      a6,3          // a6 <-- mask for checking byte alignment
  movi      a8,2          // value for adjusting index
  movi      a10,1         // used to decrement width by 1
  movi      a11,4         // used to decrement width by 4

align_left_edge:
  beqz      a3,skip       // skip if no pixels left to draw
  and       a7,a5,a6      // a7 <-- x & 3
  beqz      a7,left_aligned // don't align if already aligned
  xor       a9,a5,a8      // a9 <-- x ^ 2
  add       a9,a9,a4      // a9 <-- m_line8 + x
  s8i       a2,a9,0       // m_line8[index] <-- m_color
  sub       a3,a3,a10     // a3 <-- remaining line width
  add       a5,a5,a10     // a5 <-- next x position
  bgez      a5,align_left_edge // go check alignment
left_aligned:

  blt       a3,a11,middle_done // go if no full words to set
do_middle:
  add       a9,a5,a4      // a9 <-- m_line8 + x
  s32i      a2,a9,0       // m_line32[index] <-- m_color (4 pixels)
  sub       a3,a3,a11     // a3 <-- remaining line width
  add       a5,a5,a11     // a5 <-- next x position
  bge       a3,a11,do_middle // go if more full words to set
middle_done:

right_edge:
  beqz      a3,skip       // skip if no pixels left to draw
  xor       a9,a5,a8      // a9 <-- x ^ 2
  add       a9,a9,a4      // a9 <-- m_line8 + x
  s8i       a2,a9,0       // m_line8[index] <-- m_color
  sub       a3,a3,a10     // a3 <-- remaining line width
  add       a5,a5,a10     // a5 <-- next x position
  bgez      a5,right_edge // go finish drawing
skip:
  retw.n
