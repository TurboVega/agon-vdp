// di_tile_map_asm.S - Low-level assembler function for drawing tile maps 
//
// A tile map is a set of rectangular tiles, where each tile is a bitmap of
// the same size (width and height). Tiles are arranged in a rectangular
// grid, where the entire portion of the grid that fits within the visible
// area of the screen may be displayed at any given moment. In other words
// multiple tiles show at the same time.
//
// The tile map may be scrolled in any of 8 directions, by setting m_x and m_y
// to the scroll distances on the 2 axes. Just be careful not to scroll too far,
// such that the visible screen would contain pixels not in the logical map.
//
// Copyright (c) 2023 Curtis Whitley
// 
// The function defined in this source may be specified
// in a C source that calls it, using this declaration:
//
// extern "C" {
// IRAM_ATTR void DiTileMap_paint(void* this_ptr, const DiPaintParams *params);
// }
//
// Upon entry to the function, registers are set as follows:
//
// a0: return address
// a1: stack pointer
// a2: DiTileMap* pointer (i.e., 'this')
//     [a2] vtable pointer
//     [a2+4] m_x
//     [a2+8] m_y
//     [a2+12] m_width
//     [a2+16] m_x_extent
//     [a2+20] m_height
//     [a2+24] m_y_extent
//     [a2+28] m_bitmaps
//     [a2+32] m_columns
//     [a2+36] m_rows
//     [a2+40] m_words_per_line
//     [a2+44] m_bytes_per_line
//     [a2+48] m_words_per_bitmap
//     [a2+52] m_bytes_per_bitmap
//     [a2+56] m_words_for_bitmaps
//     [a2+60] m_bytes_for_bitmaps
//     [a2+64] m_words_per_row
//     [a2+68] m_bytes_per_row
//     [a2+72] m_words_for_tiles
//     [a2+76] m_bytes_for_tiles
//     [a2+80] m_words_for_offsets
//     [a2+84] m_bytes_for_offsets
//     [a2+88] m_visible_columns
//     [a2+92] m_visible_rows
//     [a2+96] m_words_per_position
//     [a2+100] m_bytes_per_position
//     [a2+104] m_draw_words_per_line
//     [a2+108] m_tiles
//     [a2+112] m_pixels
//     [a2+116] m_offsets
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
  .global   DiTileMap_paint
  .type     DiTileMap_paint,@function

DiTileMap_paint:
  entry     sp,16
  l32i      a4,a3,12    // a4 <-- m_scrolled_y
  l32i      a5,a2,8     // a5 <-- m_y
  add       a4,a5,a4    // a4 <-- m_y + m_scrolled_y <-- index
  slli      a4,a4,3     // a4 <-- (index * 2) * 4
  l32i      a5,a2,116   // a5 <-- m_offsets
  add       a4,a4,a5    // a4 <-- m_offsets + index * 8, i.e., &m_offsets[index * 2]
  l32i      a5,a4,4     // a5 <-- m_offsets[index * 2 + 1], i.e., offset to bitmap line
  l32i      a4,a4,0     // a4 <-- m_offsets[index * 2], i.e.,  points to tile map row
  l32i      a6,a2,88    // a6 <-- m_visible_columns
  movi      a11,1       // a11 <-- count for decrements
  l32i      a8,a3,0     // a8 <-- m_line32

  l32i      a12,a2,4          // a12 <-- m_x
  l32i      a14,a2,12         // a14 <-- m_width
  blt       a12,a14,within_tile // go if x is within tile width
adjust_width:
  sub       a12,a12,a14       // a12 <-- x reduced by tile width
  addi      a4,a4,4           // a4 <-- advance tile map pointer by 1 column
  bge       a12,a14,adjust_width // go if x is at or beyond tile width
within_tile:
  beqz      a12,no_extra_col  // go if not showing a partial tile column
  addi      a6,a6,1           // a6 <-- display an extra tile column
no_extra_col:

  movi      a14,0             // a14 <-- assume offset based on position is zero
  l32i      a13,a2,100        // a13 <-- m_bytes_per_position
  bbci      a12,1,not_2_or_3  // go if lower 2 bits less than 2
  bbsi      a12,0,x_off_3     // go if lower 2 bits equal 3
  slli      a14,a13,1         // a14 <-- bitmap start + 2 more position
  bbci      a12,0,inc_done
not_2_or_3:
  bbci      a12,0,inc_done    // go if lower bit is zero
  slli      a14,a13,1         // a14 <-- bitmap start + 2 more positions
x_off_3:
  add       a14,a14,a13       // a14 <-- bitmap start + 1 more position
inc_done:

  movi      a13,0xFFFFFFFC    // a13 <-- mask to clear lower 2 bits
  and       a3,a12,a13        // a3 <-- adjustment to offset to bitmap data word, based on m_x
  l32i      a7,a4,0           // a7 <-- points to bitmap for current tile
  add       a7,a7,a3          // a7 <-- adjusted for top bits of m_x byte offset

  l32i      a10,a2,104        // a10 <-- m_draw_words_per_line
  srli      a3,a12,2          // a3 <-- (reduced x) / 4
  sub       a10,a10,a3        // a10 <-- adjusted count of words in bitmap line

  add       a5,a5,a14         // a5 <-- combine offsets to src pixels
  movi      a14,0             // a14 <-- don't skip one output word
  bbsi      a12,0,do_inc      // go if not at position 0
  bbci      a12,1,dont_inc    // go if not at position 0
do_inc:
  addi      a5,a5,4           // a5 <-- skips 1 src data word
  movi      a14,4             // a14 <-- do skip one output word
  sub       a10,a10,a11       // a10 <-- adjusted count of words in bitmap line
dont_inc:
  movi      a3,0              // a3 <-- tail end of prior tile's bitmap data

do_tiles:
  add       a7,a7,a5          // a7 <-- points to bitmap line within current tile

// For speed in the loop, we remove the need to clear a3 on every loop,
// by unrolling the first iteration into separate code.

  beqz      a10,end_loop      // skip loop if nothing to do
  l32i      a9,a7,0           // a9 <-- 4 src pixels
  or        a9,a9,a3          // a9 <-- src pixels across edge of 2 tiles
  s32i      a9,a8,0           // m_line32[] <-- src pixel data
  addi      a7,a7,4           // a7 <-- points to next src pixel
  addi      a8,a8,4           // a8 <-- points to next dst pixel
  sub       a10,a10,a11       // a10 <-- first iteration is done

// Here we have the remaining iterations, without using a3.

  loopnez   a10,end_loop      // loop to copy pixel data
  l32i      a9,a7,0           // a9 <-- 4 src pixels
  s32i      a9,a8,0           // m_line32[] <-- src pixel data
  addi      a7,a7,4           // a7 <-- points to next src pixel
  addi      a8,a8,4           // a8 <-- points to next dst pixel
end_loop:

  l32i      a3,a7,0           // a3 <-- tail-end src pixels
  l32i      a10,a2,104        // a10 <-- m_draw_words_per_line
  addi      a4,a4,4           // a4 <-- points to next bitmap address
  l32i      a7,a4,0           // a7 <-- points to bitmap for current tile
  sub       a6,a6,a11         // a6 <-- remaining number of columns
  sub       a5,a5,a14         // a5 <-- return extra adjustment to zero
  movi      a14,0             // a14 <-- don't need to make correction next time
  bnez      a6,do_tiles       // go back if more tiles to do

  retw.n
