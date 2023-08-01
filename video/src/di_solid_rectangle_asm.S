// di_solid_rectangle.S - Function definitions for drawing solid rectangles
//
// A solid rectangle is filled with a single color.
//
// Copyright (c) 2023 Curtis Whitley
// 
// The function defined in this source may be specified
// in a C source that calls it, using this declaration:
//
// extern "C" {
// IRAM_ATTR void DiSolidRectangle(void* this_ptr, const DiPaintParams *params);
// }
//
// Upon entry to the function, registers are set as follows:
//
// a0: return address
// a1: stack pointer
// a2: DiSolidRectangle* pointer (i.e., 'this')
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
  .global   DiSolidRectangle_paint
  .type     DiSolidRectangle_paint,@function

DiSolidRectangle_paint:
  entry     sp,16
  l32i      a4,a3,12      // a4 <-- m_scrolled_y
  l32i      a5,a2,8       // a5 <-- m_y
  blt       a4,a5,skip    // skip if scan line is above rectangle
  l32i      a6,a2,24      // a6 <-- m_y_extent
  bge       a4,a6,skip    // skip if scan line is below rectangle
  l32i      a4,a3,4       // a4 <-- m_line8
  l32i      a5,a2,4       // a5 <-- m_x
  l32i      a6,a3,16      // a6 <-- m_horiz_scroll
  add       a5,a5,a6      // a5 (left_hand_x) <-- m_x + m_horiz_scroll
  l32i      a7,a3,24      // a7 <-- m_screen_width
  bge       a5,a7,skip    // skip if left_hand_x >= m_screen_width
  bgez      a5,left_ok    // don't shorten line if left_hand_x >= 0
  xor       a5,a5,a5      // a5 (left_hand_x) <-- 0
left_ok:
  l32i      a8,a2,16      // a8 <-- m_x_extent  
  add       a8,a8,a6      // a8 (right_hand_x) <-- m_x_extent + m_horiz_scroll
  beqz      a8,skip       // skip if right_hand_x == 0
  bltz      a8,skip       // skip if right_hand_x < 0
  blt       a8,a7,right_ok  // don't shorten line if right_hand_x < m_screen_width
  mov       a8,a7         // a8 (right_hand_x) <-- m_screen_width
right_ok:
  sub       a3,a8,a5      // a3 <-- final line width
  beqz      a3,skip       // skip if nothing to draw
  bltz      a3,skip       // skip if nothing to draw
  l32i      a2,a2,28      // a2 <-- m_color (4 bytes of same color)

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
