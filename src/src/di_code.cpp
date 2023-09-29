// di_code.cpp - Supports creating ESP32 functions dynamically.
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

#include "di_code.h"
#include "di_constants.h"
#include "di_primitive_const.h"
#include "../agon.h"
#include "freertos/FreeRTOS.h"
#include <string.h>
#include <vector>

#define EXTRA_CODE_SIZE 8

// Input registers:
#define REG_RETURN_ADDR     a0
#define REG_STACK_PTR       a1
#define REG_THIS_PTR        a2
#define REG_LINE_PTR        a3
#define REG_LINE_INDEX      a4
// Temporary registers:
#define REG_SAVE_RET_INNER  a2
#define REG_SAVE_RET_DEEP   a3
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
#define REG_SAVE_COLOR      a15     // also the transparent color when copying pixels

#define RET_ADDR_IN_STACK   0

#define FIX_OFFSET(off)    ((off)^2)

extern uint32_t fcn_draw_256_pixels_in_loop;
extern uint32_t fcn_draw_128_pixels;
extern uint32_t fcn_draw_128_pixels_last;
extern uint32_t fcn_draw_64_pixels;
extern uint32_t fcn_draw_64_pixels_last;
extern uint32_t fcn_draw_32_pixels;
extern uint32_t fcn_draw_32_pixels_last;
extern uint32_t fcn_draw_16_pixels;
extern uint32_t fcn_draw_16_pixels_last;
extern uint32_t fcn_draw_8_pixels;
extern uint32_t fcn_draw_8_pixels_last;
extern uint32_t fcn_get_blend_25_for_4_pixels;
extern uint32_t fcn_get_blend_50_for_4_pixels;
extern uint32_t fcn_get_blend_75_for_4_pixels;
extern uint32_t fcn_dummy;


extern uint32_t fcn_copy_256_pixels_in_loop;
extern uint32_t fcn_copy_128_pixels;
extern uint32_t fcn_copy_128_pixels_last;
extern uint32_t fcn_copy_64_pixels;
extern uint32_t fcn_copy_64_pixels_last;
extern uint32_t fcn_copy_32_pixels;
extern uint32_t fcn_copy_32_pixels_last;
extern uint32_t fcn_copy_16_pixels;
extern uint32_t fcn_copy_16_pixels_last;
extern uint32_t fcn_copy_8_pixels;
extern uint32_t fcn_copy_8_pixels_last;

extern uint32_t fcn_color_blend_25_for_256_pixels_in_loop;
extern uint32_t fcn_color_blend_25_for_128_pixels;
extern uint32_t fcn_color_blend_25_for_128_pixels_last;
extern uint32_t fcn_color_blend_25_for_64_pixels;
extern uint32_t fcn_color_blend_25_for_64_pixels_last;
extern uint32_t fcn_color_blend_25_for_32_pixels;
extern uint32_t fcn_color_blend_25_for_32_pixels_last;
extern uint32_t fcn_color_blend_25_for_16_pixels;
extern uint32_t fcn_color_blend_25_for_16_pixels_last;
extern uint32_t fcn_color_blend_25_for_8_pixels;
extern uint32_t fcn_color_blend_25_for_8_pixels_last;
extern uint32_t fcn_color_blend_25_for_1_pixel_at_offset_0;
extern uint32_t fcn_color_blend_25_for_1_pixel_at_offset_0_last;
extern uint32_t fcn_color_blend_25_for_1_pixel_at_offset_1;
extern uint32_t fcn_color_blend_25_for_1_pixel_at_offset_1_last;
extern uint32_t fcn_color_blend_25_for_1_pixel_at_offset_2;
extern uint32_t fcn_color_blend_25_for_1_pixel_at_offset_2_last;
extern uint32_t fcn_color_blend_25_for_1_pixel_at_offset_3;
extern uint32_t fcn_color_blend_25_for_1_pixel_at_offset_3_last;
extern uint32_t fcn_color_blend_25_for_2_pixels_at_offset_0;
extern uint32_t fcn_color_blend_25_for_2_pixels_at_offset_0_last;
extern uint32_t fcn_color_blend_25_for_2_pixels_at_offset_1;
extern uint32_t fcn_color_blend_25_for_2_pixels_at_offset_1_last;
extern uint32_t fcn_color_blend_25_for_2_pixels_at_offset_2;
extern uint32_t fcn_color_blend_25_for_2_pixels_at_offset_2_last;
extern uint32_t fcn_color_blend_25_for_3_pixels_at_offset_0;
extern uint32_t fcn_color_blend_25_for_3_pixels_at_offset_0_last;
extern uint32_t fcn_color_blend_25_for_3_pixels_at_offset_1;
extern uint32_t fcn_color_blend_25_for_3_pixels_at_offset_1_last;
extern uint32_t fcn_color_blend_25_for_4_pixels_at_offset_0;
extern uint32_t fcn_color_blend_25_for_4_pixels_at_offset_0_last;

extern uint32_t fcn_color_blend_50_for_256_pixels_in_loop;
extern uint32_t fcn_color_blend_50_for_128_pixels;
extern uint32_t fcn_color_blend_50_for_128_pixels_last;
extern uint32_t fcn_color_blend_50_for_64_pixels;
extern uint32_t fcn_color_blend_50_for_64_pixels_last;
extern uint32_t fcn_color_blend_50_for_32_pixels;
extern uint32_t fcn_color_blend_50_for_32_pixels_last;
extern uint32_t fcn_color_blend_50_for_16_pixels;
extern uint32_t fcn_color_blend_50_for_16_pixels_last;
extern uint32_t fcn_color_blend_50_for_8_pixels;
extern uint32_t fcn_color_blend_50_for_8_pixels_last;
extern uint32_t fcn_color_blend_50_for_1_pixel_at_offset_0;
extern uint32_t fcn_color_blend_50_for_1_pixel_at_offset_0_last;
extern uint32_t fcn_color_blend_50_for_1_pixel_at_offset_1;
extern uint32_t fcn_color_blend_50_for_1_pixel_at_offset_1_last;
extern uint32_t fcn_color_blend_50_for_1_pixel_at_offset_2;
extern uint32_t fcn_color_blend_50_for_1_pixel_at_offset_2_last;
extern uint32_t fcn_color_blend_50_for_1_pixel_at_offset_3;
extern uint32_t fcn_color_blend_50_for_1_pixel_at_offset_3_last;
extern uint32_t fcn_color_blend_50_for_2_pixels_at_offset_0;
extern uint32_t fcn_color_blend_50_for_2_pixels_at_offset_0_last;
extern uint32_t fcn_color_blend_50_for_2_pixels_at_offset_1;
extern uint32_t fcn_color_blend_50_for_2_pixels_at_offset_1_last;
extern uint32_t fcn_color_blend_50_for_2_pixels_at_offset_2;
extern uint32_t fcn_color_blend_50_for_2_pixels_at_offset_2_last;
extern uint32_t fcn_color_blend_50_for_3_pixels_at_offset_0;
extern uint32_t fcn_color_blend_50_for_3_pixels_at_offset_0_last;
extern uint32_t fcn_color_blend_50_for_3_pixels_at_offset_1;
extern uint32_t fcn_color_blend_50_for_3_pixels_at_offset_1_last;
extern uint32_t fcn_color_blend_50_for_4_pixels_at_offset_0;
extern uint32_t fcn_color_blend_50_for_4_pixels_at_offset_0_last;

extern uint32_t fcn_color_blend_75_for_256_pixels_in_loop;
extern uint32_t fcn_color_blend_75_for_128_pixels;
extern uint32_t fcn_color_blend_75_for_128_pixels_last;
extern uint32_t fcn_color_blend_75_for_64_pixels;
extern uint32_t fcn_color_blend_75_for_64_pixels_last;
extern uint32_t fcn_color_blend_75_for_32_pixels;
extern uint32_t fcn_color_blend_75_for_32_pixels_last;
extern uint32_t fcn_color_blend_75_for_16_pixels;
extern uint32_t fcn_color_blend_75_for_16_pixels_last;
extern uint32_t fcn_color_blend_75_for_8_pixels;
extern uint32_t fcn_color_blend_75_for_8_pixels_last;
extern uint32_t fcn_color_blend_75_for_1_pixel_at_offset_0;
extern uint32_t fcn_color_blend_75_for_1_pixel_at_offset_0_last;
extern uint32_t fcn_color_blend_75_for_1_pixel_at_offset_1;
extern uint32_t fcn_color_blend_75_for_1_pixel_at_offset_1_last;
extern uint32_t fcn_color_blend_75_for_1_pixel_at_offset_2;
extern uint32_t fcn_color_blend_75_for_1_pixel_at_offset_2_last;
extern uint32_t fcn_color_blend_75_for_1_pixel_at_offset_3;
extern uint32_t fcn_color_blend_75_for_1_pixel_at_offset_3_last;
extern uint32_t fcn_color_blend_75_for_2_pixels_at_offset_0;
extern uint32_t fcn_color_blend_75_for_2_pixels_at_offset_0_last;
extern uint32_t fcn_color_blend_75_for_2_pixels_at_offset_1;
extern uint32_t fcn_color_blend_75_for_2_pixels_at_offset_1_last;
extern uint32_t fcn_color_blend_75_for_2_pixels_at_offset_2;
extern uint32_t fcn_color_blend_75_for_2_pixels_at_offset_2_last;
extern uint32_t fcn_color_blend_75_for_3_pixels_at_offset_0;
extern uint32_t fcn_color_blend_75_for_3_pixels_at_offset_0_last;
extern uint32_t fcn_color_blend_75_for_3_pixels_at_offset_1;
extern uint32_t fcn_color_blend_75_for_3_pixels_at_offset_1_last;
extern uint32_t fcn_color_blend_75_for_4_pixels_at_offset_0;
extern uint32_t fcn_color_blend_75_for_4_pixels_at_offset_0_last;

extern uint32_t fcn_src_blend_25_for_256_pixels_in_loop;
extern uint32_t fcn_src_blend_25_for_128_pixels;
extern uint32_t fcn_src_blend_25_for_128_pixels_last;
extern uint32_t fcn_src_blend_25_for_64_pixels;
extern uint32_t fcn_src_blend_25_for_64_pixels_last;
extern uint32_t fcn_src_blend_25_for_32_pixels;
extern uint32_t fcn_src_blend_25_for_32_pixels_last;
extern uint32_t fcn_src_blend_25_for_16_pixels;
extern uint32_t fcn_src_blend_25_for_16_pixels_last;
extern uint32_t fcn_src_blend_25_for_8_pixels;
extern uint32_t fcn_src_blend_25_for_8_pixels_last;
extern uint32_t fcn_src_blend_25_for_1_pixel_at_offset_0;
extern uint32_t fcn_src_blend_25_for_1_pixel_at_offset_0_last;
extern uint32_t fcn_src_blend_25_for_1_pixel_at_offset_1;
extern uint32_t fcn_src_blend_25_for_1_pixel_at_offset_1_last;
extern uint32_t fcn_src_blend_25_for_1_pixel_at_offset_2;
extern uint32_t fcn_src_blend_25_for_1_pixel_at_offset_2_last;
extern uint32_t fcn_src_blend_25_for_1_pixel_at_offset_3;
extern uint32_t fcn_src_blend_25_for_1_pixel_at_offset_3_last;
extern uint32_t fcn_src_blend_25_for_2_pixels_at_offset_0;
extern uint32_t fcn_src_blend_25_for_2_pixels_at_offset_0_last;
extern uint32_t fcn_src_blend_25_for_2_pixels_at_offset_1;
extern uint32_t fcn_src_blend_25_for_2_pixels_at_offset_1_last;
extern uint32_t fcn_src_blend_25_for_2_pixels_at_offset_2;
extern uint32_t fcn_src_blend_25_for_2_pixels_at_offset_2_last;
extern uint32_t fcn_src_blend_25_for_3_pixels_at_offset_0;
extern uint32_t fcn_src_blend_25_for_3_pixels_at_offset_0_last;
extern uint32_t fcn_src_blend_25_for_3_pixels_at_offset_1;
extern uint32_t fcn_src_blend_25_for_3_pixels_at_offset_1_last;
extern uint32_t fcn_src_blend_25_for_4_pixels_at_offset_0;
extern uint32_t fcn_src_blend_25_for_4_pixels_at_offset_0_last;

extern uint32_t fcn_src_blend_50_for_256_pixels_in_loop;
extern uint32_t fcn_src_blend_50_for_128_pixels;
extern uint32_t fcn_src_blend_50_for_128_pixels_last;
extern uint32_t fcn_src_blend_50_for_64_pixels;
extern uint32_t fcn_src_blend_50_for_64_pixels_last;
extern uint32_t fcn_src_blend_50_for_32_pixels;
extern uint32_t fcn_src_blend_50_for_32_pixels_last;
extern uint32_t fcn_src_blend_50_for_16_pixels;
extern uint32_t fcn_src_blend_50_for_16_pixels_last;
extern uint32_t fcn_src_blend_50_for_8_pixels;
extern uint32_t fcn_src_blend_50_for_8_pixels_last;
extern uint32_t fcn_src_blend_50_for_1_pixel_at_offset_0;
extern uint32_t fcn_src_blend_50_for_1_pixel_at_offset_0_last;
extern uint32_t fcn_src_blend_50_for_1_pixel_at_offset_1;
extern uint32_t fcn_src_blend_50_for_1_pixel_at_offset_1_last;
extern uint32_t fcn_src_blend_50_for_1_pixel_at_offset_2;
extern uint32_t fcn_src_blend_50_for_1_pixel_at_offset_2_last;
extern uint32_t fcn_src_blend_50_for_1_pixel_at_offset_3;
extern uint32_t fcn_src_blend_50_for_1_pixel_at_offset_3_last;
extern uint32_t fcn_src_blend_50_for_2_pixels_at_offset_0;
extern uint32_t fcn_src_blend_50_for_2_pixels_at_offset_0_last;
extern uint32_t fcn_src_blend_50_for_2_pixels_at_offset_1;
extern uint32_t fcn_src_blend_50_for_2_pixels_at_offset_1_last;
extern uint32_t fcn_src_blend_50_for_2_pixels_at_offset_2;
extern uint32_t fcn_src_blend_50_for_2_pixels_at_offset_2_last;
extern uint32_t fcn_src_blend_50_for_3_pixels_at_offset_0;
extern uint32_t fcn_src_blend_50_for_3_pixels_at_offset_0_last;
extern uint32_t fcn_src_blend_50_for_3_pixels_at_offset_1;
extern uint32_t fcn_src_blend_50_for_3_pixels_at_offset_1_last;
extern uint32_t fcn_src_blend_50_for_4_pixels_at_offset_0;
extern uint32_t fcn_src_blend_50_for_4_pixels_at_offset_0_last;

extern uint32_t fcn_src_blend_75_for_256_pixels_in_loop;
extern uint32_t fcn_src_blend_75_for_128_pixels;
extern uint32_t fcn_src_blend_75_for_128_pixels_last;
extern uint32_t fcn_src_blend_75_for_64_pixels;
extern uint32_t fcn_src_blend_75_for_64_pixels_last;
extern uint32_t fcn_src_blend_75_for_32_pixels;
extern uint32_t fcn_src_blend_75_for_32_pixels_last;
extern uint32_t fcn_src_blend_75_for_16_pixels;
extern uint32_t fcn_src_blend_75_for_16_pixels_last;
extern uint32_t fcn_src_blend_75_for_8_pixels;
extern uint32_t fcn_src_blend_75_for_8_pixels_last;
extern uint32_t fcn_src_blend_75_for_1_pixel_at_offset_0;
extern uint32_t fcn_src_blend_75_for_1_pixel_at_offset_0_last;
extern uint32_t fcn_src_blend_75_for_1_pixel_at_offset_1;
extern uint32_t fcn_src_blend_75_for_1_pixel_at_offset_1_last;
extern uint32_t fcn_src_blend_75_for_1_pixel_at_offset_2;
extern uint32_t fcn_src_blend_75_for_1_pixel_at_offset_2_last;
extern uint32_t fcn_src_blend_75_for_1_pixel_at_offset_3;
extern uint32_t fcn_src_blend_75_for_1_pixel_at_offset_3_last;
extern uint32_t fcn_src_blend_75_for_2_pixels_at_offset_0;
extern uint32_t fcn_src_blend_75_for_2_pixels_at_offset_0_last;
extern uint32_t fcn_src_blend_75_for_2_pixels_at_offset_1;
extern uint32_t fcn_src_blend_75_for_2_pixels_at_offset_1_last;
extern uint32_t fcn_src_blend_75_for_2_pixels_at_offset_2;
extern uint32_t fcn_src_blend_75_for_2_pixels_at_offset_2_last;
extern uint32_t fcn_src_blend_75_for_3_pixels_at_offset_0;
extern uint32_t fcn_src_blend_75_for_3_pixels_at_offset_0_last;
extern uint32_t fcn_src_blend_75_for_3_pixels_at_offset_1;
extern uint32_t fcn_src_blend_75_for_3_pixels_at_offset_1_last;
extern uint32_t fcn_src_blend_75_for_4_pixels_at_offset_0;
extern uint32_t fcn_src_blend_75_for_4_pixels_at_offset_0_last;

EspFunction::EspFunction() {
    init_members();
}

EspFunction::~EspFunction() {
    if (m_code) {
        heap_caps_free(m_code);
    }
}

void EspFunction::init_members() {
    m_alloc_size = 0;
    m_code_size = 0;
    m_code_index = 0;
    m_code = 0;
}

void EspFunction::draw_line(EspFixups& fixups, uint32_t x, uint32_t width, bool outer_fcn, uint8_t opaqueness) {
    auto at_jump = (outer_fcn ? enter_outer_function() : enter_inner_function());
    auto at_data = begin_data();

    auto aligned_x = x & 0xFFFFFFFC;
    auto x_offset = x & 3;
    auto at_x = d32(aligned_x);

    uint32_t at_isolate_br = 0;
    uint32_t at_isolate_g = 0;
    if (opaqueness != 100) {
        at_isolate_br = d32(0x33333333); // mask to isolate blue & red, removing green
        at_isolate_g = d32(0x0C0C0C0C); // mask to isolate green, removing red & blue
    }

    begin_code(at_jump);

    set_reg_dst_pixel_ptr(at_x);

    if (opaqueness != 100) {
        l32r_from(REG_ISOLATE_BR, at_isolate_br);
        l32r_from(REG_ISOLATE_G, at_isolate_g);
    }

    if (outer_fcn) {
        l32i(REG_PIXEL_COLOR, REG_THIS_PTR, FLD_color);
        s32i(REG_RETURN_ADDR, REG_STACK_PTR, RET_ADDR_IN_STACK);
    } else {
        mov(REG_SAVE_RET_INNER, REG_RETURN_ADDR);
    }

    if (opaqueness != 100) {
        mov(REG_SAVE_COLOR, REG_PIXEL_COLOR);
    }

    uint32_t p_fcn = 0;

    while (width) {
        auto offset = x_offset & 3;
        uint32_t sub = 1;
        switch (offset) {
            case 0:
                if (width >= 4) {
                    if (width >= 256) {
                        // Need at least 64 full words
                        auto times = width / 256;
                        movi(REG_LOOP_INDEX, times);
                        switch (opaqueness) {
                            case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_256_pixels_in_loop; break;
                            case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_256_pixels_in_loop; break;
                            case 75: p_fcn =  (uint32_t) &fcn_color_blend_75_for_256_pixels_in_loop; break;
                            case 100: p_fcn = (uint32_t) &fcn_draw_256_pixels_in_loop; break;
                        }
                        sub = times * 256;
                    } else if (width >= 128) {
                        // Need at least 32 full words
                        if (width > 128) {
                            switch (opaqueness) {
                                case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_128_pixels; break;
                                case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_128_pixels; break;
                                case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_128_pixels; break;
                                case 100: p_fcn = (uint32_t) &fcn_draw_128_pixels; break;
                            }
                        } else {
                            switch (opaqueness) {
                                case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_128_pixels_last; break;
                                case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_128_pixels_last; break;
                                case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_128_pixels_last; break;
                                case 100: p_fcn = (uint32_t) &fcn_draw_128_pixels_last; break;
                            }
                        }
                        sub = 128;
                    } else if (width >= 64) {
                        // Need at least 16 full words
                        if (width > 64) {
                            switch (opaqueness) {
                                case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_64_pixels; break;
                                case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_64_pixels; break;
                                case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_64_pixels; break;
                                case 100: p_fcn = (uint32_t) &fcn_draw_64_pixels; break;
                            }
                        } else {
                            switch (opaqueness) {
                                case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_64_pixels_last; break;
                                case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_64_pixels_last; break;
                                case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_64_pixels_last; break;
                                case 100: p_fcn = (uint32_t) &fcn_draw_64_pixels_last; break;
                            }
                        }
                        sub = 64;
                    } else if (width >= 32) {
                        // Need at least 8 full words
                        if (width > 32) {
                            switch (opaqueness) {
                                case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_32_pixels; break;
                                case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_32_pixels; break;
                                case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_32_pixels; break;
                                case 100: p_fcn = (uint32_t) &fcn_draw_32_pixels; break;
                            }
                        } else {
                            switch (opaqueness) {
                                case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_32_pixels_last; break;
                                case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_32_pixels_last; break;
                                case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_32_pixels_last; break;
                                case 100: p_fcn = (uint32_t) &fcn_draw_32_pixels_last; break;
                            }
                        }
                        sub = 32;
                    } else if (width >= 16) {
                        // Need at least 4 full words
                        if (width > 16) {
                            switch (opaqueness) {
                                case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_16_pixels; break;
                                case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_16_pixels; break;
                                case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_16_pixels; break;
                                case 100: p_fcn = (uint32_t) &fcn_draw_16_pixels; break;
                            }
                        } else {
                            switch (opaqueness) {
                                case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_16_pixels_last; break;
                                case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_16_pixels_last; break;
                                case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_16_pixels_last; break;
                                case 100: p_fcn = (uint32_t) &fcn_draw_16_pixels_last; break;
                            }
                        }
                        sub = 16;
                    } else if (width >= 8) {
                        // Need at least 2 full words
                        if (width > 8) {
                            switch (opaqueness) {
                                case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_8_pixels; break;
                                case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_8_pixels; break;
                                case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_8_pixels; break;
                                case 100: p_fcn = (uint32_t) &fcn_draw_8_pixels; break;
                            }
                        } else {
                            switch (opaqueness) {
                                case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_8_pixels_last; break;
                                case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_8_pixels_last; break;
                                case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_8_pixels_last; break;
                                case 100: p_fcn = (uint32_t) &fcn_draw_8_pixels_last; break;
                            }
                        }
                        sub = 8;
                    } else {
                        // Need at least 1 full word
                        if (width > 4) {
                            switch (opaqueness) {
                                case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_4_pixels_at_offset_0; break;
                                case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_4_pixels_at_offset_0; break;
                                case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_4_pixels_at_offset_0; break;
                                case 100:
                                    s32i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0);
                                    addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                                    break;
                            }
                        } else {
                            switch (opaqueness) {
                                case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_4_pixels_at_offset_0_last; break;
                                case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_4_pixels_at_offset_0_last; break;
                                case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_4_pixels_at_offset_0_last; break;
                                case 100:
                                    s32i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0);
                                    break;
                            }
                        }
                        sub = 4;
                    }
                } else if (width == 3) {
                    switch (opaqueness) {
                        case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_3_pixels_at_offset_0_last; break;
                        case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_3_pixels_at_offset_0_last; break;
                        case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_3_pixels_at_offset_0_last; break;
                        case 100:
                            s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0));
                            s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                            break;
                    }
                    sub = 3;
                } else if (width == 2) {
                    switch (opaqueness) {
                        case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_2_pixels_at_offset_0_last; break;
                        case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_2_pixels_at_offset_0_last; break;
                        case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_2_pixels_at_offset_0_last; break;
                        case 100:
                            s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0));
                            break;
                    }
                    sub = 2;
                } else { // width == 1
                    switch (opaqueness) {
                        case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_1_pixel_at_offset_0_last; break;
                        case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_1_pixel_at_offset_0_last; break;
                        case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_1_pixel_at_offset_0_last; break;
                        case 100:
                            s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0));
                            break;
                    }
                }
                break;

            case 1:
                if (width >= 3) {
                    if (width > 3) {
                        switch (opaqueness) {
                            case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_3_pixels_at_offset_1; break;
                            case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_3_pixels_at_offset_1; break;
                            case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_3_pixels_at_offset_1; break;
                            case 100:
                                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1));    
                                s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                                addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                                break;
                        }
                    } else {
                        switch (opaqueness) {
                            case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_3_pixels_at_offset_1_last; break;
                            case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_3_pixels_at_offset_1_last; break;
                            case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_3_pixels_at_offset_1_last; break;
                            case 100:
                                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1));    
                                s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                                break;
                        }
                    }
                    sub = 3;                
                } else if (width == 2) {
                    switch (opaqueness) {
                        case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_2_pixels_at_offset_1_last; break;
                        case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_2_pixels_at_offset_1_last; break;
                        case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_2_pixels_at_offset_1_last; break;
                        case 100:
                            s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1));
                            s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                            break;
                    }
                    sub = 2;
                } else { // width == 1
                    switch (opaqueness) {
                        case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_1_pixel_at_offset_1_last; break;
                        case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_1_pixel_at_offset_1_last; break;
                        case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_1_pixel_at_offset_1_last; break;
                        case 100:
                            s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1));
                            break;
                    }
                }
                break;

            case 2:
                if (width >= 2) {
                    if (width > 2) {
                        switch (opaqueness) {
                            case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_2_pixels_at_offset_2; break;
                            case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_2_pixels_at_offset_2; break;
                            case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_2_pixels_at_offset_2; break;
                            case 100:
                                s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                                addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                                break;
                        }
                    } else {
                        switch (opaqueness) {
                            case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_2_pixels_at_offset_2_last; break;
                            case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_2_pixels_at_offset_2_last; break;
                            case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_2_pixels_at_offset_2_last; break;
                            case 100:
                                s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                                break;
                        }
                    }
                    sub = 2;
                } else { // width == 1
                    switch (opaqueness) {
                        case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_1_pixel_at_offset_2_last; break;
                        case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_1_pixel_at_offset_2_last; break;
                        case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_1_pixel_at_offset_2_last; break;
                        case 100:
                            s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                            break;
                    }
                }
                break;
            
            case 3:
                if (width > 1) {
                    switch (opaqueness) {
                        case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_1_pixel_at_offset_3; break;
                        case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_1_pixel_at_offset_3; break;
                        case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_1_pixel_at_offset_3; break;
                        case 100:
                            s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(3));
                            addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                            break;
                    }
                } else {
                    switch (opaqueness) {
                        case 25: p_fcn = (uint32_t) &fcn_color_blend_25_for_1_pixel_at_offset_3_last; break;
                        case 50: p_fcn = (uint32_t) &fcn_color_blend_50_for_1_pixel_at_offset_3_last; break;
                        case 75: p_fcn = (uint32_t) &fcn_color_blend_75_for_1_pixel_at_offset_3_last; break;
                        case 100:
                            s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(3));
                            break;
                    }
                }
                break;
        }
        width -= sub;
        x_offset += sub;
        if (p_fcn) {
            fixups.push_back(EspFixup { get_code_index(), p_fcn });
            call0(0);
            p_fcn = 0;
        }
    }

    if (outer_fcn) {
        l32i(REG_RETURN_ADDR, REG_STACK_PTR, RET_ADDR_IN_STACK);
        retw();
    } else {
        jx(REG_SAVE_RET_INNER);
    }
}
extern void debug_log(const char* fmt, ...);
void EspFunction::copy_line(EspFixups& fixups, uint32_t x, uint32_t width, bool outer_fcn,
        bool is_transparent, uint8_t transparent_color, uint32_t* src_pixels) {
    auto at_jump = (outer_fcn ? enter_outer_function() : enter_inner_function());
    auto at_data = begin_data();

    auto aligned_x = x & 0xFFFFFFFC;
    auto x_offset = x & 3;
    auto at_x = d32(aligned_x);
    auto at_src = d32((uint32_t)src_pixels);

    uint32_t at_isolate_br = 0;
    uint32_t at_isolate_g = 0;
    if (is_transparent) {
        at_isolate_br = d32(0x33333333); // mask to isolate blue & red, removing green
        at_isolate_g = d32(0x0C0C0C0C); // mask to isolate green, removing red & blue
    }

    begin_code(at_jump);

    set_reg_dst_pixel_ptr(at_x);
    l32r_from(REG_SRC_PIXEL_PTR, at_src);

    if (is_transparent) {
        l32r_from(REG_ISOLATE_BR, at_isolate_br);
        l32r_from(REG_ISOLATE_G, at_isolate_g);
    }

    if (outer_fcn) {
        s32i(REG_RETURN_ADDR, REG_STACK_PTR, RET_ADDR_IN_STACK);
    } else {
        mov(REG_SAVE_RET_INNER, REG_RETURN_ADDR);
    }

    uint32_t p_fcn = 0;
    uint32_t rem_width = width;
    uint8_t* p_src_bytes = (uint8_t*) src_pixels;

    while (rem_width) {
        debug_log("src=%08X, xo=%u, rw=%u, ", src_pixels, x_offset, rem_width);

        uint8_t opaqueness = 100;
        if (!is_transparent) {
            // Transfer all pixels at 100% opaqueness.
            width = rem_width;            
        } else {
            // Determine the width of adjacent, similarly transparent (or opaque) pixels in the line.
            // The colors do not have to be equal.
            width = 1;
            uint32_t index = FIX_OFFSET(x_offset);
            uint8_t first_alpha = p_src_bytes[index] & 0xC0;
            for (uint32_t i = 1; i < rem_width; i++) {
                index = FIX_OFFSET(x_offset + i);
                uint8_t next_alpha = p_src_bytes[index] & 0xC0;
                if (next_alpha == first_alpha) {
                    width++;
                } else {
                    break;
                }
            }
            debug_log("w=%u, fa=%02X, ", width, first_alpha);

            // This tests using inverted alpha masks.
            switch (first_alpha) {
                case PIXEL_ALPHA_INV_25_MASK: opaqueness = 25; break;
                case PIXEL_ALPHA_INV_50_MASK: opaqueness = 50; break;
                case PIXEL_ALPHA_INV_75_MASK: opaqueness = 75; break;
                default: opaqueness = 100; break;
            }
            debug_log("op=%hu, ", opaqueness);
        }
        rem_width -= width;
        debug_log("rw=%u\n", rem_width);

        // Use the series of pixels, rather than the rest of the line, if necessary.
        while (width) {
            auto offset = x_offset & 3;
            uint32_t sub = 1;
            switch (offset) {
                case 0:
                    if (width >= 4) {
                        if (width >= 256) {
                            // Need at least 64 full words
                            auto times = width / 256;
                            movi(REG_LOOP_INDEX, times);
                            switch (opaqueness) {
                                case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_256_pixels_in_loop; break;
                                case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_256_pixels_in_loop; break;
                                case 75: p_fcn =  (uint32_t) &fcn_src_blend_75_for_256_pixels_in_loop; break;
                                case 100: p_fcn = (uint32_t) &fcn_copy_256_pixels_in_loop; break;
                            }
                            sub = times * 256;
                        } else if (width >= 128) {
                            // Need at least 32 full words
                            if (width > 128 || rem_width) {
                                switch (opaqueness) {
                                    case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_128_pixels; break;
                                    case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_128_pixels; break;
                                    case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_128_pixels; break;
                                    case 100: p_fcn = (uint32_t) &fcn_copy_128_pixels; break;
                                }
                            } else {
                                switch (opaqueness) {
                                    case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_128_pixels_last; break;
                                    case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_128_pixels_last; break;
                                    case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_128_pixels_last; break;
                                    case 100: p_fcn = (uint32_t) &fcn_copy_128_pixels_last; break;
                                }
                            }
                            sub = 128;
                        } else if (width >= 64) {
                            // Need at least 16 full words
                            if (width > 64 || rem_width) {
                                switch (opaqueness) {
                                    case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_64_pixels; break;
                                    case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_64_pixels; break;
                                    case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_64_pixels; break;
                                    case 100: p_fcn = (uint32_t) &fcn_copy_64_pixels; break;
                                }
                            } else {
                                switch (opaqueness) {
                                    case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_64_pixels_last; break;
                                    case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_64_pixels_last; break;
                                    case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_64_pixels_last; break;
                                    case 100: p_fcn = (uint32_t) &fcn_copy_64_pixels_last; break;
                                }
                            }
                            sub = 64;
                        } else if (width >= 32) {
                            // Need at least 8 full words
                            if (width > 32 || rem_width) {
                                switch (opaqueness) {
                                    case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_32_pixels; break;
                                    case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_32_pixels; break;
                                    case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_32_pixels; break;
                                    case 100: p_fcn = (uint32_t) &fcn_copy_32_pixels; break;
                                }
                            } else {
                                switch (opaqueness) {
                                    case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_32_pixels_last; break;
                                    case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_32_pixels_last; break;
                                    case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_32_pixels_last; break;
                                    case 100: p_fcn = (uint32_t) &fcn_copy_32_pixels_last; break;
                                }
                            }
                            sub = 32;
                        } else if (width >= 16) {
                            // Need at least 4 full words
                            if (width > 16 || rem_width) {
                                switch (opaqueness) {
                                    case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_16_pixels; break;
                                    case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_16_pixels; break;
                                    case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_16_pixels; break;
                                    case 100: p_fcn = (uint32_t) &fcn_copy_16_pixels; break;
                                }
                            } else {
                                switch (opaqueness) {
                                    case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_16_pixels_last; break;
                                    case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_16_pixels_last; break;
                                    case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_16_pixels_last; break;
                                    case 100: p_fcn = (uint32_t) &fcn_copy_16_pixels_last; break;
                                }
                            }
                            sub = 16;
                        } else if (width >= 8) {
                            // Need at least 2 full words
                            if (width > 8 || rem_width) {
                                switch (opaqueness) {
                                    case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_8_pixels; break;
                                    case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_8_pixels; break;
                                    case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_8_pixels; break;
                                    case 100: p_fcn = (uint32_t) &fcn_copy_8_pixels; break;
                                }
                            } else {
                                switch (opaqueness) {
                                    case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_8_pixels_last; break;
                                    case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_8_pixels_last; break;
                                    case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_8_pixels_last; break;
                                    case 100: p_fcn = (uint32_t) &fcn_copy_8_pixels_last; break;
                                }
                            }
                            sub = 8;
                        } else {
                            // Need at least 1 full word
                            if (width > 4 || rem_width) {
                                switch (opaqueness) {
                                    case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_4_pixels_at_offset_0; break;
                                    case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_4_pixels_at_offset_0; break;
                                    case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_4_pixels_at_offset_0; break;
                                    case 100:
                                        l32i(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, 0);
                                        s32i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0);
                                        addi(REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4);
                                        addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                                        break;
                                }
                            } else {
                                switch (opaqueness) {
                                    case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_4_pixels_at_offset_0_last; break;
                                    case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_4_pixels_at_offset_0_last; break;
                                    case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_4_pixels_at_offset_0_last; break;
                                    case 100:
                                        l32i(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, 0);
                                        s32i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0);
                                        break;
                                }
                            }
                            sub = 4;
                        }
                    } else if (width == 3) {
                        switch (opaqueness) {
                            case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_3_pixels_at_offset_0_last; break;
                            case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_3_pixels_at_offset_0_last; break;
                            case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_3_pixels_at_offset_0_last; break;
                            case 100:
                                l32i(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(0));
                                s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0));
                                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                                break;
                        }
                        sub = 3;
                    } else if (width == 2) {
                        switch (opaqueness) {
                            case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_2_pixels_at_offset_0_last; break;
                            case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_2_pixels_at_offset_0_last; break;
                            case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_2_pixels_at_offset_0_last; break;
                            case 100:
                                l16ui(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(0));
                                s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0));
                                break;
                        }
                        sub = 2;
                    } else { // width == 1
                        switch (opaqueness) {
                            case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_1_pixel_at_offset_0_last; break;
                            case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_1_pixel_at_offset_0_last; break;
                            case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_1_pixel_at_offset_0_last; break;
                            case 100:
                                l8ui(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(0));
                                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0));
                                break;
                        }
                    }
                    break;

                case 1:
                    if (width >= 3) {
                        if (width > 3) {
                            switch (opaqueness) {
                                case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_3_pixels_at_offset_1; break;
                                case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_3_pixels_at_offset_1; break;
                                case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_3_pixels_at_offset_1; break;
                                case 100:
                                    l32i(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, 0);    
                                    s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1));    
                                    s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                                    addi(REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4);
                                    addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                                    break;
                            }
                        } else {
                            switch (opaqueness) {
                                case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_3_pixels_at_offset_1_last; break;
                                case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_3_pixels_at_offset_1_last; break;
                                case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_3_pixels_at_offset_1_last; break;
                                case 100:
                                    l32i(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, 0);    
                                    s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1));    
                                    s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                                    break;
                            }
                        }
                        sub = 3;                
                    } else if (width == 2) {
                        switch (opaqueness) {
                            case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_2_pixels_at_offset_1_last; break;
                            case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_2_pixels_at_offset_1_last; break;
                            case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_2_pixels_at_offset_1_last; break;
                            case 100:
                                l32i(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, 0);    
                                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1));
                                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                                break;
                        }
                        sub = 2;
                    } else { // width == 1
                        switch (opaqueness) {
                            case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_1_pixel_at_offset_1_last; break;
                            case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_1_pixel_at_offset_1_last; break;
                            case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_1_pixel_at_offset_1_last; break;
                            case 100:
                                l8ui(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(1));    
                                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1));
                                break;
                        }
                    }
                    break;

                case 2:
                    if (width >= 2) {
                        if (width > 2) {
                            switch (opaqueness) {
                                case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_2_pixels_at_offset_2; break;
                                case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_2_pixels_at_offset_2; break;
                                case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_2_pixels_at_offset_2; break;
                                case 100:
                                    l16ui(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(2));
                                    s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                                    addi(REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4);
                                    addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                                    break;
                            }
                        } else {
                            switch (opaqueness) {
                                case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_2_pixels_at_offset_2_last; break;
                                case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_2_pixels_at_offset_2_last; break;
                                case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_2_pixels_at_offset_2_last; break;
                                case 100:
                                    l16ui(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(2));
                                    s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                                    break;
                            }
                        }
                        sub = 2;
                    } else { // width == 1
                        switch (opaqueness) {
                            case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_1_pixel_at_offset_2_last; break;
                            case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_1_pixel_at_offset_2_last; break;
                            case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_1_pixel_at_offset_2_last; break;
                            case 100:
                                l8ui(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(2));
                                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
                                break;
                        }
                    }
                    break;
                
                case 3:
                    if (width > 1) {
                        switch (opaqueness) {
                            case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_1_pixel_at_offset_3; break;
                            case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_1_pixel_at_offset_3; break;
                            case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_1_pixel_at_offset_3; break;
                            case 100:
                                l8ui(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(3));
                                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(3));
                                addi(REG_SRC_PIXEL_PTR, REG_SRC_PIXEL_PTR, 4);
                                addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
                                break;
                        }
                    } else {
                        switch (opaqueness) {
                            case 25: p_fcn = (uint32_t) &fcn_src_blend_25_for_1_pixel_at_offset_3_last; break;
                            case 50: p_fcn = (uint32_t) &fcn_src_blend_50_for_1_pixel_at_offset_3_last; break;
                            case 75: p_fcn = (uint32_t) &fcn_src_blend_75_for_1_pixel_at_offset_3_last; break;
                            case 100:
                                l8ui(REG_PIXEL_COLOR, REG_SRC_PIXEL_PTR, FIX_OFFSET(3));
                                s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(3));
                                break;
                        }
                    }
                    break;
            }
            width -= sub;
            x_offset += sub;
            if (p_fcn) {
                fixups.push_back(EspFixup { get_code_index(), p_fcn });
                call0(0);
                p_fcn = 0;
            }
        }
    }

    if (outer_fcn) {
        l32i(REG_RETURN_ADDR, REG_STACK_PTR, RET_ADDR_IN_STACK);
        retw();
    } else {
        jx(REG_SAVE_RET_INNER);
    }
}

void EspFunction::do_fixups(EspFixups& fixups) {
    uint32_t save_pc = get_code_index();
    for (auto fixup = fixups.begin();
        fixup != fixups.end();
        ++fixup) {
        set_code_index(fixup->code_index);
        call_inner_fcn(fixup->fcn_address);
    }
    set_code_index(save_pc);
}

uint32_t EspFunction::enter_outer_function() {
    entry(sp, 32);
    auto at_jump = get_code_index();
    j(0);
    return at_jump;
}

uint32_t EspFunction::enter_inner_function() {
    auto at_jump = get_code_index();
    j(0);
    return at_jump;
}

uint32_t EspFunction::begin_data() {
    align32();
    return get_code_index();
}

uint32_t EspFunction::init_jump_table(uint32_t num_items) {
    /* 00 */ entry(sp, 32);
    /* 03 */ l32i(REG_ABS_Y, REG_THIS_PTR, FLD_abs_y);
    /* 06 */ s32i(REG_RETURN_ADDR, REG_STACK_PTR, RET_ADDR_IN_STACK);
    /* 09 */ call0(0); // 8 + 0*4 + 4
    /* 12 */ sub(REG_LINE_INDEX, REG_LINE_INDEX, REG_ABS_Y);
    /* 15 */ slli(REG_JUMP_ADDRESS, REG_LINE_INDEX, 2);
    /* 18 */ addi(REG_JUMP_ADDRESS, REG_JUMP_ADDRESS, 24);
    /* 21 */ add(REG_JUMP_ADDRESS, REG_JUMP_ADDRESS, REG_RETURN_ADDR);
    /* 24 */ l32i(REG_PIXEL_COLOR, REG_THIS_PTR, FLD_color);
    /* 27 */ callx0(REG_JUMP_ADDRESS);
    /* 30 */ l32i(REG_RETURN_ADDR, REG_STACK_PTR, RET_ADDR_IN_STACK);
    /* 33 */ retw();
    /* 36 */ auto at_jump_table = get_code_index();
    for (uint32_t i = 0; i < num_items; i++) {
        /* 36+i*4 */ ret(); // will be changed to j(?) later
        /* 39+i*4 */ align32();
    }
    return at_jump_table;
}

void EspFunction::begin_code(uint32_t at_jump) {
    align32();
    j_to_here(at_jump);
}

void EspFunction::set_reg_dst_pixel_ptr(uint32_t at_x) {
    l32r_from(REG_DST_PIXEL_PTR, at_x);
    add(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, REG_LINE_PTR);
}

void EspFunction::call_inner_fcn(uint32_t real_address) {
    uint32_t offset = (real_address - 4 - (get_real_address(get_code_index() & 0xFFFFFFFC))) & 0xFFFFF;
    call0(offset);
}

void EspFunction::store(uint8_t instr_byte) {
    auto i = m_code_index >> 2;
    switch (m_code_index & 3) {
        case 0:
            m_code[i] = (m_code[i] & 0xFFFFFF00) | (uint32_t)instr_byte;
            break;
        case 1:
            m_code[i] = (m_code[i] & 0xFFFF00FF) | ((uint32_t)instr_byte) << 8;
            break;
        case 2:
            m_code[i] = (m_code[i] & 0xFF00FFFF) | ((uint32_t)instr_byte) << 16;
            break;
        case 3:
            m_code[i] = (m_code[i] & 0x00FFFFFF) | ((uint32_t)instr_byte) << 24;
            break;
    }

    if (++m_code_index > m_code_size) {
        m_code_size = m_code_index;
    }
}

void EspFunction::align16() {
    if (m_code_index & 1) {
        d8(0);
    }
}

void EspFunction::align32() {
    align16();
    if (m_code_index & 2) {
        d16(0);
    }
}

void EspFunction::j_to_here(uint32_t from) {
    uint32_t save_pc = get_code_index();
    set_code_index(from);
    j(save_pc - from - 4);
    set_code_index(save_pc);
}

void EspFunction::l32r_from(reg_t reg, uint32_t from) {
    l32r(reg, from - ((get_code_index() + 3) & 0xFFFFFFFC));
}

uint16_t EspFunction::dup8_to_16(uint8_t value) {
    return (((uint16_t)value) << 8) | ((uint16_t)value);
}

uint32_t EspFunction::dup8_to_32(uint8_t value) {
    return (dup8_to_16(value) << 16) | dup8_to_16(value);
}

uint32_t EspFunction::dup16_to_32(uint16_t value) {
    return (((uint32_t)value) << 16) | ((uint32_t)value);
}

void EspFunction::allocate(uint32_t size) {
    if (m_alloc_size) {
        if (m_alloc_size - m_code_index < size) {
            size_t new_size = (size_t)(m_alloc_size + size + EXTRA_CODE_SIZE + 3) &0xFFFFFFFC;
            void* p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_EXEC);
            memcpy(p, m_code, (m_code_size + 3) &0xFFFFFFFC);
            heap_caps_free(m_code);
            m_alloc_size = (uint32_t)new_size;
            m_code = (uint32_t*)p;
        }
    } else {
        size_t new_size = (size_t)(size + EXTRA_CODE_SIZE + 3) &0xFFFFFFFC;
        void* p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_EXEC);
        m_alloc_size = (uint32_t)new_size;
        m_code = (uint32_t*)p;
    }
}

uint32_t EspFunction::write8(const char* mnemonic, instr_t data) {
    allocate(1);
    auto at_data = get_code_index();
    store((uint8_t)(data & 0xFF));
    return at_data;
}

uint32_t EspFunction::write16(const char* mnemonic, instr_t data) {
    allocate(2);
    auto at_data = get_code_index();
    store((uint8_t)(data & 0xFF));
    store((uint8_t)((data >> 8) & 0xFF));
    return at_data;
}

uint32_t EspFunction::write24(const char* mnemonic, instr_t data) {
    allocate(3);
    auto at_data = get_code_index();
    store((uint8_t)(data & 0xFF));
    store((uint8_t)((data >> 8) & 0xFF));
    store((uint8_t)((data >> 16) & 0xFF));
    return at_data;
}

uint32_t EspFunction::write32(const char* mnemonic, instr_t data) {
    allocate(4);
    auto at_data = get_code_index();
    store((uint8_t)(data & 0xFF));
    store((uint8_t)((data >> 8) & 0xFF));
    store((uint8_t)((data >> 16) & 0xFF));
    store((uint8_t)((data >> 24) & 0xFF));
    return at_data;
}

instr_t isieo(uint32_t instr, reg_t src, int32_t imm, u_off_t offset) {
    if (imm == -1) imm = 0;
    else if (imm == 10) imm = 9;
    else if (imm == 12) imm = 10;
    else if (imm == 16) imm = 11;
    else if (imm == 32) imm = 12;
    else if (imm == 64) imm = 13;
    else if (imm == 128) imm = 14;
    else if (imm == 256) imm = 15;
    return instr | (offset << 16) | (imm << 12) | (src << 8);
}