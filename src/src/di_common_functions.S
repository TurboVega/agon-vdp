// di_common_functions.S - Low-level assembler functions for setting pixels
//
// Copyright (c) 2023 Curtis Whitley
// 
// The functions below are intended to be called using the CALL0 or CALLX0
// instructions, and are thus not intended to be called directly from C++,
// which would require using instructions such as CALL8.
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
  .align 4

  .global   fcn_draw_256_pixels_in_loop
  .global   fcn_draw_128_pixels
  .global   fcn_draw_128_pixels_last
  .global   fcn_draw_64_pixels
  .global   fcn_draw_64_pixels_last
  .global   fcn_draw_32_pixels
  .global   fcn_draw_32_pixels_last
  .global   fcn_draw_16_pixels
  .global   fcn_draw_16_pixels_last
  .global   fcn_draw_8_pixels
  .global   fcn_draw_8_pixels_last
  .global   fcn_get_blend_25_for_4_pixels
  .global   fcn_get_blend_50_for_4_pixels
  .global   fcn_get_blend_75_for_4_pixels
  .global   fcn_dummy

  .global   fcn_copy_256_pixels_in_loop
  .global   fcn_copy_128_pixels
  .global   fcn_copy_128_pixels_last
  .global   fcn_copy_64_pixels
  .global   fcn_copy_64_pixels_last
  .global   fcn_copy_32_pixels
  .global   fcn_copy_32_pixels_last
  .global   fcn_copy_16_pixels
  .global   fcn_copy_16_pixels_last
  .global   fcn_copy_8_pixels
  .global   fcn_copy_8_pixels_last

  .global   fcn_blend_25_for_256_pixels_in_loop
  .global   fcn_blend_25_for_128_pixels
  .global   fcn_blend_25_for_128_pixels_last
  .global   fcn_blend_25_for_64_pixels
  .global   fcn_blend_25_for_64_pixels_last
  .global   fcn_blend_25_for_32_pixels
  .global   fcn_blend_25_for_32_pixels_last
  .global   fcn_blend_25_for_16_pixels
  .global   fcn_blend_25_for_16_pixels_last
  .global   fcn_blend_25_for_8_pixels
  .global   fcn_blend_25_for_8_pixels_last
  .global   fcn_blend_25_for_1_pixel_at_offset_0
  .global   fcn_blend_25_for_1_pixel_at_offset_0_last
  .global   fcn_blend_25_for_1_pixel_at_offset_1
  .global   fcn_blend_25_for_1_pixel_at_offset_1_last
  .global   fcn_blend_25_for_1_pixel_at_offset_2
  .global   fcn_blend_25_for_1_pixel_at_offset_2_last
  .global   fcn_blend_25_for_1_pixel_at_offset_3
  .global   fcn_blend_25_for_1_pixel_at_offset_3_last
  .global   fcn_blend_25_for_2_pixels_at_offset_0
  .global   fcn_blend_25_for_2_pixels_at_offset_0_last
  .global   fcn_blend_25_for_2_pixels_at_offset_1
  .global   fcn_blend_25_for_2_pixels_at_offset_1_last
  .global   fcn_blend_25_for_2_pixels_at_offset_2
  .global   fcn_blend_25_for_2_pixels_at_offset_2_last
  .global   fcn_blend_25_for_3_pixels_at_offset_0
  .global   fcn_blend_25_for_3_pixels_at_offset_0_last
  .global   fcn_blend_25_for_3_pixels_at_offset_1
  .global   fcn_blend_25_for_3_pixels_at_offset_1_last
  .global   fcn_blend_25_for_4_pixels_at_offset_0
  .global   fcn_blend_25_for_4_pixels_at_offset_0_last

  .global   fcn_blend_50_for_256_pixels_in_loop
  .global   fcn_blend_50_for_128_pixels
  .global   fcn_blend_50_for_128_pixels_last
  .global   fcn_blend_50_for_64_pixels
  .global   fcn_blend_50_for_64_pixels_last
  .global   fcn_blend_50_for_32_pixels
  .global   fcn_blend_50_for_32_pixels_last
  .global   fcn_blend_50_for_16_pixels
  .global   fcn_blend_50_for_16_pixels_last
  .global   fcn_blend_50_for_8_pixels
  .global   fcn_blend_50_for_8_pixels_last
  .global   fcn_blend_50_for_1_pixel_at_offset_0
  .global   fcn_blend_50_for_1_pixel_at_offset_0_last
  .global   fcn_blend_50_for_1_pixel_at_offset_1
  .global   fcn_blend_50_for_1_pixel_at_offset_1_last
  .global   fcn_blend_50_for_1_pixel_at_offset_2
  .global   fcn_blend_50_for_1_pixel_at_offset_2_last
  .global   fcn_blend_50_for_1_pixel_at_offset_3
  .global   fcn_blend_50_for_1_pixel_at_offset_3_last
  .global   fcn_blend_50_for_2_pixels_at_offset_0
  .global   fcn_blend_50_for_2_pixels_at_offset_0_last
  .global   fcn_blend_50_for_2_pixels_at_offset_1
  .global   fcn_blend_50_for_2_pixels_at_offset_1_last
  .global   fcn_blend_50_for_2_pixels_at_offset_2
  .global   fcn_blend_50_for_2_pixels_at_offset_2_last
  .global   fcn_blend_50_for_3_pixels_at_offset_0
  .global   fcn_blend_50_for_3_pixels_at_offset_0_last
  .global   fcn_blend_50_for_3_pixels_at_offset_1
  .global   fcn_blend_50_for_3_pixels_at_offset_1_last
  .global   fcn_blend_50_for_4_pixels_at_offset_0
  .global   fcn_blend_50_for_4_pixels_at_offset_0_last

  .global   fcn_blend_75_for_256_pixels_in_loop
  .global   fcn_blend_75_for_128_pixels
  .global   fcn_blend_75_for_128_pixels_last
  .global   fcn_blend_75_for_64_pixels
  .global   fcn_blend_75_for_64_pixels_last
  .global   fcn_blend_75_for_32_pixels
  .global   fcn_blend_75_for_32_pixels_last
  .global   fcn_blend_75_for_16_pixels
  .global   fcn_blend_75_for_16_pixels_last
  .global   fcn_blend_75_for_8_pixels
  .global   fcn_blend_75_for_8_pixels_last
  .global   fcn_blend_75_for_1_pixel_at_offset_0
  .global   fcn_blend_75_for_1_pixel_at_offset_0_last
  .global   fcn_blend_75_for_1_pixel_at_offset_1
  .global   fcn_blend_75_for_1_pixel_at_offset_1_last
  .global   fcn_blend_75_for_1_pixel_at_offset_2
  .global   fcn_blend_75_for_1_pixel_at_offset_2_last
  .global   fcn_blend_75_for_1_pixel_at_offset_3
  .global   fcn_blend_75_for_1_pixel_at_offset_3_last
  .global   fcn_blend_75_for_2_pixels_at_offset_0
  .global   fcn_blend_75_for_2_pixels_at_offset_0_last
  .global   fcn_blend_75_for_2_pixels_at_offset_1
  .global   fcn_blend_75_for_2_pixels_at_offset_1_last
  .global   fcn_blend_75_for_2_pixels_at_offset_2
  .global   fcn_blend_75_for_2_pixels_at_offset_2_last
  .global   fcn_blend_75_for_3_pixels_at_offset_0
  .global   fcn_blend_75_for_3_pixels_at_offset_0_last
  .global   fcn_blend_75_for_3_pixels_at_offset_1
  .global   fcn_blend_75_for_3_pixels_at_offset_1_last
  .global   fcn_blend_75_for_4_pixels_at_offset_0
  .global   fcn_blend_75_for_4_pixels_at_offset_0_last

// Input registers:
#define REG_RETURN_ADDR     a0
#define REG_STACK_PTR       a1
#define REG_THIS_PTR        a2
#define REG_LINE_PTR        a3
#define REG_LINE_INDEX      a4
// Temporary registers:
#define REG_SAVE_RET_INNER  a2
#define REG_ABS_Y           a6
#define REG_DST_PIXEL_PTR   a5
#define REG_SRC_PIXEL_PTR   a6
#define REG_PIXEL_COLOR     a7
#define REG_LOOP_INDEX      a4
#define REG_SRC_PIXELS      a8
#define REG_SRC_BR_PIXELS   a9
#define REG_DST_BR_PIXELS   a10
#define REG_SRC_G_PIXELS    a8
#define REG_DST_G_PIXELS    a11
#define REG_DOUBLE_COLOR    a12
#define REG_ISOLATE_BR      a13     // 0x33333333: mask to isolate blue & red, removing green
#define REG_ISOLATE_G       a14     // 0x0C0C0C0C: mask to isolate green, removing red & blue
#define REG_JUMP_ADDRESS    a14
#define REG_SAVE_RET_OUTER  a15

#define FIX_OFFSET(off)    ((off)^2)

  .macro set_1_pixel_at_offset_0
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
  .endm

  .macro set_1_pixel_at_offset_1
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)
  .endm

  .macro set_1_pixel_at_offset_2
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro set_1_pixel_at_offset_3
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(3)
  .endm

  .macro set_2_pixels_at_offset_0
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
  .endm

  .macro set_2_pixels_at_offset_1
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro set_2_pixels_at_offset_2
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)    
  .endm

  .macro set_3_pixels_at_offset_0
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro set_3_pixels_at_offset_1
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)    
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro set_4_pixels_at_offset offset
    s32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, \offset
  .endm

  .macro copy_1_pixel_at_offset_0
    l8ui   REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(0)
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
  .endm

  .macro copy_1_pixel_at_offset_1
    l8ui   REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(1)
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)
  .endm

  .macro copy_1_pixel_at_offset_2
    l8ui   REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(2)
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro copy_1_pixel_at_offset_3
    l8ui   REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(3)
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(3)
  .endm

  .macro copy_2_pixels_at_offset_0
    l16ui   REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(0)
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
  .endm

  .macro copy_2_pixels_at_offset_1
    l16ui   REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(1)
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)
    l16ui   REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(2)
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro copy_2_pixels_at_offset_2
    l16ui   REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(2)    
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)    
  .endm

  .macro copy_3_pixels_at_offset_0
    l16ui   REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(0)
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
    l16ui   REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(2)
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro copy_3_pixels_at_offset_1
    l8ui   REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(1)    
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1) 
    l16ui   REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(2)
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro copy_4_pixels_at_offset word_offset
    l32i   REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, \word_offset
    s32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, \word_offset
  .endm

// In REG_PIXEL_COLOR
// 31..24 23..16 15..08 07..00
// pixel1 pixel0 pixel3 pixel2

  .macro blend_25_for_1_pixel_at_offset_0
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_25_for_4_pixels
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 16
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
  .endm

  .macro blend_25_for_1_pixel_at_offset_1
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_25_for_4_pixels
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 24
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)
  .endm

  .macro blend_25_for_1_pixel_at_offset_2
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_25_for_4_pixels
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro blend_25_for_1_pixel_at_offset_3
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_25_for_4_pixels
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 8
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(3)
  .endm

  .macro blend_25_for_2_pixels_at_offset_0
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_25_for_4_pixels
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 16
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
  .endm

  .macro blend_25_for_2_pixels_at_offset_1
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_25_for_4_pixels
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 24
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)
  .endm

  .macro blend_25_for_2_pixels_at_offset_2
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_25_for_4_pixels
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro blend_25_for_3_pixels_at_offset_0
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_25_for_4_pixels
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 16
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
  .endm

  .macro blend_25_for_3_pixels_at_offset_1
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_25_for_4_pixels
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 24
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)
  .endm

  .macro blend_25_for_4_pixels_at_offset wo
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, \wo
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, \wo
    call0  fcn_get_blend_25_for_4_pixels
    s32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, \wo
  .endm

// In REG_PIXEL_COLOR
// 31..24 23..16 15..08 07..00
// pixel1 pixel0 pixel3 pixel2

  .macro blend_50_for_1_pixel_at_offset_0
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_50_for_4_pixels
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 16
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
  .endm

  .macro blend_50_for_1_pixel_at_offset_1
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_50_for_4_pixels
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 24
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)
  .endm

  .macro blend_50_for_1_pixel_at_offset_2
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_50_for_4_pixels
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro blend_50_for_1_pixel_at_offset_3
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_50_for_4_pixels
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 8
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(3)
  .endm

  .macro blend_50_for_2_pixels_at_offset_0
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_50_for_4_pixels
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 16
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
  .endm

  .macro blend_50_for_2_pixels_at_offset_1
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_50_for_4_pixels
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 24
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)
  .endm

  .macro blend_50_for_2_pixels_at_offset_2
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_50_for_4_pixels
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro blend_50_for_3_pixels_at_offset_0
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_50_for_4_pixels
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 16
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
  .endm

  .macro blend_50_for_3_pixels_at_offset_1
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_50_for_4_pixels
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 24
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)
  .endm

  .macro blend_50_for_4_pixels_at_offset wo
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, \wo
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, \wo
    call0  fcn_get_blend_50_for_4_pixels
    s32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, \wo
  .endm

// In REG_PIXEL_COLOR
// 31..24 23..16 15..08 07..00
// pixel1 pixel0 pixel3 pixel2

  .macro blend_75_for_1_pixel_at_offset_0
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_75_for_4_pixels
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 16
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
  .endm

  .macro blend_75_for_1_pixel_at_offset_1
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_75_for_4_pixels
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 24
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)
  .endm

  .macro blend_75_for_1_pixel_at_offset_2
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_75_for_4_pixels
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro blend_75_for_1_pixel_at_offset_3
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_75_for_4_pixels
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 8
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(3)
  .endm

  .macro blend_75_for_2_pixels_at_offset_0
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_75_for_4_pixels
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 16
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
  .endm

  .macro blend_75_for_2_pixels_at_offset_1
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_75_for_4_pixels
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 24
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)
  .endm

  .macro blend_75_for_2_pixels_at_offset_2
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_75_for_4_pixels
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
  .endm

  .macro blend_75_for_3_pixels_at_offset_0
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_75_for_4_pixels
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 16
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0)
  .endm

  .macro blend_75_for_3_pixels_at_offset_1
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, 0
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0
    call0  fcn_get_blend_75_for_4_pixels
    s16i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2)
    srli  REG_PIXEL_COLOR, REG_PIXEL_COLOR, 24
    s8i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1)
  .endm

  .macro blend_75_for_4_pixels_at_offset wo
    l32i   REG_SRC_PIXELS, REG_SRC_PIXEL_PTR, \wo
    l32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, \wo
    call0  fcn_get_blend_75_for_4_pixels
    s32i   REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, \wo
  .endm


  .align 4
fcn_draw_256_pixels_in_loop:
begin_loop:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  set_4_pixels_at_offset 8
  set_4_pixels_at_offset 12
  set_4_pixels_at_offset 16
  set_4_pixels_at_offset 20
  set_4_pixels_at_offset 24
  set_4_pixels_at_offset 28
  set_4_pixels_at_offset 32
  set_4_pixels_at_offset 36
  set_4_pixels_at_offset 40
  set_4_pixels_at_offset 44
  set_4_pixels_at_offset 48
  set_4_pixels_at_offset 52
  set_4_pixels_at_offset 56
  set_4_pixels_at_offset 60
  set_4_pixels_at_offset 64
  set_4_pixels_at_offset 68
  set_4_pixels_at_offset 72
  set_4_pixels_at_offset 76
  set_4_pixels_at_offset 80
  set_4_pixels_at_offset 84
  set_4_pixels_at_offset 88
  set_4_pixels_at_offset 92
  set_4_pixels_at_offset 96
  set_4_pixels_at_offset 100
  set_4_pixels_at_offset 104
  set_4_pixels_at_offset 108
  set_4_pixels_at_offset 112
  set_4_pixels_at_offset 116
  set_4_pixels_at_offset 120
  set_4_pixels_at_offset 124

  set_4_pixels_at_offset 128
  set_4_pixels_at_offset 132
  set_4_pixels_at_offset 136
  set_4_pixels_at_offset 140
  set_4_pixels_at_offset 144
  set_4_pixels_at_offset 148
  set_4_pixels_at_offset 152
  set_4_pixels_at_offset 156
  set_4_pixels_at_offset 160
  set_4_pixels_at_offset 164
  set_4_pixels_at_offset 168
  set_4_pixels_at_offset 172
  set_4_pixels_at_offset 176
  set_4_pixels_at_offset 180
  set_4_pixels_at_offset 184
  set_4_pixels_at_offset 188
  set_4_pixels_at_offset 192
  set_4_pixels_at_offset 196
  set_4_pixels_at_offset 200
  set_4_pixels_at_offset 204
  set_4_pixels_at_offset 208
  set_4_pixels_at_offset 212
  set_4_pixels_at_offset 216
  set_4_pixels_at_offset 220
  set_4_pixels_at_offset 224
  set_4_pixels_at_offset 228
  set_4_pixels_at_offset 232
  set_4_pixels_at_offset 236
  set_4_pixels_at_offset 240
  set_4_pixels_at_offset 244
  set_4_pixels_at_offset 248
  set_4_pixels_at_offset 252

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  addi   REG_LOOP_INDEX, REG_LOOP_INDEX, -1
  bnez   REG_LOOP_INDEX, begin_loop
  ret

  .align 4
fcn_draw_128_pixels:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  set_4_pixels_at_offset 8
  set_4_pixels_at_offset 12
  set_4_pixels_at_offset 16
  set_4_pixels_at_offset 20
  set_4_pixels_at_offset 24
  set_4_pixels_at_offset 28
  set_4_pixels_at_offset 32
  set_4_pixels_at_offset 36
  set_4_pixels_at_offset 40
  set_4_pixels_at_offset 44
  set_4_pixels_at_offset 48
  set_4_pixels_at_offset 52
  set_4_pixels_at_offset 56
  set_4_pixels_at_offset 60
  set_4_pixels_at_offset 64
  set_4_pixels_at_offset 68
  set_4_pixels_at_offset 72
  set_4_pixels_at_offset 76
  set_4_pixels_at_offset 80
  set_4_pixels_at_offset 84
  set_4_pixels_at_offset 88
  set_4_pixels_at_offset 92
  set_4_pixels_at_offset 96
  set_4_pixels_at_offset 100
  set_4_pixels_at_offset 104
  set_4_pixels_at_offset 108
  set_4_pixels_at_offset 112
  set_4_pixels_at_offset 116
  set_4_pixels_at_offset 120
  set_4_pixels_at_offset 124
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  ret

  .align 4
fcn_draw_128_pixels_last:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  set_4_pixels_at_offset 8
  set_4_pixels_at_offset 12
  set_4_pixels_at_offset 16
  set_4_pixels_at_offset 20
  set_4_pixels_at_offset 24
  set_4_pixels_at_offset 28
  set_4_pixels_at_offset 32
  set_4_pixels_at_offset 36
  set_4_pixels_at_offset 40
  set_4_pixels_at_offset 44
  set_4_pixels_at_offset 48
  set_4_pixels_at_offset 52
  set_4_pixels_at_offset 56
  set_4_pixels_at_offset 60
  set_4_pixels_at_offset 64
  set_4_pixels_at_offset 68
  set_4_pixels_at_offset 72
  set_4_pixels_at_offset 76
  set_4_pixels_at_offset 80
  set_4_pixels_at_offset 84
  set_4_pixels_at_offset 88
  set_4_pixels_at_offset 92
  set_4_pixels_at_offset 96
  set_4_pixels_at_offset 100
  set_4_pixels_at_offset 104
  set_4_pixels_at_offset 108
  set_4_pixels_at_offset 112
  set_4_pixels_at_offset 116
  set_4_pixels_at_offset 120
  set_4_pixels_at_offset 124
  ret

  .align 4
fcn_draw_64_pixels:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  set_4_pixels_at_offset 8
  set_4_pixels_at_offset 12
  set_4_pixels_at_offset 16
  set_4_pixels_at_offset 20
  set_4_pixels_at_offset 24
  set_4_pixels_at_offset 28
  set_4_pixels_at_offset 32
  set_4_pixels_at_offset 36
  set_4_pixels_at_offset 40
  set_4_pixels_at_offset 44
  set_4_pixels_at_offset 48
  set_4_pixels_at_offset 52
  set_4_pixels_at_offset 56
  set_4_pixels_at_offset 60
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  ret

  .align 4
fcn_draw_64_pixels_last:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  set_4_pixels_at_offset 8
  set_4_pixels_at_offset 12
  set_4_pixels_at_offset 16
  set_4_pixels_at_offset 20
  set_4_pixels_at_offset 24
  set_4_pixels_at_offset 28
  set_4_pixels_at_offset 32
  set_4_pixels_at_offset 36
  set_4_pixels_at_offset 40
  set_4_pixels_at_offset 44
  set_4_pixels_at_offset 48
  set_4_pixels_at_offset 52
  set_4_pixels_at_offset 56
  set_4_pixels_at_offset 60
  ret

  .align 4
fcn_draw_32_pixels:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  set_4_pixels_at_offset 8
  set_4_pixels_at_offset 12
  set_4_pixels_at_offset 16
  set_4_pixels_at_offset 20
  set_4_pixels_at_offset 24
  set_4_pixels_at_offset 28
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 32
  ret

  .align 4
fcn_draw_32_pixels_last:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  set_4_pixels_at_offset 8
  set_4_pixels_at_offset 12
  set_4_pixels_at_offset 16
  set_4_pixels_at_offset 20
  set_4_pixels_at_offset 24
  set_4_pixels_at_offset 28
  ret

  .align 4
fcn_draw_16_pixels:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  set_4_pixels_at_offset 8
  set_4_pixels_at_offset 12
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16
  ret

  .align 4
fcn_draw_16_pixels_last:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  set_4_pixels_at_offset 8
  set_4_pixels_at_offset 12
  ret

  .align 4
fcn_draw_8_pixels:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 8
  ret

  .align 4
fcn_draw_8_pixels_last:
  set_4_pixels_at_offset 0
  set_4_pixels_at_offset 4
  ret

  .align 4
fcn_get_blend_25_for_4_pixels:
  and   REG_SRC_BR_PIXELS, REG_SRC_PIXELS, REG_ISOLATE_BR // src blue & red (no green)
  and   REG_DST_BR_PIXELS, REG_PIXEL_COLOR, REG_ISOLATE_BR // dst blue & red (no green)
  slli  REG_DOUBLE_COLOR, REG_DST_BR_PIXELS, 1 // double the dst color values
  add   REG_DST_BR_PIXELS, REG_DST_BR_PIXELS, REG_DOUBLE_COLOR // add dst color values again (total of 3X)
  add   REG_DST_BR_PIXELS, REG_DST_BR_PIXELS, REG_SRC_BR_PIXELS // sum of blue & sum of red
  srli  REG_DST_BR_PIXELS, REG_DST_BR_PIXELS, 2 // averages of mixed blue & mixed red, in 4 bits each
  and   REG_DST_BR_PIXELS, REG_DST_G_PIXELS, REG_ISOLATE_BR // averages in 2 bits each
  and   REG_SRC_G_PIXELS, REG_SRC_PIXELS, REG_ISOLATE_G // src green (no red or blue)
  and   REG_DST_G_PIXELS, REG_PIXEL_COLOR, REG_ISOLATE_G // dst green (no red or blue)
  slli  REG_DOUBLE_COLOR, REG_DST_G_PIXELS, 1 // double the dst color values
  add   REG_DST_G_PIXELS, REG_DST_G_PIXELS, REG_DOUBLE_COLOR // add dst color values again (total of 3X)
  add   REG_DST_G_PIXELS, REG_DST_G_PIXELS, REG_SRC_G_PIXELS // sum of green
  srli  REG_DST_G_PIXELS, REG_DST_G_PIXELS, 1 // averages of mixed green, in 4 bits each
  and   REG_DST_G_PIXELS, REG_DST_G_PIXELS, REG_ISOLATE_G // averages in 2 bits each
  or    REG_PIXEL_COLOR, REG_SRC_BR_PIXELS, REG_DST_G_PIXELS // 4 mixed pixels
  ret

  .align 4
fcn_get_blend_50_for_4_pixels:
  and   REG_SRC_BR_PIXELS, REG_SRC_PIXELS, REG_ISOLATE_BR // src blue & red (no green)
  and   REG_DST_BR_PIXELS, REG_PIXEL_COLOR, REG_ISOLATE_BR // dst blue & red (no green)
  add   REG_DST_BR_PIXELS, REG_DST_BR_PIXELS, REG_SRC_BR_PIXELS // sum of blue & sum of red
  srli  REG_DST_BR_PIXELS, REG_DST_BR_PIXELS, 1 // averages of mixed blue & mixed red, in 3 bits each
  and   REG_DST_BR_PIXELS, REG_DST_G_PIXELS, REG_ISOLATE_BR // averages in 2 bits each
  and   REG_SRC_G_PIXELS, REG_SRC_PIXELS, REG_ISOLATE_G // src green (no red or blue)
  and   REG_DST_G_PIXELS, REG_PIXEL_COLOR, REG_ISOLATE_G // dst green (no red or blue)
  add   REG_DST_G_PIXELS, REG_DST_G_PIXELS, REG_SRC_G_PIXELS // sum of green
  srli  REG_DST_G_PIXELS, REG_DST_G_PIXELS, 1 // averages of mixed green, in 3 bits each
  and   REG_DST_G_PIXELS, REG_DST_G_PIXELS, REG_ISOLATE_G // averages in 2 bits each
  or    REG_PIXEL_COLOR, REG_SRC_BR_PIXELS, REG_DST_G_PIXELS // 4 mixed pixels
  ret

  .align 4
fcn_get_blend_75_for_4_pixels:
  and   REG_SRC_BR_PIXELS, REG_SRC_PIXELS, REG_ISOLATE_BR // src blue & red (no green)
  and   REG_DST_BR_PIXELS, REG_PIXEL_COLOR, REG_ISOLATE_BR // dst blue & red (no green)
  slli  REG_DOUBLE_COLOR, REG_SRC_BR_PIXELS, 1 // double the src color values
  add   REG_SRC_BR_PIXELS, REG_SRC_BR_PIXELS, REG_DOUBLE_COLOR // add src color values again (total of 3X)
  add   REG_DST_BR_PIXELS, REG_DST_BR_PIXELS, REG_SRC_BR_PIXELS // sum of blue & sum of red
  srli  REG_DST_BR_PIXELS, REG_DST_BR_PIXELS, 2 // averages of mixed blue & mixed red, in 4 bits each
  and   REG_DST_BR_PIXELS, REG_DST_G_PIXELS, REG_ISOLATE_BR // averages in 2 bits each
  and   REG_SRC_G_PIXELS, REG_SRC_PIXELS, REG_ISOLATE_G // src green (no red or blue)
  and   REG_DST_G_PIXELS, REG_PIXEL_COLOR, REG_ISOLATE_G // dst green (no red or blue)
  slli  REG_DOUBLE_COLOR, REG_SRC_G_PIXELS, 1 // double the src color values
  add   REG_SRC_G_PIXELS, REG_SRC_G_PIXELS, REG_DOUBLE_COLOR // add src color values again (total of 3X)
  add   REG_DST_G_PIXELS, REG_DST_G_PIXELS, REG_SRC_G_PIXELS // sum of green
  srli  REG_DST_G_PIXELS, REG_DST_G_PIXELS, 1 // averages of mixed green, in 4 bits each
  and   REG_DST_G_PIXELS, REG_DST_G_PIXELS, REG_ISOLATE_G // averages in 2 bits each
  or    REG_PIXEL_COLOR, REG_SRC_BR_PIXELS, REG_DST_G_PIXELS // 4 mixed pixels
  ret

  .align 4
fcn_dummy:
  ret

  .align 4
fcn_copy_256_pixels_in_loop:
begin_copy_loop:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4
  copy_4_pixels_at_offset 8
  copy_4_pixels_at_offset 12
  copy_4_pixels_at_offset 16
  copy_4_pixels_at_offset 20
  copy_4_pixels_at_offset 24
  copy_4_pixels_at_offset 28
  copy_4_pixels_at_offset 32
  copy_4_pixels_at_offset 36
  copy_4_pixels_at_offset 40
  copy_4_pixels_at_offset 44
  copy_4_pixels_at_offset 48
  copy_4_pixels_at_offset 52
  copy_4_pixels_at_offset 56
  copy_4_pixels_at_offset 60
  copy_4_pixels_at_offset 64
  copy_4_pixels_at_offset 68
  copy_4_pixels_at_offset 72
  copy_4_pixels_at_offset 76
  copy_4_pixels_at_offset 80
  copy_4_pixels_at_offset 84
  copy_4_pixels_at_offset 88
  copy_4_pixels_at_offset 92
  copy_4_pixels_at_offset 96
  copy_4_pixels_at_offset 100
  copy_4_pixels_at_offset 104
  copy_4_pixels_at_offset 108
  copy_4_pixels_at_offset 112
  copy_4_pixels_at_offset 116
  copy_4_pixels_at_offset 120
  copy_4_pixels_at_offset 124

  copy_4_pixels_at_offset 128
  copy_4_pixels_at_offset 132
  copy_4_pixels_at_offset 136
  copy_4_pixels_at_offset 140
  copy_4_pixels_at_offset 144
  copy_4_pixels_at_offset 148
  copy_4_pixels_at_offset 152
  copy_4_pixels_at_offset 156
  copy_4_pixels_at_offset 160
  copy_4_pixels_at_offset 164
  copy_4_pixels_at_offset 168
  copy_4_pixels_at_offset 172
  copy_4_pixels_at_offset 176
  copy_4_pixels_at_offset 180
  copy_4_pixels_at_offset 184
  copy_4_pixels_at_offset 188
  copy_4_pixels_at_offset 192
  copy_4_pixels_at_offset 196
  copy_4_pixels_at_offset 200
  copy_4_pixels_at_offset 204
  copy_4_pixels_at_offset 208
  copy_4_pixels_at_offset 212
  copy_4_pixels_at_offset 216
  copy_4_pixels_at_offset 220
  copy_4_pixels_at_offset 224
  copy_4_pixels_at_offset 228
  copy_4_pixels_at_offset 232
  copy_4_pixels_at_offset 236
  copy_4_pixels_at_offset 240
  copy_4_pixels_at_offset 244
  copy_4_pixels_at_offset 248
  copy_4_pixels_at_offset 252

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 120
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 120
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 16

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  addi   REG_LOOP_INDEX, REG_LOOP_INDEX, -1
  bnez   REG_LOOP_INDEX, begin_copy_loop
  ret

  .align 4
fcn_copy_128_pixels:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4
  copy_4_pixels_at_offset 8
  copy_4_pixels_at_offset 12
  copy_4_pixels_at_offset 16
  copy_4_pixels_at_offset 20
  copy_4_pixels_at_offset 24
  copy_4_pixels_at_offset 28
  copy_4_pixels_at_offset 32
  copy_4_pixels_at_offset 36
  copy_4_pixels_at_offset 40
  copy_4_pixels_at_offset 44
  copy_4_pixels_at_offset 48
  copy_4_pixels_at_offset 52
  copy_4_pixels_at_offset 56
  copy_4_pixels_at_offset 60
  copy_4_pixels_at_offset 64
  copy_4_pixels_at_offset 68
  copy_4_pixels_at_offset 72
  copy_4_pixels_at_offset 76
  copy_4_pixels_at_offset 80
  copy_4_pixels_at_offset 84
  copy_4_pixels_at_offset 88
  copy_4_pixels_at_offset 92
  copy_4_pixels_at_offset 96
  copy_4_pixels_at_offset 100
  copy_4_pixels_at_offset 104
  copy_4_pixels_at_offset 108
  copy_4_pixels_at_offset 112
  copy_4_pixels_at_offset 116
  copy_4_pixels_at_offset 120
  copy_4_pixels_at_offset 124
  
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  ret

  .align 4
fcn_copy_128_pixels_last:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4
  copy_4_pixels_at_offset 8
  copy_4_pixels_at_offset 12
  copy_4_pixels_at_offset 16
  copy_4_pixels_at_offset 20
  copy_4_pixels_at_offset 24
  copy_4_pixels_at_offset 28
  copy_4_pixels_at_offset 32
  copy_4_pixels_at_offset 36
  copy_4_pixels_at_offset 40
  copy_4_pixels_at_offset 44
  copy_4_pixels_at_offset 48
  copy_4_pixels_at_offset 52
  copy_4_pixels_at_offset 56
  copy_4_pixels_at_offset 60
  copy_4_pixels_at_offset 64
  copy_4_pixels_at_offset 68
  copy_4_pixels_at_offset 72
  copy_4_pixels_at_offset 76
  copy_4_pixels_at_offset 80
  copy_4_pixels_at_offset 84
  copy_4_pixels_at_offset 88
  copy_4_pixels_at_offset 92
  copy_4_pixels_at_offset 96
  copy_4_pixels_at_offset 100
  copy_4_pixels_at_offset 104
  copy_4_pixels_at_offset 108
  copy_4_pixels_at_offset 112
  copy_4_pixels_at_offset 116
  copy_4_pixels_at_offset 120
  copy_4_pixels_at_offset 124
  ret

  .align 4
fcn_copy_64_pixels:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4
  copy_4_pixels_at_offset 8
  copy_4_pixels_at_offset 12
  copy_4_pixels_at_offset 16
  copy_4_pixels_at_offset 20
  copy_4_pixels_at_offset 24
  copy_4_pixels_at_offset 28
  copy_4_pixels_at_offset 32
  copy_4_pixels_at_offset 36
  copy_4_pixels_at_offset 40
  copy_4_pixels_at_offset 44
  copy_4_pixels_at_offset 48
  copy_4_pixels_at_offset 52
  copy_4_pixels_at_offset 56
  copy_4_pixels_at_offset 60
  
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  ret

  .align 4
fcn_copy_64_pixels_last:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4
  copy_4_pixels_at_offset 8
  copy_4_pixels_at_offset 12
  copy_4_pixels_at_offset 16
  copy_4_pixels_at_offset 20
  copy_4_pixels_at_offset 24
  copy_4_pixels_at_offset 28
  copy_4_pixels_at_offset 32
  copy_4_pixels_at_offset 36
  copy_4_pixels_at_offset 40
  copy_4_pixels_at_offset 44
  copy_4_pixels_at_offset 48
  copy_4_pixels_at_offset 52
  copy_4_pixels_at_offset 56
  copy_4_pixels_at_offset 60
  ret

  .align 4
fcn_copy_32_pixels:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4
  copy_4_pixels_at_offset 8
  copy_4_pixels_at_offset 12
  copy_4_pixels_at_offset 16
  copy_4_pixels_at_offset 20
  copy_4_pixels_at_offset 24
  copy_4_pixels_at_offset 28
  
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 32
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 32

  ret

  .align 4
fcn_copy_32_pixels_last:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4
  copy_4_pixels_at_offset 8
  copy_4_pixels_at_offset 12
  copy_4_pixels_at_offset 16
  copy_4_pixels_at_offset 20
  copy_4_pixels_at_offset 24
  copy_4_pixels_at_offset 28
  ret

  .align 4
fcn_copy_16_pixels:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4
  copy_4_pixels_at_offset 8
  copy_4_pixels_at_offset 12

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 16
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  ret

  .align 4
fcn_copy_16_pixels_last:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4
  copy_4_pixels_at_offset 8
  copy_4_pixels_at_offset 12
  ret

  .align 4
fcn_copy_8_pixels:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 8
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 8

  ret

  .align 4
fcn_copy_8_pixels_last:
  copy_4_pixels_at_offset 0
  copy_4_pixels_at_offset 4
  ret

  .align 4
fcn_blend_25_for_256_pixels_in_loop:
begin_blend_25_loop:
  blend_25_for_4_pixels_at_offset 0
  blend_25_for_4_pixels_at_offset 4
  blend_25_for_4_pixels_at_offset 8
  blend_25_for_4_pixels_at_offset 12
  blend_25_for_4_pixels_at_offset 16
  blend_25_for_4_pixels_at_offset 20
  blend_25_for_4_pixels_at_offset 24
  blend_25_for_4_pixels_at_offset 28
  blend_25_for_4_pixels_at_offset 32
  blend_25_for_4_pixels_at_offset 36
  blend_25_for_4_pixels_at_offset 40
  blend_25_for_4_pixels_at_offset 44
  blend_25_for_4_pixels_at_offset 48
  blend_25_for_4_pixels_at_offset 52
  blend_25_for_4_pixels_at_offset 56
  blend_25_for_4_pixels_at_offset 60
  blend_25_for_4_pixels_at_offset 64
  blend_25_for_4_pixels_at_offset 68
  blend_25_for_4_pixels_at_offset 72
  blend_25_for_4_pixels_at_offset 76
  blend_25_for_4_pixels_at_offset 80
  blend_25_for_4_pixels_at_offset 84
  blend_25_for_4_pixels_at_offset 88
  blend_25_for_4_pixels_at_offset 92
  blend_25_for_4_pixels_at_offset 96
  blend_25_for_4_pixels_at_offset 100
  blend_25_for_4_pixels_at_offset 104
  blend_25_for_4_pixels_at_offset 108
  blend_25_for_4_pixels_at_offset 112
  blend_25_for_4_pixels_at_offset 116
  blend_25_for_4_pixels_at_offset 120
  blend_25_for_4_pixels_at_offset 124

  blend_25_for_4_pixels_at_offset 128
  blend_25_for_4_pixels_at_offset 132
  blend_25_for_4_pixels_at_offset 136
  blend_25_for_4_pixels_at_offset 140
  blend_25_for_4_pixels_at_offset 144
  blend_25_for_4_pixels_at_offset 148
  blend_25_for_4_pixels_at_offset 152
  blend_25_for_4_pixels_at_offset 156
  blend_25_for_4_pixels_at_offset 160
  blend_25_for_4_pixels_at_offset 164
  blend_25_for_4_pixels_at_offset 168
  blend_25_for_4_pixels_at_offset 172
  blend_25_for_4_pixels_at_offset 176
  blend_25_for_4_pixels_at_offset 180
  blend_25_for_4_pixels_at_offset 184
  blend_25_for_4_pixels_at_offset 188
  blend_25_for_4_pixels_at_offset 192
  blend_25_for_4_pixels_at_offset 196
  blend_25_for_4_pixels_at_offset 200
  blend_25_for_4_pixels_at_offset 204
  blend_25_for_4_pixels_at_offset 208
  blend_25_for_4_pixels_at_offset 212
  blend_25_for_4_pixels_at_offset 216
  blend_25_for_4_pixels_at_offset 220
  blend_25_for_4_pixels_at_offset 224
  blend_25_for_4_pixels_at_offset 228
  blend_25_for_4_pixels_at_offset 232
  blend_25_for_4_pixels_at_offset 236
  blend_25_for_4_pixels_at_offset 240
  blend_25_for_4_pixels_at_offset 244
  blend_25_for_4_pixels_at_offset 248
  blend_25_for_4_pixels_at_offset 252

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 120
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 120
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 16

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  addi   REG_LOOP_INDEX, REG_LOOP_INDEX, -1
  bnez   REG_LOOP_INDEX, begin_blend_25_loop
  ret

  .align 4
fcn_blend_25_for_128_pixels:
  blend_25_for_4_pixels_at_offset 0
  blend_25_for_4_pixels_at_offset 4
  blend_25_for_4_pixels_at_offset 8
  blend_25_for_4_pixels_at_offset 12
  blend_25_for_4_pixels_at_offset 16
  blend_25_for_4_pixels_at_offset 20
  blend_25_for_4_pixels_at_offset 24
  blend_25_for_4_pixels_at_offset 28
  blend_25_for_4_pixels_at_offset 32
  blend_25_for_4_pixels_at_offset 36
  blend_25_for_4_pixels_at_offset 40
  blend_25_for_4_pixels_at_offset 44
  blend_25_for_4_pixels_at_offset 48
  blend_25_for_4_pixels_at_offset 52
  blend_25_for_4_pixels_at_offset 56
  blend_25_for_4_pixels_at_offset 60
  blend_25_for_4_pixels_at_offset 64
  blend_25_for_4_pixels_at_offset 68
  blend_25_for_4_pixels_at_offset 72
  blend_25_for_4_pixels_at_offset 76
  blend_25_for_4_pixels_at_offset 80
  blend_25_for_4_pixels_at_offset 84
  blend_25_for_4_pixels_at_offset 88
  blend_25_for_4_pixels_at_offset 92
  blend_25_for_4_pixels_at_offset 96
  blend_25_for_4_pixels_at_offset 100
  blend_25_for_4_pixels_at_offset 104
  blend_25_for_4_pixels_at_offset 108
  blend_25_for_4_pixels_at_offset 112
  blend_25_for_4_pixels_at_offset 116
  blend_25_for_4_pixels_at_offset 120
  blend_25_for_4_pixels_at_offset 124
  
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  ret

  .align 4
fcn_blend_25_for_128_pixels_last:
  blend_25_for_4_pixels_at_offset 0
  blend_25_for_4_pixels_at_offset 4
  blend_25_for_4_pixels_at_offset 8
  blend_25_for_4_pixels_at_offset 12
  blend_25_for_4_pixels_at_offset 16
  blend_25_for_4_pixels_at_offset 20
  blend_25_for_4_pixels_at_offset 24
  blend_25_for_4_pixels_at_offset 28
  blend_25_for_4_pixels_at_offset 32
  blend_25_for_4_pixels_at_offset 36
  blend_25_for_4_pixels_at_offset 40
  blend_25_for_4_pixels_at_offset 44
  blend_25_for_4_pixels_at_offset 48
  blend_25_for_4_pixels_at_offset 52
  blend_25_for_4_pixels_at_offset 56
  blend_25_for_4_pixels_at_offset 60
  blend_25_for_4_pixels_at_offset 64
  blend_25_for_4_pixels_at_offset 68
  blend_25_for_4_pixels_at_offset 72
  blend_25_for_4_pixels_at_offset 76
  blend_25_for_4_pixels_at_offset 80
  blend_25_for_4_pixels_at_offset 84
  blend_25_for_4_pixels_at_offset 88
  blend_25_for_4_pixels_at_offset 92
  blend_25_for_4_pixels_at_offset 96
  blend_25_for_4_pixels_at_offset 100
  blend_25_for_4_pixels_at_offset 104
  blend_25_for_4_pixels_at_offset 108
  blend_25_for_4_pixels_at_offset 112
  blend_25_for_4_pixels_at_offset 116
  blend_25_for_4_pixels_at_offset 120
  blend_25_for_4_pixels_at_offset 124
  ret

  .align 4
fcn_blend_25_for_64_pixels:
  blend_25_for_4_pixels_at_offset 0
  blend_25_for_4_pixels_at_offset 4
  blend_25_for_4_pixels_at_offset 8
  blend_25_for_4_pixels_at_offset 12
  blend_25_for_4_pixels_at_offset 16
  blend_25_for_4_pixels_at_offset 20
  blend_25_for_4_pixels_at_offset 24
  blend_25_for_4_pixels_at_offset 28
  blend_25_for_4_pixels_at_offset 32
  blend_25_for_4_pixels_at_offset 36
  blend_25_for_4_pixels_at_offset 40
  blend_25_for_4_pixels_at_offset 44
  blend_25_for_4_pixels_at_offset 48
  blend_25_for_4_pixels_at_offset 52
  blend_25_for_4_pixels_at_offset 56
  blend_25_for_4_pixels_at_offset 60
  
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  ret

  .align 4
fcn_blend_25_for_64_pixels_last:
  blend_25_for_4_pixels_at_offset 0
  blend_25_for_4_pixels_at_offset 4
  blend_25_for_4_pixels_at_offset 8
  blend_25_for_4_pixels_at_offset 12
  blend_25_for_4_pixels_at_offset 16
  blend_25_for_4_pixels_at_offset 20
  blend_25_for_4_pixels_at_offset 24
  blend_25_for_4_pixels_at_offset 28
  blend_25_for_4_pixels_at_offset 32
  blend_25_for_4_pixels_at_offset 36
  blend_25_for_4_pixels_at_offset 40
  blend_25_for_4_pixels_at_offset 44
  blend_25_for_4_pixels_at_offset 48
  blend_25_for_4_pixels_at_offset 52
  blend_25_for_4_pixels_at_offset 56
  blend_25_for_4_pixels_at_offset 60
  ret

  .align 4
fcn_blend_25_for_32_pixels:
  blend_25_for_4_pixels_at_offset 0
  blend_25_for_4_pixels_at_offset 4
  blend_25_for_4_pixels_at_offset 8
  blend_25_for_4_pixels_at_offset 12
  blend_25_for_4_pixels_at_offset 16
  blend_25_for_4_pixels_at_offset 20
  blend_25_for_4_pixels_at_offset 24
  blend_25_for_4_pixels_at_offset 28
  
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 32
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 32

  ret

  .align 4
fcn_blend_25_for_32_pixels_last:
  blend_25_for_4_pixels_at_offset 0
  blend_25_for_4_pixels_at_offset 4
  blend_25_for_4_pixels_at_offset 8
  blend_25_for_4_pixels_at_offset 12
  blend_25_for_4_pixels_at_offset 16
  blend_25_for_4_pixels_at_offset 20
  blend_25_for_4_pixels_at_offset 24
  blend_25_for_4_pixels_at_offset 28
  ret

  .align 4
fcn_blend_25_for_16_pixels:
  blend_25_for_4_pixels_at_offset 0
  blend_25_for_4_pixels_at_offset 4
  blend_25_for_4_pixels_at_offset 8
  blend_25_for_4_pixels_at_offset 12

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 16
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  ret

  .align 4
fcn_blend_25_for_16_pixels_last:
  blend_25_for_4_pixels_at_offset 0
  blend_25_for_4_pixels_at_offset 4
  blend_25_for_4_pixels_at_offset 8
  blend_25_for_4_pixels_at_offset 12
  ret

  .align 4
fcn_blend_25_for_8_pixels:
  blend_25_for_4_pixels_at_offset 0
  blend_25_for_4_pixels_at_offset 4

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 8
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 8

  ret

  .align 4
fcn_blend_25_for_8_pixels_last:
  blend_25_for_4_pixels_at_offset 0
  blend_25_for_4_pixels_at_offset 4
  ret

  .align 4
fcn_blend_25_for_1_pixel_at_offset_0:
  blend_25_for_1_pixel_at_offset_0
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_25_for_1_pixel_at_offset_0_last:
  blend_25_for_1_pixel_at_offset_0
  ret

  .align 4
fcn_blend_25_for_1_pixel_at_offset_1:
  blend_25_for_1_pixel_at_offset_1
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_25_for_1_pixel_at_offset_1_last:
  blend_25_for_1_pixel_at_offset_1
  ret

  .align 4
fcn_blend_25_for_1_pixel_at_offset_2:
  blend_25_for_1_pixel_at_offset_2
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_25_for_1_pixel_at_offset_2_last:
  blend_25_for_1_pixel_at_offset_2
  ret

  .align 4
fcn_blend_25_for_1_pixel_at_offset_3:
  blend_25_for_1_pixel_at_offset_3
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_25_for_1_pixel_at_offset_3_last:
  blend_25_for_1_pixel_at_offset_3
  ret

  .align 4
fcn_blend_25_for_2_pixels_at_offset_0:
  blend_25_for_2_pixels_at_offset_0
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_25_for_2_pixels_at_offset_0_last:
  blend_25_for_2_pixels_at_offset_0
  ret

  .align 4
fcn_blend_25_for_2_pixels_at_offset_1:
  blend_25_for_2_pixels_at_offset_1
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_25_for_2_pixels_at_offset_1_last:
  blend_25_for_2_pixels_at_offset_1
  ret

  .align 4
fcn_blend_25_for_2_pixels_at_offset_2:
  blend_25_for_2_pixels_at_offset_2
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_25_for_2_pixels_at_offset_2_last:
  blend_25_for_2_pixels_at_offset_2
  ret

  .align 4
fcn_blend_25_for_3_pixels_at_offset_0:
  blend_25_for_3_pixels_at_offset_0
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_25_for_3_pixels_at_offset_0_last:
  blend_25_for_3_pixels_at_offset_0
  ret

  .align 4
fcn_blend_25_for_3_pixels_at_offset_1:
  blend_25_for_3_pixels_at_offset_1
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_25_for_3_pixels_at_offset_1_last:
  blend_25_for_3_pixels_at_offset_1
  ret

  .align 4
fcn_blend_25_for_4_pixels_at_offset_0:
  blend_25_for_4_pixels_at_offset 0
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_25_for_4_pixels_at_offset_0_last:
  blend_25_for_4_pixels_at_offset 0
  ret

  .align 4
fcn_blend_50_for_256_pixels_in_loop:
begin_blend_50_loop:
  blend_50_for_4_pixels_at_offset 0
  blend_50_for_4_pixels_at_offset 4
  blend_50_for_4_pixels_at_offset 8
  blend_50_for_4_pixels_at_offset 12
  blend_50_for_4_pixels_at_offset 16
  blend_50_for_4_pixels_at_offset 20
  blend_50_for_4_pixels_at_offset 24
  blend_50_for_4_pixels_at_offset 28
  blend_50_for_4_pixels_at_offset 32
  blend_50_for_4_pixels_at_offset 36
  blend_50_for_4_pixels_at_offset 40
  blend_50_for_4_pixels_at_offset 44
  blend_50_for_4_pixels_at_offset 48
  blend_50_for_4_pixels_at_offset 52
  blend_50_for_4_pixels_at_offset 56
  blend_50_for_4_pixels_at_offset 60
  blend_50_for_4_pixels_at_offset 64
  blend_50_for_4_pixels_at_offset 68
  blend_50_for_4_pixels_at_offset 72
  blend_50_for_4_pixels_at_offset 76
  blend_50_for_4_pixels_at_offset 80
  blend_50_for_4_pixels_at_offset 84
  blend_50_for_4_pixels_at_offset 88
  blend_50_for_4_pixels_at_offset 92
  blend_50_for_4_pixels_at_offset 96
  blend_50_for_4_pixels_at_offset 100
  blend_50_for_4_pixels_at_offset 104
  blend_50_for_4_pixels_at_offset 108
  blend_50_for_4_pixels_at_offset 112
  blend_50_for_4_pixels_at_offset 116
  blend_50_for_4_pixels_at_offset 120
  blend_50_for_4_pixels_at_offset 124

  blend_50_for_4_pixels_at_offset 128
  blend_50_for_4_pixels_at_offset 132
  blend_50_for_4_pixels_at_offset 136
  blend_50_for_4_pixels_at_offset 140
  blend_50_for_4_pixels_at_offset 144
  blend_50_for_4_pixels_at_offset 148
  blend_50_for_4_pixels_at_offset 152
  blend_50_for_4_pixels_at_offset 156
  blend_50_for_4_pixels_at_offset 160
  blend_50_for_4_pixels_at_offset 164
  blend_50_for_4_pixels_at_offset 168
  blend_50_for_4_pixels_at_offset 172
  blend_50_for_4_pixels_at_offset 176
  blend_50_for_4_pixels_at_offset 180
  blend_50_for_4_pixels_at_offset 184
  blend_50_for_4_pixels_at_offset 188
  blend_50_for_4_pixels_at_offset 192
  blend_50_for_4_pixels_at_offset 196
  blend_50_for_4_pixels_at_offset 200
  blend_50_for_4_pixels_at_offset 204
  blend_50_for_4_pixels_at_offset 208
  blend_50_for_4_pixels_at_offset 212
  blend_50_for_4_pixels_at_offset 216
  blend_50_for_4_pixels_at_offset 220
  blend_50_for_4_pixels_at_offset 224
  blend_50_for_4_pixels_at_offset 228
  blend_50_for_4_pixels_at_offset 232
  blend_50_for_4_pixels_at_offset 236
  blend_50_for_4_pixels_at_offset 240
  blend_50_for_4_pixels_at_offset 244
  blend_50_for_4_pixels_at_offset 248
  blend_50_for_4_pixels_at_offset 252

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 120
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 120
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 16

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  addi   REG_LOOP_INDEX, REG_LOOP_INDEX, -1
  bnez   REG_LOOP_INDEX, begin_blend_50_loop
  ret

  .align 4
fcn_blend_50_for_128_pixels:
  blend_50_for_4_pixels_at_offset 0
  blend_50_for_4_pixels_at_offset 4
  blend_50_for_4_pixels_at_offset 8
  blend_50_for_4_pixels_at_offset 12
  blend_50_for_4_pixels_at_offset 16
  blend_50_for_4_pixels_at_offset 20
  blend_50_for_4_pixels_at_offset 24
  blend_50_for_4_pixels_at_offset 28
  blend_50_for_4_pixels_at_offset 32
  blend_50_for_4_pixels_at_offset 36
  blend_50_for_4_pixels_at_offset 40
  blend_50_for_4_pixels_at_offset 44
  blend_50_for_4_pixels_at_offset 48
  blend_50_for_4_pixels_at_offset 52
  blend_50_for_4_pixels_at_offset 56
  blend_50_for_4_pixels_at_offset 60
  blend_50_for_4_pixels_at_offset 64
  blend_50_for_4_pixels_at_offset 68
  blend_50_for_4_pixels_at_offset 72
  blend_50_for_4_pixels_at_offset 76
  blend_50_for_4_pixels_at_offset 80
  blend_50_for_4_pixels_at_offset 84
  blend_50_for_4_pixels_at_offset 88
  blend_50_for_4_pixels_at_offset 92
  blend_50_for_4_pixels_at_offset 96
  blend_50_for_4_pixels_at_offset 100
  blend_50_for_4_pixels_at_offset 104
  blend_50_for_4_pixels_at_offset 108
  blend_50_for_4_pixels_at_offset 112
  blend_50_for_4_pixels_at_offset 116
  blend_50_for_4_pixels_at_offset 120
  blend_50_for_4_pixels_at_offset 124
  
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  ret

  .align 4
fcn_blend_50_for_128_pixels_last:
  blend_50_for_4_pixels_at_offset 0
  blend_50_for_4_pixels_at_offset 4
  blend_50_for_4_pixels_at_offset 8
  blend_50_for_4_pixels_at_offset 12
  blend_50_for_4_pixels_at_offset 16
  blend_50_for_4_pixels_at_offset 20
  blend_50_for_4_pixels_at_offset 24
  blend_50_for_4_pixels_at_offset 28
  blend_50_for_4_pixels_at_offset 32
  blend_50_for_4_pixels_at_offset 36
  blend_50_for_4_pixels_at_offset 40
  blend_50_for_4_pixels_at_offset 44
  blend_50_for_4_pixels_at_offset 48
  blend_50_for_4_pixels_at_offset 52
  blend_50_for_4_pixels_at_offset 56
  blend_50_for_4_pixels_at_offset 60
  blend_50_for_4_pixels_at_offset 64
  blend_50_for_4_pixels_at_offset 68
  blend_50_for_4_pixels_at_offset 72
  blend_50_for_4_pixels_at_offset 76
  blend_50_for_4_pixels_at_offset 80
  blend_50_for_4_pixels_at_offset 84
  blend_50_for_4_pixels_at_offset 88
  blend_50_for_4_pixels_at_offset 92
  blend_50_for_4_pixels_at_offset 96
  blend_50_for_4_pixels_at_offset 100
  blend_50_for_4_pixels_at_offset 104
  blend_50_for_4_pixels_at_offset 108
  blend_50_for_4_pixels_at_offset 112
  blend_50_for_4_pixels_at_offset 116
  blend_50_for_4_pixels_at_offset 120
  blend_50_for_4_pixels_at_offset 124
  ret

  .align 4
fcn_blend_50_for_64_pixels:
  blend_50_for_4_pixels_at_offset 0
  blend_50_for_4_pixels_at_offset 4
  blend_50_for_4_pixels_at_offset 8
  blend_50_for_4_pixels_at_offset 12
  blend_50_for_4_pixels_at_offset 16
  blend_50_for_4_pixels_at_offset 20
  blend_50_for_4_pixels_at_offset 24
  blend_50_for_4_pixels_at_offset 28
  blend_50_for_4_pixels_at_offset 32
  blend_50_for_4_pixels_at_offset 36
  blend_50_for_4_pixels_at_offset 40
  blend_50_for_4_pixels_at_offset 44
  blend_50_for_4_pixels_at_offset 48
  blend_50_for_4_pixels_at_offset 52
  blend_50_for_4_pixels_at_offset 56
  blend_50_for_4_pixels_at_offset 60

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  ret

  .align 4
fcn_blend_50_for_64_pixels_last:
  blend_50_for_4_pixels_at_offset 0
  blend_50_for_4_pixels_at_offset 4
  blend_50_for_4_pixels_at_offset 8
  blend_50_for_4_pixels_at_offset 12
  blend_50_for_4_pixels_at_offset 16
  blend_50_for_4_pixels_at_offset 20
  blend_50_for_4_pixels_at_offset 24
  blend_50_for_4_pixels_at_offset 28
  blend_50_for_4_pixels_at_offset 32
  blend_50_for_4_pixels_at_offset 36
  blend_50_for_4_pixels_at_offset 40
  blend_50_for_4_pixels_at_offset 44
  blend_50_for_4_pixels_at_offset 48
  blend_50_for_4_pixels_at_offset 52
  blend_50_for_4_pixels_at_offset 56
  blend_50_for_4_pixels_at_offset 60
  ret

  .align 4
fcn_blend_50_for_32_pixels:
  blend_50_for_4_pixels_at_offset 0
  blend_50_for_4_pixels_at_offset 4
  blend_50_for_4_pixels_at_offset 8
  blend_50_for_4_pixels_at_offset 12
  blend_50_for_4_pixels_at_offset 16
  blend_50_for_4_pixels_at_offset 20
  blend_50_for_4_pixels_at_offset 24
  blend_50_for_4_pixels_at_offset 28

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 32
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 32

  ret

  .align 4
fcn_blend_50_for_32_pixels_last:
  blend_50_for_4_pixels_at_offset 0
  blend_50_for_4_pixels_at_offset 4
  blend_50_for_4_pixels_at_offset 8
  blend_50_for_4_pixels_at_offset 12
  blend_50_for_4_pixels_at_offset 16
  blend_50_for_4_pixels_at_offset 20
  blend_50_for_4_pixels_at_offset 24
  blend_50_for_4_pixels_at_offset 28
  ret

  .align 4
fcn_blend_50_for_16_pixels:
  blend_50_for_4_pixels_at_offset 0
  blend_50_for_4_pixels_at_offset 4
  blend_50_for_4_pixels_at_offset 8
  blend_50_for_4_pixels_at_offset 12

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 16
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  ret

  .align 4
fcn_blend_50_for_16_pixels_last:
  blend_50_for_4_pixels_at_offset 0
  blend_50_for_4_pixels_at_offset 4
  blend_50_for_4_pixels_at_offset 8
  blend_50_for_4_pixels_at_offset 12
  ret

  .align 4
fcn_blend_50_for_8_pixels:
  blend_50_for_4_pixels_at_offset 0
  blend_50_for_4_pixels_at_offset 4

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 8
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 8

  ret

  .align 4
fcn_blend_50_for_8_pixels_last:
  blend_50_for_4_pixels_at_offset 0
  blend_50_for_4_pixels_at_offset 4
  ret

  .align 4
fcn_blend_50_for_1_pixel_at_offset_0:
  blend_50_for_1_pixel_at_offset_0
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_50_for_1_pixel_at_offset_0_last:
  blend_50_for_1_pixel_at_offset_0
  ret

  .align 4
fcn_blend_50_for_1_pixel_at_offset_1:
  blend_50_for_1_pixel_at_offset_1
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_50_for_1_pixel_at_offset_1_last:
  blend_50_for_1_pixel_at_offset_1
  ret

  .align 4
fcn_blend_50_for_1_pixel_at_offset_2:
  blend_50_for_1_pixel_at_offset_2
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_50_for_1_pixel_at_offset_2_last:
  blend_50_for_1_pixel_at_offset_2
  ret

  .align 4
fcn_blend_50_for_1_pixel_at_offset_3:
  blend_50_for_1_pixel_at_offset_3
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_50_for_1_pixel_at_offset_3_last:
  blend_50_for_1_pixel_at_offset_3
  ret

  .align 4
fcn_blend_50_for_2_pixels_at_offset_0:
  blend_50_for_2_pixels_at_offset_0
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_50_for_2_pixels_at_offset_0_last:
  blend_50_for_2_pixels_at_offset_0
  ret

  .align 4
fcn_blend_50_for_2_pixels_at_offset_1:
  blend_50_for_2_pixels_at_offset_1
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_50_for_2_pixels_at_offset_1_last:
  blend_50_for_2_pixels_at_offset_1
  ret

  .align 4
fcn_blend_50_for_2_pixels_at_offset_2:
  blend_50_for_2_pixels_at_offset_2
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_50_for_2_pixels_at_offset_2_last:
  blend_50_for_2_pixels_at_offset_2
  ret

  .align 4
fcn_blend_50_for_3_pixels_at_offset_0:
  blend_50_for_3_pixels_at_offset_0
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_50_for_3_pixels_at_offset_0_last:
  blend_50_for_3_pixels_at_offset_0
  ret

  .align 4
fcn_blend_50_for_3_pixels_at_offset_1:
  blend_50_for_3_pixels_at_offset_1
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_50_for_3_pixels_at_offset_1_last:
  blend_50_for_3_pixels_at_offset_1
  ret

  .align 4
fcn_blend_50_for_4_pixels_at_offset_0:
  blend_50_for_4_pixels_at_offset 0
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_50_for_4_pixels_at_offset_0_last:
  blend_50_for_4_pixels_at_offset 0
  ret

  .align 4
fcn_blend_75_for_256_pixels_in_loop:
begin_blend_75_loop:
  blend_75_for_4_pixels_at_offset 0
  blend_75_for_4_pixels_at_offset 4
  blend_75_for_4_pixels_at_offset 8
  blend_75_for_4_pixels_at_offset 12
  blend_75_for_4_pixels_at_offset 16
  blend_75_for_4_pixels_at_offset 20
  blend_75_for_4_pixels_at_offset 24
  blend_75_for_4_pixels_at_offset 28
  blend_75_for_4_pixels_at_offset 32
  blend_75_for_4_pixels_at_offset 36
  blend_75_for_4_pixels_at_offset 40
  blend_75_for_4_pixels_at_offset 44
  blend_75_for_4_pixels_at_offset 48
  blend_75_for_4_pixels_at_offset 52
  blend_75_for_4_pixels_at_offset 56
  blend_75_for_4_pixels_at_offset 60
  blend_75_for_4_pixels_at_offset 64
  blend_75_for_4_pixels_at_offset 68
  blend_75_for_4_pixels_at_offset 72
  blend_75_for_4_pixels_at_offset 76
  blend_75_for_4_pixels_at_offset 80
  blend_75_for_4_pixels_at_offset 84
  blend_75_for_4_pixels_at_offset 88
  blend_75_for_4_pixels_at_offset 92
  blend_75_for_4_pixels_at_offset 96
  blend_75_for_4_pixels_at_offset 100
  blend_75_for_4_pixels_at_offset 104
  blend_75_for_4_pixels_at_offset 108
  blend_75_for_4_pixels_at_offset 112
  blend_75_for_4_pixels_at_offset 116
  blend_75_for_4_pixels_at_offset 120
  blend_75_for_4_pixels_at_offset 124

  blend_75_for_4_pixels_at_offset 128
  blend_75_for_4_pixels_at_offset 132
  blend_75_for_4_pixels_at_offset 136
  blend_75_for_4_pixels_at_offset 140
  blend_75_for_4_pixels_at_offset 144
  blend_75_for_4_pixels_at_offset 148
  blend_75_for_4_pixels_at_offset 152
  blend_75_for_4_pixels_at_offset 156
  blend_75_for_4_pixels_at_offset 160
  blend_75_for_4_pixels_at_offset 164
  blend_75_for_4_pixels_at_offset 168
  blend_75_for_4_pixels_at_offset 172
  blend_75_for_4_pixels_at_offset 176
  blend_75_for_4_pixels_at_offset 180
  blend_75_for_4_pixels_at_offset 184
  blend_75_for_4_pixels_at_offset 188
  blend_75_for_4_pixels_at_offset 192
  blend_75_for_4_pixels_at_offset 196
  blend_75_for_4_pixels_at_offset 200
  blend_75_for_4_pixels_at_offset 204
  blend_75_for_4_pixels_at_offset 208
  blend_75_for_4_pixels_at_offset 212
  blend_75_for_4_pixels_at_offset 216
  blend_75_for_4_pixels_at_offset 220
  blend_75_for_4_pixels_at_offset 224
  blend_75_for_4_pixels_at_offset 228
  blend_75_for_4_pixels_at_offset 232
  blend_75_for_4_pixels_at_offset 236
  blend_75_for_4_pixels_at_offset 240
  blend_75_for_4_pixels_at_offset 244
  blend_75_for_4_pixels_at_offset 248
  blend_75_for_4_pixels_at_offset 252

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 120
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 120
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 16

  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 120
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  addi   REG_LOOP_INDEX, REG_LOOP_INDEX, -1
  bnez   REG_LOOP_INDEX, begin_blend_75_loop
  ret

  .align 4
fcn_blend_75_for_128_pixels:
  blend_75_for_4_pixels_at_offset 0
  blend_75_for_4_pixels_at_offset 4
  blend_75_for_4_pixels_at_offset 8
  blend_75_for_4_pixels_at_offset 12
  blend_75_for_4_pixels_at_offset 16
  blend_75_for_4_pixels_at_offset 20
  blend_75_for_4_pixels_at_offset 24
  blend_75_for_4_pixels_at_offset 28
  blend_75_for_4_pixels_at_offset 32
  blend_75_for_4_pixels_at_offset 36
  blend_75_for_4_pixels_at_offset 40
  blend_75_for_4_pixels_at_offset 44
  blend_75_for_4_pixels_at_offset 48
  blend_75_for_4_pixels_at_offset 52
  blend_75_for_4_pixels_at_offset 56
  blend_75_for_4_pixels_at_offset 60
  blend_75_for_4_pixels_at_offset 64
  blend_75_for_4_pixels_at_offset 68
  blend_75_for_4_pixels_at_offset 72
  blend_75_for_4_pixels_at_offset 76
  blend_75_for_4_pixels_at_offset 80
  blend_75_for_4_pixels_at_offset 84
  blend_75_for_4_pixels_at_offset 88
  blend_75_for_4_pixels_at_offset 92
  blend_75_for_4_pixels_at_offset 96
  blend_75_for_4_pixels_at_offset 100
  blend_75_for_4_pixels_at_offset 104
  blend_75_for_4_pixels_at_offset 108
  blend_75_for_4_pixels_at_offset 112
  blend_75_for_4_pixels_at_offset 116
  blend_75_for_4_pixels_at_offset 120
  blend_75_for_4_pixels_at_offset 124

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  ret

  .align 4
fcn_blend_75_for_128_pixels_last:
  blend_75_for_4_pixels_at_offset 0
  blend_75_for_4_pixels_at_offset 4
  blend_75_for_4_pixels_at_offset 8
  blend_75_for_4_pixels_at_offset 12
  blend_75_for_4_pixels_at_offset 16
  blend_75_for_4_pixels_at_offset 20
  blend_75_for_4_pixels_at_offset 24
  blend_75_for_4_pixels_at_offset 28
  blend_75_for_4_pixels_at_offset 32
  blend_75_for_4_pixels_at_offset 36
  blend_75_for_4_pixels_at_offset 40
  blend_75_for_4_pixels_at_offset 44
  blend_75_for_4_pixels_at_offset 48
  blend_75_for_4_pixels_at_offset 52
  blend_75_for_4_pixels_at_offset 56
  blend_75_for_4_pixels_at_offset 60
  blend_75_for_4_pixels_at_offset 64
  blend_75_for_4_pixels_at_offset 68
  blend_75_for_4_pixels_at_offset 72
  blend_75_for_4_pixels_at_offset 76
  blend_75_for_4_pixels_at_offset 80
  blend_75_for_4_pixels_at_offset 84
  blend_75_for_4_pixels_at_offset 88
  blend_75_for_4_pixels_at_offset 92
  blend_75_for_4_pixels_at_offset 96
  blend_75_for_4_pixels_at_offset 100
  blend_75_for_4_pixels_at_offset 104
  blend_75_for_4_pixels_at_offset 108
  blend_75_for_4_pixels_at_offset 112
  blend_75_for_4_pixels_at_offset 116
  blend_75_for_4_pixels_at_offset 120
  blend_75_for_4_pixels_at_offset 124
  ret

  .align 4
fcn_blend_75_for_64_pixels:
  blend_75_for_4_pixels_at_offset 0
  blend_75_for_4_pixels_at_offset 4
  blend_75_for_4_pixels_at_offset 8
  blend_75_for_4_pixels_at_offset 12
  blend_75_for_4_pixels_at_offset 16
  blend_75_for_4_pixels_at_offset 20
  blend_75_for_4_pixels_at_offset 24
  blend_75_for_4_pixels_at_offset 28
  blend_75_for_4_pixels_at_offset 32
  blend_75_for_4_pixels_at_offset 36
  blend_75_for_4_pixels_at_offset 40
  blend_75_for_4_pixels_at_offset 44
  blend_75_for_4_pixels_at_offset 48
  blend_75_for_4_pixels_at_offset 52
  blend_75_for_4_pixels_at_offset 56
  blend_75_for_4_pixels_at_offset 60

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 64
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64

  ret

  .align 4
fcn_blend_75_for_64_pixels_last:
  blend_75_for_4_pixels_at_offset 0
  blend_75_for_4_pixels_at_offset 4
  blend_75_for_4_pixels_at_offset 8
  blend_75_for_4_pixels_at_offset 12
  blend_75_for_4_pixels_at_offset 16
  blend_75_for_4_pixels_at_offset 20
  blend_75_for_4_pixels_at_offset 24
  blend_75_for_4_pixels_at_offset 28
  blend_75_for_4_pixels_at_offset 32
  blend_75_for_4_pixels_at_offset 36
  blend_75_for_4_pixels_at_offset 40
  blend_75_for_4_pixels_at_offset 44
  blend_75_for_4_pixels_at_offset 48
  blend_75_for_4_pixels_at_offset 52
  blend_75_for_4_pixels_at_offset 56
  blend_75_for_4_pixels_at_offset 60
  ret

  .align 4
fcn_blend_75_for_32_pixels:
  blend_75_for_4_pixels_at_offset 0
  blend_75_for_4_pixels_at_offset 4
  blend_75_for_4_pixels_at_offset 8
  blend_75_for_4_pixels_at_offset 12
  blend_75_for_4_pixels_at_offset 16
  blend_75_for_4_pixels_at_offset 20
  blend_75_for_4_pixels_at_offset 24
  blend_75_for_4_pixels_at_offset 28

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 32
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 32

  ret

  .align 4
fcn_blend_75_for_32_pixels_last:
  blend_75_for_4_pixels_at_offset 0
  blend_75_for_4_pixels_at_offset 4
  blend_75_for_4_pixels_at_offset 8
  blend_75_for_4_pixels_at_offset 12
  blend_75_for_4_pixels_at_offset 16
  blend_75_for_4_pixels_at_offset 20
  blend_75_for_4_pixels_at_offset 24
  blend_75_for_4_pixels_at_offset 28
  ret

  .align 4
fcn_blend_75_for_16_pixels:
  blend_75_for_4_pixels_at_offset 0
  blend_75_for_4_pixels_at_offset 4
  blend_75_for_4_pixels_at_offset 8
  blend_75_for_4_pixels_at_offset 12

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 16
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 16

  ret

  .align 4
fcn_blend_75_for_16_pixels_last:
  blend_75_for_4_pixels_at_offset 0
  blend_75_for_4_pixels_at_offset 4
  blend_75_for_4_pixels_at_offset 8
  blend_75_for_4_pixels_at_offset 12
  ret

  .align 4
fcn_blend_75_for_8_pixels:
  blend_75_for_4_pixels_at_offset 0
  blend_75_for_4_pixels_at_offset 4

  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 8
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 8

  ret

  .align 4
fcn_blend_75_for_8_pixels_last:
  blend_75_for_4_pixels_at_offset 0
  blend_75_for_4_pixels_at_offset 4
  ret

  .align 4
fcn_blend_75_for_1_pixel_at_offset_0:
  blend_75_for_1_pixel_at_offset_0
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_75_for_1_pixel_at_offset_0_last:
  blend_75_for_1_pixel_at_offset_0
  ret

  .align 4
fcn_blend_75_for_1_pixel_at_offset_1:
  blend_75_for_1_pixel_at_offset_1
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_75_for_1_pixel_at_offset_1_last:
  blend_75_for_1_pixel_at_offset_1
  ret

  .align 4
fcn_blend_75_for_1_pixel_at_offset_2:
  blend_75_for_1_pixel_at_offset_2
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_75_for_1_pixel_at_offset_2_last:
  blend_75_for_1_pixel_at_offset_2
  ret

  .align 4
fcn_blend_75_for_1_pixel_at_offset_3:
  blend_75_for_1_pixel_at_offset_3
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_75_for_1_pixel_at_offset_3_last:
  blend_75_for_1_pixel_at_offset_3
  ret

  .align 4
fcn_blend_75_for_2_pixels_at_offset_0:
  blend_75_for_2_pixels_at_offset_0
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_75_for_2_pixels_at_offset_0_last:
  blend_75_for_2_pixels_at_offset_0
  ret

  .align 4
fcn_blend_75_for_2_pixels_at_offset_1:
  blend_75_for_2_pixels_at_offset_1
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_75_for_2_pixels_at_offset_1_last:
  blend_75_for_2_pixels_at_offset_1
  ret

  .align 4
fcn_blend_75_for_2_pixels_at_offset_2:
  blend_75_for_2_pixels_at_offset_2
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_75_for_2_pixels_at_offset_2_last:
  blend_75_for_2_pixels_at_offset_2
  ret

  .align 4
fcn_blend_75_for_3_pixels_at_offset_0:
  blend_75_for_3_pixels_at_offset_0
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_75_for_3_pixels_at_offset_0_last:
  blend_75_for_3_pixels_at_offset_0
  ret

  .align 4
fcn_blend_75_for_3_pixels_at_offset_1:
  blend_75_for_3_pixels_at_offset_1
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_75_for_3_pixels_at_offset_1_last:
  blend_75_for_3_pixels_at_offset_1
  ret

  .align 4
fcn_blend_75_for_4_pixels_at_offset_0:
  blend_75_for_4_pixels_at_offset 0
  addi   REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4
  addi   REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4
  ret

  .align 4
fcn_blend_75_for_4_pixels_at_offset_0_last:
  blend_75_for_4_pixels_at_offset 0
  ret
