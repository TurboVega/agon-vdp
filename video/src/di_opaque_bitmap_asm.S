// di_opaque_bitmap_asm.S - Low-level assembler function for drawing opaque bitmaps 
//
// An opaque bitmap is a rectangle of fully opaque pixels of various colors.
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
//
// The function defined in this source may be specified
// in a C source that calls it, using this declaration:
//
// extern "C" {
// IRAM_ATTR void DiOpaqueBitmap_paint(void* this_ptr, const DiPaintParams *params);
// }
//
// Upon entry to the function, registers are set as follows:
//
// a0: return address
// a1: stack pointer
// a2: DiOpaqueBitmap* pointer (i.e., 'this')
//     [a2] vtable pointer
//     [a2+4] DiPrimitive base class data members
//     [a2 + sizeof_DiPrimitive + 0] m_words_per_line
//     [a2 + sizeof_DiPrimitive + 4] m_bytes_per_line
//     [a2 + sizeof_DiPrimitive + 8] m_words_per_position;
//     [a2 + sizeof_DiPrimitive + 12] m_bytes_per_position;
//     [a2 + sizeof_DiPrimitive + 16] m_visible_start
//     [a2 + sizeof_DiPrimitive + 20] m_scroll_mode
//     [a2 + sizeof_DiPrimitive + 24] m_pixels
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
  .global   DiOpaqueBitmap_paint
  .type     DiOpaqueBitmap_paint,@function

// Assume that the logical pixels in one line of the original source bitmap are
// arranged like this:
//
// [Start pixels] [Middle pixels] [End pixels]
//   0  1  2  3      4  5  6  7     8  9 10 11
//   a  b  c  d      m  n  o  p     w  x  y  z
//
// This means that the physical pixels (data bytes) in the stored bitmap are
// arranged like this, because of how the I2S hardware expects to see them
// for DMA purposes:
//
// [Start pixels] [Middle pixels] [End pixels]
//   0  1  2  3      4  5  6  7     8  9 10 11
//   c  d  a  b      o  p  m  n     y  z  w  x
//
// Contained in the m_pixels array are 4 copies of the bitmap pixel data.
// Each copy has its pixels horizontally offset by a different number of
// pixels (i.e., by a different number of bytes, since each byte contains
// a single pixel). The 4 positions of pixel data in the array are:
//
// Position A: pixels are at offset 0, and not not shifted to the right or to the left.
// Position B: pixels are at offset 1, being shifted to the right by 1 pixel, which is
//             similar to being shifted to the left by 3 pixels.
// Position C: pixels are at offset 2, being shifted to the right by 2 pixels, which is
//             similar to being shifted to the left by 2 pixels.
// Position D: pixels are at offset 3, being shifted to the right by 3 pixels, which is
//             similar to being shifted to the left by 1 pixel.
//
// When copying pixel data to the DMA scan line, both the source X position and
// the destination X position may be located at any of the 4 pixel offsets, from
// a word (4-byte) aligned boundary. The following table shows which copy of the
// pixel data to use when copying source pixels to destination pixels. The first
// arrangement in each pair is the logical arrangement, and the second arrangement
// in each pair is the physical arrangement.
//
// Dst offset-->   --- Dst offset 0 ----   ------ Dst offset 1 ------  ------ Dst offset 2 ------  ------ Dst offset 3 ------
//
// Src offset 0:   A:abcd.mnop.mnop.wxyz   B:-abc.dmno.pmno.pwxy.z---  C:--ab.cdmn.opmn.opwx.yz--  D:---a.bcdm.nopm.nopw.xzy-
//                 A:cdab.opmn.opmn.yzwx   B:bc-a.nodm.nopm.xypw.--z-  C:ab--.mncd.mnop.wxop.--yz  D:-a--.dmbc.pmno.pwno.y-xz
//
// Src offset 1:   D:bcdm.nopm.nopw.xyz-   A:-bcd.mnop.mnop.wxyz.----  B:--bc.dmno.pmno.pwxy.z---  C:---b.cdmn.opmn.opwx.yz--
//                 D:dmbc.pmno.pwno.z-xy   A:cd-b.opmn.opmn.yzwx.----  B:bc--.nodm.nopm.xypw.--z-  C:-b--.mncd.mnop.wxop.--yz
//
// Src offset 2:   C:cdmn.opmn.opwx.yz--   D:-cdm.nopm.nopw.xyz-.----  A:--cd.mnop.mnop.wxyz.----  B:---c.dmno.pmno.pwxy.z---
//                 C:mncd.mnop.wxop.--yz   D:dm-c.pmno.pwno.z-xy.----  A:cd--.opmn.opmn.yzwx.----  B:-c--.nodm.nopm.xypw.z---
//
// Src offset 3:   B:dmno.pmno.pwxy.z---   C:-dmn.opmn.opwx.yz--.----  D:--dm.nopm.nopw.xyz-.----  A:---d.mnop.mnop.wxyz.----
//                 B:nodm.nopm.xypw.--z-   C:mn-d.mnop.wxop.--yz.----  D:dm--.pmno.pwno.z-xy.----  A:-d--.opmn.opmn.yzwx.----
//

  .macro    DO_START_PIXELS mask

  l32i      a6,a4,0       // a6 <-- 4 dst pixels
  movi      a11,\mask     // a11 <-- mask for keeping dst pixels
  and       a11,a11,a6    // a11 <-- keepable dst pixels
  l32i      a6,a14,0      // a6 <-- 4 src pixels
  or        a6,a6,a11     // a6 <-- new combined pixels
  s32i      a6,a4,0       // m_line32[] <-- src pixel data
  add       a4,a4,a10     // a4 <-- points to next dst pixels
  add       a14,a14,a10   // a14 <-- pointer to next 4 source pixels

  .endm

  .macro    DO_END_PIXELS mask

  l32i      a6,a4,0       // a6 <-- 4 dst pixels
  movi      a11,\mask     // a11 <-- mask for keeping dst pixels
  and       a11,a11,a6    // a11 <-- keepable dst pixels
  l32i      a6,a14,0      // a6 <-- 4 src pixels
  or        a6,a6,a11     // a6 <-- new combined pixels
  s32i      a6,a4,0       // m_line32[] <-- src pixel data

  .endm

  .align    4
DiOpaqueBitmap_paint:
  entry     sp,16
  l32i      a4,a3,m_line_index // a4 <-- m_line_index
  l32i      a5,a2,m_draw_y // a5 <-- m_draw_y
  blt       a4,a5,skip2   // skip if scan line is above bitmap
  l32i      a6,a2,m_draw_y_extent // a6 <-- m_draw_y_extent
  bge       a4,a6,skip2   // skip if scan line is below bitmap
  l32i      a5,a2,m_abs_y // a5 <-- m_abs_y
  sub       a12,a4,a5     // a12 (y offset) <-- m_line_index - m_abs_y
  l32i      a4,a3,m_line8 // a4 <-- m_line8
  l32i      a5,a2,m_draw_x // a5 <-- m_draw_x
  l32i      a8,a2,m_draw_x_extent // a8 <-- m_draw_x_extent
  bge       a5,a8,skip2   // skip if left_hand_x >= m_draw_x_extent

  l32i      a7,a2,(sizeof_DiPrimitive + 16) // a7 <-- m_visible_start
  l32i      a3,a2,m_abs_x // a3 <-- m_abs_x
  bgez      a3,dont_move  // don't move start if left_hand_x >= 0
  bbsi      a3,0,move_start // go move start if left_hand_x is not a multiple of 4
  bbci      a3,1,dont_move // do not move start if left_hand_x is a multiple of 4
move_start:
  addi      a7,a7,4       // move start pixels address by one word
dont_move:

  sub       a3,a8,a5      // a3 <-- final line width
  beqz      a3,skip2      // skip if nothing to draw
  bltz      a3,skip2      // skip if nothing to draw

  l32i      a14,a2,(sizeof_DiPrimitive+4) // a14 <-- m_bytes_per_line
  mul16u    a14,a14,a12   // a14 <-- m_bytes_per_line * (m_line_index - m_abs_y)
  add       a14,a14,a7    // a14 <-- pointer to bitmap data line
  l32i      a11,a2,(sizeof_DiPrimitive+12) // a11 <-- m_bytes_per_position
  movi      a7,1          // a7 <-- value of 1 for adjusting
  movi      a8,2          // a8 <-- value of 2 for adjusting
  movi      a9,3          // a9 <-- value of 3 for adjusting
  movi      a10,4         // a10 <-- value of 4 for adjusting
  mov       a15,a0        // a15 <-- save return address

  movi      a2,0xFFFFFFFC // a2 <-- mask to remove lower 2 bits
  and       a6,a2,a13     // a6 <-- offset to word-aligned src pixels
  add       a14,a14,a6    // a14 <-- points to word-aligned src pixels
  and       a6,a2,a5      // a6 <-- offset to word-aligned dst pixels
  add       a4,a4,a6      // a4 <-- points to word-aligned dst pixels

//.......................................
// Determine the alignment of the source and destination pixels.
// a13 <-- offset to first visible src pixel

  bbsi      a13,1,src_offset_2_or_3     // go if first visible pixel is at offset 2 or 3
  bbsi      a13,0,src_offset_1          // go if first visible pixel is at offset 1
  bbci      a13,0,src_offset_0          // go, because first visible pixel is at offset 0
src_offset_2_or_3:
  bbsi      a13,0,src_offset_3          // go if first visible pixel is at offset 3
  bbci      a13,0,src_offset_2          // go, because first visible pixel is at offset 2

skip2:
  retw

// At this point:
// a3 <-- final line width
// a4 <-- m_line8 (or m_line32) + offset to word-aligned src pixels
// a5 <-- final start x
// a13 <-- offset to first visible src pixel
// a14 <-- pointer to bitmap data line

//.......................................

src_offset_0:
  bbsi     a5,1,so_0_dst_offset_2_or_3  // go if first destination pixel is at offset 2 or 3
  bbsi     a5,0,so_0_dst_offset_1       // go if first destination pixel is at offset 1

// Copy source pixels from offset 0 to destination pixels at offset 0.
so_0_dst_offset_0:
  // Logical  A:abcd.mnop.mnop.wxyz
  // Physical A:cdab.opmn.opmn.yzwx
  // a14 <-- pointer to source pixels (position A)
  call0    do_middle_pixels
  mov      a0,a15
  retw

// Copy source pixels from offset 0 to destination pixels at offset 1.
so_0_dst_offset_1:
  // Logical  B:-abc.dmno.pmno.pwxy.z---
  // Physical B:bc-a.nodm.nopm.xypw.--z-
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  DO_START_PIXELS 0x00FF0000
  //addi     a5,a5,3
  sub      a3,a3,a9
  call0    do_middle_pixels
  DO_END_PIXELS 0xFF00FFFF
  mov      a0,a15
  retw

so_0_dst_offset_2_or_3:
  bbsi     a5,0,so_0_dst_offset_3       // go if first destination pixel is at offset 3

// Copy source pixels from offset 0 to destination pixels at offset 2.
so_0_dst_offset_2:
  // Logical  C:--ab.cdmn.opmn.opwx.yz--
  // Physical C:ab--.mncd.mnop.wxop.--yz
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  add      a14,a14,a11                  // (position C)
  DO_START_PIXELS 0xFFFF0000
  //addi     a5,a5,2
  sub      a3,a3,a8
  call0    do_middle_pixels
  DO_END_PIXELS 0x0000FFFF
  mov      a0,a15
  retw

// Copy source pixels from offset 0 to destination pixels at offset 3.
so_0_dst_offset_3:
  // Logical  D:---a.bcdm.nopm.nopw.xzy-
  // Physical D:-a--.dmbc.pmno.pwno.y-xz
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  add      a14,a14,a11                  // (position C)
  add      a14,a14,a11                  // (position D)
  DO_START_PIXELS 0xFFFF00FF
  //addi     a5,a5,1
  sub      a3,a3,a7
  call0    do_middle_pixels
  DO_END_PIXELS 0x0000FF00
  mov      a0,a15
  retw

//.......................................

src_offset_1:
  bbsi     a5,1,so_1_dst_offset_2_or_3  // go if first destination pixel is at offset 2 or 3
  bbsi     a5,0,so_1_dst_offset_1       // go if first destination pixel is at offset 1

// Copy source pixels from offset 1 to destination pixels at offset 0.
so_1_dst_offset_0:
  // Logical  D:bcdm.nopm.nopw.xyz-
  // Physical D:dmbc.pmno.pwno.z-xy
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  add      a14,a14,a11                  // (position C)
  add      a14,a14,a11                  // (position D)
  call0    do_middle_pixels
  DO_END_PIXELS 0x0000FF00
  mov      a0,a15
  retw

// Copy source pixels from offset 1 to destination pixels at offset 1.
so_1_dst_offset_1:
  // Logical  A:-bcd.mnop.mnop.wxyz.----
  // Physical A:cd-b.opmn.opmn.yzwx.----
  // a14 <-- pointer to source pixels (position A)
  DO_START_PIXELS 0x00FF0000
  //addi     a5,a5,3
  sub      a3,a3,a9
  call0    do_middle_pixels
  mov      a0,a15
  retw

so_1_dst_offset_2_or_3:
  bbsi     a5,0,so_1_dst_offset_3     // go if first destination pixel is at offset 3

// Copy source pixels from offset 1 to destination pixels at offset 2.
so_1_dst_offset_2:
  // Logical  B:--bc.dmno.pmno.pwxy.z---
  // Physical B:bc--.nodm.nopm.xypw.--z-
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  DO_START_PIXELS 0xFFFF0000
  //addi     a5,a5,2
  sub      a3,a3,a8
  call0    do_middle_pixels
  DO_END_PIXELS 0xFF00FFFF
  mov      a0,a15
  retw

// Copy source pixels from offset 1 to destination pixels at offset 3.
so_1_dst_offset_3:
  // Logical  C:---b.cdmn.opmn.opwx.yz--
  // Physical C:-b--.mncd.mnop.wxop.--yz
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  add      a14,a14,a11                  // (position C)
  DO_START_PIXELS 0xFFFF00FF
  //addi     a5,a5,1
  sub      a3,a3,a7
  call0    do_middle_pixels
  DO_END_PIXELS 0x0000FFFF
  mov      a0,a15
  retw

//.......................................

src_offset_2:
  bbsi     a5,1,so_2_dst_offset_2_or_3  // go if first destination pixel is at offset 2 or 3
  bbsi     a5,0,so_2_dst_offset_1       // go if first destination pixel is at offset 1

// Copy source pixels from offset 2 to destination pixels at offset 0.
so_2_dst_offset_0:
  // Logical  C:cdmn.opmn.opwx.yz--
  // Physical C:mncd.mnop.wxop.--yz
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  add      a14,a14,a11                  // (position C)
  call0    do_middle_pixels
  DO_END_PIXELS 0x0000FFFF
  mov      a0,a15
  retw

// Copy source pixels from offset 2 to destination pixels at offset 1.
so_2_dst_offset_1:
  // Logical  D:-cdm.nopm.nopw.xyz-.----
  // Physical D:dm-c.pmno.pwno.z-xy.----
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  add      a14,a14,a11                  // (position C)
  add      a14,a14,a11                  // (position D)
  DO_START_PIXELS 0x00FF0000
  //addi     a5,a5,3
  sub      a3,a3,a9
  call0    do_middle_pixels
  DO_END_PIXELS 0x0000FF00
  mov      a0,a15
  retw

so_2_dst_offset_2_or_3:
  bbsi     a5,0,so_2_dst_offset_3       // go if first destination pixel is at offset 3

// Copy source pixels from offset 2 to destination pixels at offset 2.
so_2_dst_offset_2:
  // Logical  A:--cd.mnop.mnop.wxyz.----
  // Physical A:cd--.opmn.opmn.yzwx.----
  // a14 <-- pointer to source pixels (position A)
  DO_START_PIXELS 0xFFFF0000
  //addi     a5,a5,2
  sub      a3,a3,a8
  call0    do_middle_pixels
  mov      a0,a15
  retw

// Copy source pixels from offset 2 to destination pixels at offset 3.
so_2_dst_offset_3:
  // Logical  B:---c.dmno.pmno.pwxy.z---
  // Physical B:-c--.nodm.nopm.xypw.z---
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  DO_START_PIXELS 0xFFFF00FF
  //addi     a5,a5,1
  sub      a3,a3,a7
  call0    do_middle_pixels
  DO_END_PIXELS 0xFFFFFF00
  mov      a0,a15
  retw

//.......................................

src_offset_3:
  bbsi     a5,1,so_3_dst_offset_2_or_3  // go if first destination pixel is at offset 2 or 3
  bbsi     a5,0,so_3_dst_offset_1       // go if first destination pixel is at offset 1

// Copy source pixels from offset 3 to destination pixels at offset 0.
so_3_dst_offset_0:
  // Logical  B:dmno.pmno.pwxy.z---
  // Physical B:nodm.nopm.xypw.--z-
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  //sub      a5,a5,a7
  call0    do_middle_pixels
  DO_END_PIXELS 0xFF00FFFF
  mov      a0,a15
  retw

// Copy source pixels from offset 3 to destination pixels at offset 1.
so_3_dst_offset_1:
  // Logical  C:-dmn.opmn.opwx.yz--.----
  // Physical C:mn-d.mnop.wxop.--yz.----
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  add      a14,a14,a11                  // (position C)
  DO_START_PIXELS 0x00FF0000
  //addi     a5,a5,3
  sub      a3,a3,a9
  call0    do_middle_pixels
  DO_END_PIXELS 0x0000FFFF
  mov      a0,a15
  retw

so_3_dst_offset_2_or_3:
  bbsi     a5,0,so_3_dst_offset_3       // go if first destination pixel is at offset 3

// Copy source pixels from offset 3 to destination pixels at offset 2.
so_3_dst_offset_2:
  // Logical  D:--dm.nopm.nopw.xyz-.----
  // Physical D:dm--.pmno.pwno.z-xy.----
  add      a14,a14,a11                  // a14 <-- pointer to source pixels (position B)
  add      a14,a14,a11                  // (position C)
  add      a14,a14,a11                  // (position D)
  DO_START_PIXELS 0xFFFF0000
  //addi     a5,a5,2
  sub      a3,a3,a8
  call0    do_middle_pixels
  DO_END_PIXELS 0x0000FF00
  mov      a0,a15
  retw

// Copy source pixels from offset 3 to destination pixels at offset 3.
so_3_dst_offset_3:
  // Logical  A:---d.mnop.mnop.wxyz.----
  // Physical A:-d--.opmn.opmn.yzwx.----
  // a14 <-- pointer to source pixels (position A)
  DO_START_PIXELS 0xFFFF00FF
  //addi     a5,a5,1
  sub      a3,a3,a7
  call0    do_middle_pixels
  mov      a0,a15
  retw

//.......................................
// a4 <-- m_line8 (or m_line32)
// a5 <-- final start x
// a13 <-- offset to first visible source pixel
// a14 <-- pointer to bitmap data line

do_middle_pixels:
  srli      a11,a3,2      // a11 <-- number of full words left to copy
  loopnez   a11,end_loop  // loop to copy pixel data
  l32i      a9,a14,0      // a9 <-- 4 src pixels
  s32i      a9,a4,0       // m_line32[] <-- src pixel data
  add       a4,a4,a10     // a4 <-- points to next 4 dst pixels
  add       a14,a14,a10   // a14 <-- points to next 4 src pixels
end_loop:
  ret
