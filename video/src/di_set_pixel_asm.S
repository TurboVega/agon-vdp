// di_set_pixel_asm.S - Low-level assembler function for setting individual pixels
//
// A pixel is the smallest visible dot on the screen.
//
// Copyright (c) 2023 Curtis Whitley
// 
// The function defined in this source may be specified
// in a C source that calls it, using this declaration:
//
// extern "C" {
// IRAM_ATTR void DiSetPixel_paint(void* this_ptr, const DiPaintParams *params);
// }
//
// Upon entry to the function, registers are set as follows:
//
// a0: return address
// a1: stack pointer
// a2: DiSetPixel* pointer (i.e., 'this')
//     [a2] vtable pointer
//     [a2+4] DiPrimitive base class data members
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
  .global   DiSetPixel_paint
  .type     DiSetPixel_paint,@function

DiSetPixel_paint:
  entry     sp,16
  l32i      a4,a3,m_line_index    // a4 <-- m_line_index
  l32i      a5,a2,m_draw_y        // a5 <-- m_draw_y
  bne       a4,a5,skip            // skip if this is not line for pixel
  l32i      a5,a2,m_abs_x         // a5 <-- m_abs_x
  l32i      a6,a2,m_draw_x        // a6 <-- m_draw_x
  blt       a5,a6,skip            // skip if m_abs_x < m_draw_x
  l32i      a6,a2,m_draw_x_extent // a6 <-- m_draw_x_extent
  bge       a5,a6,skip            // skip if m_abs_x >= m_draw_x_extent
  movi      a6,2                  // value for adjusting index
  xor       a5,a5,a6              // a5 <-- m_abs_x ^ 2
  l32i      a4,a3,m_line8         // a4 <-- m_line8
  add       a4,a4,a5              // a4 <-- m_line8 + m_abs_x
  l8ui      a5,a2,m_color         // a5 <-- m_color
  s8i       a5,a4,0               // m_line8[index] <-- m_color
skip:
  retw.n
